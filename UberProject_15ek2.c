/*UberProject_15ek2.c*/
/*
    Written by Erik Koning
    November 14th 2017

    Uses Dijkstras algorithm to find shortest path between two nodes

    Prebuilds a 2 dimension array where row is the start, column is the destination, representing all possible routes and their distances as the value at that index

    Uses these distance values to minimize wait time for uber rider requests

    The architecture of this code has all functions above the main(). 
*/

#include "graph.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_CSV_LINE_LEN 1024	//max size of a line to read 10mb

#define INFINITY 99999			//given to edges to represent a respective distance not yet set.
		
#define numNodes 50				

static NODE staticGraph;    /* static means BSS which means blank static storage therefore zero-ed */
NODE *graph = & staticGraph;

static REQNODE staticRequestList;
REQNODE *requestList = & staticRequestList;     //pointer to a BBS of type REQNODE

int networkDistanceArr[numNodes+1][numNodes+1] = { {-1} };


 
/* *************************  QUEUE ***************************** */
#define MAXQLEN 3000	//3000 becuase there are 2000 something possible combinations
int     front = 0;      // index of next element to remove
int     length = 0;     // number of elements currently in queue
                        // next place to insert is (front+length)
                        // mod MAXQLEN
int     q[MAXQLEN];     // queue of integers

int qempty (void)
{
    return length==0;
}

int qfull (void)
{
    if(length>MAXQLEN)
    {
        printf("queue broken");
        return length>MAXQLEN;
    }
    return length==MAXQLEN;
}

int qinsert (int n)
// Add value n to the queue
{
    // First, check if even possible
    if (qfull())    return 0;
    // Can insert - new value gets placed at the end.
    q[(front+length) % MAXQLEN] = n;
    length++;
    return 1;
}

int qremove (int *n)
{
    // Check if there is anything to remove
    if (qempty())   return 0;
    // Front element is one to remove
    *n = q[front];
    front = (front + 1) % MAXQLEN;
    length--;
    return 1;
}

void qprint (void)
{
    int     i;
    for (i=0; i<length; i++)    {
        int k = (front+i)%MAXQLEN;      // compute actual array index
        printf ("(%d:%d) ", k, q[k]);
    }
    if(length == 0)
        printf("empty queue");
    putchar('\n');
}


/* ******************************** LINKED LIST HELPER FUNCTIONS & GRAPH ************************** */
/* Exit function to handle fatal errors*/
void err_exit(char* msg)                                   //no point using the "__inline" non-stardard function. dont tell compiler what to do, kinda bad practice
{                                                           
    printf("[Fatal Error]: %s \nExiting...\n", msg);
    exit(1);
}

/* Function to create an adjacency list node*/
NODE * createNode(NODE * node, int vertex)
{
    NODE * newNode = (NODE *) malloc(sizeof(NODE));

    if(!newNode)
        err_exit("Unable to allocate memory for new node");

    newNode->distance = INFINITY;
    newNode->searched = 0;
    newNode->vertex = vertex;
    newNode->bestStepBack = NULL;
    newNode->edge = NULL;
    newNode->next = node->next;

    node->next = (struct NODE *) newNode;
 
    return newNode;
}

/* Function to create an edge */
EDGE * createEdge(NODE * node, int vertex, int weight)
{
    EDGE * newEdge = (EDGE *) malloc(sizeof(EDGE));             //"(EDGE *)" code not really needed, but good coding practice
    if(!newEdge)
        err_exit("Unable to allocate memory for new edge");

    newEdge->weight = weight;
    newEdge->next = node->edge;
    newEdge->vertex = vertex;

    node->edge = (struct EDGE *) newEdge;                       //the link is now known from src node to new edge(another vertex)
 
    return newEdge;
}

