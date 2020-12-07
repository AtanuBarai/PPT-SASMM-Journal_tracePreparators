# Author: Atanu Barai <atanu@nmsu.edu>
# Date: 13 September 2020
# Last Edit : Dec 7 2020
# Purpose: Replaces basic block random numbers with basic block names, reports any overlap of basic
# blocks
# Copyright: PEARL Laboratory, ECE, NMSU. Part of the PPT open source project.
# Python3 Version, used pandas to work with large trace files

import sys
import math

CACHE_LINE_SIZE = 64

rshift = int(math.log2(CACHE_LINE_SIZE))


def main(trace_file):
    '''
    The main function
    '''
    count = 0
    file_p = open(trace_file, 'r')
    o_t_file = open('processed_trace.dat', 'w')
    while True:
        line = file_p.readline()
        if not line:
            break
        else:
            if line[:2] == '0x':
                count += 1
                addr = int(line, 16)
                addr = addr >> rshift
                o_t_file.write(line)
    file_p.close()
    o_t_file.close()
    with open("stat.txt", 'w') as o_t_file:
        o_t_file.write(
            "Length of Processed CLine Trace(excluding BB labels): " + str(count))


if __name__ == "__main__":
    main(sys.argv[1])
