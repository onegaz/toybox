#!/usr/bin/env python3

from __future__ import print_function
import subprocess 
import sys
import os
import argparse
import time
import datetime
import re

def createIntentActivityFlagsMap():
    activityflags={}    
#     activityflags[0x00000001] = "FLAG_GRANT_READ_URI_PERMISSION"
#     activityflags[0x00000002] = "FLAG_GRANT_WRITE_URI_PERMISSION"
#     activityflags[0x00000004] = "FLAG_FROM_BACKGROUND"
#     activityflags[0x00000008] = "FLAG_DEBUG_LOG_RESOLUTION"
#     activityflags[0x00000010] = "FLAG_EXCLUDE_STOPPED_PACKAGES"
#     activityflags[0x00000020] = "FLAG_INCLUDE_STOPPED_PACKAGES"
#     activityflags[0x00000040] = "FLAG_GRANT_PERSISTABLE_URI_PERMISSION"
#     activityflags[0x00000080] = "FLAG_GRANT_PREFIX_URI_PERMISSION"
#     activityflags[0x00000100] = "FLAG_DIRECT_BOOT_AUTO"
#     activityflags[0x00000200] = "FLAG_IGNORE_EPHEMERAL"
#     activityflags[0x00000004] = "FLAG_FROM_BACKGROUND"
    activityflags[0x40000000] = "FLAG_ACTIVITY_NO_HISTORY"
    activityflags[0x20000000] = "FLAG_ACTIVITY_SINGLE_TOP"
    activityflags[0x10000000] = "FLAG_ACTIVITY_NEW_TASK"
    activityflags[0x08000000] = "FLAG_ACTIVITY_MULTIPLE_TASK"
    activityflags[0x04000000] = "FLAG_ACTIVITY_CLEAR_TOP"
    activityflags[0x02000000] = "FLAG_ACTIVITY_FORWARD_RESULT"
    activityflags[0x01000000] = "FLAG_ACTIVITY_PREVIOUS_IS_TOP"
    activityflags[0x00800000] = "FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS"
    activityflags[0x00400000] = "FLAG_ACTIVITY_BROUGHT_TO_FRONT"
    activityflags[0x00200000] = "FLAG_ACTIVITY_RESET_TASK_IF_NEEDED"
    activityflags[0x00100000] = "FLAG_ACTIVITY_LAUNCHED_FROM_HISTORY"
    activityflags[0x00080000] = "FLAG_ACTIVITY_CLEAR_WHEN_TASK_RESET"
    activityflags[0x00040000] = "FLAG_ACTIVITY_NO_USER_ACTION"
    activityflags[0X00020000] = "FLAG_ACTIVITY_REORDER_TO_FRONT"
    activityflags[0X00010000] = "FLAG_ACTIVITY_NO_ANIMATION"
    activityflags[0x00008000] = "FLAG_ACTIVITY_CLEAR_TASK"
    activityflags[0X00004000] = "FLAG_ACTIVITY_TASK_ON_HOME"
    activityflags[0x00002000] = "FLAG_ACTIVITY_RETAIN_IN_RECENTS"
    activityflags[0x00001000] = "FLAG_ACTIVITY_LAUNCH_ADJACENT"
    activityflags[0x00000800] = "FLAG_ACTIVITY_MATCH_EXTERNAL"   
    return activityflags
 
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Accepted arguments')
    parser.add_argument('-f', '--flags', help='input android intent flags')
    parser.add_argument('--hex', help='parse as hex string', required=False) 
    args = vars(parser.parse_args())
    
    if args['flags'] is None:
        print("Please specify an android intent flags")
        parser.print_help(sys.stderr)
        sys.exit(1)
    flagstring = args['flags']
    flags = 0
    if args['hex'] is not None or flagstring.startswith("0x") or flagstring.startswith("0X"):
        flags = int(flagstring, 16)
    else:
        flags = int(flagstring, 10)

    activityflags=createIntentActivityFlagsMap()    
    flagnames = ""
    for flag in activityflags.keys():
        if (flag & flags)>0:
            if len(flagnames)>1:
                flagnames = flagnames + " | "
            flagnames = flagnames + activityflags[flag]
            flags = flags - flag
    if len(flagnames)>0:
        print(flagstring + " = " + flagnames)
    if flags != 0:
        print("unknown bits left: " + hex(flags))
