// EventApproach.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#define _TCHAR char

#include "CLista.h"

#include "Statistics.h"

#include "Event.h"
// Set _DEBUG_ 1 if you want to see tracing of all events, 0, if not
#define _DEBUG_ 1

// Client event codes
#define ARRIVE 1
#define ATM_SERVICE 2
#define TELLER_SERVICE 3
#define MANAGER_SERVICE 4

#define CALL 5
#define WAIT 6
#define TALK 7
#define HANG 8

// CLista.h is a template for linked list class
#include "CLista.h"
// Class definition for forward definition
class CCall;

class CClient;

// Client queue and Call queue declaration
CLista<CClient *> *client_atm_queue;
CLista<CClient *> *client_teller_queue;
CLista<CClient *> *client_manager_queue;
CLista<CCall *> *call_queue;

// clerk_free = flag for clerk iddle
bool atm_free=true;
bool teller_free=true;
bool manager_free=true;

// simulation executive instantiation
CEventExecutive *executive;

// Simulation paramters
double sim_time,call_arrival, arrival_mean, min_service,max_service, min_call,max_call,
arrival_manager_prob, arrival_teller_prob, atm_service_mean,atm_service_stddev,teller_service_mean,
teller_service_stddev,manager_service_mean,manager_service_stddev,atm_to_teller_prob,atm_to_manager_prob,
call_max_wait;

// Statistical repository
CStatistics call_wait(        "output/CallWait.txt",        ADD_FILE ),
            call_system(      "output/CallSystem.txt",      ADD_FILE ),
            call_duration(    "output/CallDuration.txt",    ADD_FILE ),
            call_attended(    "output/CallAttended.txt",    ADD_FILE ),

            client_wait(      "output/ClientWait.txt",      ADD_FILE ),
            client_system(    "output/ClientSystem.txt",    ADD_FILE ),

            manager_wait(     "output/ManagerWait.txt",     ADD_FILE ),
            manager_duration( "output/ManagerDuration.txt", ADD_FILE ),

            teller_wait(      "output/TellerWait.txt",      ADD_FILE ),
            teller_duration(  "output/TellerDuration.txt",  ADD_FILE ),

            atm_wait(         "output/ATMWait.txt",         ADD_FILE ),
            atm_duration(     "output/ATMDuration.txt",     ADD_FILE );

class CCall:public CEntity // Call class as a sub class of CEntity
{
public:
    double arrive, start, end; // attributes for storing arrival time, and start and end conversation time
    bool timeout, call_ended;
    CCall():CEntity() { // Cosntructor
        arrive=start=end=0.0;
    }
    void ScheduleEndCall();
    void ArriveCall();
    void EndCallService();
    void EndCall();
    void ExecuteEvent(int event);
};

class CClient:public CEntity // Client class as a sub class of CEntity
{
public:
    // attributes for storing arrival time, and start and end service time
    double arrive, start, end;

    CClient():CEntity()
    {};
    void ScheduleEndATMService();
    void ScheduleEndManagerService();
    void ScheduleEndTellerService();
    void ArriveClient(); // Client arrival event
    void GoToATM();
    void GoToManager();
    void GoToTeller();
    void GoOut();
    void EndATMService();
    void EndManagerService();
    void EndTellerService();

    void ExecuteEvent(int event); // Event execution
};

void CClient::ScheduleEndATMService()
{
    double time1,time=executive->SimulationTime();

    CDistribution dist;
    SetEvent(ATM_SERVICE);
    if (_DEBUG_) {
        printf("Service Starts %f \n", time);
    }

    atm_wait.Add(time-start); // put the wainting time in statistics repository
    client_wait.Add(time-start);
    // Calculate the end of service time
    time1=time+dist.NormalLimited(atm_service_mean, atm_service_stddev, min_service, max_service);
    // Schedule end of service time
    executive->AddEvent(time1,ATM_SERVICE,(CEntity *)this);
    start=time;
}