REQNODE * createRequest(REQNODE *node,int id, int time, int start, int end)
{
    REQNODE * newRequest = (REQNODE *) malloc(sizeof(REQNODE));
    if(!newRequest)
        err_exit("Unable to allocate memory for new edge");

    newRequest->distance = INFINITY;
    newRequest->id = id;
    newRequest->time = time;
    newRequest->start = start;
    newRequest->end = end;
    newRequest->waiting = 0;
    newRequest->served = 0;
    newRequest->next = node->next;                          //insert new request after end

    node->next = (struct REQNODE *) newRequest;

    return newRequest;
}
 
//returns a pointer of the node as specified by the vertex value
NODE * findNode (int vertex)
{
    NODE *child;

    for (child = graph; child != NULL; child = child->next)
       if (child->vertex == vertex)
           return (child);
    return (NULL);
}

void displayGraph (void)
{
    NODE *child;
    EDGE *edge;

    for (child = graph; child != NULL; child = child->next)
    {
        printf("%d-", child->vertex);
        for (edge = child->edge; edge != NULL; edge = edge->next)
            printf("-w%d->%d", edge->weight, edge->vertex);

        printf("\n");
    }
    printf("\n");
}

//neccessary so previous dynamic variables used for finding shortest oath between two points does not carry over to a new search
void clearGraph (void)
{
    NODE *child;

    for (child = graph; child != NULL; child = child->next)
    {
        child->distance = INFINITY;
        child->searched = 0;
        child->bestStepBack = NULL;
    }
}

/************************** DIJKSTRAS FUNTIONS ***************************/
//given a node, and updates all dynamic variables for all that nodes adjacent edges
void checkAdjEdges(NODE *child){            
    int dist;
    NODE *setNode;
    EDGE* edge;

    //look at every edge of child and calculate relative distance for those nodes
    for(edge=child->edge; edge != NULL; edge = edge->next)
    {
        qinsert(edge->vertex);

        dist =  child->distance + edge->weight;
        setNode = findNode(edge->vertex);           //get node of what the edge goes to  

        if(dist < setNode->distance){               //if the nodes relative distance from the src is greater than the new found dist. The setNode's distance shouldnt be that far
            setNode->distance = dist;
            setNode->bestStepBack = child;
        }
    }
    child->searched = 1;                            //do not search a node that this function has already run on. Dont search a node that has already been searched.
}

//Dijkstra implementation
int dijkstra(int src, int dest){
    if(src == dest) return 0;
    int n;                              //dummy variable. recieves the removed item from queue
    NODE *child = findNode(src); 
    child->distance = 0;                //starting point, already here
    checkAdjEdges(child);               //looks at all adjacent edges from src node

    while(!qempty())
    {
        child = findNode(q[front]);
        qremove(&n);

        if(child->searched == 0)
            checkAdjEdges(child);
    }

    child = findNode(dest);
    return (child->distance);
}

//returns, and sets the available requests to be waiting if their timestamp is greater than or equal too the current time
int countAvailable(REQNODE * requestNode, int currTime)
{
    int count = 0;
    while(requestNode != NULL)
    {
        if(requestNode->served != 1 && currTime >= requestNode->time)
        {
            count++;
            requestNode->waiting = 1;
        }
        requestNode = requestNode->next;
    }
    return count;
}

//returns which node should be choosen based soley on which one is closest
REQNODE * closestAvailable(REQNODE * requestNode, int numAvailable, int currentLocation)
{
    REQNODE *best;
    int closest = INFINITY;
    int i = 0;
    while(i<numAvailable)
    {
    	//if a rider is waiting to be served
        if(requestNode->waiting == 1 && requestNode->served != 1)
        {
            if(networkDistanceArr[currentLocation][requestNode->start]<closest)
            {
                best = requestNode;
                closest = networkDistanceArr[currentLocation][requestNode->start];
            }
            i++;
        }
        requestNode = requestNode->next;
    }
    return best;
}

//finds the next available reuqest
REQNODE * nextAvailable(REQNODE * requestNode)
{
	while(requestNode->next->next != NULL)
	{
		if(requestNode->waiting == 1 && requestNode->served != 1)
      		return requestNode;
        requestNode = requestNode->next;
    }
    return requestNode;
}

