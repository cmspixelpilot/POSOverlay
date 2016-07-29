import sys, os
from JMTTools import *

portcardmap = None
for x in sys.argv:
    if x.startswith('portcardmap='):
        portcardmap = portcardmap_dat(config_fn('portcardmap/%i' % int(x.split('portcardmap=')[1])))
        sys.argv.remove(x)
        break

modules = sys.argv[1:]
if not modules:
    raise ValueError('need some modules in argv!')

modules.sort()
print 'modules are:'
for m in modules:
    print m

detconfig_key, detconfig_dir = new_config_key('detconfig')
print 'new detconfig key is', detconfig_key
if portcardmap:
    portcardmap_key, portcardmap_dir = new_config_key('portcardmap')
    print 'new portcardmap key is', portcardmap_key
raw_input('[enter to continue]')

os.mkdir(detconfig_dir)
if portcardmap:
    os.mkdir(portcardmap_dir)

det_f = open(os.path.join(detconfig_dir, 'detectconfig.dat'), 'wt')
det_f.write('Rocs:\n')
for m in sorted(modules):
    for i in xrange(16):
        line = m + '_ROC%i' % i + '\n'
        det_f.write(line)
        print line,
det_f.close()

if portcardmap:
    pcmap_f = open(os.path.join(portcardmap_dir, 'portcardmap.dat'), 'wt')
    pcmap_f.write('# Portcard              Module                     AOH channel\n')
    for m in sorted(modules):
        for aorb in 'AB':
            pc, poh = portcardmap.p[m]
            line = '%s %s %s %s\n' % (pc, m, aorb, poh)
            pcmap_f.write(line)
            print line,
    pcmap_f.close()

cmds = [ '$BUILD_HOME/pixel/bin/PixelConfigDBCmd.exe --insertVersionAlias detconfig %i Default' % detconfig_key ]
if portcardmap:
    cmds += ['$BUILD_HOME/pixel/bin/PixelConfigDBCmd.exe --insertVersionAlias portcardmap %i Default' % portcardmap_key ]
for cmd in cmds:
    print cmd
    os.system(cmd)
