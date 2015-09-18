#!/usr/bin/env python

#author: Benedikt Vormwald
#email:  benedikt.vormwald@cern.ch

import os, time, sys
import xml.etree.ElementTree as ET
from datetime import datetime
from StringIO import StringIO
from subprocess import Popen, PIPE

remote = ('tcds::ici::ICIController','tcds::pi::PIController','tcds::deadwood::DeadWood','psx','RCMSStateListener','jobcontrol')

xdaqconfigfile='Configuration_PixeliCISupervisor_srv-c2f38-15-01.xml'
#xdaqconfigfile='Configuration_PixeliCISupervisor_srv-c2f38-16-01.xml'
#xdaqconfigfile='XDAQ_ConfigurationFPix_TCDS.local.xml'

if(len(sys.argv)>1):
  xdaqconfigfile=str(sys.argv[1])
else:
  print('Please give XDAQ configuration file as argument.')
  quit()

print('')
print('#####################################################')
print('Parsing XDAQ config file: '+xdaqconfigfile)

commandargslist=list()
xmlstring=open(xdaqconfigfile,"r").read()

it = ET.iterparse(StringIO(xmlstring))
for _, el in it:
  if '}' in el.tag:
    el.tag = el.tag.split('}', 1)[1]  # strip all namespaces
root = it.root
#tree = ET.parse(xdaqconfigfile)
#root = tree.getroot()

systemhostname=os.getenv('HOSTNAME','localhost')
xdaqroot=os.getenv('XDAQ_ROOT','XDAQ/PATH/NOT/SET')
command=xdaqroot+'/bin/xdaq.sh' 

if(xdaqroot=='XDAQ/PATH/NOT/SET'):
  print('POS not correctly initialized.')
  quit()

timestring=datetime.now().strftime("%Y%m%d-%H%M%S")

for child in root:
  if ('Context' in child.tag):
    spliturl=child.attrib["url"].strip("http://")
    spliturl=spliturl.split(':')
    host=spliturl[0].replace('.cms','')
    port=spliturl[1]
    for child2 in child:
      if ('Application' in child2.tag):
        classname=child2.attrib["class"].split("::")[-1]
        print('-----------------------------------------------------')
        print('found XDAQ Application: '+classname)
        if (not(child2.attrib["class"] in remote)):
          logfile=timestring+'_'+classname+'_'+port+'.log'
          commandargs='-z pixel -p '+port+' -c '+xdaqconfigfile
          #commandargs='-e /nfshome0/pixelpilot/build/TriDAS/pixel/XDAQConfiguration/Profile.xml -z pixel -p '+port+' -c '+xdaqconfigfile
          if (systemhostname!=host):
            print(' - skipped, configured for a different machine ('+host+')')
            print('    --> make sure it is started there!')
          else:
            #print(command+' '+commandargs)
            print(' + will be started: http://'+host+'.cms:'+port)
            commandargslist.append((classname,commandargs,logfile))
        else:
          print(' o remote application')


print('#####################################################')
print('')

if (len(commandargslist)==0):
  print('No processes to start. Quit.')
  quit()
else:
  print('Starting processes...')

running_procs=list()
for classname,commandargs,logfile in commandargslist:
  log=open(logfile,'w')
  running_procs.append((classname,Popen([command, commandargs], stdout=log, stderr=log)))
  print('Started: '+classname+' (logfile: '+logfile+')')

print('')

while running_procs:
  failed=0
  for e in running_procs:
    classname=e[0]
    proc=e[1]
    retcode = proc.poll()
    if retcode is not None: # Process finished.
      running_procs.remove(e)
      print('!!!'+classname+' terminated unexpectedly. Check logfile for details.!!!')
      failed=1
    else: # No process is done, wait a bit and check again.
      time.sleep(.1)
      continue
  if(failed==1):
    break
    
print('')
print('')
