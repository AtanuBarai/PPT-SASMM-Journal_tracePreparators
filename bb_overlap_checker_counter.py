# Author    : Atanu Barai <atanu@nmsu.edu>
# Date      : Dec 4 2020
# Purpose   : reports any overlap of basic blocks in the memory trace,
#             provides basic block counts, generates a memory trace with shared
#             addresses.
# Copyright : PEARL Laboratory, ECE, NMSU. Part of the PPT open source project.


import sys
import time


def main(bb_file, trace_file):
    '''
    The main function
    '''
    bb_table = open(bb_file, 'r').read().strip().split('\n')
    bb_counts = {}
    shared_bbs = []
    shared_addresses = []
    for b_t in bb_table:
        b_t_info = b_t.split(': ')
        print(b_t_info[0], b_t_info[1], b_t_info[2])
        bb_counts[b_t_info[1]] = 0
        if 'shared_trace' in b_t_info[2]:
            shared_bbs.append(b_t_info[1].strip())
    count = 0
    found = 1
    shared_bb_start = 0
    min_address = sys.maxint
    max_address = 0
    file_p = open(trace_file, 'r')
    start = time.time()
    for line in file_p:
        count += 1
        if 'BB START' in line:
            bb_id = line.split(': ')[1].strip()
            if bb_id in shared_bbs:
                print "Found Shared Address"
                shared_bb_start = 1
            bb_counts[bb_id] += 1
            # Code for checking BB overlap
            if found == 1:
                found = 0
            else:
                print "\nBB START before BB END in line ", count, " ", line
                exit()
        elif 'BB DONE' in line:
            shared_bb_start = 0
            # Code for checking BB overlap
            if found == 0:
                found = 1
            else:
                print "\nBB END before BB START in line ", count, " ", line
                exit()
        elif line[:2] == '0x':
            if shared_bb_start == 1:
                shared_addresses.append(line)
            line = line.strip()
            addr = int(line, 16)
            if addr > max_address:
                max_address = addr
            elif addr < min_address:
                min_address = addr

    end = time.time()
    print "No overlap found"
    print "Overlap check computation time ", end - start
    file_p.close()

    with open('bb_counts.dat', 'w') as w_t_file:
        for key, item in bb_counts.items():
            w_t_file.write(key + ", " + str(item) + "\n")
    shared_addresses = list(set(shared_addresses))
    with open('shared_addresses.dat', 'w') as w_t_file:
        for address in shared_addresses:
            w_t_file.write(address)
    with open('min_max_addr.dat', 'w') as w_t_file:
        w_t_file.write("Min value of address: " + str(hex(min_address)))
        w_t_file.write("\nMax value of address: " + str(hex(max_address)))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
