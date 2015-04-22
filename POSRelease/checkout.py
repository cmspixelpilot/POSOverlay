#!/usr/bin/env python

# setup POS based on packages.txt

import os
import io
import time
import subprocess
import thread
import sys
import getpass
import optparse
import shutil


def getPassword(userName):
  
  if (userName == "pixelpro") or (userName == "pixeldev"):
    print "%s is not a user that can check out from SVN." %userName
    svnInputName = raw_input( "Please enter your CERN username: ")
    if svnInputName != '':
      userName = svnInputName
    else:
      print "No username given. Exiting."
      exit(1)
  print "Using %s as username for SVN checkouts from CERN. Use \'--userName YourUserNameAtCERN\' option to change it." %userName
  userPass = getpass.getpass('Please enter CERN password for user %s: ' %userName)
  return userPass


def createSoftlinks(linkList, thisDir, force):
  
  os.chdir( "../" )
  for linkFile in linkList:
    if (force):
      os.remove("%s"%linkFile)
    if not (os.path.exists("%s"%linkFile)) and not (os.path.islink("%s"%linkFile)):
      print "Creating softlink to %s/%s in %s" %(thisDir, linkFile, os.getcwd())
      os.symlink( thisDir+"/%s"%linkFile, "%s"%linkFile )
    else:
      print "%s already exists in %s - omitting symbolic link creation" %(linkFile, os.getcwd())
  os.chdir( thisDir )


def getPackageDict(packagesFileName):
  packageDict = {}
  if (not packagesFileName.startswith("/")):
    script_dir = os.path.dirname(os.path.abspath(__file__)) #<-- absolute dir the script is in
    packagesFileName = os.path.join(script_dir, packagesFileName)
    print packagesFileName
    print script_dir
  packageFile = open( packagesFileName, 'r' )
  for line in packageFile:
    line = line.strip("\n")
    if line.startswith("#") or line == '': # skip commented and empty lines
      continue
    package = Package(line)
    packageDict[package.name] = package
  packageFile.close()
  return packageDict


def executeSvn(svnCommand, package, logDir):
  errLogName = logDir + "/" + package.plainName + ".err"
  outLogName = logDir + "/" + package.plainName + ".log"
  errLog = open( errLogName, "w")
  outLog = open( outLogName, "w")
  # perform the svn checkout
  lock=thread.allocate_lock()
  lock.acquire()
  with io.open(outLogName, 'wb') as logWriter:
    with io.open(outLogName, 'rb', 1) as logReader:
      with io.open(errLogName, 'wb') as errWriter:
        with io.open(errLogName, 'rb', 1) as errReader:
          process = subprocess.Popen( svnCommand, stdout=logWriter, stderr=errWriter, shell=True)
          while process.poll() is None:
            sys.stdout.write(logReader.read())
            sys.stdout.write(errReader.read())
            time.sleep(0.5)
            # Read the remaining
            sys.stdout.write(logReader.read())
            sys.stdout.write(errReader.read())
  # p = subprocess.Popen( svnCommand, stdout=outLog, stderr=errLog, shell=True)
  # p.wait()
  lock.release()
  outLog.close()
  errLog.close()

  if (os.stat(errLogName)[6]!=0):
    print "There has been an error - stopping execution"
    errLog = open(errLogName, "r")
    for line in errLog:
      print line
    errLog.close()
    exit(1)
  if (os.stat(outLogName)[6]!=0):
    # there has been output to stdout
    return True
  else:
    return False


class Package:
 
  def __init__(self, line):
    self.line = line.strip("\n")
    self.repository = line.split("/")[0]
    self.name = line.split(self.repository+"/")[1]
    self.versionType = "plain"
    if line.find("/trunk") >= 0:
      self.name = (line.rsplit("/trunk", 1)[0]).split("/", 1)[1]
      self.versionType = "trunk"
    elif line.find("/tags") >= 0:
      self.name = (line.rsplit("/tags", 1)[0]).split("/", 1)[1]
      self.versionType = "tag"
    elif line.find("/branches") >= 0:
      self.name = (line.rsplit("/branches", 1)[0]).split("/", 1)[1]
      self.versionType = "branch"
    self.plainName = self.name.replace("/","-")


