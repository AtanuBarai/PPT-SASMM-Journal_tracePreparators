# Author: Atanu Barai <atanu@nmsu.edu>
# Date: 13 September 2020
# Purpose: Replaces basic block random numbers with basic block names, reports any overlap of basic
# blocks
# Copyright: PEARL Laboratory, ECE, NMSU. Part of the PPT open source project.
# Python3 Version, used pandas to work with large trace files

import sys
import math

CACHE_LINE_SIZE = 64

rshift = int(math.log2(CACHE_LINE_SIZE))


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
    file_p = open(trace_file, 'r')
    o_t_file = open('processed_trace.dat', 'w')
    while True:
        count += 1
        line = file_p.readline()
        if not line:
            break
        else:
            if '0x' in line:
                addr = int(line, 0)
                addr = addr >> rshift
                o_t_file.write(line)
    file_p.close()
    o_t_file.close()


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
