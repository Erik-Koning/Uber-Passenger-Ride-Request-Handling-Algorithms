/* intqueue.h - singly-linked-list implementation of a queue. 
 * ----------
 * SYSC 2006
 */

struct intnode {
    int value;
    struct intnode *next;
};

typedef struct intnode IntNode;

struct intlinkedqueue {
    IntNode *front;
    IntNode *rear;
    int size;
};

typedef struct intlinkedqueue IntQueue;

IntQueue *intqueue_construct(void);
void intqueue_removeall(IntQueue *queue);
void intqueue_destroy(IntQueue *queue);
_Bool intqueue_is_empty(const IntQueue *queue);
int intqueue_size(const IntQueue *queue);
void intqueue_enqueue(IntQueue *queue, int value);
_Bool intqueue_dequeue(IntQueue *queue, int *element);
_Bool intqueue_front(const IntQueue *queue, int *element);
void intqueue_print(const IntQueue *queue);
