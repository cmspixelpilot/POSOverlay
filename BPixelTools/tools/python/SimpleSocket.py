from __future__ import generators
import socket,sys,threading,thread,time,re

class SimpleSocket:

    headerSize=5
    packetFormat="%1s%4i%s"
    maxPacketLength=4096;
    DEBUG=True

    def __init__(self,host=None,port=None,timeout=0):
        self.host=host
        self.port=port
        self.prompt=">"
        self.isOpenFlag=False
        self.log=""

        if not host==None and not port==None:
            # initialize as a client
            if not self.connect(timeout):
                sys.exit(1)
            
        elif host==None and not port==None:
            # implement a server on the given port
            self.serverSocket = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
            self.serverSocket.bind(('',port))
            self.serverSocket.listen(5)
        else:
            print "sorry, don't know what to do with just a host namne"
            sys.exit(1)


    def debug(self,flag):
        self.DEBUG=flag

        
    def connect(self,timeout=5):
        # initialize as a client
        ntry=0
        self.socket = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
        while ntry<=timeout:
            try:
                self.socket.connect ( (self.host,self.port) )
                self.isOpenFlag=True
                for l in self.listen():
                    print l,
                return True
            except socket.error:
                ntry+=1
                if ntry<=timeout:
                    time.sleep(1)  # wait a little before you try again
                
        print "connection to ",self.host,":",self.port," failed"
        return False

            
    def reconnect(self,timeout=0):
        if self.isOpen(): return True
        try:
            connected=self.connect()
            return connected(timeout)
        except:
            return False
        

    def accept(self):
        """ accept a client connection """
        self.socket,address=self.serverSocket.accept()
        self.isOpenFlag=True
        self.send("Greetings, earthling\n")
        self.done()
        print "accepted connection from ",address[0]," on port ",self.port
        return True
        
            
    def recv(self,n):
        """ receive exactly n characters, do multiple polls if necessary """
        if n==0: return ""
        msg=self.socket.recv( n )
        while len(msg)<n:
            mrec=self.socket.recv( n-len(msg) )
            if mrec=="": return msg
            msg+=mrec
        return msg

    def done(self):
        """ only used by servers"""
        self.send("",'D')
        
    def getHeader(self):
        """ receive a header, return type and size
        client and server code """
        header=self.recv(self.headerSize)
        if header=="":
            print "Error receiving data"
            self.close()
            return " ",0
        headerType=header[0]
        if header[1:].strip()=="":
            messageSize=0
        else:
            try:
                messageSize=int( header[1:].strip())
            except ValueError:
                print "bad header ",header
                messageSize=0
        return headerType,messageSize


    def getPacket(self):
        """ receive a full packet, i.e. header + message,
        mostly server code"""
        if not self.isOpen(): return " ",""
        messageType,messageSize=self.getHeader()
        if not self.isOpen(): return " ",""
        message=self.recv(messageSize)
        return messageType,message

    
            
    def listen(self):
        """ generator function returning packets until a done has
        been received
        this is a client method """
        if not self.isOpen(): return
        messageType,messageSize=self.getHeader()

        #print "listen : ", messageType,messageSize
        while not messageType=='D':
            
            if messageType=="P":  # prompt
                self.prompt=self.recv(messageSize)
                
            elif messageType=="C": # close
                print "connection closed by server. ",self.recv(messageSize)
                self.close()
                
            elif messageType==' ': # data
                msg=self.recv(messageSize)
                self.log+=msg
                yield msg
##                 msgs=self.recv(messageSize)
##                 for msg in msgs.split("\n"):
##                     yield msg
                
            else:
                print "unknown type [%s] %s"%(messageType,self.recv(messageSize))

            # get the next header
            if not self.isOpen(): return
            messageType,messageSize=self.getHeader()
            #print "listen : ", messageType,messageSize


    def readline(self):
        """ line oriented transfer:
        return data packtes line by line, allow lines
        split across two packets """
        msg=""
        for p in self.listen():
            if p=="": continue
            lines=(msg+p).split("\n")
            for l in lines[:-1]:
                yield l
            msg=lines[-1]  # in case packet did not end with a line break
                
        if not msg=="":
            print msg
            
        
    def readAll(self):
        """ return everyting in one go, i.e. concatenate all packtets until done """
        slurp=""
        for p in self.listen():
            #print "p=[",p,"]"
            slurp+=p
        return slurp
            
    def readlines(self):
        """ return everyting in one go, i.e. concatenate all packtets until done """
        slurp=[]
        for l in self.readline():
            slurp.append(l)
        return slurp
            
    def send(self,msg,msgType=' ',closeOnError=True):
        self.log=""
        totalsent = 0
        packet=self.packetFormat%(msgType,len(msg),msg)
        while totalsent < len(packet):
            try:
                sent = self.socket.send(packet[totalsent:])
                totalsent+= sent
                if sent == 0:
                    if closeOnError: self.close(notifyServer=False)
                    return self
                    #raise RuntimeError,"socket connection broken"
            except socket.error:
                print "connection to server died",closeOnError
                if closeOnError: self.close(notifyServer=False)
                return self
        return self


    def close(self,notifyServer=True):
        """ close the connection """
        #print "closing ",self.isOpenFlag,notifyServer
        if self.isOpen():
            if notifyServer:
                try:
                    self.send("",'C',closeOnError=False)
                    time.sleep(0.2)
                except socket.error:
                    print "server does not respond to close",notifyServer
            
            try:
                self.socket.close()
            except:
                print "failed to close"
                
        self.isOpenFlag=False
        #print "done closing ",self.isOpenFlag
        

    def isOpen(self):
        return self.isOpenFlag

    def printLog(self,header=None):
        if not self.DEBUG: return
        if not header==None:
            print header
        for l in self.log.split("\n"):
            print ">> ",l