//This is a very ugly function, but it shaved off 44 seconds in my data set.
REQNODE * closestAvailableConsideringOrder(REQNODE * requestNode, int numAvailable, int currentLocation, int elapsedTime)
{
	REQNODE *head = requestNode;
    REQNODE *best;
    REQNODE *best2;
    REQNODE *chall;
    REQNODE *chall2;
    int originalTime = elapsedTime;
    int originalLocation = currentLocation;
    int closest = INFINITY;
    int timeForBest = 0;
    int timeChallenger = 0;
    int i = 0;

    if(head->next->next == NULL)
    {
    	requestNode->distance = INFINITY;
    	return requestNode;
    }


	//****************************FIRST ROUTE TO BE COMPARED********************************
    while(i<numAvailable)
    {
    	//if a rider is waiting to be served
        if(requestNode->waiting == 1 && requestNode->served != 1)
        {
            if(networkDistanceArr[currentLocation][requestNode->start]<closest)
            {
                best = requestNode;
                closest = networkDistanceArr[currentLocation][requestNode->start];
            }
            i++;
        }
        requestNode = requestNode->next;
    }
    best->served = 1;						//need to undo this if turns out "best" variable request not actually best

    timeForBest += networkDistanceArr[currentLocation][best->start]+networkDistanceArr[best->start][best->end];
    elapsedTime += networkDistanceArr[currentLocation][best->start]+networkDistanceArr[best->start][best->end];
    currentLocation = best->end;
    closest = INFINITY;

    numAvailable = countAvailable(head,elapsedTime);
    while(numAvailable == 0)
    {
    	elapsedTime++;
    	timeForBest++;
    	numAvailable = countAvailable(head,elapsedTime);

    }


    requestNode = head;
    i = 0;
    while(i<numAvailable)
    {
    	//if a rider is waiting to be served
        if(requestNode->waiting == 1 && requestNode->served != 1)
        {
            if(networkDistanceArr[currentLocation][requestNode->start]<closest)
            {
                best2 = requestNode;
                closest = networkDistanceArr[currentLocation][requestNode->start];
            }
            i++;
        }
        requestNode = requestNode->next;
    }

    timeForBest += networkDistanceArr[currentLocation][best2->start]+networkDistanceArr[best2->start][best2->end];

    //**********************CHALLENGER CALCULATION************************
    closest = INFINITY;
    elapsedTime = originalTime;
    requestNode = head;
    currentLocation = originalLocation;
    numAvailable = countAvailable(requestNode,elapsedTime);

    while(numAvailable == 0)
    {
    	elapsedTime++;
    	timeChallenger++;
    	numAvailable = countAvailable(head,elapsedTime);
    }

    //IN THIS CASE THE BEST IS NOT AVAILABLE TO DO GOING TO CHECK A DIFFERENT ROUTE
    i = 0;
    while(i<numAvailable)
    {
    	//if a rider is waiting to be served
        if(requestNode->waiting == 1 && requestNode->served != 1)
        {
            if(networkDistanceArr[currentLocation][requestNode->start]<closest)
            {
                chall = requestNode;
                closest = networkDistanceArr[currentLocation][requestNode->start];
            }
            i++;
        }
        requestNode = requestNode->next;
    }
    chall->served = 1;						//need to undo this if turns out "best" variable request not actually best

    timeChallenger += networkDistanceArr[currentLocation][chall->start]+networkDistanceArr[chall->start][chall->end];
    elapsedTime += networkDistanceArr[currentLocation][chall->start]+networkDistanceArr[chall->start][chall->end];

    //update available count
    numAvailable = countAvailable(head,elapsedTime);
    while(numAvailable == 0)
    {
    	elapsedTime++;
    	timeChallenger++;
    	numAvailable = countAvailable(head,elapsedTime);
    }
    currentLocation = chall->end;

    currentLocation = chall->end;
    requestNode = head;
    closest = INFINITY;
    i = 0;
    while(i<numAvailable)
    {
    	//if a rider is waiting to be served
        if(requestNode->waiting == 1 && requestNode->served != 1)
        {
            if(networkDistanceArr[currentLocation][requestNode->start]<closest)
            {
                chall2 = requestNode;
                closest = networkDistanceArr[currentLocation][requestNode->start];
            }
            i++;
        }
        requestNode = requestNode->next;
    }

    timeChallenger += networkDistanceArr[currentLocation][chall2->start]+networkDistanceArr[chall2->start][chall2->end];

    //saves the times in the dynamic value in the node
    if(timeChallenger<timeForBest)
    {
    	best->served = 0;
    	chall->distance = timeChallenger;
    	return chall;
    }
    else
    {
    	chall->served = 0;
    	best->distance = timeForBest;
    	return best;
    }
}

