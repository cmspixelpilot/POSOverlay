#!/usr/bin/env python

# setup POS based on packages.txt

import os
import subprocess
import thread
import sys
import getpass

# basic settings
svnBase = 'https://svn.cern.ch/reps'
thisDir = os.getcwd()
packagesFileName = thisDir + "/packages.txt"


# SVN authentication
if "CERNUSER" in os.environ:
  userName = os.environ['CERNUSER']
else:
  userName = os.environ['USER']
if (userName == "pixelpro") or (userName == "pixeldev"):
  svnInputName = raw_input( "Please enter your CERN username [default: %s]: " %userName)
  if svnInputName != '':
    userName = svnInputName
print "Using %s as username for SVN checkouts from CERN. Do \'export CERNUSER=YourUserNameAtCERN\' to change it." %userName
userPass = getpass.getpass('Please enter CERN password for user %s: ' %userName) 

# link Makefile and RPM.version
os.chdir( "../" )
linkList = ["Makefile", "RPM.version", "VERSION"]
for linkFile in linkList:
  if not (os.path.exists("%s"%linkFile)) and not (os.path.islink("%s"%linkFile)):
    print os.getcwd()
    os.symlink( thisDir+"/%s"%linkFile, "%s"%linkFile )
  else:
    print "%s already exists in %s - omitting symbolic link creation" %(linkFile, os.getcwd())

logDir = "%s/log" %thisDir
if not os.path.exists(logDir):
  os.makedirs(logDir)

# check out all packages in packages.txt
packageFile = open( packagesFileName, 'r' )
for line in packageFile:
  line = line.strip("\n")
  if line.startswith("#") or line == '': # skip commented and empty lines
    continue
  packageName = line.rsplit("/", 1)[1]
  # find out whether we're checking out from plain/branches/tags/trunk
  versionType = "plain"
  if line.find("/trunk") >= 0:
    packageName = (line.rsplit("/trunk", 1)[0]).split("/", 1)[1]
    versionType = "trunk"
  elif line.find("/tags") >= 0:
    packageName = (line.rsplit("/tags", 1)[0]).split("/", 1)[1]
    versionType = "tag"
  elif line.find("/branches") >= 0:
    packageName = (line.rsplit("/branches", 1)[0]).split("/", 1)[1]
    versionType = "branch"
  print "Working on package %s: checking out %s version" %(packageName, versionType)
  svnCommand = "svn co --username %s --password %s %s/%s %s" %(userName, userPass, svnBase, line.strip("\n"), packageName)
  errLogName = logDir + "/" + packageName + ".err"
  outLog = open( logDir + "/" + packageName + ".log", "w")
  errLog = open( errLogName, "w")
  # perform the svn checkout
  lock=thread.allocate_lock()
  lock.acquire()
  p = subprocess.Popen( svnCommand, stdout=outLog, stderr=errLog, shell=True)
  p.wait()
  lock.release()
  outLog.close()
  errLog.close()

  if (os.stat(errLogName)[6]!=0):
    print "There has been an error - stopping execution"
    errLog = open(errLogName, "r")
    for line in errLog:
      print line
    errLog.close()
    break
  else:
    print "%s checked out successfully" % packageName
  print "-----------------------------------------------"

packageFile.close()

print "Now try \"cd ../ && make\""
