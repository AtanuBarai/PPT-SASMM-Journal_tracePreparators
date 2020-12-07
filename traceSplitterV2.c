/*
// Author: Atanu Barai <atanu@nmsu.edu>
// Purpose: Splits BB labeled memory trace to different cores, assumes there is no overlap 
// in the memory trace. Takes shared addresses into consideration.
// min_max_addr.dat, bb_counts.dat, shared_addresses.dat files needs to be in the folder
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define CACHE_LINE_SIZE 64 //Cache block/line size in bytes

#define MAX_CORES 16

typedef struct
{
    char bb_id[20];
    unsigned long count;
    unsigned long bb_per_core;
    unsigned long done_count;
} bb_counts;
bb_counts *bb_count_arr;
char *line = NULL;
char *ret;
size_t len = 0;
ssize_t read;
unsigned long long minAddr, maxAddr, offset, i = 0, trace_len_per_core, j;
unsigned long long shared_addr_arr[100];
int num_cores, core, n_shared_addr, rshift, is_shared, n_bb, k, current_bb_index;
FILE *memTraceFile;
FILE *sharedAddrFile;
FILE *bbCountFile;
FILE *bbListFile;
FILE *minMaxAddrFile;
FILE *outfile[MAX_CORES];

char str[30];

unsigned long get_bb_index(char *bb_id)
{
    int index;
    for (index = 0; index < n_bb; index++)
    {
        if (strcmp(bb_count_arr[index].bb_id, bb_id) == 0)
        {
            // printf("MATCH\n");
            return index;
        }
    }
    return -1;
}

int main(int argc, char *argv[])
{
    unsigned long long number, temp_number;
    if (argc != 4)
    {
        printf("%d of 3 arguments received\n", (argc - 1));
        printf("Command Line Arguments: bb_file, mem_trace, number of cores\n");
        exit(1);
    }
    else
    {
        printf("%d of 3 arguments received\n", (argc - 1));
        bbListFile = fopen(argv[1], "r");
        memTraceFile = fopen(argv[2], "r");
        num_cores = atoi(argv[3]);
    }

    sharedAddrFile = fopen("shared_addresses.dat", "r");
    bbCountFile = fopen("bb_counts.dat", "r");
    minMaxAddrFile = fopen("min_max_addr.dat", "r");

    rshift = log2(CACHE_LINE_SIZE);
    printf("rshift %d\n", rshift);

    if (memTraceFile == NULL || bbListFile == NULL)
    {
        printf("Error opening command line files\n");
        exit(1);
    }
    if (sharedAddrFile == NULL || bbCountFile == NULL || minMaxAddrFile == NULL)
    {
        printf("Error opening min_max_addr.dat, bb_counts.dat and/or shared_addresses.dat file\n");
        exit(1);
    }

    // BB list file
    while ((read = getline(&line, &len, bbListFile)) != -1)
        n_bb++;

    bb_count_arr = (bb_counts *)malloc(n_bb * sizeof(bb_counts));

    // BB count file
    i = 0;
    while ((read = getline(&line, &len, bbCountFile)) != -1)
    {
        ret = strtok(line, ",");
        strcpy(bb_count_arr[i].bb_id, ret);
        ret = strtok(NULL, ":");
        bb_count_arr[i].count = strtoul(ret, NULL, 0);
        if (bb_count_arr[i].count < num_cores)
            bb_count_arr[i].bb_per_core = 1;
        else
            bb_count_arr[i].bb_per_core = bb_count_arr[i].count / num_cores;
        bb_count_arr[i].done_count = 0;
        i++;
    }
    fclose(bbCountFile);

    // Min Max Address
    while ((read = getline(&line, &len, minMaxAddrFile)) != -1)
    {
        if (strstr(line, "Min") != NULL)
        {
            ret = strchr(line, ':');
            ret += 2;
            minAddr = strtoul(ret, NULL, 0);
        }
        else if (strstr(line, "Max") != NULL)
        {
            ret = strchr(line, ':');
            ret += 2;
            maxAddr = strtoul(ret, NULL, 0);
        }
    }
    fclose(minMaxAddrFile);
    printf("Here\n");
    printf("Max Addr: %#llx, Min Addr: %#llx\n", maxAddr, minAddr);

    // Get unique addresses from the shared trace
    n_shared_addr = 0;
    //printf("\n%d shared addresses\n", n_shared_addr);
    while ((read = getline(&line, &len, sharedAddrFile)) != -1)
    {
        number = strtoull(line, NULL, 0);
        i = 0;
        while (i < n_shared_addr && shared_addr_arr[i] != number)
            i++;
        if (i == n_shared_addr)
        {
            shared_addr_arr[i] = number;
            printf("%#llx ", shared_addr_arr[i]);
            n_shared_addr++;
        }
    }
    fclose(sharedAddrFile);
    printf("\n%d shared addresses\n", n_shared_addr);

    for (core = 0; core < num_cores; core++)
    {
        snprintf(str, 26, "%dcores_trace_core%d.dat", num_cores, core);
        printf("Trace: %s\n", str);
        outfile[core] = fopen(str, "w");
        if (outfile[core] == NULL)
        {
            printf("Error in opening input file on %dth time!\n", core + 1);
            exit(1);
        }
    }

    offset = maxAddr - minAddr;
    printf("Splitting the trace into %d traces\n", num_cores);
    while ((read = getline(&line, &len, memTraceFile)) != -1)
    {
        number = strtoull(line, NULL, 0);
        if (number != 0 && line[0] == '0' && line[1] == 'x') // An address
        {
            // Check if the address is shared or not
            is_shared = 0;
            for (j = 0; j < n_shared_addr; j++)
            {
                if (shared_addr_arr[j] == number)
                {
                    is_shared = 1;
                    // printf("Shared Address %#llx %#llx\n", number, (number >> rshift));
                    break;
                }
            }

            if (bb_count_arr[current_bb_index].bb_per_core == 1)
            {
                // Make a copy to all the traces, add offset if not shared address
                for (core = 0; core < num_cores; core++)
                {
                    temp_number = number;
                    if (!is_shared)
                        temp_number = temp_number + (offset * core);
                    temp_number = temp_number >> rshift;
                    fprintf(outfile[core], "%#llx\n", temp_number);
                }
            }
            else
            {
                // Assign to one of the traces
                core = bb_count_arr[current_bb_index].done_count / bb_count_arr[current_bb_index].bb_per_core;
                if (!is_shared)
                    number = number + (offset * core);
                number = number >> rshift;
                if (core < num_cores)
                    fprintf(outfile[core], "%#llx\n", number);
                if (core >= num_cores)
                    fprintf(outfile[num_cores - 1], "%#llx\n", number);
            }
        }
        else if (strstr(line, "BB START") != NULL)
        {
            ret = strchr(line, ':');
            ret += 2;
            strtok(ret, "\n");
            current_bb_index = get_bb_index(ret);
        }
        else if (strstr(line, "BB DONE") != NULL)
        {
            ret = strchr(line, ':');
            ret += 2;
            strtok(ret, "\n");
            bb_count_arr[current_bb_index].done_count += 1;
        }
    }
    printf("Split Done. Releasing resources\n");
    if (line)
        free(line);
    fclose(memTraceFile);
    for (core = 0; core < num_cores; core++)
        fclose(outfile[core]);
    return 0;
}
