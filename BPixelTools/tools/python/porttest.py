import socket,time,sys
host="localhost"
port=int(sys.argv[1])
s = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
try:
    s.connect ( (host,port) )
    print "port %d already in use"%(port)
    s.send("C   0")
    time.sleep(0.5)
    s.close()
    sys.exit(1)
except socket.error:
    s.bind(('',port))
    #print "port %d unused"%port
    s.close()
    sys.exit(0)