void CClient::ScheduleEndTellerService()
{
    double time1,time=executive->SimulationTime();

    CDistribution dist;
    SetEvent(TELLER_SERVICE);
    if (_DEBUG_) {
        printf("Service Starts %f \n", time);
    }

    teller_wait.Add(time-start); // put the wainting time in statistics repository
    client_wait.Add(time-start);
    // Calculate the end of service time
    time1=time+dist.NormalLimited(teller_service_mean, teller_service_stddev, min_service, max_service);
    // Schedule end of service time
    executive->AddEvent(time1,TELLER_SERVICE,(CEntity *)this);
    start=time;
}

void CClient::ScheduleEndManagerService()
{
    double time1,time=executive->SimulationTime();

    CDistribution dist;
    SetEvent(MANAGER_SERVICE);
    if (_DEBUG_) {
        printf("Service Starts %f \n", time);
    }

    manager_wait.Add(time-start); // put the wainting time in statistics repository
    client_wait.Add(time-start);
    // Calculate the end of service time
    time1=time+dist.NormalLimited(manager_service_mean, manager_service_stddev, min_service, max_service);
    // Schedule end of service time
    executive->AddEvent(time1,MANAGER_SERVICE,(CEntity *)this);
    start=time;
}

void CClient::ArriveClient()  // Arrival event handling
{
    double time1,time=executive->SimulationTime();
    CClient *client=new CClient();
    CDistribution dist;
    // Current client arrival
    start=arrive=time;
    if (_DEBUG_) {
        printf("Client Arrives %f \n", time);
    }
    // Next client arrival time calculation
    time1=time+dist.Exponential(arrival_mean);
    client->SetEvent(ARRIVE);
    // Schedule next client arrival
    executive->AddEvent(time1,ARRIVE,client);

    // decide where the client goes
    double r = dist.Random();
    if (r < arrival_teller_prob) {
        GoToTeller();
    } else if (r < arrival_teller_prob + arrival_manager_prob) {
        GoToManager();
    } else {
        GoToATM();
    }

}

void CClient::GoToTeller()
{
    start = executive->SimulationTime();
    if (teller_free && client_teller_queue->EhVazia()) { // handle this client nobody is waiting
        ScheduleEndTellerService();
        teller_free=false;
    }
    // Clerk is busy, put it in the queue
    else {
        client_teller_queue->InserirFim(this);
    }
}

void CClient::GoToATM()
{
    start = executive->SimulationTime();
    if (atm_free && client_atm_queue->EhVazia()) { // handle this client nobody is waiting
        ScheduleEndATMService();
        atm_free=false;
    }
    // Clerk is busy, put it in the queue
    else {
        client_atm_queue->InserirFim(this);
    }
}

void CClient::GoToManager()
{
    CCall *call;
    CClient *client;

    start = executive->SimulationTime();
    if (manager_free)
    {
        if (!call_queue->EhVazia())
        {
            client_manager_queue->InserirFim(this);
            // Get client from client queue
            call=(CCall*)call_queue->ObterInfo();
            call_queue->Remover();
            call->ScheduleEndCall();
            manager_free=false;
        }
        else if (!client_manager_queue->EhVazia())
        {
            client_manager_queue->InserirFim(this); // Put call into call queue
            // Get client from client queue
            client=(CClient*)client_manager_queue->ObterInfo();
            client_manager_queue->Remover();
            client->ScheduleEndManagerService();
            manager_free=false;
        }
        else { // handle this client nobody is waiting
            ScheduleEndManagerService();
            manager_free=false;
        }
    }
    // Clerk is busy, put it in the queue
    else {
        client_manager_queue->InserirFim(this);
    }
}

void CClient::EndATMService()  // service End handling
{
    CClient *client;
    //CCall  *call;
    CDistribution dist;
    double time=executive->SimulationTime();
    if (_DEBUG_) {
        printf("Service Ends %f \n", time);
    }
    end=time;

    // Statistical storage of client time in system ans service duration
    atm_duration.Add(end-start);

    if (!client_atm_queue->EhVazia()) { // There is a cliente in the queue
        // Get the client in front of the queue, and remove it
        client=(CClient *)client_atm_queue->ObterInfo();
        client_atm_queue->Remover();
        client->ScheduleEndATMService();
    }
    // No event scheduled, clerk is free
    else {
        atm_free=true;
    }

    double r = dist.Random();
    if (r < atm_to_teller_prob)
    {
        GoToTeller();
    }
    else if (r < atm_to_teller_prob + atm_to_manager_prob)
    {
        GoToManager();
    }
    else
    {
        GoOut();
    }

}

