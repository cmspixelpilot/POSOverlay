from SimpleSocket import SimpleSocket
import sys,readline,socket
""" console client for connection to multiple SimpleServer(Sockets) """

# read the server list from mclient.conf
# one line per server: key host port, e.g. "d pc4226 2000"
serverlist=[]
try:
    for l in open("mclient.conf").readlines():
        serverlist.append(l.split())
except:
    serverlist=[("d",   "localhost",2000),
                ("caen","localhost",2001),
                ("ccu", "localhost",2002),
                ("fed", "localhost",2003)]
                

servers={}
for key,host,port  in serverlist:
    #if SimpleSocket.acceptsConnections(host,int(port)):
    servers[key]=SimpleSocket(host,int(port),timeout=2)
    print "connected to ",key

try:
    readline.read_history_file(".history")
except:
    pass

defaultServer=""
prompt=">"

while len(servers)>0:

    # user input of the form [serverKey] [request]
    # example: "d module 23 roc 5 Vcal 120" sends request module... to d
    #          "d"                          makes d the default
    # or       "module 23 roc 5 Vcal 120"   works if d is the default

    if defaultServer in servers.keys():
        prompt=servers[defaultServer].prompt
        
    cmd=raw_input("%s:%s " %(defaultServer,prompt)).strip()

    if cmd.strip()=="": continue
    
    if cmd=="exit":
        for key,server in servers.items(): server.close()
        break

    if cmd=="shutdown":
        for key,server in servers.items(): server.send("shutdown")
        break

    if cmd in servers.keys():
        serverKey, defaultServer=cmd,cmd
        continue
    
    elif cmd.split()[0] in servers.keys():
        #serverKey=cmd.split()[0]
        #cmd=cmd[cmd.find(" ")+1:]
        serverKey,cmd=cmd.split(" ",1)
        
    elif defaultServer in servers.keys():
        serverKey=defaultServer
    else:
        print "no valid server specified"
        continue

    # send the request to the selected server and print the response
    servers[serverKey].send(cmd)
    for l in servers[serverKey].readline():
        print l

    # has the socket been closed?
    if not servers[serverKey].isOpen():
        del servers[serverKey]
        if defaultServer==serverKey:
            defaultServer=""


readline.write_history_file(".history")
