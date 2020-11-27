// Author: Atanu Barai <atanu@nmsu.edu>
// Purpose: Interleaves memory trace
// Last Edit Nov 26
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define CACHE_LINE_SIZE 64 //Cache block/line size in bytes 

#define MAX_CORES 16

int main(int argc, char *argv[])
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned long long i = 0, trace_len_per_core[MAX_CORES], number;
    int num_cores, core;
    FILE *outfile;
    FILE *infile[MAX_CORES];
    int rshift = log2(CACHE_LINE_SIZE);
    char str[30];

    if(argc != 2)
    {
        printf("%d of 1 arguments received\n", (argc-1));
        printf("Command Line Arguments: number of cores\n");
        exit(1);
    }
    else
    {
        printf("%d of 1 arguments received\n", (argc-1));
        num_cores = atoi(argv[1]);
        printf("Number of Cores: %d\n", num_cores);
    }

    for(core = 0; core < num_cores; core++)
    {
        sprintf(str, "l3-%dcores_trace_core%d.dat",num_cores, core);
        infile[core] = fopen(str,"r");
        if(infile[core] == NULL)
        {
            printf("Error in opening input file on %dth time!\n", core+1);
            exit(1);
        }
    }
    sprintf(str, "L3_Filtered_InterleavedTrace%dCores.dat",num_cores);
    outfile = fopen(str,"w");
    if(outfile == NULL)
    {
        printf("Error in opening input file \n");
        exit(1);
    }

    
    printf("Interleaving Start\n");
    while(i < num_cores)
    {
        for(core = 0; core < num_cores; core++)
        {
            if((read = getline(&line, &len, infile[core])) != -1)
                fprintf(outfile, "%s", line);
            else
                ++i;
        }
    } 
    printf("Interleaving Done. Releasing Resources\n");
    if (line)
        free(line);
    fclose(outfile);
    for(core = 0; core < num_cores; core++)
        fclose(infile[core]);
    printf("Resources Released\n");
    return 0;
}