void CClient::EndTellerService()  // service End handling
{
    CClient *client;
    //CCall  *call;
    CDistribution dist;
    double time=executive->SimulationTime();
    if (_DEBUG_) {
        printf("Service Ends %f \n", time);
    }
    end=time;

    // Statistical storage of client time in system ans service duration
    teller_duration.Add(end-start);

    if (!client_teller_queue->EhVazia()) { // There is a cliente in the queue
        // Get the client in front of the queue, and remove it
        client=(CClient *)client_teller_queue->ObterInfo();
        client_teller_queue->Remover();
        client->ScheduleEndTellerService();

    }
    // No event scheduled, clerk is free
    else {
        teller_free=true;
    }

    GoOut();
}

void CClient::EndManagerService()  // service End handling
{
    CClient *client;
    CCall  *call;
    CDistribution dist;
    double time=executive->SimulationTime();
    if (_DEBUG_) {
        printf("Service Ends %f \n", time);
    }
    end=time;
    // Statistical storage of client time in system ans service duration
    manager_duration.Add(end-start);

    if (!call_queue->EhVazia()) { // No client in the queue, but there is call waiting
        // Get the call from the front queue
        call=(CCall *)call_queue->ObterInfo();
        call_queue->Remover();
        call->ScheduleEndCall();
    }
    else if (!client_manager_queue->EhVazia()) { // There is a cliente in the queue
        // Get the client in front of the queue, and remove it
        client=(CClient *)client_manager_queue->ObterInfo();
        client_manager_queue->Remover();
        client->ScheduleEndManagerService();

    }
    // No event scheduled, clerk is free
    else {
        manager_free=true;
    }

    GoOut();
}

void CClient::GoOut()
{
    double time=executive->SimulationTime();

    if (_DEBUG_) {
        printf("Client goes out at %f \n", time);
    }

    client_system.Add(time - arrive);

    delete this;
}

void CClient::ExecuteEvent(int event) // Event execution calls
{
    switch (event) {
    case ARRIVE:
        ArriveClient();  // Client Arrival call
        break;
    case ATM_SERVICE:
        EndATMService();
        break;
    case TELLER_SERVICE:
        EndTellerService();
        break;
    case MANAGER_SERVICE:
        EndManagerService();
        break;
    }
}

void CCall::ScheduleEndCall()
{
    double time1,time=executive->SimulationTime();
    CDistribution dist;

    if (_DEBUG_) {
        printf("Call Starts %f \n", time);
    }

    // Collect stats on call waiting
    double wait = time-start;
    call_wait.Add(wait);

    start=time;

    if (wait > call_max_wait)
    {
        if (_DEBUG_) {
            printf("Call hanged\n");
        }

        call_attended.Add(0);
        call_system.Add(call_max_wait);

        EndCall();
    } else {

        call_attended.Add(1);

        // calculate end of conversation time
        time1=time+dist.Uniform(min_call,max_call);
        SetEvent(TALK);

        // Schedule end of conversation
        executive->AddEvent(time1,TALK,(CEntity *)this);
    }
}

void CCall::ArriveCall() // Call arrival handling
{
    double time1,time=executive->SimulationTime();
    timeout = false; call_ended = false;
    //CClient *client;
    CCall *call=new CCall();
    CDistribution dist;
    if (_DEBUG_) {
        printf("Call Arrives %f \n", time);
    }
    // Calculate next  call arrival
    time1=time+dist.Exponential(call_arrival);
    start=arrive=time;
    // Schedule next call arrival
    executive->AddEvent(time1,CALL,call);

    if (manager_free)
    {
        if (!call_queue->EhVazia())
        {
            call_queue->InserirFim(this);
            // Get client from client queue
            call=(CCall*)call_queue->ObterInfo();
            call_queue->Remover();
            call->ScheduleEndCall();
            manager_free=false;
        }
        else
        {
            ScheduleEndCall();
            manager_free=false;
        }
    }
    // Put call on the queue
    else {
        call_queue->InserirFim(this);
    }

}

