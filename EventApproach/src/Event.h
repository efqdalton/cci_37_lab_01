
#ifndef __CEVENT__
#define __CEVENT__
#include "CLista.h"



class CEntity
{
    // Entity class is the parent of children classes
public:
    int current_event; // Current event code

    CEntity() {
        current_event=0;
    }
    CEntity(int ev) {
        current_event=ev;
    }
    void SetEvent(int ev) { // Set event code
        current_event=ev;

    }
    int CurrentEvent() { // retrieve current event
        return current_event;
    }


    virtual void ExecuteEvent(int event) { // This virtual method should
// Model
// switch (event){
//  case EVENT1: Event1(); break;  // Event1 is a method implemented in a subclass
//  case EVENT2: Event2(); break;  // Event2 is a method implemented in a sub class
// }
    }
};

class CEvent
{
    // CEvent is a node of event list
public:
    double time;	 // time scheduled to event ocurr
    int event;		 // Event code
    CEntity *entity; // Entity pointer
public:
    CEvent() { // Event constructor : SetEvent must be used later
        time=0;
        event=0;
        entity=NULL;
    }
    CEvent(double tim, int even, CEntity *ent) { // Event constructor
        time=tim;
        event=even;
        entity=ent;
    }

    void SetEvent( double tim, int even, CEntity *ent) { // Set Event
        time=tim;
        event=even;
        entity=ent;
    }
    double Time() { // Retrieve time
        return time;
    }

    int Event() { // Retrieve event code
        return event;
    }

    CEntity * Entity() { // Retrieve entity pointer
        return entity;
    }



};



typedef CLista<CEvent *> CEventQueue;


bool ComparaMenor(CEvent *info1, CEvent *info2) // function used to sort the event queue according to time
{
    return info1->time>info2->time;
}

class CEventExecutive  // Class to support event based executive
{
public :
    CEventQueue *event_list;  // Event list
    double simulation_time;  // Simulation current time
    double simulation_end;   // Simulation end time

    CEventExecutive() { // Contructor : must use SetSimulationEnd later
        event_list = new CEventQueue;
        simulation_time=simulation_end=0.0;
    }
    CEventExecutive(double sim_end) { // Contructor
        event_list = new CEventQueue;
        simulation_time=0.0;
        simulation_end=sim_end;
    }

    ~CEventExecutive() {
        delete event_list;
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

    CEvent * GetEvent() { // Get event in the head of the list
        return event_list->ObterInfo();
    }

    double TimeScan() { // Get the time of the head of the list
        CEvent *even=GetEvent();
        simulation_time=even->time;
        return even->time;
    }

    double GetEventTime() { // Get event of the head of the list (queue)
        CEvent *even=GetEvent();
        return even->time;
    }

    void AddEvent(double tim, int ev, CEntity *ent) { // Add event inserting in a sorted list by time
        CEvent *event = new CEvent(tim,ev,ent);
        event_list->InserirOrd(event,ComparaMenor);
    }

    void RemoveEvent() { // Remove the event in the head of the queue
        event_list->Remover();
    }

    void ExecuteEvents() { // Simulation event execution at time simulation
        CEvent *even;
        double sim_time=SimulationTime();
        // Execute all event at sim_time
        while (sim_time==GetEventTime()) {
            even=GetEvent();		// Get event at the front of the queue
            even->entity->ExecuteEvent(even->event); // Call for executing event method implemented in a subclass
            RemoveEvent();  // Remove event
        }
    }
};



#endif