def main():

  # basic settings
  svnBase = 'https://svn.cern.ch/reps'
  thisDir = os.getcwd()
  packagesFileName = thisDir + "/packages.txt"
  
  # option parsing
  parser=optparse.OptionParser(usage="python %prog [options]")
  parser.add_option("--update", dest="update",
                    action="store_true", default=False,
                    help="update all packages")
  parser.add_option("--replace", dest="replace",
                    action="store_true", default=False,
                    help="replace all packages")
  parser.add_option("--switch", dest="switch",
                    action="store_true", default=False,
                    help="switch all packages")
  parser.add_option("--force", dest="force",
                    action="store_true", default=False,
                    help="delete and replace without questions")
  parser.add_option("--noauth", dest="noauth",
                    action="store_true", default=False,
                    help="replace all packages deleting old ones")                  
  parser.add_option("-p", "--packageFile", action="store",
                    dest="packagesFileName", default=packagesFileName,
                    help="choose packages.txt file")
  parser.add_option("-u", "--userName", action="store",
                    dest="userName", default=os.environ["USER"],
                    help="username used for SVN operations")
  
  (options, args)=parser.parse_args()
  print "-"*100
  print "Running with the following options:"
  for opt, value in options.__dict__.items():
    print "--> %20s: %s" %(opt, value)
  print "-"*100
  update = options.update
  replace = options.replace
  switch = options.switch
  checkout = True
  if (update or replace or switch):
    checkout = False
  force = options.force
  noauth = options.noauth
  packagesFileName = options.packagesFileName
  userName = options.userName
  option_types = (update, replace, switch, checkout)
  true_count =  sum([1 for ot in option_types if ot])
  if true_count > 1:
      parser.error("options --update, --replace, --switch and --checkout are mutually exclusive")

  # SVN authentication
  authString = ""
  if (not noauth):
    userPass = getPassword(userName)
    authString = "--username %s" %userName
    if (userPass != ""):
      authString += " --password %s" %userPass
    else:
      print "No password provided, will try to work without password."

  # link Makefile and RPM.version etc.
  linkList = ["Makefile", "RPM.version", "VERSION"]
  createSoftlinks(linkList, thisDir, force)

  # create log dir
  logDir = "%s/log" %thisDir
  if not os.path.exists(logDir):
    os.makedirs(logDir)

  # read in packages from packagesFile
  packageDict = getPackageDict(packagesFileName)
   # change one directory up for svn actions
  os.chdir( "../" )
  
  if (checkout):
    print "Checking out packages to %s" %os.getcwd()
    for name,package in packageDict.items():
      print "Working on package %s (in %s): checking out %s version" %(name, package.repository, package.versionType)
      if (force):
        packageDir = os.getcwd()+"/"+name
        if (os.path.exists(packageDir)):
          print "Deleting %s before checkout" %packageDir
          shutil.rmtree(packageDir, ignore_errors=True)
      svnCommand = "svn co %s %s/%s %s" %(authString, svnBase, package.line, name)
      executeSvn(svnCommand, package, logDir)
      print "%s checked out successfully" % name
      print "-----------------------------------------------"
    print "Check-out done."
  
  elif (switch):
    print "Switching packages in %s" %os.getcwd()
    for name,package in packageDict.items():
      print "Working on package %s (in %s): checking out %s version" %(name, package.repository, package.versionType)
      svnCommand = "svn switch %s %s/%s %s" %(authString, svnBase, package.line, name)
      executeSvn(svnCommand, package, logDir)
      print "%s switched successfully" % name
      print "-----------------------------------------------"
    print "Switching done."
  
  elif (replace):
    print "Replacing packages in %s" %os.getcwd()
    # first check if there are pending changes
    for name,package in packageDict.items():
      packageDir = os.getcwd()+"/"+name
      if (os.path.exists(packageDir)):
        print "Checking package %s for pending changes" %(name)
        svnCommand = "svn diff %s" %(name)
        hasChange = executeSvn(svnCommand, package, logDir)
        if (hasChange):
          print "There are pending changes in %s - will not continue unless force option provided." %name
          if (not force):
            exit(1)
    # then delete all directories apart from POSRelease and CMSSW CalibFormats
    for directory in os.listdir(os.getcwd()):
      if os.path.isdir(directory) and (directory.find("POSRelease") < 0) and (directory.find("CalibFormats") < 0):
        print "Deleting directory %s" %directory
        shutil.rmtree(directory, ignore_errors=True)
    # and check-out new packages
    for name,package in packageDict.items():
      print "Working on package %s (in %s): checking out %s version" %(name, package.repository, package.versionType)
      svnCommand = "svn co %s %s/%s %s" %(authString, svnBase, package.line, name)
      executeSvn(svnCommand, package, logDir)
      print "%s checked-out successfully" % name
      print "-----------------------------------------------"
    print "Replacing done."
    
  elif (update):
    print "Updating packages in %s" %os.getcwd()
    for name,package in packageDict.items():
      print "Working on package %s" %(name)
      svnCommand = "svn up %s %s" %(authString, name)
      hasChange = executeSvn(svnCommand, package, logDir)
      if hasChange:
        print "%s updated successfully" % name
      print "-----------------------------------------------"
    print "Updating done."



if __name__ == "__main__":
  main()
