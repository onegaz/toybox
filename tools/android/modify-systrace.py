#!/usr/bin/env python3

from __future__ import print_function
import subprocess 
import sys
import os
import argparse
import time
import datetime
import re
import mmap

'''
modify tr-ui-find-control so that the search box is longer.
2377:
before: width: 170px;
after: width: 370px;

2393: 
before: width: 167px;
after: width: 367px;
'''
def modify(filepath):
    with open(filepath, "a+b") as f:
        mm = mmap.mmap(f.fileno(), 0, prot=mmap.ACCESS_WRITE)
        pos1 = mm.find(b'width: 170px;')
        if pos1 < 1:
            print("Unsupported file, can't find width: 170px; in " + filepath)
            sys.exit()
        pos2 = mm.find(b'width: 167px;', pos1)
        if pos1 < 1:
            print("Unsupported file, can't find width: 167px; in " + filepath)
            sys.exit()
        print("pos1={} pos2={}".format(pos1, pos2))
        # pos1=97642 pos2=97979
        if pos1 != 97642 or pos2 != 97979:
            print("Unsupported file "+ filepath)
            sys.exit()
        mm.seek(pos1 + 7, os.SEEK_SET)
        mm.write_byte(ord('3'[0]))
        mm.seek(pos2 + 7, os.SEEK_SET)
        mm.write_byte(ord('3'[0]))        
        #mm[pos1+7]='3'
        #mm[pos2+7]='3'
        mm.flush()
        mm.close()
        
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Accepted arguments')
    parser.add_argument('-f', '--file', help='input systrace file')

    args = vars(parser.parse_args())
    
    if args['file'] is None or not os.path.exists(args['file']):
        print("Please specify an existing systrace file")
        parser.print_help(sys.stderr)
        sys.exit(1)
    now = time.strftime("%Y%m%d-%H%M%S ")
    print(now + args['file'])
    modify(args['file'])
