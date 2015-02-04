from SimpleSocket import SimpleSocket
import sys,readline
""" console client for SimpleSocket servers or SimpleServer applications"""

if not len(sys.argv)==3:
    print "usage: python client.py <host> <port>"
    sys.exit(1)
    
host,port=sys.argv[1],int(sys.argv[2])


try:
    readline.read_history_file(".history")
except:
    pass

mySocket=SimpleSocket( host, port)


while mySocket.isOpen():
    cmd=raw_input("%s " %(mySocket.prompt))
    mySocket.send(cmd)
    for l in mySocket.readline():
        print l
    
mySocket.close()
readline.write_history_file(".history")
