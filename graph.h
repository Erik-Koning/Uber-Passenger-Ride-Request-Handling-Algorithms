/*graph.h*/
#include <stdio.h>
#include <stdlib.h>
 
typedef struct EDGE
{
    //Constant Variables
    int weight;            /* The weight to reach this node from source, used to calculate distance*/
    int vertex;			    /* node it connects to */
    struct EDGE *next;		/* Pointer to the next edge*/
} EDGE;

typedef struct NODE
{
    //Dynamic Variables for use in algorithm
    int distance;           /*Relative distance value from src node to this node used in algorithm*/
    int searched;           /* boolean flag for searched or not*/
    struct NODE *bestStepBack;
    //Constant Variables
    int vertex;
    struct EDGE *edge;		/* Pointer to the list of edges */
    struct NODE *next;		/* Pointer to the next node*/
} NODE;

typedef struct REQNODE
{   
    //dynamic variables for algorithm
    int distance;           /*used to store the distance score the respective REQNODE would result if it was chosen*/
    //Constant
    int id;                 /* identifier for node */
    int time;   
    int start;
    int end;
    int waiting;            /* set to 1 if the nodes time stamp is greater than the current elapsed time */
    int served;             /* set to 1 if the node has  been served */
    struct REQNODE *next;   /* pointer to next request */
} REQNODE;