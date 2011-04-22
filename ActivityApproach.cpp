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
#define STARTSERVICE  2
#define ENDSERVICE    3
#define ARRIVECALL    4
#define STARTCALL     5
#define ENDCALL       6


// CLista.h is a template for linked list class
#include "CLista.h"

// Client queue and Call queue declaration
CLista<CEntity *> *client_queue;
CLista<CEntity *> *call_queue;

// clerk_free = flag for clerk iddle
bool clerk_free = true;

class CClerk:public CActivity // Call class as a sub class of CActivity
{
public:
// attributes for storing arrival time, and start and end conversation time
    CClerk():CActivity() // Constructor
    {}
    void ArriveClient(); // Client arrival activity

    void StartService(); // Client arrival activity

    void EndService(); // Client service end activity


    // Call Arrive method
    void ArriveCall();
    // Call Arrive method
    void StartCall();
    // Call End  method
    void EndCall();
    // Execute Activity method acording to Activity
    void ExecuteActivity();
};



class CClerkExecutive: CActivityExecutive
{

public:
    CClerkExecutive(): CActivityExecutive()
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

    CClerk * GetActivity() { // Get activity in the head of the list
        return (CClerk *)CActivityExecutive::GetActivity();
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
        CClerk *activ;
        double sim_time=SimulationTime();
        // Execute all activity at sim_time
        while (sim_time==GetActivityTime()) {
            activ=GetActivity();    // Get activity at the front of the queue
            activ->ExecuteActivity(); // Call for executing activity method implemented in a subclass
            RemoveActivity();  // Remove activity

        }
    }

};


CClerkExecutive *executive;
// Simulation paramters
// double sim_time, call_arrival, arrival_mean, min_service,max_service, min_call, max_call;
double sim_time;
double call_arrival, arrival_mean, arrival_manager_prob, arrival_teller_prob, atm_service_mean, atm_service_stddev, teller_service_mean, teller_service_stddev, manager_service_mean, manager_service_stddev, atm_to_teller_prob, atm_to_manager_prob, min_service, max_service, min_call, max_call;

// Statistical repository
CStatistics client_wait("output/ClientWait.txt", ADD_FILE),
            call_wait("output/CallWait.txt", ADD_FILE),
            call_system("output/CallSystem.txt", ADD_FILE),
            client_system("output/ClientSystem.txt", ADD_FILE),
            call_duration("output/CallDuration.txt", ADD_FILE),
            service_duration("output/ServiceDuration.txt", ADD_FILE);



void CClerk::ArriveClient()  // Arrival activity handling
{
    double time1,sim_time=executive->SimulationTime();

    if (activity==ARRIVE && time==sim_time) {
        CEntity *client=new CEntity();
        CDistribution dist;

        // Current client arrival
        entity->arrive=time;
        if (_DEBUG_) {
            printf("Client Arrives %f \n", time);
        }

        // Next client arrival time calculation
        time1=sim_time+dist.Exponential(arrival_mean);
        client->SetActivity(ARRIVE);

        // Schedule next client arrival
        executive->AddActivity(time1,ARRIVE,client);
        client_queue->InserirFim(entity);

    }
}

void CClerk::EndService()  // service End handling
{
    double sim_time=executive->SimulationTime();


    if (activity==ENDSERVICE && time==sim_time) {
        if (_DEBUG_) {
            printf("Service Ends %f \n", time);
        }
        entity->end=time;
        // Statistical storage of client time in system ans service duration
        client_system.Add(entity->end-entity->arrive);
        service_duration.Add(entity->end-entity->start);

        clerk_free=true;
        delete entity;
    }
}

void CClerk::StartService()  // service Start handling
{
    double time1,sim_time=executive->SimulationTime();
    CDistribution dist;
    CEntity *client;
    if (clerk_free) { // Clerk is free
        if (!client_queue->EhVazia()) { // There is client in the queue
            // Get client from client queue
            client=(CEntity*)client_queue->ObterInfo();
            client_queue->Remover();
            client->SetActivity(ENDSERVICE);
            client->start=time;
            // Collect statistics on client waiting
            client_wait.Add(client->start-client->arrive);
            if (_DEBUG_) {
                printf("Service Starts %f \n", time);
            }
            // Calculate service ending time
            time1=sim_time+dist.Uniform(min_service,max_service);
            executive->AddActivity(time1,ENDSERVICE,client);
            clerk_free=false;
        }
    }

}

