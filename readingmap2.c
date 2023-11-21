// readingmap2.c
// - counts the number of nodes in the map
// - allocates memory for the nodes and loads the information of each node.
// - loads the edges and shows the information of a particular node.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
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
    newElement->next = NULL;
    if (priority_qe->end==NULL && priority_qe->start==NULL){
        priority_qe->start = newElement;
        priority_qe->end = newElement;
    }
    else{
        struct queue_element* iterator = NULL;
        iterator=priority_qe->start;
        while ((iterator != NULL)){
            if (distance + heuristic < iterator->h_dist){
                newElement->next = iterator;
                newElement->previuos = iterator->previuos;
                if (iterator->node_element->id == priority_qe->start->node_element->id){
                    priority_qe->start->next = newElement->next;
                    priority_qe->start = newElement;
                }
                break;
            }
            unsigned long id_iterator = iterator->node_element->id;
            unsigned long id_final = priority_qe->end->node_element->id;
            if (id_iterator == id_final){
                priority_qe->end->next=newElement;
                priority_qe->end=newElement;
                break;
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
    queue_element* first=qe->start;
    qe->start=first->next;
    free(first);
    // qe->start = qe->start->next;
}
int nodeIsNotEnqueued(queue* qe, node* node_to_check){
    //es podria fer utilitzant la funcio que ja ens ha donat....
    struct queue_element* iterator = NULL;
    iterator = qe->start;
    while (iterator != NULL){
        if (iterator->node_element->id == node_to_check->id) {
            printf("pas 1 \n");
            return 0;
            }
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
    struct expanded_queue_element* newElement=(expanded_queue_element*)malloc(sizeof(expanded_queue_element));
    newElement->id = visited_element->node_element->id;
    newElement->lat = visited_element->node_element->lat;
    newElement->lon = visited_element ->node_element->lon;
    newElement->distance = visited_element->dist;
    newElement->next = NULL;
	if (expanded_qe->end==NULL && expanded_qe->start==NULL){
        //vigila aqui elisa
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
    queue priority_queue;
    ex_queue expanded_nodes;
    queue_element* current = NULL;
    expanded_nodes.start = NULL;
    expanded_nodes.end = NULL;
    priority_queue.start = NULL;
    priority_queue.end = NULL;
    enqueue_with_priority(&priority_queue, start, 0, get_distance(start, goal));
    current = priority_queue.start;
    printf("el id del goal es %lu \n", goal->id);
    printf("el id del current es %lu \n", current->node_element->id);
    if(priority_queue.start->node_element->nsucc == 0) return expanded_nodes; //only if we begin with a single, unconnected point in the graph
    while (priority_queue.start != NULL){
        if (current->node_element->id == goal->id) {
            printf("doncs ja estaria \n");
            break;
        }
        for (i = 0; i < current->node_element->nsucc; i++) {
            double dist_to_current_child = current->dist + get_distance(current->node_element, &node_list[current->node_element->successors[i]]);
            update_priority_queue(&priority_queue, dist_to_current_child, &node_list[current->node_element->successors[i]], goal);
            printf("el index del fill es %lu \n", current->node_element->successors[i]);
        }
        expand(&expanded_nodes,&priority_queue,current);
        current = current->next;
    }
    return expanded_nodes;
}

int main(int argc,char *argv[])
{
    clock_t start_time;
    FILE *mapfile;
    unsigned long nnodes;
    char *line=NULL;
    size_t len;

    start_time = clock();

    if(argc<2){
        mapfile = fopen("andorra.csv", "r");
        printf("Opening map andorra.csv.\n");
    }
    else{
        mapfile = fopen(argv[1], "r");
        printf("Opening map %s\n",argv[1]);
    }    if (mapfile == NULL)
    {
        printf("Error when opening the file\n");
        return 1;
    }
    // count the nodes
    nnodes=0UL;
    while (getline(&line, &len, mapfile) != -1)
    {
        if (strncmp(line, "node", 4) == 0)
        {
            nnodes++;
        }
    }
    printf("Total number of nodes is %ld\n", nnodes);
    printf("Elapsed time: %f seconds\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);

    rewind(mapfile);
    
    start_time = clock();
    node *nodes;
    char *tmpline , *field , *ptr;
    unsigned long index=0;

    nodes = (node*) malloc(nnodes*sizeof(node));
    if(nodes==NULL){
        printf("Error when allocating the memory for the nodes\n");
        return 2;
    }

    while (getline(&line, &len, mapfile) != -1)
    {
        if (strncmp(line, "#", 1) == 0) continue;
        tmpline = line; // make a copy of line to tmpline to keep the pointer of line
        field = strsep(&tmpline, "|");
        if (strcmp(field, "node") == 0)
        {
            field = strsep(&tmpline, "|");
            nodes[index].id = strtoul(field, &ptr, 10);
            field = strsep(&tmpline, "|");
            strcpy(nodes[index].name,field);
            for (int i = 0; i < 7; i++)
                field = strsep(&tmpline, "|");
            nodes[index].lat = atof(field);
            field = strsep(&tmpline, "|");
            nodes[index].lon = atof(field);

            nodes[index].nsucc = 0; // start with 0 successors

            index++;
        }
    }
    printf("Assigned data to %ld nodes\n", index);
    printf("Elapsed time: %f seconds\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);
    printf("Last node has:\n id=%lu\n GPS=(%lf,%lf)\n Name=%s\n",nodes[index-1].id, nodes[index-1].lat, nodes[index-1].lon, nodes[index-1].name);
    
    rewind(mapfile);
    
    start_time = clock();
    int oneway;
    unsigned long nedges = 0, origin, dest, originId, destId;
    while (getline(&line, &len, mapfile) != -1)
    {
        if (strncmp(line, "#", 1) == 0) continue;
        tmpline = line; // make a copy of line to tmpline to keep the pointer of line
        field = strsep(&tmpline, "|");
        if (strcmp(field, "way") == 0)
        {
            for (int i = 0; i < 7; i++) field = strsep(&tmpline, "|"); // skip 7 fields
            if (strcmp(field, "") == 0) oneway = 0; // no oneway
            else if (strcmp(field, "oneway") == 0) oneway = 1;
            else continue; // No correct information
            field = strsep(&tmpline, "|"); // skip 1 field
            field = strsep(&tmpline, "|");
            if (field == NULL) continue;
            originId = strtoul(field, &ptr, 10);
            origin = searchNode(originId,nodes,nnodes);
            while(1)
            {
                field = strsep(&tmpline, "|");
                if (field == NULL) break;
                destId = strtoul(field, &ptr, 10);
                dest = searchNode(destId,nodes,nnodes);
                if((origin == nnodes+1)||(dest == nnodes+1))
                {
                    originId = destId;
                    origin = dest;
                    continue;
                }
                if(origin==dest) continue;
                // Check if the edge did appear in a previous way
                int newdest = 1;
                for(int i=0;i<nodes[origin].nsucc;i++)
                    if(nodes[origin].successors[i]==dest){
                        newdest = 0;
                        break;
                    }
                if(newdest){
                    if(nodes[origin].nsucc>=MAXSUCC){
                        printf("Maximum number of successors (%d) reached in node %lu.\n",MAXSUCC,nodes[origin].id);
                        return 5;
                    }
                    nodes[origin].successors[nodes[origin].nsucc]=dest;
                    nodes[origin].nsucc++;
                    nedges++;
                }
                if(!oneway)
                {   
                    // Check if the edge did appear in a previous way
                    int newor = 1;
                    for(int i=0;i<nodes[dest].nsucc;i++)
                        if(nodes[dest].successors[i]==origin){
                            newor = 0;
                            break;
                        }
                    if(newor){
                        if(nodes[dest].nsucc>=MAXSUCC){
                            printf("Maximum number of successors (%d) reached in node %lu.\n",MAXSUCC,nodes[dest].id);
                            return 5;
                        }
                        nodes[dest].successors[nodes[dest].nsucc]=origin;
                        nodes[dest].nsucc++;
                        nedges++;
                    }
                }
                originId = destId;
                origin = dest;
            }
        }
    }
    
    fclose(mapfile);
    printf("Assigned %ld edges\n", nedges);
    printf("Elapsed time: %f seconds\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);
    // Look for a node with more than 4 successors
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

    //my code
    // unsigned long start_id = 240949599;
    // unsigned long goal_id = 195977239;
    // unsigned long index_start = searchNode(start_id,nodes,nnodes);
    // unsigned long index_goal = searchNode(goal_id,nodes,nnodes);

    node start = nodes[28];
    node goal = nodes[26709];

    printf("el id del start es %lu \n", start.id);
    printf("el id del goal es %lu \n", goal.id);

    ex_queue shortest_path;
    shortest_path = A_star(nodes, &start, &goal, nnodes);
    printf("ha passat un miracle ? \n");

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
