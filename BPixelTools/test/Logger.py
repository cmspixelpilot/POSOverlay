import os
from datetime import date, datetime

class Logger:
   def __init__(self):
      self.log=[]
   def logMsg(self,msg,type=0):
      # 0=info, 1=error, 2=ok
      self.log.append((msg,type))
      self.printMsg(msg,type)
   def error(self,msg): self.logMsg(msg,1)
   def ok(self,msg):    self.logMsg(msg,2)
   def info(self,msg):  self.logMsg(msg,0)
   def printMsg(self,msg,type):
      color=type
      #if color<>0: os.system("tput setaf %d"%(color))
      if color == 1:
         print 'JMTERROR:',
      elif color == 2:
         print 'JMTOK:',
      elif color != 0:
         print 'JMT{%i}:' % color,
      print msg
      #if color<>0: os.system("tput setaf 0")
      
   def printLog(self,title='message summary',level=1):
      if len(self.log)==0:
         pass
      else:
         print 80*'-'
         print title
         print 80*'-'
         for msg,type in self.log:
            if type>=level:
               self.printMsg(msg,type)
         print 80*'-'

         
   def getFilename(self,filename):
      """ add date and version """
      base,ext=os.path.splitext(filename)
      f=base+"_"+datetime.today().isoformat()
      if os.path.exists(f+ext):
            template=f+"-%03d"
            for n in range(1,100):
               if not os.path.exists(template%n+ext):
                  return template%n+ext
            print "no valid filename found for ",filename
            return "temp"+ext
      else:
         return f+ext


   def saveLog(self,name='logger',level=0):
      if len(self.log)==0:
         pass
      else:
         filename=self.getFilename(name+".log")
         f=open(filename,'w')
         for msg,type in self.log:
            if type>=level:
               f.write(msg+"\n")
