import sys, os
from JMTTools import *

min_key = int(sys.argv[1])
modules = sys.argv[2:]
if not modules:
    raise ValueError('need some modules in argv!')

modules.sort()
print 'modules are:'
for m in modules:
    print m

max_seen = 0

for line in open(config_fn('aliases.txt')):
    line = line.strip().split()
    if line[0] == 'detconfig':
        max_seen = max(max_seen, int(line[1]))

new_key = max(max_seen, min_key) + 1
print 'new key is', new_key
raw_input('[enter to continue]')

detconfig_dir = config_fn('detconfig/' + str(new_key))
assert not os.path.isdir(detconfig_dir)
os.mkdir(detconfig_dir)

det_f = open(os.path.join(detconfig_dir, 'detectconfig.dat'), 'wt')
det_f.write('Rocs:\n')
for m in sorted(modules):
    for i in xrange(16):
        line = m + '_ROC%i' % i + '\n'
        det_f.write(line)
        print line,
det_f.close()

cmd = '$BUILD_HOME/pixel/bin/PixelConfigDBCmd.exe --insertVersionAlias detconfig %i Default' % new_key
print cmd
os.system(cmd)
