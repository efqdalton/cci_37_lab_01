// ActivityApproach.cpp : Defines the entry point for the console application.

#include <stdio.h>
#define _TCHAR char

#include "CLista.h"
#include "Statistics.h"
#include "Activity.h"

// Set _DEBUG_ 1 if you want to see tracing of all Activitys, 0, if not
#define _DEBUG_ 1

// Activity code definition
#define ARRIVE        1
#define STARTMANAGER  2
#define ENDMANAGER    3
#define ARRIVECALL    4
#define STARTCALL     5
#define ENDCALL       6
#define STARTATM      7
#define ENDATM        8
#define STARTTELLER   9
#define ENDTELLER     10


// CLista.h is a template for linked list class
#include "CLista.h"

// Client queue and Call queue declaration
CLista<CEntity *> *client_queue;
CLista<CEntity *> *call_queue;
CLista<CEntity *> *manager_queue;
CLista<CEntity *> *atm_queue;
CLista<CEntity *> *teller_queue;

// flags for idle
bool manager_free = true;
bool teller_free  = true;
bool atm_free     = true;


class CBank:public CActivity // Call class as a sub class of CActivity
{
public:
    // attributes for storing arrival time, and start and end conversation time
    CBank():CActivity() // Constructor
    {}

    void ArriveClient();
    void ArriveCall();

    void StartManager();
    void EndManager();

    void StartCall();
    void EndCall();

    void StartTeller();
    void EndTeller();

    void StartATM();
    void EndATM();

    void ExecuteActivity();
};

class CBankExecutive: CActivityExecutive
{

public:
    CBankExecutive(): CActivityExecutive()
    {}

    double SimulationTime() {
        return CActivityExecutive::SimulationTime();
    }

    double SimulationEnd() {
        return CActivityExecutive::SimulationEnd();
    }

    void SetSimulationEnd( double sim) {
        simulation_end=sim;
    }

    CBank * GetActivity() { // Get activity in the head of the list
        return (CBank *)CActivityExecutive::GetActivity();
    }

    double TimeScan() { // Get the time of the head of the list
        return  CActivityExecutive:: TimeScan();
    }

    double GetActivityTime() { // Get activity of the head of the list (queue)
        return CActivityExecutive:: GetActivityTime();

    }
    void AddActivity(double tim, int act, CEntity *ent) { // Add activity inserting in a sorted list by time
        CActivityExecutive::AddActivity( tim, act, ent);
    }

    void RemoveActivity() { // Remove the activity in the head of the queue
        CActivityExecutive::RemoveActivity();
    }

    void ExecuteActivities() { // Simulation activity execution at time simulation
        CBank *activ;
        double sim_time=SimulationTime();

        // Execute all activity at sim_time
        while(sim_time == GetActivityTime()){
            activ = GetActivity();      // Get activity at the front of the queue
            activ->ExecuteActivity(); // Call for executing activity method implemented in a subclass
            RemoveActivity();         // Remove activity
        }
    }
};


CBankExecutive *executive;
// Simulation paramters
// double sim_time, call_arrival, arrival_mean, min_service, max_service, min_call, max_call;
double sim_time;
double call_arrival, arrival_mean, arrival_manager_prob, arrival_teller_prob, atm_service_mean, atm_service_stddev, teller_service_mean, teller_service_stddev, manager_service_mean, manager_service_stddev, atm_to_teller_prob, atm_to_manager_prob, min_service, max_service, min_call, max_call, call_max_wait;

// Statistical repository
CStatistics call_wait(        "output/CallWait.txt",        ADD_FILE ),
            call_system(      "output/CallSystem.txt",      ADD_FILE ),
            call_duration(    "output/CallDuration.txt",    ADD_FILE ),

            client_wait(      "output/ClientWait.txt",      ADD_FILE ),
            client_system(    "output/ClientSystem.txt",    ADD_FILE ),

            manager_wait(     "output/ManagerWait.txt",     ADD_FILE ),
            manager_duration( "output/ManagerDuration.txt", ADD_FILE ),

            teller_wait(      "output/TellerWait.txt",      ADD_FILE ),
            teller_duration(  "output/TellerDuration.txt",  ADD_FILE ),

            atm_wait(         "output/ATMWait.txt",         ADD_FILE ),
            atm_duration(     "output/ATMDuration.txt",     ADD_FILE );


void CBank::ArriveClient()  // Arrival activity handling
{
    double time1, sim_time = executive->SimulationTime();

    if (activity == ARRIVE && time == sim_time) {
        CEntity *client = new CEntity();
        CDistribution dist;

        // Current client arrival
        entity->arrive = time;
        entity->start  = time;
        if(_DEBUG_) printf("Client Arrives %f \n", time);

        // Next client arrival time calculation
        time1 = sim_time + dist.Exponential(arrival_mean);
        client->SetActivity(ARRIVE);

        // Schedule next client arrival
        executive->AddActivity(time1, ARRIVE, client);

        // decide where the client goes
        double r = dist.Random();
        if(r < arrival_teller_prob){
            teller_queue->InserirFim(entity);
        }else if(r < arrival_teller_prob + arrival_manager_prob){
            manager_queue->InserirFim(entity);
        }else{
            atm_queue->InserirFim(entity);
        }

    }
}

