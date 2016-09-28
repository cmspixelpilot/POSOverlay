import optparse 

usage = 'usage: %prog -r runNum'
parser = optparse.OptionParser(usage)
parser.add_option('-i', '--inputFile', dest='infile', type='string', help='Input file')
parser.add_option('-o', '--outputFile', dest='ofile', default='outfile.txt', type='string', help='Output File')

(opt, args) = parser.parse_args()

try:
    ifile = open("%s.txt"%(opt.infile),'r')
except IOError:
    print "Cannot open ", "%s.txt"%(opt.infile)

else:
    lines = ifile.readlines() 
    rocs = [l.split()[0]  for l in lines[3:]]
    ofile = open(opt.ofile, 'w')
    for r in rocs:
        if(r == rocs[-1]): 
            print "Last roc"
            ofile.write('%s'%(r))
        else:ofile.write('%s\n'%(r))
#        ofile.write('%s\n'%(r))


    ofile.close()

