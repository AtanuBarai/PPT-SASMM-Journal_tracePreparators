// Author: Atanu Barai <atanu@nmsu.edu>
// Purpose: Converts a memory address trace to cache line trace, also outputs shared addresses

// #define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#define CACHE_LINE_SIZE 64 //Cache block/line size in bytes 

int main(int argc, char *argv[])
{
    int rshift = log2(CACHE_LINE_SIZE);

    char * line = NULL;
    char * cptr = NULL;
    size_t len = 0;
    ssize_t read;
    unsigned long long trace_len = 0;
    long int number, maxVal = LONG_MIN, minVal = LONG_MAX;
    FILE *infile;
    FILE *outfile;
    FILE *statfile;
    FILE *sharedaddrfile;

    const char shared_marker[] = "shared_trace";
    char shared_bbs[10][20];
    unsigned n_shared_bbs = 0, i;
    int start_logging_shared_address = 0;
    

    if(argc != 3)
    {
        printf("Input file missing in command line argument\n");
        printf("./addr2cacheline bb_table mem_tracer\n");
    }
    else
    {
	printf("%s %s\n", argv[1], argv[2]);
    }
    infile = fopen(argv[1],"r");
    if(infile == NULL)
    {
        printf("Error in opening BB table file!\n");
    }
    else
    {
        while ((read = getline(&line, &len, infile)) != -1)
        {
            if (strstr(line, shared_marker) != NULL) 
            {
                //Shared marker found
                cptr = strtok(line, " ");
                cptr = strtok(NULL, ":");
                sprintf(shared_bbs[n_shared_bbs], cptr);
                ++n_shared_bbs;
            }
        }
        for(i = 0; i<n_shared_bbs;i++)
            printf("Shared Basic Block %d: %s\n", i+1, shared_bbs[i]);
        //exit(1);
    	fclose(infile);
    }
    infile = fopen(argv[2],"r");
    outfile = fopen("processed_cline_trace.dat", "w");
    sharedaddrfile = fopen("shared_addresses.txt", "w");
    if (infile == NULL || outfile == NULL || sharedaddrfile == NULL)
    {
        printf("Error in opening trace files!\n");
        // Program exits if the file pointer returns NULL.
        exit(1);
    }   
    else 
    {
	printf("Traversing the memory trace\n\n\n");
        while ((read = getline(&line, &len, infile)) != -1) 
        //while (trace_len < 30) 
        {    
            //printf("%zu  ", len);
                //printf("%zu  ", read);

       	    //read = getline(&line, &len, infile); 
            number = strtoul(line, NULL, 0);
    
            //printf("%s", line);
            if(number != 0 && line[0] == '0' && line[1] == 'x')
            {
                ++trace_len;
                number = number >> rshift;
                if(number > maxVal) maxVal = number;
                if(number < minVal) minVal = number;
                //printf("%#lx  ", number);
                //fseek(infile, -(read+1), SEEK_CUR);
                fprintf(outfile, "%#lx\n", number);
                if(start_logging_shared_address == 1)
                    fprintf(sharedaddrfile, "%#lx\n", number);
            }
            else
            {
                //printf("%s", line);
                for(i = 0; i < n_shared_bbs; i++)
                {
		    //printf("Shared BB: %s\n",shared_bbs[i]);
                    if(strstr(line, shared_bbs[i]) != NULL)
                    {
			//printf("Found Shared BB %s\n", shared_bbs[i]);
                        if(start_logging_shared_address == 0)
                        {
			    start_logging_shared_address = 1;
			    printf("Start Shared BB %s\n", shared_bbs[i]);
			}
                        else
			{
                            start_logging_shared_address = 0;
			    printf("End Shared BB %s\n", shared_bbs[i]);
			}
                    }
                }
                // fprintf(outfile, "%s", line);
            }
        }
        if (line)
            free(line);
    }
    fclose(infile);
    fclose(outfile);
    fclose(sharedaddrfile);
    statfile = fopen("stat.txt", "w");
    fprintf(statfile, "Length of Processed Cache Line Trace(excluding basic block labels): %llu\n", trace_len);
    fprintf(statfile, "MinVal of CLine Address = %#lx\nMaxVal of CLine Address = %#lx", minVal, maxVal);
    fclose(statfile);
    return 0;
}

