// readingmap2.c
// - counts the number of nodes in the map
// - allocates memory for the nodes and loads the information of each node.
// - loads the edges and shows the information of a particular node.
// - writes the information in a binary file
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAXSUCC 23  // in an optimal implementation it must be removed


typedef struct {
    unsigned long id; // Node identification
    char* name; // in an optimal implementation it must change to a pointer
    double lat, lon;  // Node position
    unsigned short nsucc;  // Number of node successors; i. e. length of successors
    unsigned long* successors; // in an optimal implementation it must change to a pointer
} node;

unsigned long searchNode(unsigned long id, node *nodes, unsigned long nnodes);

int main(int argc,char *argv[])
{
    clock_t start_time;
    FILE *mapfile;
    unsigned long nnodes;
    char *line=NULL;
    size_t len;

    start_time = clock();

    char mapname[80];
    strcpy(mapname,"andorra.csv");

    if(argc>1) strcpy(mapname,argv[1]);

    mapfile = fopen(mapname, "r");
    if (mapfile == NULL)
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
    // printf("Elapsed time: %f seconds\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);

    rewind(mapfile);
    
    // start_time = clock();
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
    // printf("Elapsed time: %f seconds\n", (float)(clock() - start_time) / CLOCKS_PER_SEC);
    printf("Last node has:\n id=%lu\n GPS=(%lf,%lf)\n Name=%s\n",nodes[index-1].id, nodes[index-1].lat, nodes[index-1].lon, nodes[index-1].name);
    
    rewind(mapfile);
    
    // start_time = clock();
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

    FILE *binmapfile;
    char binmapname[80];
    strcpy(binmapname,mapname);
    strcat(binmapname,".bin");

    binmapfile = fopen(binmapname,"wb");
    fwrite(&nnodes,sizeof(unsigned long),1,binmapfile);
    fwrite(nodes,sizeof(node),nnodes,binmapfile);
    for (int i = 0; i < nnodes; i++){
        unsigned name_size = strlen(nodes[i].name);
        fwrite(&name_size, sizeof(unsigned),1,binmapfile);
        fwrite(nodes[i].name, sizeof(char),strlen(nodes[i].name), binmapfile);
    }
    for (int i = 0; i < nnodes; i++){
        fwrite(nodes[i].successors, sizeof(unsigned),nodes[i].nsucc,binmapfile);
    }
    fclose(binmapfile);

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