# queries    
    def query(self,msg,regexp=""):
        """ send a message "msg" to the server and receive the response.
        if no regular expression is given, the full response is returned to
        the caller. If a regular expression containing "%s" is given,
        only the matching part of the server response is returned"""
        if regexp=="":
            self.send(msg)
            reply=""
            for l in self.listen():
                reply+=l
            return reply.strip("\0");
        else:
            regexp=regexp.replace("%s","(.*)")
            self.send(msg)
            value=None
            for l in self.readline():
                m=re.match(regexp,l)
                if m:
                    value=m.group(1)
            if value is None:
                print "SimpleSocket.query: no matching response to ",msg
                self.printLog()
                return None
            else:
                return value



    def fquery(self,msg,regexp=""):
        """ query a float. If a regular expression is given,
         put %f where the queried number will be """
        if regexp=="":
            try:
                return float(self.query(msg))
            except ValueError:
                self.printLog("conversion error, server response was:")
                return None

        else:
            # see http://docs.python.org/lib/node49.html
            regexp=regexp.replace("%f","\s*([-+]?(\d+(\.\d*)?|\.\d+)([eE][-+]?\d+)?)\s*")
            self.send(msg)
            value=None
            for l in self.readline():
                m=re.match(regexp,l)
                if m:
                    try:
                        value=float(m.group(1))
                    except ValueError:
                        self.printLog("conversion error, server response was:")
                        value=None
            if value is None:
                print "fquery: no matching response response to ",msg
                self.printLog("server response was")
                return float(0)
            else:
                return value



 
    def xquery(self,msg,regexp=""):
        """ query a hexadecimal number. If a regular expression is given,
         put %x where the queried number will be """
        if regexp=="":
            return eval(self.query(msg))
        else:
            # see http://docs.python.org/lib/node49.html
            regexp=regexp.replace("%x","\s*(0[xX][\dA-Fa-f]+)\s*")
            self.send(msg)
            value=None
            for l in self.readline():
                m=re.match(regexp,l)
                if m:
                    try:
                        value=eval(m.group(1))
                    except ValueError:
                        value=None
            if value is None:
                print "fquery: no matching response response to ",msg
                self.printLog()
                return int(0)
            else:
                return value

 
    def iquery(self,msg,regexp=""):
        """ query an integer. If a regular expression is given,
         put %d where the queried number will be """
        if regexp=="":
            try:
                return int(self.query(msg))
            except ValueError:
                print "conversion error in SimpleSocket.iquery in response to ",msg
                self.printLog("server response was")
        else:
            # see http://docs.python.org/lib/node49.html
            regexp=regexp.replace("%d","\s*([-+]?\d+)\s*")
            self.send(msg)
            value=None
            for l in self.readline():
                m=re.match(regexp,l)
                if m:
                    try:
                        value=int(m.group(1))
                    except ValueError:
                        value=None

            if value is None:
                print "SimpleSocket.iquery: no matching response to ",msg
                self.printLog()
                return int(0)
            else:
                return value


    def aquery(self,msg,regexp=""):
        """ query a list of integers. If a regular expression is used,
        put %s where the list of numbers would be"""
        l=self.query(msg,regexp)
        if l is None:
            return None
        else:
            try:
                return [int(x) for x in l.split()]
            except ValueError:
                print "conversion error in SimpleSocekt.aquery in response to ",msg
                self.printLog()
                return []
                
        

    def __del__(self):
        self.close()








#########################################################################3



 
class SimpleProxy(threading.Thread):

    def __init__(self, incomingPort, srvSocket):
        threading.Thread.__init__(self)
        self.incomingPort=incomingPort
        self.srvSocket=srvSocket  # socket is assumed to be open and waiting

        self.stopped=0
        
        if self.incomingPort == 0:
            readline.read_history_file(".history")
        else:
            self.incomingSocket=SimpleSocket(port=incomingPort)
            
    def stop(self):
        """ the gentle way """
        self.stopped=1

    def kill(self):
        """ the hard way """
        pid=os.getpid()
        os.kill("kill -9 "+str(pid))
               
    def run(self):
        # implement a pass through server
        while not self.stopped and self.incomingSocket.accept():

            while not self.stopped:  # client connection loop
                # the client always sends a single packet, no done 
                requestType,request=self.incomingSocket.getPacket()
                #print "proxy transmitted request ",request

                if requestType=='C' or request=="close" or request=="exit":
                    break
                
                self.srvSocket.send(request,requestType)
                
                if request=="shutdown":
                    print "proxy shutting down"
                    self.incomingSocket.close()
                    return

                # the server may respond many packets, followed by done
                while True:
                    replyType,replyMessage=self.srvSocket.getPacket()
                    self.incomingSocket.send(replyMessage,replyType)
                    if replyType=='D' or replyType=='C':
                        break
                if replyType=='C':
                    print "server closes connection"
                    break
                
            print "connection was closed by the client"
            # keep the server connection alive, just close the client connection
            self.incomingSocket.close()
