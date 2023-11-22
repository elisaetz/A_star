// readingmap2.c
// - loads the information of nodes in a binary file and shows the information of a particular node.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define MAXSUCC 23  // in an optimal implementation it must be removed
#define R_earth 6371000 // in m ?
#define PI 3.141592653589793238462643383279502884197 

typedef struct {
    unsigned long id; // Node identification
    char name[200]; // in an optimal implementation it must change to a pointer
    double lat, lon;  // Node position
    unsigned short nsucc;  // Number of node successors; i. e. length of successors
    unsigned long successors[MAXSUCC]; // in an optimal implementation it must change to a pointer
} node;

unsigned long searchNode(unsigned long id, node *nodes, unsigned long nnodes);

typedef struct queue_element{
    node* node_element;
    double dist, h_dist; //distance to origin
    struct queue_element* next;
    struct queue_element* previuos;
}queue_element;

typedef struct expanded_queue_element{
    unsigned long id;
    double lat, lon;
    double distance;
    struct expanded_queue_element* next;
}expanded_queue_element;
//Here the first and last elements of the queue will be saved
typedef struct{
    queue_element* start;
    queue_element* end;
}queue;

typedef struct{
    expanded_queue_element* start;
    expanded_queue_element* end;
}ex_queue;

void print_queue(queue* qe){
    printf("the queueue has \n");
    queue_element* iterator = NULL;
    iterator = qe->start;
    int i = 0;
    while (iterator != NULL){
        printf("the element with id %lu and distance %lf \n", iterator->node_element->id, iterator->h_dist);
        iterator = iterator->next;
        i++;
        if(i>10) break;
    }
}

double get_distance(node* father_node, node* child_node){
    if (father_node->id == child_node->id) return 0;
    double phi_1 = father_node->lat*PI/180;
    double phi_2 = child_node->lat*PI/180;
    double phi_difference = (father_node->lat - child_node->lat)*PI/180;
    double lambda_difference = (father_node->lon - child_node->lon)*PI/180;
    double a = sin(phi_difference/2)*sin(phi_difference/2) + cos(phi_1)*cos(phi_2)*sin(lambda_difference/2)*sin(lambda_difference/2);
    double c = 2*atan2(sqrt(a),sqrt(1-a));
    double distance = R_earth*c;
    return distance;
}
double heuristisc(node* nd, node* goal){
    return get_distance(nd,goal);
}
// fa falta ficar el parent???
void enqueue_with_priority(queue* priority_qe, node* node_to_order, double distance, double heuristic){
    struct queue_element* newElement=(queue_element*)malloc(sizeof(queue_element));
    newElement->node_element = node_to_order;
    newElement->dist = distance;
    newElement->h_dist = heuristic + distance;
    newElement->next = NULL;
    if (priority_qe->end==NULL && priority_qe->start==NULL){
        priority_qe->start = newElement;
        priority_qe->end = newElement;
    }
    else{
        struct queue_element* iterator = NULL;
        iterator=priority_qe->start;
        while ((iterator != NULL)){
            if (newElement->h_dist < iterator->h_dist){
                if (iterator->node_element->id == priority_qe->start->node_element->id){
                    printf("MAL enqueue \n");
                    priority_qe->start->next = newElement->next;
                    priority_qe->start->next->previuos = newElement->previuos;
                    priority_qe->start = newElement;
                }
                else{
                    newElement->previuos = iterator->previuos;
                    newElement->next = iterator;
                    iterator->previuos->next = newElement;
                    iterator->previuos = newElement;
                }
                break;
            }
            else if (iterator->node_element->id == priority_qe->end->node_element->id){
                iterator->next = newElement;
                newElement->previuos = priority_qe->end;
                priority_qe->end = newElement;
                break;
            }
            iterator=iterator->next;
        }
    }
}
void requeue_with_priority(queue* priority_qe, queue_element* element_to_order, double distance, double heuristic){
    //take element from queue
    // struct queue_element* newElement=(queue_element*)malloc(sizeof(queue_element));
    // update distance and remove the position
    element_to_order->dist = distance;
    element_to_order->h_dist = heuristic + distance;
    if (element_to_order->node_element->id == priority_qe->end->node_element->id){
        // I si nomes hi ha 1 element ???!!!!!
        element_to_order->previuos->next = NULL;
        priority_qe->end = element_to_order->previuos;
    }
    else{
        element_to_order->previuos->next = element_to_order->next;
        element_to_order->next->previuos = element_to_order->previuos;
    }
    element_to_order->next = NULL;
    element_to_order->previuos = NULL;
    struct queue_element* iterator = NULL;
    iterator=priority_qe->start;
    while ((iterator!=NULL)){
        if (element_to_order->h_dist < iterator->h_dist){
            if (iterator->node_element->id == priority_qe->start->node_element->id){
                printf("MAL \n");
                priority_qe->start->next = element_to_order->next;
                priority_qe->start->next->previuos = element_to_order->previuos;
                priority_qe->start = element_to_order;
            }
            else{
                element_to_order->previuos = iterator->previuos;
                element_to_order->next = iterator;
                iterator->previuos->next = element_to_order;
                iterator->previuos = element_to_order;
            }
            break;
        }
        else if (iterator->node_element->id == priority_qe->end->node_element->id){
            iterator->next = element_to_order;
            element_to_order->previuos = priority_qe->end;
            priority_qe->end = element_to_order;
            break;
        }
        iterator=iterator->next;
    }
}
//this function will delete the first element of the queue
void dequeue(queue *qe){
    queue_element* first=qe->start;
    qe->start=first->next;
    free(first);
    // qe->start = qe->start->next;
}
void update_priority_queue(queue* qe, double provisional_distance, node* node_to_check, node* goal, unsigned* list, unsigned index){
    queue_element* iterator = NULL;
    double prov_h = provisional_distance + heuristisc(node_to_check,goal);
    if (list[index] != 1){
        enqueue_with_priority(qe, node_to_check, provisional_distance, heuristisc(node_to_check, goal));
        list[index] = 1;
    }
    else{
        iterator = qe->start;
        while (iterator != NULL){
            if ((node_to_check->id == iterator->node_element->id) && iterator->h_dist > prov_h){
                requeue_with_priority(qe, iterator, provisional_distance, heuristisc(iterator->node_element, goal));
                break;
            }
            iterator = iterator->next;
        }
    }
    // s'ha de poder fer amb 1 loop nomes, sense la funcio u sure???
}
void expand(ex_queue* expanded_qe, queue* priority_qe, queue_element* visited_element){
    //guardem l'element que visitem a una llista i el treiem de l'altra
    struct expanded_queue_element* newElement=(expanded_queue_element*)malloc(sizeof(expanded_queue_element));
    newElement->id = visited_element->node_element->id;
    newElement->lat = visited_element->node_element->lat;
    newElement->lon = visited_element ->node_element->lon;
    newElement->distance = visited_element->dist;
    newElement->next = NULL;
	if (expanded_qe->end==NULL && expanded_qe->start==NULL){
		expanded_qe->end = newElement;
        expanded_qe->start=newElement;
	}
	else{
		expanded_qe->end->next=newElement;
		expanded_qe->end=newElement;
	}
    dequeue(priority_qe);
}
//this function applies the BQS algorithm for a list of nodes, starting by the element [root] from that list
ex_queue A_star(node node_list[], node* start, node* goal, unsigned long order){
    unsigned i;
    unsigned* enqueued_list;
    queue priority_queue;
    ex_queue expanded_nodes;
    queue_element* current = NULL;

    enqueued_list = (unsigned*)malloc(order*sizeof(unsigned));

    expanded_nodes.start = NULL;
    expanded_nodes.end = NULL;
    priority_queue.start = NULL;
    priority_queue.end = NULL;

    enqueue_with_priority(&priority_queue, start, 0, get_distance(start, goal));
    enqueued_list[searchNode(start->id,node_list,order)] = 1;
    current = priority_queue.start;

    if(priority_queue.start->node_element->nsucc == 0) return expanded_nodes; //only if we begin with a single, unconnected point in the graph
    
    while (priority_queue.start != NULL){ 
        if (current->node_element->id == goal->id) {
            printf("hem arribat, el miracle real ha passat \n");
            break;
        }
        for (i = 0; i < current->node_element->nsucc; i++) {
            double dist_to_current_child = current->dist + get_distance(current->node_element, &node_list[current->node_element->successors[i]]);
            update_priority_queue(&priority_queue, dist_to_current_child, &node_list[current->node_element->successors[i]], goal, enqueued_list, current->node_element->successors[i]);
        }
        expand(&expanded_nodes,&priority_queue,current);
        current = current->next;
    }
    return expanded_nodes;
}


