# Author: Atanu Barai <atanu@nmsu.edu>
# Date: 13 September 2020
# Purpose: reports any overlap of basic blocks
# Copyright: PEARL Laboratory, ECE, NMSU. Part of the PPT open source project.
# Python3 Version, used pandas to work with large trace files

import sys
import math
import random
import time


def main(bb_file, trace_file):
    '''
    The main function
    '''
    bb_table = open(bb_file, 'r').read().strip().split('\n')
    shared_bbs = []
    for b_t in bb_table:
        b_t_info = b_t.split(': ')
        print(b_t_info[0], b_t_info[1], b_t_info[2])
        if 'shared_trace' in b_t_info[2]:
            shared_bbs.append(b_t_info[1])
    count = 0
    found = 1
    file_p = open(trace_file, 'r')
    start = time.time()
    while True:
        count += 1
        line = file_p.readline()
        if not line:
            break
        if 'BB START' in line:
            if found == 1:
                found = 0
            else:
                print "\nBB START before BB END in line ", count, " ", line
                exit()
        elif 'BB DONE' in line:
            if found == 0:
                found = 1
            else:
                print "\nBB END before BB START in line ", count, " ", line
                exit()
    end = time.time()
    print "\nDuplicate check computation time ", end - start
    file_p.close()


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
