// Author: Atanu Barai <atanu@nmsu.edu>
// Purpose: Interleaves memory trace

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
    unsigned long long trace_len = 0, i = 0, trace_len_per_core, number;
    int num_cores, core;
    FILE *outfile;
    FILE *infile[MAX_CORES];
    int rshift = log2(CACHE_LINE_SIZE);
    char str[30];

    if(argc != 3)
    {
        printf("%d of 2 arguments received\n", (argc-1));
        printf("Command Line Arguments: original trace length, number of cores\n");
        exit(1);
    }
    else
    {
        printf("%d of 2 arguments received\n", (argc-1));
        trace_len = atoll(argv[1]);
        num_cores = atoi(argv[2]);
        printf("Trace Length: %llu, Number of Cores: %d\n", trace_len, num_cores);
    }

    for(core = 0; core < num_cores; core++)
    {
        sprintf(str, "%dcores_trace_core%d.dat",num_cores, core);
        infile[core] = fopen(str,"r");
        if(infile[core] == NULL)
        {
            printf("Error in opening input file on %dth time!\n", core+1);
            exit(1);
        }
    }
    sprintf(str, "InterleavedTrace%dCores.dat",num_cores);
    outfile = fopen(str,"w");
    if(outfile == NULL)
    {
        printf("Error in opening input file \n");
        exit(1);
    }

    trace_len_per_core = trace_len / num_cores;
    printf("Interleaving Start\n");
    while(i < trace_len_per_core)
    {
        for(core = 0; core < num_cores; core++)
        {
            if((read = getline(&line, &len, infile[core])) != -1)
                fprintf(outfile, "%s", line);
        }
        ++i;
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

