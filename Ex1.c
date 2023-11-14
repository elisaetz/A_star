//Name: Elisa Etzkorn NIA:1535940
#include <stdio.h>
#include <stdlib.h>

//defining structs
typedef struct{
    unsigned nedges;
    unsigned edges[8]; // limit of edges in a node = 8
}node;
//We define an element of our queue
typedef struct queue_element{
	node v_node;
	struct queue_element* next;
}queue_element;
//Here the first and last elements of the queue will be saved
typedef struct{
	queue_element* start;
	queue_element* end;
}queue;

//defining functions, this first one to add an element to the end of the queue
void enqueue(queue* qe, node nd){
	struct queue_element* newElement=(queue_element*)malloc(sizeof(queue_element));
	newElement->v_node=nd;
    newElement->next=NULL;
	if (qe->end==NULL && qe->start==NULL){
		qe->end=newElement;
        qe->start=newElement;
		qe->start->next=newElement;
	}
	else{
		qe->end->next=newElement;
		qe->end=newElement;
	}
}
//this function will delete the first element of the queue
void dequeue(queue *qe){
	qe->start=qe->start->next;
}
//this function applies the BQS algorithm for a list of nodes, starting by the element [root] from that list
unsigned* BFS(node node_list[], unsigned root, unsigned order){
    unsigned i;
    unsigned* enqueued_nodes = (unsigned*)malloc(order * sizeof(unsigned));
    for (i = 0; i < order; i++) enqueued_nodes[i] = 0; //none of the nodes are visited
    queue Queue;
    Queue.start=NULL;
    Queue.end=NULL;
    enqueue(&Queue,node_list[root]);
    enqueued_nodes[root]=1;
    if(Queue.start->v_node.nedges==0) return enqueued_nodes; //only if we begin with a single, unconnected point in the graph
    while (Queue.start!=NULL){
        for (i = 0; i < Queue.start->v_node.nedges; i++) {
            if (enqueued_nodes[Queue.start->v_node.edges[i]] == 0) {
                enqueue(&Queue, node_list[Queue.start->v_node.edges[i]]);
                enqueued_nodes[Queue.start->v_node.edges[i]] = 1;
            }
        }
        dequeue(&Queue);
    }
    return enqueued_nodes;
}
//this function returns 1 if the BQS algorithm defined before goes through all the nodes and 0 otherwise
unsigned IsConnected(unsigned enq_nodes[],unsigned order){
    unsigned i;
    unsigned sum=0;
    for (i=0;i<order;i++) sum+=enq_nodes[i];
    if (sum==order) {
        return 1;
    } else return 0;
}

int main(int argc,char *argv[]){
    FILE * defgraph;
    node * nodelist;
    unsigned i,j;
    unsigned gsize,gorder,or,dest,groot;
    
    defgraph=fopen(argv[1],"r");
    if(defgraph==NULL){
        printf("\nERROR: Data file not found.\n");
        return -1;
    }    
    fscanf(defgraph,"%u",&gorder);
    fscanf(defgraph,"%u",&gsize);
    
    //free space for our variables (not changed)
    nodelist = (node *) malloc(gorder*sizeof(node));
    
    //read and safe graph (not changed)
    for (i=0; i<gorder; i++) nodelist[i].nedges = 0;

    for (j=0; j<gsize; j++){
        fscanf(defgraph,"%u %u",&or,&dest);
        nodelist[or].edges[nodelist[or].nedges]=dest;
        nodelist[or].nedges++;
        nodelist[dest].edges[nodelist[dest].nedges]=or;
        nodelist[dest].nedges++;       
    }    
    fclose(defgraph);
    /*
    Here to ask the user with what root the want to start

    printf("Please enter a number between 0 and %u node for the root: ",gorder-1);
    scanf("%u",&groot);
    */

    //we give a root for the BFS to start there
    groot=rand()%gorder;

    printf("%u", IsConnected(BFS(nodelist,groot,gorder),gorder));

    return 0;
}