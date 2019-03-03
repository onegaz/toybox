#!/usr/bin/env python

from __future__ import print_function
import subprocess 
import sys
import os
import argparse
import time
import datetime

columnkey = 0

def getColumnCount(line):
    line = line.rstrip()
    commacount = line.count(",")
    if not line.endswith(","):
        commacount += 1
    return commacount

def maybeAppendComma(line, commacnt):
    count = line.count(",")
    if count < commacnt:
        return line + ","*(commacnt-count)
#     if line.endswith(","):
#         return line
    return line + ","

def csvToMap(csvfilepath, columnkey):
    cvsMap = {}
    with open(csvfilepath) as fp:
        lineno = 1
        for line in fp:
            line = line.rstrip()
            columns=line.split(',', columnkey+2)
            cvsMap[columns[columnkey]] = line
            #print("line {} contents {} LEFT {}".format(lineno, columns[columnkey], columns[columnkey+1]))
            lineno += 1
    return cvsMap;

def csvKeys(csvfilepath, columnkey):
    cvsKeyList = []
    with open(csvfilepath) as fp:
        lineno = 1
        for line in fp:
            line = line.rstrip()
            columns=line.split(',', columnkey+2)
            cvsKeyList.append(columns[columnkey])
            #print("line {} contents {} LEFT {}".format(lineno, columns[columnkey], columns[columnkey+1]))
            lineno += 1
    return cvsKeyList;
                
def mergecsv(basefile, extrafile, outfile):
    extralines = csvToMap(extrafile, columnkey)
    basecsvmap = csvToMap(basefile, columnkey)
    lines_only_in_base = 0
    lines_not_in_base = 0
    lines_in_both = 0
    mergeoutfile = open(outfile, "w")
    columnCount = 0
    with open(basefile) as fp:
        lineno = 1
        for line in fp:
            columns=line.split(',', columnkey+2)
#             if columnCount < 1:
            columnCount = max(columnCount, getColumnCount(line))
            
            if columns[columnkey] in extralines:
                lines_in_both += 1
                mergeoutfile.write(maybeAppendComma(line.rstrip(), columnCount) + extralines[columns[columnkey]] + os.linesep)
            else:
                lines_only_in_base += 1
                mergeoutfile.write(line)
                print("No match for line {} {}".format(lineno, line), end='')
                #mergeoutfile.write(line + "," * columnCount)
            lineno += 1
            
    secondCvsKeyList = csvKeys(extrafile, columnkey)
    for key in secondCvsKeyList:
        if key in basecsvmap:
            continue
        lines_not_in_base += 1
        mergeoutfile.write(","*columnCount + extralines[key] + os.linesep)
               
#     for key, line in extralines.iteritems():
#         if key in basecsvmap:
#             continue
#         lines_not_in_base += 1
#         mergeoutfile.write(","*columnCount + line + os.linesep)
    print("merged {} lines, {} lines only in base, {} lines not in base".format(lines_in_both, lines_only_in_base, lines_not_in_base))
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Accepted arguments')
    parser.add_argument('--base', help='path of base csv file', required=True)
    parser.add_argument('--extra', help='path of extra csv file', required=True)
    parser.add_argument('--out', help='path of output csv file', required=True)
    parser.add_argument('--column', help='0 based column number to merge', required=True)
    args = vars(parser.parse_args())
    
    if args['base'] is None or not os.path.exists(args['base']):
        print("Please specify an existing csv file as base")
        parser.print_help(sys.stderr)
        sys.exit(1)
    if args['extra'] is None or not os.path.exists(args['extra']):
        print("Please specify an existing csv file to merge with base")
        parser.print_help(sys.stderr)
        sys.exit(1)
    if args['out'] is None:
        parser.print_help(sys.stderr)
        sys.exit(1)    
        
    columnkey = int(args['column'])
    
    start_time = datetime.datetime.now()
    
    try:
        mergecsv(args['base'], args['extra'], args['out'])
    except OSError as ose:
         print ("OS error({0}): {1}".format(ose.errno, ose.strerror))
   
    end_time = datetime.datetime.now()
    duration = end_time - start_time
    print("It took {} seconds, check {}".format(duration.total_seconds(), args['out']))
    sys.exit(0)