int main(int argc,char *argv[])
{
    clock_t start_time;
    unsigned long nnodes;

    start_time = clock();

    char binmapname[80];
    strcpy(binmapname,"andorra.csv.bin");

    if(argc>1) strcpy(binmapname,argv[1]);

    FILE *binmapfile;
    start_time = clock();

    binmapfile = fopen(binmapname,"rb");
    fread(&nnodes,sizeof(unsigned long),1,binmapfile);

    node * nodes;

    nodes = (node*) malloc(nnodes*sizeof(node));
    if(nodes==NULL){
        printf("Error when allocating the memory for the nodes\n");
        return 2;
    }

    fread(nodes,sizeof(node),nnodes,binmapfile);
    fclose(binmapfile);

    printf("Total number of nodes is %ld\n", nnodes);
    printf("Elapsed time: %f seconds\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);

    // Look for a node with more than 4 successors
    unsigned long index;

    for(unsigned long i=0; i<nnodes; i++) // print nodes with more than 2 successors
    {
        if(nodes[i].nsucc>4){
            index = i;
            break;
        }
        /*{
            printf("Node %lu has id=%lu and %u successors\n",i,nodes[i].id, nodes[i].nsucc);
        }*/
    }
    printf("Node %lu has id=%lu and %u successors:\n",index,nodes[index].id, nodes[index].nsucc);
    for(int i=0; i<nodes[index].nsucc; i++) printf("  Node %lu with id %lu.\n",nodes[index].successors[i], nodes[nodes[index].successors[i]].id);

    
    // my code

    // unsigned long start_id = 240949599;
    // unsigned long goal_id = 195977239;
    // unsigned long index_start = searchNode(start_id,nodes,nnodes);
    // unsigned long index_goal = searchNode(goal_id,nodes,nnodes);

    node start = nodes[78];
    node goal = nodes[31978];

    printf("el id del start es %lu \n", start.id);
    printf("el id del goal es %lu \n", goal.id);

    ex_queue shortest_path;
    shortest_path = A_star(nodes, &start, &goal, nnodes);
    return 0;

}

unsigned long searchNode(unsigned long id, node *nodes, unsigned long nnodes)
{
    // we know that the nodes where numrically ordered by id, so we can do a binary search.
    unsigned long l = 0, r = nnodes - 1, m;
    while (l <= r)
    {
        m = l + (r - l) / 2;
        if (nodes[m].id == id) return m;
        if (nodes[m].id < id)
            l = m + 1;
        else
            r = m - 1;
    }

    // id not found, we return nnodes+1
    return nnodes+1;
}
