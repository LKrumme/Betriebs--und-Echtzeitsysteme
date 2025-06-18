#include <stdlib.h>
#include "lists.h"
#include "global.h"
//Reihenfolge: A, B, C, D, E
//Laufezeiten: 13, 25, 9, 12, 40 Minuten
//Prioritäten: 1, 5, 3, 2, 4 

typedef struct Process {
    LIST_NODE_HEADER(struct Process);
    char *name; 
    int time;
    int priority;
    int timeWorkedOn;
} ProcessNode;

typedef struct {
    LIST_HEADER(ProcessNode);
} ProcessList;

static ProcessList list;

//Variablendeklarationen
int summeZeit=0;
double verweilzeit=0;
double mittlereVerweilzeit=0;
int finishedProcesses=0;

void workOnProcess(ProcessList list){
    ProcessNode *current = list.head; 

    while (current != NULL){
        summeZeit = summeZeit+current->time;
        verweilzeit = verweilzeit+summeZeit;
        printf("%s wurde abgearbeitet (Aktuelle Zeit: %d) \n", current->name, summeZeit);
        current = current->next;
    }
    mittlereVerweilzeit = verweilzeit/List_count(&list);
    printf("Verweilzeit: %g \n", verweilzeit);
    printf("Mittlere Verweilzeit: %f \n\n", mittlereVerweilzeit);
    summeZeit=0; verweilzeit=0; mittlereVerweilzeit=0;
}

void roundRobinWork(ProcessList list, int zeitscheibendauer){
    printf("Es wird an den Jobs zu folgenden Anteilen gearbeitet: \n");
    ProcessNode *current = list.head; 
    while (current != NULL){
        if(current->timeWorkedOn < current->time){
            summeZeit = summeZeit+zeitscheibendauer;
            current->timeWorkedOn = current->timeWorkedOn+zeitscheibendauer;
            printf("Es wurde %dmin an %s gearbeitet.\n", zeitscheibendauer, current->name);
        }
        if (current -> timeWorkedOn == current -> time){
            verweilzeit = verweilzeit+summeZeit;
            finishedProcesses = finishedProcesses+1;
            current-> timeWorkedOn = current->timeWorkedOn+1; //Um raus zu sein.
        }
        current = current->next;
    }
}

void roundRobin(ProcessList list){
    while (1){
        roundRobinWork(list,1);
        if (finishedProcesses==List_count(&list))
            break;
    }
    ProcessNode *current = list.head; 
    while (current != NULL){
        current->timeWorkedOn=0;
        current = current->next;
    }
    mittlereVerweilzeit = verweilzeit/List_count(&list);
    printf("Verweilzeit: %g \n", verweilzeit);
    printf("Mittlere Verweilzeit: %f \n\n", mittlereVerweilzeit);
    summeZeit=0; verweilzeit=0; mittlereVerweilzeit=0; finishedProcesses=0;
}

void roundRobinPrioWork(ProcessList list){
    printf("Es wird an den Jobs zu folgenden Anteilen gearbeitet: \n");
    ProcessNode *current = list.head;
    int zeitscheibendauer; 
    while (current != NULL){
        zeitscheibendauer = current->priority;
        if(current->timeWorkedOn < current->time){
            summeZeit = summeZeit+zeitscheibendauer;
            current->timeWorkedOn = current->timeWorkedOn+zeitscheibendauer;
            printf("Es wurde %dmin an %s gearbeitet.\n", zeitscheibendauer, current->name);
        }
        if (current -> timeWorkedOn == current -> time){
            verweilzeit = verweilzeit+summeZeit;
            finishedProcesses = finishedProcesses+1;
            current-> timeWorkedOn = current->timeWorkedOn+1; //Um raus zu sein.
        }
        current = current->next;
    }
}

void roundRobinPrio(ProcessList list){
    while (1){
        roundRobinPrioWork(list);
        if (finishedProcesses==List_count(&list))
            break;
    }
    mittlereVerweilzeit = verweilzeit/List_count(&list);
    printf("Verweilzeit: %g \n", verweilzeit);
    printf("Mittlere Verweilzeit: %f \n\n", mittlereVerweilzeit);
    summeZeit=0; verweilzeit=0; mittlereVerweilzeit=0; finishedProcesses=0; 
}

int compareProcessByTime(ProcessNode *node1, ProcessNode *node2, void *userData){
    if (node1->time > node2->time)
        return 1;
    else if (node1->time < node2->time)
        return -1;
    else
        return 0;
}

int compareProcessByPrio(ProcessNode *node1, ProcessNode *node2, void *userData){
    if (node1->priority < node2->priority)
        return 1;
    else if (node1->priority > node2->priority)
        return -1;
    else
        return 0;
}

int main (void){

    

    //Liste initialisieren
    List_init(&list);

    //Prozesse erstellen;
    ProcessNode *a = LIST_NEW_NODE(ProcessNode);
    a->name = "A";
    a->time = 13; 
    a->priority = 1;
    a->timeWorkedOn = 0;

    ProcessNode *b = LIST_NEW_NODE(ProcessNode);
    b->name = "B";
    b->time = 25;
    b->priority = 5;
    b->timeWorkedOn = 0;

    ProcessNode *c = LIST_NEW_NODE(ProcessNode);
    c->name = "C";
    c->time = 9; 
    c->priority = 3;
    c->timeWorkedOn = 0;

    ProcessNode *d = LIST_NEW_NODE(ProcessNode);
    d->name = "D";
    d->time = 12;
    d->priority = 2;
    d->timeWorkedOn = 0;

    ProcessNode *e = LIST_NEW_NODE(ProcessNode);
    e->name = "E";
    e->time = 40;
    e->priority = 4;
    e->timeWorkedOn = 0;

    //Prozesse zur Liste hinzufügen 
    List_append(&list, a);
    List_append(&list, b);
    List_append(&list, c);
    List_append(&list, d);
    List_append(&list, e);

    //Round Robin
    printf("Round Robin \n");
    roundRobin(list);


    //RR mit Prio
    printf("Round Robin mit Prio \n");
    roundRobinPrio(list);

    //FCFS
    printf("FCFS \n");
    workOnProcess(list);

    //SJF
    List_sort(&list, (ListNodeCompareFunction) compareProcessByTime, NULL);

    printf("SJF \n");
    workOnProcess(list);

    //Prioritätsgesteuertes Scheduling
    List_sort(&list, (ListNodeCompareFunction) compareProcessByPrio, NULL);

    printf("Prio Scheduling \n");
    workOnProcess(list);


    List_done(&list, NULL, NULL);
    return 0;
}