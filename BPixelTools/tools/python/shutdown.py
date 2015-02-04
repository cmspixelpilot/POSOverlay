import socket,sys
host='localhost'

if len(sys.argv)==1:
    ports=range(2000,2005)
else:
    ports=[int(s) for s in sys.argv[1:]]
    
for port in ports:
    s = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
    try:
        s.connect ( (host,port) )
        print "port %d accepting connections"%(port)
        s.send("    8shutdown")
        try:
            s.close()
        except:
            pass
        print "port %d shut down"%port
        
    except socket.error:
        try:
            s.bind(('',port))
            print "port %d unused"%port
            s.close()
        except:
            print "port %d is busy"%port