void CBank::StartManager()  // service Start handling
{
    double time1, sim_time = executive->SimulationTime();
    CDistribution dist;
    CEntity       *client;

    if(manager_free) { // Manager is free
        if (!manager_queue->EhVazia()) { // There is client in the queue
            // Get client from client queue
            client = (CEntity*) manager_queue->ObterInfo();
            manager_queue->Remover();
            client->SetActivity(ENDMANAGER);

            // Collect statistics on client waiting
            client_wait.Add(time - client->start);
            manager_wait.Add(time - client->start);
            client->start = time;

            if(_DEBUG_) printf("Service Starts %f \n", time);

            // Calculate service ending time
            time1 = sim_time + dist.NormalLimited(manager_service_mean, manager_service_stddev, min_service, max_service);
            executive->AddActivity(time1, ENDMANAGER, client);
            manager_free = false;
        }
    }

}

void CBank::EndManager()  // service End handling
{
    double sim_time = executive->SimulationTime();

    if (activity == ENDMANAGER && time == sim_time) {
        if(_DEBUG_) printf("Service Ends %f \n", time);

        // Statistical storage of client time in system ans service duration
        client_system.Add(time - entity->arrive);
        manager_duration.Add(time - entity->start);
        entity->end = time;

        manager_free = true;
        delete entity;
    }
}

void CBank::ArriveCall() // Call arrival handling
{
    double time1, sim_time = executive->SimulationTime();

    // Test if it is the current activity
    if (activity == ARRIVECALL && time == sim_time) {
        CEntity *call=new CEntity();
        CDistribution dist;
        if(_DEBUG_) printf("Call Arrives %f \n", time);

        // Calculate next  call arrival
        time1 = sim_time + dist.Exponential(call_arrival);
        entity->arrive = time;
        entity->start  = time;

        // Schedule next call arrival
        executive->AddActivity(time1, ARRIVECALL, call);

        // Put call on the queue
        call_queue->InserirFim(entity);
    }
}

void CBank::StartCall()  // Call start handling
{
    double time1, sim_time = executive->SimulationTime();
    CDistribution dist;
    CEntity * call;

    if(manager_free){
        if(!call_queue->EhVazia()){

            // Get call from queue
            call = (CEntity *) call_queue->ObterInfo();
            call_queue->Remover();
            call->SetActivity(STARTCALL);
            call->start=time;

            // Collect stats on call waiting
            call_wait.Add(call->start - call->arrive);
            if(_DEBUG_) printf("Call Starts %f \n", time);

            // Calculate call ending time
            time1 = sim_time + dist.Uniform(min_call, max_call);

            // Schedule end of conversation time
            executive->AddActivity(time1, ENDCALL, call);

            // manager isn't free
            manager_free = false;
        }
    }
}

void CBank::EndCall()  // Call ending handling
{
    double sim_time = executive->SimulationTime();

    if(activity == ENDCALL && time == sim_time){

        if(_DEBUG_) printf("Call Ends %f \n", time);
        entity->end = time;

        // Statistical storage of c
        call_system.Add(entity->end - entity->arrive);
        call_duration.Add(entity->end - entity->start);

        // manager is free
        manager_free = true;
        delete entity;
    }
}

void CBank::StartTeller()
{
  double time1, sim_time = executive->SimulationTime();
  CDistribution dist;
  CEntity * client;

  if(teller_free){
      if(!teller_queue->EhVazia()){

          // Get call from queue
          client = (CEntity *) teller_queue->ObterInfo();
          teller_queue->Remover();
          client->SetActivity(STARTTELLER);

          // Collect stats on call waiting
          client_wait.Add(time - client->start);
          teller_wait.Add(time - client->start);
          client->start = time;

          if(_DEBUG_) printf("Teller Starts %f \n", time);

          // Calculate call ending time
          time1 = sim_time + dist.NormalLimited(teller_service_mean, teller_service_stddev, min_service, max_service);

          // Schedule end of conversation time
          executive->AddActivity(time1, ENDTELLER, client);

          // teller isn't free
          teller_free = false;
      }
  }
}

void CBank::EndTeller()
{
  double sim_time = executive->SimulationTime();

  if(activity == ENDTELLER && time == sim_time){

      if(_DEBUG_) printf("Teller Ends %f \n", time);

      // Statistical storage of c
      client_system.Add(time - entity->arrive);
      teller_duration.Add(time - entity->start);
      entity->end = time;

      // teller is free
      teller_free = true;
      delete entity;
  }
}

void CBank::StartATM()
{
  double time1, sim_time = executive->SimulationTime();
  CDistribution dist;
  CEntity * client;

  if(atm_free){
      if(!atm_queue->EhVazia()){

          // Get call from queue
          client = (CEntity *) atm_queue->ObterInfo();
          atm_queue->Remover();
          client->SetActivity(STARTATM);

          // Collect stats on call waiting
          client_wait.Add(time - client->start);
          atm_wait.Add(time - client->start);
          client->start = time;

          if(_DEBUG_) printf("ATM Starts %f \n", time);

          // Calculate call ending time
          time1 = sim_time + dist.NormalLimited(atm_service_mean, atm_service_stddev, min_service, max_service);

          // Schedule end of conversation time
          executive->AddActivity(time1, ENDATM, client);

          // atm isn't free
          atm_free = false;
      }
  }
}

