import re
import sys

#declarations
header  = "011111111100"
trailer = "011111111110"
ROC_header1 = "011111111000"
ROC_header2 = "011111111001"
ROC_header3 = "011111111010"
ROC_header4 = "011111111011"
#size of bits
event_number_size = 8
data_id_size      = 8
roc_header_size   = 10 # another 2 is the read back data
col_addr_size     = 6
row_addr_size     = 9
pixel_hit_size    = 9

firstSequence = []

fileinput = open(sys.argv[1], 'r')
for line in fileinput:
  splitline = line.split()[(int(sys.argv[2]))] #must be 2 or 3
  firstSequence.append(splitline)

first  = ''.join(firstSequence)

position_of_start_first  = first.find(header)
position_of_end_first    = first.find(trailer)

#find how many ROCs fired
ROCoccurancesRAW = []
for m in re.finditer(ROC_header1, first):
  ROCoccurancesRAW.append(str(m.start()))
for m in re.finditer(ROC_header2, first):
  ROCoccurancesRAW.append(str(m.start()))
for m in re.finditer(ROC_header3, first):
  ROCoccurancesRAW.append(str(m.start()))
for m in re.finditer(ROC_header4, first):
  ROCoccurancesRAW.append(str(m.start()))

ROCoccurances = ' '.join(ROCoccurancesRAW)

temp = []
FindHeader = []
Trailer = []
ColAddr = []
RowAddr = []
PulseHeight = []
foundHeader = 0
foundROCHeader = 0
i = position_of_start_first - 1
internal_counter = 0
internal_counter_event = 0
RocNumber = 1
while i < position_of_end_first+len(trailer):
  i += 1
  internal_counter += 1

  if foundHeader == 0:
    temp.append(first[i])
    #found header
    if internal_counter==len(header):
      print "Header: "+''.join(temp)
      temp = []
    #found event number
    if internal_counter==len(header)+event_number_size:
      print "Event number: "+''.join(temp)
      temp = []
    # found ID number
    if internal_counter==len(header)+event_number_size+data_id_size:
      print "Data ID: "+''.join(temp)
      temp = []
      foundHeader = 1
      continue

  # found header, event number and ID so search for ROC and event details
  if foundHeader and i<position_of_end_first: 
    FindHeader.append(first[i])

    for number in ROCoccurancesRAW:
      if i == (int(number)):
        foundROCHeader = 0

    if ''.join(FindHeader) == ROC_header1 or ''.join(FindHeader) == ROC_header2 or ''.join(FindHeader) == ROC_header3 or ''.join(FindHeader) == ROC_header4:
      print '--> ROC#', RocNumber, 'and the binary is', ''.join(FindHeader)
      FindHeader = []
      RocNumber += 1
      foundROCHeader = 1
      continue
    if foundROCHeader:
      internal_counter_event += 1
      ColAddr.append(first[i])
      if internal_counter_event==col_addr_size:
        print 'Col Addr ',''.join(ColAddr)
        FindHeader = []
        ColAddr = []
      if internal_counter_event==col_addr_size+row_addr_size:
        print 'Row Addr ', "".join(ColAddr)
        FindHeader = []
        ColAddr = []
      if internal_counter_event==(col_addr_size+row_addr_size+pixel_hit_size):
        print 'Pulse Height Bit ', "".join(ColAddr)
        FindHeader = []
        ColAddr = []
        internal_counter_event = 0

  if i >= position_of_end_first:
    Trailer.append(first[i])
    if len(Trailer)==len(trailer):
      print 'TBM Trailer: ',''.join(Trailer)
