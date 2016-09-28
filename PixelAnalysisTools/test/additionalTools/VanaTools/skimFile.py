import optparse

usage = 'usage: %prog -r runNum'
parser = optparse.OptionParser(usage)
parser.add_option('-i', '--inputFile', dest='infile', type='string', help='Input file')
parser.add_option('-o', '--outputFile', dest='ofile', default='outfile.txt', type='string', help='Output File')

(opt, args) = parser.parse_args()


try:
    ofile = open("%s.txt"%(opt.infile),'r')
except IOError:
    print "Cannot open ", "%s.txt"%(opt.infile)

else:
    lines = ofile.readlines() 
    rocs = [l.split()[0:3]  for l in lines[3:] if float(l.split()[1]) < 35.0]
    ofile = open(opt.ofile, 'w')
    for r in rocs:
        ofile.write('%s  %s  %s\n'%(r[0], r[1], r[2]) )
    ofile.close()
