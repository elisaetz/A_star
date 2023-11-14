// readingmap2.c
// - loads the information of nodes in a binary file and shows the information of a particular node.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAXSUCC 23  // in an optimal implementation it must be removed

typedef struct {
    unsigned long id; // Node identification
    char name[200]; // in an optimal implementation it must change to a pointer
    double lat, lon;  // Node position
    unsigned short nsucc;  // Number of node successors; i. e. length of successors
    unsigned long successors[MAXSUCC]; // in an optimal implementation it must change to a pointer
} node;

unsigned long searchNode(unsigned long id, node *nodes, unsigned long nnodes);

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
