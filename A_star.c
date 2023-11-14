#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MAXSUCC 23  // in an optimal implementation it must be removed
#define R_earth 6371 // in km ?
#define PI 3.141592653589793238462643383279502884197 

typedef struct {
    unsigned long id; // Node identification
    char name[200]; // in an optimal implementation it must change to a pointer
    double lat, lon;  // Node position
    unsigned short nsucc;  // Number of node successors; i. e. length of successors
    unsigned long successors[MAXSUCC]; // in an optimal implementation it must change to a pointer
} node;

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

double get_distance(node* father_node, node* child_node){
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
    newElement->next=NULL;
    if (priority_qe->end==NULL && priority_qe->start==NULL){
        priority_qe->end = newElement;
        priority_qe->start = newElement;
        priority_qe->start->next = newElement;
    }
    else{
        struct queue_element* iterator = NULL;
        iterator=priority_qe->start;
        while ((iterator!=NULL)){
            if (distance + heuristic < iterator->h_dist){
                newElement->next = iterator;
                newElement->previuos = iterator->previuos;
                if (iterator->node_element->id == priority_qe->start->node_element->id){
                    priority_qe->start->next = newElement->next;
                    priority_qe->start = newElement;
                }
                continue;
            }
            else if (iterator->node_element->id == priority_qe->end->node_element->id){
                priority_qe->end->next=newElement;
                priority_qe->end=newElement;
            }
            iterator=iterator->next;
        }
    }
}
void requeue_with_priority(queue* priority_qe, queue_element* element_to_order, double distance, double heuristic){
    //take element from queue
    element_to_order->previuos->next = element_to_order->next;
    element_to_order->next->previuos = element_to_order->previuos;
    element_to_order->dist = distance;
    element_to_order->h_dist = heuristic + distance;
    struct queue_element* iterator = NULL;
    iterator=priority_qe->start;
    while ((iterator!=NULL)){
        if (distance + heuristic < iterator->h_dist){
            element_to_order->next = iterator;
            element_to_order->previuos = iterator->previuos;
            if (iterator->node_element->id == priority_qe->start->node_element->id){
                priority_qe->start->next = element_to_order->next;
                priority_qe->start = element_to_order;
            }
            continue;
        }
        else if (iterator->node_element->id == priority_qe->end->node_element->id){
            priority_qe->end->next=element_to_order;
            priority_qe->end=element_to_order;
        }
        iterator=iterator->next;
    }
}
//this function will delete the first element of the queue
void dequeue(queue *qe){
    qe->start=qe->start->next;
}
int nodeIsNotEnqueued(queue* qe, node* node_to_check){
    struct queue_element* iterator = NULL;
    iterator = qe->start;
    while (iterator != NULL){
        if (iterator->node_element->id == node_to_check->id) return 0;
        iterator = iterator->next;
    }
    return 1;    
}

void update_priority_queue(queue* qe, double provisional_distance, node* node_to_check, node* goal){
    queue_element* iterator = NULL;
    double prov_h = provisional_distance + heuristisc(node_to_check,goal);
    if (nodeIsNotEnqueued(qe,node_to_check) == 1){
        enqueue_with_priority(qe, node_to_check, provisional_distance, heuristisc(node_to_check, goal));
    }
    else{
        iterator = qe->start;
        while (iterator != NULL){
            if ( (node_to_check->id == iterator->node_element->id) && iterator->h_dist > prov_h + provisional_distance){
                requeue_with_priority(qe, iterator, provisional_distance, prov_h);
            }
            iterator = iterator->next;
        }
    }
    // s'ha de poder fer amb 1 loop nomes, sense la funcio u sure???
}
void expand(ex_queue* expanded_qe, queue* priority_qe, queue_element* visited_element){
    //guardem l'element que visitem a una llista i el treiem de l'altra
    dequeue(priority_qe);
    struct expanded_queue_element* newElement=(expanded_queue_element*)malloc(sizeof(expanded_queue_element));
	newElement->id = visited_element->node_element->id;
    newElement->lat = visited_element->node_element->lat;
    newElement->lon = visited_element ->node_element->lon;
    newElement->distance = visited_element->dist;
    newElement->next=NULL;
	if (expanded_qe->end==NULL && expanded_qe->start==NULL){
        //vigila aqui elisa
		expanded_qe->end = newElement;
        expanded_qe->start=newElement;
		expanded_qe->start->next=newElement;
	}
	else{
		expanded_qe->end->next=newElement;
		expanded_qe->end=newElement;
	}
}
//this function applies the BQS algorithm for a list of nodes, starting by the element [root] from that list
ex_queue* A_star(node node_list[], node* start, node* goal, unsigned order){
    unsigned i;
    queue* priority_queue;
    ex_queue* expanded_nodes;
    queue_element* current = NULL;
    FILE* file;
    file = fopen("out.txt","w");
    priority_queue->start=NULL;
    priority_queue->end=NULL;
    enqueue_with_priority(priority_queue, start, 0, get_distance(start, goal));
    current = priority_queue->start;
    if(priority_queue->start->node_element->nsucc == 0) return expanded_nodes; //only if we begin with a single, unconnected point in the graph
    while (priority_queue->start!=NULL){
        expand(expanded_nodes,priority_queue,current);
        if (current->node_element->id == goal->id) {
            break;
        }
        for (i = 0; i < current->node_element->nsucc; i++) {
            double dist_to_current_child = current->dist + get_distance(current->node_element, &node_list[searchNode(current->node_element->successors[i], node_list, order)]);
            update_priority_queue(priority_queue, dist_to_current_child, current->node_element, goal);
        }
        current = current->next;
    }
    return expanded_nodes;
}

