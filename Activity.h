
#ifndef __CACTIVITY__
#define __CACTIVITY__
#include "CLista.h"

class CEntity
{
    // Entity class is the parent of children classes
public:
    int activity; // Current activity code
    double arrive, start, end;    // Marker for arrival
    // Marker for start and end of activity
    CEntity() {
        activity=0;
        arrive=start=end=0.0;
    }
    CEntity(int act) {
        activity=act;
        arrive=start=end=0.0;
    }

    void SetArrival(double av) {
        arrive=av;
    }
    void SetStart(double star) {
        start=star;
    }
    void SetEnd(double en) {
        end=en;
    }
    void SetActivity(int act) { // Set activity code
        activity=act;
    }
    double GetArrival() {
        return arrive;
    }
    double GetStart() {
        return start;
    }
    double GetEnd() {
        return end;
    }

    int GetActivity() { // retriacte current activity
        return activity;
    }



};

class CActivity
{
    // CActivity is a node of activity list
public:
    double time;   // time scheduled to activity ocurr
    int activity;    // activity code
    CEntity *entity; // Entity pointer
public:
    CActivity() { // activity constructor : Setactivity must be used later
        time=0;
        activity=0;
        entity=NULL;
    }
    CActivity(double tim, int activ, CEntity *ent) { // activity constructor
        time=tim;
        activity=activ;
        entity=ent;
    }

    void SetActivity( double tim, int activ, CEntity *ent) { // Set activity
        time=tim;
        activity=activ;
        entity=ent;
    }
    double Time() { // Retrieve time
        return time;
    }

    int Activity() { // Retrieve activity code
        return activity;
    }

    CEntity * Entity() { // Retrieve entity pointer
        return entity;
    }



};



typedef CLista<CActivity *> CActivityQueue;


bool ComparaMenor(CActivity *info1, CActivity *info2) // function used to sort the activity queue according to time
{
    return info1->time>info2->time;
}

class CActivityExecutive  // Class to support activity based executive
{
public :
    CActivityQueue *activity_list;  // activity list
    double simulation_time;  // Simulation current time
    double simulation_end;   // Simulation end time

    CActivityExecutive() { // Contructor : must use SetSimulationEnd later
        activity_list = new CActivityQueue;
        simulation_time=simulation_end=0.0;
    }
    CActivityExecutive(double sim_end) { // Contructor
        activity_list = new CActivityQueue;
        simulation_time=0.0;
        simulation_end=sim_end;
    }

    ~CActivityExecutive() {
        delete activity_list;
    }
    double SimulationTime() {
        return simulation_time;
    }

    double SimulationEnd() {
        return simulation_end;
    }

    void SetSimulationEnd( double sim) {
        simulation_end=sim;
    }

    CActivity * GetActivity() { // Get activity in the head of the list
        return activity_list->ObterInfo();
    }

    double TimeScan() { // Get the time of the head of the list
        CActivity *activ=GetActivity();
        simulation_time=activ->time;
        return activ->time;

    }

    double GetActivityTime() { // Get activity of the head of the list (queue)
        CActivity *activ=GetActivity();
        return activ->time;

    }
    void AddActivity(double tim, int act, CEntity *ent) { // Add activity inserting in a sorted list by time
        CActivity *activity = new CActivity(tim,act,ent);
        activity_list->InserirOrd(activity,ComparaMenor);
    }


    void RemoveActivity() { // Remove the activity in the head of the queue
        activity_list->Remover();
    }

    virtual void ExecuteActivities() { // Simulation activity execution at time simulation
        CActivity *activ;
        /* double sim_time=SimulationTime();
                // Execute all activity at sim_time
         while (sim_time==GetActivityTime())
        {  activ=GetActivity();   // Get activity at the front of the queue
           activ->ExecuteActivity(); // Call for executing activity method implemented in a subclass
           RemoveActivity();  // Remove activity

        }
        */
    }

};



#endif