void CBank::EndATM()
{
  double sim_time = executive->SimulationTime();
  CDistribution dist;

  if(activity == ENDATM && time == sim_time){

      if(_DEBUG_) printf("Teller Ends %f \n", time);

      // Statistical storage of c
      atm_duration.Add(time - entity->start);
      entity->end = time;
      entity->start = time;

      // atm is free
      atm_free = true;

      double r = dist.Random();
      if(r < atm_to_teller_prob){
          teller_queue->InserirFim(entity);
      }else if(r < atm_to_teller_prob + atm_to_manager_prob){
          manager_queue->InserirFim(entity);
      }else{
          client_system.Add(entity->end - entity->arrive);
          delete entity;
      }
  }
}

void CBank::ExecuteActivity() // Activity execution
{
    ArriveCall();
    ArriveClient();

    EndATM();
    EndTeller();
    EndCall();
    EndManager();

    StartATM();
    StartTeller();
    StartCall();
    StartManager();
}


void StatisticsReport() // Statistical display of mean, standard deviation, minimum and maximum
{
    printf(" \nStatistical Report \n");
    printf("Call waiting \n Mean  = %f  ",call_wait.Mean());
    printf(" Std Dev  = %f \n",call_wait.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",call_wait.min,call_wait.max);
    printf("Call duration \n Mean = %f  ",call_duration.Mean());
    printf(" Std Dev  = %f \n",call_duration.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",call_duration.min,call_duration.max);
    printf("Call in the System \n Mean = %f  ",call_system.Mean());
    printf(" Std Dev  = %f \n",call_system.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",call_system.min,call_system.max);
    printf("Client waiting \n Mean  = %f  ",client_wait.Mean());
    printf(" Std Dev  = %f \n",client_wait.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",client_wait.min,client_wait.max);
    printf("Manager duration \n Mean  = %f  ",manager_duration.Mean());
    printf(" Std Dev  = %f \n",manager_duration.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",manager_duration.min,manager_duration.max);
    printf("Client in the System \n Mean = %f  ",client_system.Mean());
    printf(" Std Dev  = %f \n",client_system.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",client_system.min,client_system.max);
}

int main(int argc, _TCHAR* argv[])
{
    char c;
    CEntity *client = new CEntity();
    CEntity *call   = new CEntity();
    executive       = new CBankExecutive();
    client_queue    = new CLista<CEntity *>;
    call_queue      = new CLista<CEntity *>;
    manager_queue   = new CLista<CEntity *>;
    atm_queue       = new CLista<CEntity *>;
    teller_queue    = new CLista<CEntity *>;
    CDistribution dist;

    // Simulation Paramters
    // um ano = 60min * 8h * 21d * 12m
    double total_time = 60*8*21*3; // Trimestre
    executive->SetSimulationEnd(total_time);
    // call_arrival=5.0; // call arrival mean - Negative exponential distribution
    // arrival_mean=3.0; // client arrival mean - Negative exponential distribution
    // min_service=0.5; // minimum of uniform distribution
    // max_service=2.0; // maximum of uniform distribution
    // min_call=0.5; // minimum of uniform distribution
    // max_call=1.5; // maximum of uniform distribution


    call_arrival           = 10.0;    // call arrival mean - Negative exponential distribution
    arrival_mean           = 5.0;     // client arrival mean - Negative exponential distribution
    arrival_manager_prob   = 0.1;
    arrival_teller_prob    = 0.2;
    atm_service_mean       = 4.0;
    atm_service_stddev     = 2.0;
    teller_service_mean    = 7.0;
    teller_service_stddev  = 3.0;
    manager_service_mean   = 10.0;
    manager_service_stddev = 4.0;
    atm_to_teller_prob     = 0.15;
    atm_to_manager_prob    = 0.15;
    min_service            = 0.2;     // minimum of uniform distribution
    max_service            = 10000.0; // maximum of uniform distribution
    min_call               = 1.0;     // minimum of uniform distribution
    max_call               = 10.0;    // maximum of uniform distribution
    call_max_wait          = 10.0;

    // Initial activitys: client arrival and call arrival

    // Schedule next client arrival
    client->SetActivity(ARRIVE);
    executive->AddActivity(dist.Exponential(arrival_mean), ARRIVE, client);

    // Schedule next call arrival
    call->SetActivity(ARRIVECALL);
    executive->AddActivity(dist.Exponential(call_arrival), ARRIVECALL, call);

    // Simulation loop
    while(executive->SimulationTime() < executive->SimulationEnd()) {
        sim_time = executive->TimeScan(); // Time Scan = get next activity time
        executive->ExecuteActivities();   // Execute all activitys at sim_time
    }

    // Report statistics on mean, std dev, min and max
    StatisticsReport();
    printf("\nPress any key to end\n");
    scanf("%c",&c);
    return 0;

}