void CClerk::ArriveCall() // Call arrival handling
{
    double time1,sim_time=executive->SimulationTime();

    // Test if it is the current activity
    if (activity==ARRIVECALL && time==sim_time) {
        CEntity *call=new CEntity();
        CDistribution dist;
        if (_DEBUG_) {
            printf("Call Arrives %f \n", time);
        }

        // Calculate next  call arrival
        time1=sim_time+dist.Exponential(call_arrival);
        entity->arrive=time;

        // Schedule next call arrival
        executive->AddActivity(time1,ARRIVECALL,call);

        // Put call on the queue
        call_queue->InserirFim(entity);
    }
}

void CClerk::StartCall()  // Call start handling
{
    double time1, sim_time=executive->SimulationTime();
    CDistribution dist;
    CEntity * call;

    if (clerk_free && client_queue->EhVazia() && (!call_queue->EhVazia())) { // Client not is waiting there is call waiting

        // Get call from queue
        call=(CEntity *)call_queue->ObterInfo();
        call_queue->Remover();
        call->SetActivity(STARTCALL);
        call->start=time;

        // Collect stats on call waiting
        call_wait.Add(call->start-call->arrive);
        if (_DEBUG_) {
            printf("Call Starts %f \n", time);
        }

        // Calculate call ending time
        time1=sim_time+dist.Uniform(min_call,max_call);

        // Schedule end of conversation time
        executive->AddActivity(time1,ENDCALL,call);

        // clerk is free
        clerk_free=false;

    }
}

void CClerk::EndCall()  // Call ending handling
{
    double sim_time=executive->SimulationTime();

    if (activity==ENDCALL && time==sim_time) {

        if (_DEBUG_) {
            printf("Call Ends %f \n", time);
        }
        entity->end=time;

        // Statistical storage of c
        call_system.Add(entity->end-entity->arrive);
        call_duration.Add(entity->end-entity->start);

        // clerk is free
        clerk_free=true;
        delete entity;
    }
}


void CClerk::ExecuteActivity() // Activity execution
{
    ArriveClient();  // Arrival of Cliente
    ArriveCall();
    EndService();// End of Service
    EndCall();
    StartService(); // Start Service
    StartCall();
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
    printf("Service duration \n Mean  = %f  ",service_duration.Mean());
    printf(" Std Dev  = %f \n",service_duration.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",service_duration.min,service_duration.max);
    printf("Client in the System \n Mean = %f  ",client_system.Mean());
    printf(" Std Dev  = %f \n",client_system.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",client_system.min,client_system.max);
}

int main(int argc, _TCHAR* argv[])
{
    char c;
    CEntity *client = new CEntity(); // client
    CEntity *call   = new CEntity();
    executive       = new CClerkExecutive();
    client_queue    = new CLista<CEntity *>;
    call_queue      = new CLista<CEntity *>;
    CDistribution dist;

    // Simulation Paramters
    executive->SetSimulationEnd(240.0);
    call_arrival=5.0; // call arrival mean - Negative exponential distribution
    arrival_mean=3.0; // client arrival mean - Negative exponential distribution
    min_service=0.5; // minimum of uniform distribution
    max_service=2.0; // maximum of uniform distribution
    min_call=0.5; // minimum of uniform distribution
    max_call=1.5; // maximum of uniform distribution


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

    call_max_wait = 10.0;

    // Initial activitys: client arrival and call arrival

    // Schedule next client arrival
    client->SetActivity(ARRIVE);
    executive->AddActivity(dist.Exponential(arrival_mean),ARRIVE,client);

    // Schedule next call arrival
    call->SetActivity(ARRIVECALL);
    executive->AddActivity(dist.Exponential(call_arrival),ARRIVECALL,call);

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

