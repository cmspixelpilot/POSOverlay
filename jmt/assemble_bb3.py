from JMTTools import *
from JMTROOTTools import *
from write_other_hc_configs import doer, HC, module_sorter_by_portcard
set_style(light=True)

the_key = disk, which = 3, 'BpO_bb3'
the_doer = doer(disk)

fns = {
    (3, '50trig'): [x.strip() for x in '''
Run_1401/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1402/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1409/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1411/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1413/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1415/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1418/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1419/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1420/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1421/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1422/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1429/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1430/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1431/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1432/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1433/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1434/TrimOutputFile_Fed_1297-1298-1299.dat
Run_1435/TrimOutputFile_Fed_1297-1298-1299.dat
Run_1436/TrimOutputFile_Fed_1297-1298-1299.dat
'''.split('\n') if x.strip()],
    (2, '50trig'): [x.strip() for x in '''
Run_1440/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1441/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1442/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1443/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1444/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1446/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1447/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1448/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1449/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1450/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1453/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1454/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1455/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1460/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1461/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1462/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1463/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1464/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1465/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1466/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1467/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1468/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1469/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1471/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1472/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1473/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1474/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1475/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1476/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1477/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1481/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1480/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1482/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1483/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1484/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1485/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1487/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1488/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1489/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1490/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1491/TrimOutputFile_Fed_1295-1296-1297.dat
Run_1492/TrimOutputFile_Fed_1295-1296-1297.dat
Run_1493/TrimOutputFile_Fed_1295-1296-1297.dat
Run_1494/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1495/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1496/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1497/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1498/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1499/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1501/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1502/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1503/TrimOutputFile_Fed_1297-1298-1299-1300.dat
Run_1504/TrimOutputFile_Fed_1299-1300.dat
Run_1505/TrimOutputFile_Fed_1299-1300.dat
Run_1506/TrimOutputFile_Fed_1299-1300.dat
'''.split('\n') if x.strip()],
    (1, 'first10'): [x.strip() for x in '''
Run_1510/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1511/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1512/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1513/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1514/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1515/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1516/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1518/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1520/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1521/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
'''.split('\n') if x.strip()],
    (1, '5trig'): [x.strip() for x in '''
Run_1523/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1524/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1525/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1526/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1527/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1528/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1529/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1530/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
'''.split('\n') if x.strip()],
    (1, '5trig250Vcal150HV'): [x.strip() for x in '''                                                                 
Run_1540/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1541/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1542/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat 
Run_1543/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1544/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1545/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1546/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
Run_1547/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299.dat
'''.split('\n') if x.strip()],
    (3, 'BpO'): [x.strip() for x in '''
Run_1599/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1600/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1601/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1602/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1603/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1604/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1606/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1608/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1609/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1610/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1611/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1612/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1614/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1615/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1616/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1617/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1618/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1620/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1621/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1622/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1623/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1624/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1626/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1627/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1628/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1629/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1630/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1632/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1633/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1634/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1635/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1636/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1637/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1639/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1640/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1641/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1642/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1643/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1644/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1645/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1647/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1648/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1649/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1650/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1651/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1653/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1654/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1655/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1656/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1657/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1659/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1660/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1661/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1662/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1663/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1665/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1666/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1667/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1668/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1669/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1671/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1672/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1673/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1674/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1675/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1676/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1678/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1679/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1680/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1681/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1682/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1684/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1685/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1686/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1687/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1688/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1689/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1690/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1691/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1692/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
'''.split('\n') if x.strip()],
    (3, 'BpO_bb3'): [x.strip() for x in '''
Run_1695/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1696/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1697/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1698/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1699/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1700/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1701/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
Run_1702/TrimOutputFile_Fed_1294-1295-1296-1297-1298-1299-1300.dat
'''.split('\n') if x.strip()],
    (2, 'BpO_bb3'): [x.strip() for x in '''
Run_1806/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
Run_1807/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
Run_1808/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
Run_1809/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
Run_1811/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
Run_1813/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
Run_1814/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
Run_1815/TrimOutputFile_Fed_1295-1296-1297-1298-1299-1300.dat
'''.split('\n') if x.strip()],
}

ns = {(3,'50trig'): 19, (2,'50trig'): 55, (1,'first10'):10, (1,'5trig'):8, (3,'BpO'):80, (3,'BpO_bb3'):8}
for k,v in ns.iteritems():
    assert len(fns[k]) == v

def make_fn():
    return 'disk%i_%s' % the_key
def pickle_fn():
    return make_fn() + '.p'
def dat_fn():
    return make_fn() + '.dat'

def from_fns():
    td_merge = merge_trim_dats(fns[the_key])
    for roc in sorted(td_merge.keys()):
        td_merge[roc] = [((e.th, e.sg) if e != 0 else e) for e in td_merge[roc]]
    return td_merge

def to_pickle(td_merge):
    cPickle.dump(dict(td_merge), open(pickle_fn() , 'wb'), -1)

def from_pickle():
    return cPickle.load(open(pickle_fn(), 'rb'))

def plot_missing(td_merge):
    max_pcnum = 3 if disk == 1 else 4
    for pcnum in xrange(1,max_pcnum+1):
        print pcnum

        c = ROOT.TCanvas('c', '', 1000, 1000)
        c.Divide(4,4)
        c.cd(0)
        ex = the_key[1]
        fn = 'missing_%s_D%i_PRT%i_%s.pdf' % (HC, disk, pcnum, ex)
        c.SaveAs(fn + '[')

        modules = [m.name for m in sorted(the_doer.modules, key=module_sorter_by_portcard) if the_doer.moduleOK(m) and m.portcardnum == pcnum]
        for module in modules:
            print module
            hs = []
            for i in xrange(16):
                roc = module + '_ROC' + str(i)
                assert td_merge.has_key(roc)
                c.cd(i+1)

                h = ROOT.TH2F('h_' + roc, roc, 52, 0, 52, 80, 0, 80)
                h.SetStats(0)
                hs.append(h)
                l = td_merge[roc]
                for i,x in enumerate(l):
                    col = i / 80
                    row = i % 80
                    if x == 0:
                        h.Fill(col, row)
                h.Draw('colz')

            c.cd(0)
            c.SaveAs(fn)
        c.SaveAs(fn + ']')

def write_dat(td_merge):
    f = open(dat_fn(), 'wt')
    for roc in sorted(td_merge.keys()):
        for i in xrange(4160):
            col = i / 80
            row = i % 80
            e = td_merge[roc][i]
            if e != 0:
                th, sg = e
                f.write('X %(roc)s %(row)s %(col)s ' % locals())
                f.write('%.6f %.6f 0 0 0\n' % (sg, th))

tdm = to_pickle()
#plot_missing(tdm)
write_dat(tdm)

#merge_trim_dats(fns[(3,'BpO')], 'BpO_disk3.dat')

