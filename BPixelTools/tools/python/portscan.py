import socket,time
host='localhost'
for port in range(2000,2011):
    s = socket.socket ( socket.AF_INET, socket.SOCK_STREAM )
    try:
        s.connect ( (host,port) )
        print "port %d accepting connections"%(port)
        s.send("C   0")
        time.sleep(0.5)
        s.close()
    except socket.error:
        try:
            s.bind(('',port))
            print "port %d unused"%port
            s.close()
        except:
            print "port %d is busy"%port
