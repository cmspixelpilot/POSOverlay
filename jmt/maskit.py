from JMTTools import *

xlat_key = 202
old_mask_key = 13

'''
for x in *.dmp; do
    echo $x
    dumpBinaryFiles $x | awk '/^data/ { print $3, $4, $5, $6, $7, $8, $9, $10 }' | sort | uniq -c
done

->

PixelAlive_1294_1062.dmp
      1 Channel- 27 ROC- 4 DCOL- 7 Pixel- 88
      9 Channel- 32 ROC- 0 DCOL- 3 Pixel- 85
      1 Channel- 8 ROC- 0 DCOL- 17 Pixel- 42
PixelAlive_1295_1062.dmp
    551 Channel- 29 ROC- 7 DCOL- 0 Pixel- 7
    580 Channel- 30 ROC- 2 DCOL- 22 Pixel- 54
      1 Channel- 30 ROC- 2 DCOL- 3 Pixel- 93
     43 Channel- 5 ROC- 2 DCOL- 20 Pixel- 21
    403 Channel- 8 ROC- 0 DCOL- 23 Pixel- 51
PixelAlive_1296_1062.dmp
      2 Channel- 1 ROC- 2 DCOL- 22 Pixel- 43
     68 Channel- 25 ROC- 0 DCOL- 24 Pixel- 103
      1 Channel- 29 ROC- 1 DCOL- 21 Pixel- 2
     50 Channel- 29 ROC- 6 DCOL- 14 Pixel- 32
PixelAlive_1297_1062.dmp
      1 Channel- 8 ROC- 1 DCOL- 19 Pixel- 114

PixelAlive_1294_1096.dmp
     31 Channel- 1 ROC- 1 DCOL- 18 Pixel- 74
      3 Channel- 30 ROC- 3 DCOL- 22 Pixel- 2
      1 Channel- 6 ROC- 0 DCOL- 10 Pixel- 61
      2 Channel- 6 ROC- 0 DCOL- 17 Pixel- 25
      1 Channel- 6 ROC- 0 DCOL- 17 Pixel- 45
      1 Channel- 6 ROC- 0 DCOL- 17 Pixel- 61
    336 Channel- 8 ROC- 6 DCOL- 15 Pixel- 65
PixelAlive_1295_1096.dmp
      4 Channel- 3 ROC- 1 DCOL- 15 Pixel- 12
    163 Channel- 3 ROC- 6 DCOL- 23 Pixel- 7

'''

to_mask = [
    (1294, 8, 6, 15, 65),
    (1295, 3, 6, 23, 7),
]

###################################

xlat = translation_dat(config_fn('nametranslation/%i/translation.dat' % xlat_key))

new_mask_key, new_mask_dir = new_config_key('mask')
os.mkdir(new_mask_dir)

masks = {}

for fedid, fedch, chroc, dc, pxl in to_mask:
    module = xlat.module_name_from_fed_ch(fedid, fedch)
    if masks.has_key(module):
        print 'got old'
        mask = masks[module]
    else:
        mask = masks[module] = mask_dat(config_fn('mask/%i/ROC_Masks_module_%s.dat' % (old_mask_key, module)))
    roc = chroc
    if fedch % 2 == 0:
        roc += 8
    print fedid, fedch, chroc, '=', module, 'roc', roc
    mask.set(0, roc, dc=dc, pxl=pxl)

for module, mask in masks.iteritems():
    print module, mask
    new_mask_fn = os.path.join(new_mask_dir, 'ROC_Masks_module_%s.dat' % module)
    mask.write(new_mask_fn)
