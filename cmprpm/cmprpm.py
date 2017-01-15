#!/usr/bin/python
# compare content of rpm, see if they are the same
# sample usage: ./cmprpm.py "/home/onega/a1.rpm" "/home/onega/a1_RC1.rpm"
# [onega@localhost cmprpm]$ python --version
# Python 2.7.5

import shutil
import inspect, os, subprocess, re
import sys, hashlib

# rpm2cpio ./packagecloud-test-1.1-1.x86_64.rpm | cpio -idmv

def getlogfilename(rpmname): # generate log file name from rpm name
    logfilename = "";
    if "/" in rpmname:
        searchkey = "(.*)/([^/]*rpm)"
        regex = re.compile('{}'.format(searchkey))
        matchresult = regex.match( rpmname) # , re.M|re.I
        if matchresult:
            logfilename = matchresult.group(2)+".txt"
            print(logfilename)
    else:
        logfilename = rpmname+".txt";
    return logfilename

def extractrpm(rpmname, mycwd): # extract files in rpm to specified folder, return log file name contains output of cpio command
    logfilename = getlogfilename(rpmname);
    if os.path.exists(mycwd):
        shutil.rmtree(mycwd)
    os.makedirs(mycwd)
    if not os.path.exists(mycwd):
        print("failed to create "+mycwd)
        return ""
    logfile = open(logfilename, "w")
    ps = subprocess.Popen(('rpm2cpio', rpmname), cwd=mycwd, stdout=subprocess.PIPE)
    output = subprocess.Popen(('cpio', '-idmv'), cwd=mycwd, universal_newlines=True, stdin=ps.stdout, stdout=logfile, stderr=logfile)
    ps.wait()
    logfile.flush()
    return logfilename

def filedigest(filepath): # calculate SHA1 checksum of specified file
    BUF_SIZE = 65536  #
    sha1 = hashlib.sha1()
    with open(filepath, 'rb') as f:
        while True:
            data = f.read(BUF_SIZE)
            if not data:
                break
            sha1.update(data)
    return sha1.hexdigest()

def folderdigest(folderpath): # return a dict with checksum of all files in specified folder
    folderchecksum={}
    for root, dirnames, filenames in os.walk(folderpath):
        for filename in filenames:
            try:
                filelongpath = os.path.join(root, filename)
                fdigest  = filedigest(filelongpath) # exception when reading symbolic link
                folderchecksum[fdigest] = filelongpath
            except (IOError, EOFError) as err:
                print(err)
                print(err.args)
                pass
    return folderchecksum

def diffdir(rpm1digest, rpm2digest, pathprefix): # print file name in rpm1 if its checksum is not found in rpm2
    difflist= [k for k in rpm1digest if k not in rpm2digest]
    for k in difflist:
        print(rpm1digest[k][len(pathprefix):])
    print("")
    return len(difflist)

usagemsg = """Please specify 2 different rpm names
    WARNING: This tool ignore empty folders.
    WARNING: This tool likely ignore symbolic links.
    WARNING: This tool only check existence of file, does not care their physical location and count.
    WARNING: this tool does not complain occurrence of the same file mismatch. E.g. one rpm install fileA at two location, another rpm install fileA at three locations.
    WARNING: This tool does not care file names. only compare their content (SHA1 digest)
    WARNING: This tool is not heavily tested. It may work with Python 2.7.5. Use it at your own risk!!!
"""

def printusage():
    print(usagemsg)
    print('Usage: '+sys.argv[0]+' <rpm1> <rpm2>')

if __name__ == "__main__":
    if (len(sys.argv) < 3) or (getlogfilename(sys.argv[1])==getlogfilename(sys.argv[2])):
    	printusage()
    	sys.exit(1)
    folder1 = os.getcwd() + "/rpm1"
    folder2 = os.getcwd() + "/rpm2"
    log1 = extractrpm(sys.argv[1], folder1)
    log2 = extractrpm(sys.argv[2], folder2)
    print("first log file is "+log1+" 2nd log file is "+log2)
    rpm1digest = folderdigest(folder1)
    rpm2digest = folderdigest(folder2)
    print ("files in {} but not exist or different in {}".format(sys.argv[1], sys.argv[2]))
    diffcnt1 = diffdir(rpm1digest, rpm2digest, folder1)
    print ("files in {} but not exist or different in {}".format(sys.argv[2], sys.argv[1]))
    diffcnt2 = diffdir(rpm2digest, rpm1digest, folder2)
    print("There are {} different files in 2 rpms".format(diffcnt1 + diffcnt2))