//necessary to optimize algorithm and not search requests that have already been handled
REQNODE * removeOldRequests(REQNODE * RequestNode)
{
    while(RequestNode->served == 1)
    {
        if(RequestNode->next == NULL)
            return NULL;
        RequestNode = RequestNode->next;
    }


    return RequestNode;
}

//returns number of requests
int countRequests()
{
    int count = 0;
    REQNODE *n = requestList;
    while(n!=NULL)
    {
        count++;
        n=n->next;
    }
    return count;
}
 
int main(int argc, char *argv[])
{
    int requestID = 0;
    int availableRiders;
    int *waitTime;
    int waitTimeSum = 0;
    int currentLocation;
    int count = 0;
    int row = 0;
    int column = 0;
    int weight;
    int numRequests = 0;
    int time;
    int start;
    int end;
    int elapsedTime = 0;
    char str[MAX_CSV_LINE_LEN];
    char *token;
    int set = 0;
    int numEdges = 0;
    int longestRoute = 0;
    int totalTimeCarActive = 0;
    int *stoppedAtDistribution;

    NODE *child;
    REQNODE *request;

    FILE *myFile = fopen("newnetwork.csv", "r");

    if (myFile == NULL)
    {
        printf("failed to open file\n");
        exit(-1);
    }

    child = graph;              //graph points to the first node
    //reads a line of size MAX_CSV_LINE_LEN of the file and places it in "str"
    while (fgets (str, MAX_CSV_LINE_LEN, myFile) != NULL)
    {
        column = 0;
        //create node/child (item in a vertical list)
        child = createNode (child, row+1);   //create node with with parent node(child), and the row index plus 1 becuase index starts at 0. RETurns the node

        token = strtok(str, ",");       //splits the string up making token the first complete string until it gets broken up by the delimiter
        while (token != NULL)
        {         
            column++;
            weight = atoi(token);       //converts a string to int number type
            if (weight != 0)
            //create edge/sibling of child, a list of adjacent nodes(items in a list going horizontal from the respective node)
                createEdge (child, column, weight);    //returns the edge, child is the src, column is the destination, and the weight is the weight of the edge

            token = strtok(NULL,",");
        }
        row++;
    }    
    fclose(myFile);   

    graph=graph->next;
    displayGraph(); 

    for(int i = 1; i <= numNodes; i++)
    {
        for(int j = 1; j <= numNodes; j++)
        {
        	numEdges++;
            networkDistanceArr[i][j] = dijkstra(i,j);

            if(networkDistanceArr[i][j] > longestRoute)		//saves longest route value
            	longestRoute = networkDistanceArr[i][j];
            //printf("node %d ---> %d :  %d\n",i,j,networkDistanceArr[i][j]);
            clearGraph();                     //must clear graph every time to reset dynamic variables used in finding shortest path from src to dest.
        }

    }
    printf("Number of edges %d\n\n",numEdges );

    myFile = fopen("newrequests.csv","r");
    if (myFile == NULL)
    {
        printf("failed to open file\n");
        exit(-1);
    }
    
    memset(str,0,sizeof(str));
    request = requestList;              //requestList points to the first node of the request linked list
    //reads a line of size MAX_CSV_LINE_LEN of the file and places it in "str"
    while (fgets (str, MAX_CSV_LINE_LEN, myFile) != NULL)
    {
        numRequests++;
        count = 0;
        token = strtok(str,",");
        while(token != NULL)
        {
            if(count == 0)
                time = atoi(token);
            else if(count == 1)
                start = atoi(token);
            else if(count == 2)
                end = atoi(token);

            token = strtok(NULL,",");
            count++;
            if(count==3){
                request = createRequest(request,requestID,time,start,end);
                requestID++;
            }
        }
    }
    fclose(myFile);

    //builds arrays based on size recorded when files were read
    waitTime = (int *)malloc(sizeof(int)*numRequests);
    stoppedAtDistribution = (int *)malloc(sizeof(int)*(numNodes+1));

    //request and requestList point to first request.
    requestList = requestList->next;                                //neccessary so that the first node is pointed to officially. Used a static data struct initially for setup so need to move through that instance
    currentLocation = requestList->start;
    //requestList should be a pointer to the first request of linked lists of requests

    //main loop that handled requests
    while(requestList != NULL)
    {
        //there are riders waiting to be picked up
        if((availableRiders = countAvailable(requestList,elapsedTime)) > 0)                 //sets available requests->waiting value as true(1), and counts number that are waiting
        {
        	//There is more than one option for who to pickup so will compare to see which pair of riders to get
        	if(set == 0)
        	{
        		//function finds a pair of riders that would yeild the shortest cummulative time of the pair (rough function)
        		request = closestAvailableConsideringOrder(requestList, availableRiders, currentLocation, elapsedTime);
        	}
        	else
        	{
            request = closestAvailable(requestList, availableRiders, currentLocation);      //returns pointer to the closest request node to the uber drivers current location
        	}
        	if(request->id >= numRequests-2)		//ensure the closest available considering order funciton doesnt spill over the number of requests
        	{
        		set = 1;
        	}

            waitTime[request->id] = (elapsedTime - request->time) + networkDistanceArr[currentLocation][request->start];          //saving the time this indexed rider had to wait to be picked up.
            waitTimeSum += (elapsedTime - request->time) + networkDistanceArr[currentLocation][request->start];

            elapsedTime += networkDistanceArr[currentLocation][request->start];
            elapsedTime += networkDistanceArr[request->start][request->end];

            totalTimeCarActive += networkDistanceArr[currentLocation][request->start];
            totalTimeCarActive += networkDistanceArr[request->start][request->end];

            stoppedAtDistribution[request->start] += 1;
            stoppedAtDistribution[request->end] += 1;

            currentLocation = request->end;
            request->served = 1;

            printf("Served rider %d he waited %d units\n",request->id,waitTime[request->id]);
            requestList = removeOldRequests(requestList);                                   //moves the head of the requestList pointer
        }
        //no riders waiting
        else
        {
            elapsedTime++;
        }
    }
    //printf("%x\n",requestList);                 //hopefully should return 0 (NULL)  becuase all requests have been removed and now empty
    printf("Time to serve all requests %d\n",elapsedTime);
    printf("Time all riders waited %d\n",waitTimeSum);
   
    myFile = fopen("outputData.csv","w");
    fprintf(myFile, "Request #(id),Time Waited \n");

    count = 0;
    while(count<numRequests){
        fprintf(myFile, "%d,%d \n", count,waitTime[count]);
        count++;
    }
    fprintf(myFile, "\n Total time to handle requests, %d",elapsedTime);
    fprintf(myFile, "\n Total time riders cumulatively spent waiting, %d",waitTimeSum);
    fprintf(myFile, "\n Longest possible route, %d",longestRoute);
    fprintf(myFile, "\n Total time car driving, %d",totalTimeCarActive);

    fprintf(myFile,"\n");
    count = 1;
    while(count<=numNodes){
        fprintf(myFile, "Node:,%d,%d \n", count,stoppedAtDistribution[count]);
        count++;
    }

    fclose(myFile);	//close file so not to corrupt it

    return 0;
}