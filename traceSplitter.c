// Author: Atanu Barai <atanu@nmsu.edu>
// Purpose: Splits memory trace to different cores

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define CACHE_LINE_SIZE 64 //Cache block/line size in bytes 

#define MAX_CORES 16

char * line = NULL;
size_t len = 0;
ssize_t read;
unsigned long long trace_len = 0, minAddr, maxAddr, offset, i = 0, trace_len_per_core, j;
unsigned long long shared_addr_arr[100];
int num_cores, core, n_shared_addr, rshift, found;
FILE *infile;
FILE *sharedfile;
FILE *outfile[MAX_CORES];

char str[30];


int main(int argc, char *argv[])
{
    unsigned long long number;
    if(argc != 7)
    {
        printf("%d of 6 arguments received\n", (argc-1));
        printf("Command Line Arguments: processed trace, trace length, number of cores, minAddr, \
            maxAddr, shared trace\n");
        exit(1);
    }
    else
    {
        printf("%d of 6 arguments received\n", (argc-1));
        trace_len = atoll(argv[2]);
        num_cores = atoi(argv[3]);
        minAddr = strtol(argv[4], NULL, 16);
        maxAddr = strtol(argv[5], NULL, 16);
        printf("Trace Length: %llu, Number of Cores: %d, Min Address %llu, Max Address %llu\n",\
            trace_len, num_cores, minAddr, maxAddr);
    }
    rshift = log2(CACHE_LINE_SIZE);
    infile = fopen(argv[1],"r");
    sharedfile = fopen(argv[6],"r");
    if(infile == NULL || sharedfile == NULL)
    {
        printf("Error in opening input file \n");
        exit(1);
    }
    // Get unique addresses from the shared trace
    n_shared_addr = 0;
    //printf("\n%d shared addresses\n", n_shared_addr);
    while ((read = getline(&line, &len, sharedfile)) != -1)
    {
        number = strtoull(line, NULL, 0);
        i = 0;
        while(i < n_shared_addr && shared_addr_arr[i] != number)
            i++;
        if(i == n_shared_addr)
        {
            shared_addr_arr[i] = number;
            printf("%llu ", shared_addr_arr[i]);
            n_shared_addr++;
        }
    }
    fclose(sharedfile);
    printf("\n%d shared addresses\n", n_shared_addr);
    
    for(core = 0; core < num_cores; core++)
    {
        snprintf(str, 26, "%dcores_trace_core%d.dat",num_cores, core);
        printf("Trace: %s\n", str);
        outfile[core] = fopen(str,"w");
        if(outfile[core] == NULL)
        {
            printf("Error in opening input file on %dth time!\n", core+1);
            exit(1);
        }
    }
    i = 0;
    offset = maxAddr - minAddr;
    trace_len_per_core = (trace_len / num_cores) + 1;
    printf("Splitting the trace into %d traces with length %llu\n", num_cores, trace_len_per_core);
    while ((read = getline(&line, &len, infile)) != -1)
    {
        number = strtoull(line, NULL, 0);
	    // printf("%llu ", number);
        found = 0;
        for(j = 0; j < n_shared_addr; j++)
        {
            if(shared_addr_arr[j] == number)
            {
                found = 1;
                break;
            }
        }
        core = i / trace_len_per_core;
        if(found)
        {
            fprintf(outfile[core], "%#llx\n", number);
        }
        else
        {
            number = number + ( offset * core );
            fprintf(outfile[core], "%#llx\n", number);
        }
        ++i;
    }
    printf("Split Done. Releasing resources\n");
    if (line)
        free(line);
    fclose(infile);
    for(core = 0; core < num_cores; core++)
        fclose(outfile[core]);
    return 0;
}