void CCall::EndCallService()
{
    double time=executive->SimulationTime();

    if (_DEBUG_) {
        printf("Call Ends %f \n", time);
    }
    end=time;
// Statistical storage of convesation time in system ans conversation duration
    call_system.Add(end-arrive);
    call_duration.Add(end-start);

    EndCall();
}

void CCall::EndCall()  // Call ending handling
{
    CClient *client;
    CCall  *call;

    if (!call_queue->EhVazia()) {
        call=(CCall *)call_queue->ObterInfo();
        call_queue->Remover();
        call->ScheduleEndCall();
    } else if (!client_manager_queue->EhVazia()) { // Client is waiting
        // Get client from the queue
        client=(CClient *)client_manager_queue->ObterInfo();
        client_manager_queue->Remover();
        client->ScheduleEndManagerService();
    }
    // clerk is free
    else {
        manager_free=true;
    }

    delete this;

}

void CCall::ExecuteEvent(int event) // Event execution
{
    switch (event) {
    case CALL:
        ArriveCall();
        break;
    case TALK:
        EndCallService();
        break;
    }
}


void StatisticsReport() // Statistical display of mean, standard deviation, minimum and maximum
{
    FILE * stats_file;
    stats_file = fopen("output/statistics.csv", "w");
    fprintf(stats_file, "Stat,Total,Mean,Dev,Min,Max\n");

    printf(" \nStatistical Report \n");

    printf("Call waiting \n Mean  = %f  ",call_wait.Mean());
    printf(" Std Dev  = %f \n",call_wait.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",call_wait.min,call_wait.max);
    fprintf(stats_file, "Call Wait,%d,%f,%f,%f,%f\n", call_wait.N(), call_wait.Mean(), call_wait.StandardDeviation(), call_wait.min, call_wait.max);

    printf("Call in the System \n Mean = %f  ",call_system.Mean());
    printf(" Std Dev  = %f \n",call_system.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",call_system.min,call_system.max);
    fprintf(stats_file, "Call in System,%d,%f,%f,%f,%f\n", call_system.N(), call_system.Mean(), call_system.StandardDeviation(), call_system.min, call_system.max);

    printf("Call duration \n Mean = %f  ",call_duration.Mean());
    printf(" Std Dev  = %f \n",call_duration.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",call_duration.min,call_duration.max);
    fprintf(stats_file, "Call Duration,%d,%f,%f,%f,%f\n", call_duration.N(), call_duration.Mean(), call_duration.StandardDeviation(), call_duration.min, call_duration.max);

    printf("Call Attended \n Mean = %f  ",call_attended.Mean());
    printf(" Std Dev  = %f \n",call_attended.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",call_attended.min,call_attended.max);
    fprintf(stats_file, "Calls Attended,%d,%f,%f,%f,%f\n", call_attended.N(), call_attended.Mean(), call_attended.StandardDeviation(), call_attended.min, call_attended.max);

    printf("Client waiting \n Mean  = %f  ",client_wait.Mean());
    printf(" Std Dev  = %f \n",client_wait.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",client_wait.min,client_wait.max);
    fprintf(stats_file, "Client Wait,%d,%f,%f,%f,%f\n", client_wait.N(), client_wait.Mean(), client_wait.StandardDeviation(), client_wait.min, client_wait.max);

    printf("Client in the System \n Mean = %f  ",client_system.Mean());
    printf(" Std Dev  = %f \n",client_system.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",client_system.min,client_system.max);
    fprintf(stats_file, "Client in System,%d,%f,%f,%f,%f\n", client_system.N(), client_system.Mean(), client_system.StandardDeviation(), client_system.min, client_system.max);

    printf("Client waiting for Manager \n Mean  = %f  ",manager_wait.Mean());
    printf(" Std Dev  = %f \n",manager_wait.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",manager_wait.min,manager_wait.max);
    fprintf(stats_file, "Manager Wait,%d,%f,%f,%f,%f\n", manager_wait.N(), manager_wait.Mean(), manager_wait.StandardDeviation(), manager_wait.min, manager_wait.max);

    printf("Manager duration \n Mean  = %f  ",manager_duration.Mean());
    printf(" Std Dev  = %f \n",manager_duration.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",manager_duration.min,manager_duration.max);
    fprintf(stats_file, "Manager Duration,%d,%f,%f,%f,%f\n", manager_duration.N(), manager_duration.Mean(), manager_duration.StandardDeviation(), manager_duration.min, manager_duration.max);

    printf("Client waiting for Teller \n Mean  = %f  ",teller_wait.Mean());
    printf(" Std Dev  = %f \n",teller_wait.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",teller_wait.min,teller_wait.max);
    fprintf(stats_file, "Teller Wait,%d,%f,%f,%f,%f\n", teller_wait.N(), teller_wait.Mean(), teller_wait.StandardDeviation(), teller_wait.min, teller_wait.max);

    printf("Teller duration \n Mean  = %f  ",teller_duration.Mean());
    printf(" Std Dev  = %f \n",teller_duration.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",teller_duration.min,teller_duration.max);
    fprintf(stats_file, "Teller Duration,%d,%f,%f,%f,%f\n", teller_duration.N(), teller_duration.Mean(), teller_duration.StandardDeviation(), teller_duration.min, teller_duration.max);

    printf("Client waiting for ATM \n Mean  = %f  ",atm_wait.Mean());
    printf(" Std Dev  = %f \n",atm_wait.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",atm_wait.min,atm_wait.max);
    fprintf(stats_file, "ATM Wait,%d,%f,%f,%f,%f\n", atm_wait.N(), atm_wait.Mean(), atm_wait.StandardDeviation(), atm_wait.min, atm_wait.max);

    printf("ATM duration \n Mean  = %f  ",atm_duration.Mean());
    printf(" Std Dev  = %f \n",atm_duration.StandardDeviation());
    printf(" Min  = %f  Max = %f \n",atm_duration.min,atm_duration.max);
    fprintf(stats_file, "ATM Duration,%d,%f,%f,%f,%f\n", atm_duration.N(), atm_duration.Mean(), atm_duration.StandardDeviation(), atm_duration.min, atm_duration.max);

    fflush(stats_file);
    fclose(stats_file);
}

int main(int argc, _TCHAR* argv[])
{
    char c;
    CClient *client= new CClient(); // client
    CCall *call= new CCall();

    // um ano = 60min * 8h * 21d * 12m
    double total_time = 60*8*21*12;
    executive=new CEventExecutive(total_time);

    // Setup queues
    call_queue=new CLista<CCall *>;
    client_atm_queue=new CLista<CClient *>;
    client_teller_queue=new CLista<CClient *>;
    client_manager_queue=new CLista<CClient *>;

    CDistribution dist;
    // Simulation Paramters
    call_arrival=10.0; // call arrival mean - Negative exponential distribution
    arrival_mean=5.0; // client arrival mean - Negative exponential distribution

    arrival_manager_prob = 0.1;
    arrival_teller_prob = 0.2;

    atm_service_mean = 4.0;
    atm_service_stddev = 2.0;

    teller_service_mean = 7.0;
    teller_service_stddev = 3.0;

    manager_service_mean = 10.0;
    manager_service_stddev = 4.0;

    atm_to_teller_prob = 0.15;
    atm_to_manager_prob = 0.15;

    min_service=0.2; // minimum of uniform distribution
    max_service=10000.0; // maximum of uniform distribution

    min_call=1.0;	// minimum of uniform distribution
    max_call=10.0;	// maximum of uniform distribution

    call_max_wait = 10.0;

    // Initial events: client arrival and call arrival
    client->SetEvent(ARRIVE);
    // Schedule next client arrival
    executive->AddEvent(dist.Exponential(arrival_mean),ARRIVE,client);

    call->SetEvent(CALL);
    // Schedule next call arrival
    executive->AddEvent(dist.Exponential(call_arrival),CALL,call);

    // Simulation loop
    while (executive->SimulationTime()<executive->SimulationEnd()) {
        sim_time=executive->TimeScan(); // Time Scan = get next event time
        executive->ExecuteEvents();  // Execute all events at sim_time
    }

    // Report statistics on mean, std dev, min and max
    StatisticsReport();
    printf("\nPress any key to end\n");
    scanf("%c",&c);
    return 0;

}

