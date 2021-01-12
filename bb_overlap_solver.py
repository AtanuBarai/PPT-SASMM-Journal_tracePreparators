# Author    : Atanu Barai <atanu@nmsu.edu>
# Date      : Jan 11 2020
# Purpose   : Just a helper script to solve the bb overlap issue instead of doing it by hand
# Copyright : PEARL Laboratory, ECE, NMSU. Part of the PPT open source project.


import sys
import time


def main(trace_file):
    '''
    The main function
    '''
    file_p = open(trace_file, 'r')
    out_file = open("mem_trace_fixed", 'w')
    start = time.time()
    for line in file_p:
        if 'BB START: 656f82a27fa2259a' in line:
        elif 'BB DONE: 40538440caf4edfe' in line:
            out_file.write(line)
            out_file.write('BB START: 656f82a27fa2259a\n')
        else:
            out_file.write(line)

    end = time.time()
    print "Overlap Fixed"
    print "OComputation time ", end - start
    file_p.close()
    out_file.close()


if __name__ == "__main__":
    main(sys.argv[1])
