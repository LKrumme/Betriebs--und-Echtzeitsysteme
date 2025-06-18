/*
 *	File:		queue.c
 *	Description:	Abstrakter Datentyp Schlange (fuer int-Werte)
 *
 *	Elmar Ludwig,  Universitaet Osnabrueck
 *	Monday, 14. June 1999
 *
 *	-----
 *
 *	Angepasst von Ruben Grest und Ruben Hockemeyer (21.10.2014).
 */

#include <stdlib.h>		/* malloc, free */

struct Queue			/* die interne Datenstruktur */
{
    struct Node *head;		/* erstes Element oder NULL */
    struct Node *tail;		/* letztes Element */
};

typedef struct Queue * Queue;

typedef void * element;

struct Node			/* Element der verketteten Liste */
{
    struct Node *next;		/* Zeiger auf naechstes Element */
    element value;		/* Wert dieses Elements */
};

/*
 * Neue Schlange anlegen, liefert NULL bei Fehler
 */
Queue queue_create (void)
{
    return calloc(1, sizeof (struct Queue));		/* einfach... */
}

/*
 * Test, ob Schlange leer ist (liefert 1 falls leer, 0 sonst)
 */
int queue_empty (Queue queue)
{
    return queue->head == 0;
}

/*
 * Element aus Schlange loeschen, liefert 0 bei Fehler
 */
int queue_delete (Queue queue)
{
    struct Node *node = queue->head;	/* erstes Element */

    if (!node) return 0;		/* Schlange leer? */

    queue->head = node->next;		/* Element aushaengen ... */
    free(node);				/* ... und auch freigeben */
    return 1;
}


/*
 * Schlange wieder loeschen und alles freigeben
 */
void queue_destroy (Queue queue)
{
    while (!queue_empty(queue)) queue_delete(queue);	/* alles raus... */
    free(queue);
}

/*
 * Element in Schlange einfuegen, liefert 0 bei Fehler
 */
int queue_insert (Queue queue, element value)
{
    struct Node *node = calloc(1, sizeof (struct Node));

    if (!node) return 0;	/* kein Speicher mehr */

    if (queue_empty(queue))	/* Fall 1: Schlange ist leer */
    {
	queue->head = node;		/* erstes Element */
	queue->tail = node;
    }
    else			/* Fall 2: Schlange nicht leer */
    {
	queue->tail->next = node;	/* am Ende anfuegen */
	queue->tail = node;
    }

    node->value = value;
    return 1;
}


/*
 * Vorderstes Element ansehen, liefert 0 bei leerer Schlange
 */
element queue_head (Queue queue)
{
    return queue->head ? queue->head->value : (element) 0;
}