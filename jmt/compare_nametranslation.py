#!/usr/bin/env python

class PixelNameTranslationEntry:
    def __init__(self, line):
        self.ok = False
        line = line.strip().split()
        if len(line) == 10:
            line.insert(1, 'A')
        if len(line) != 11:
            raise ValueError('cannot parse line: %r' % line)
        self.all = line
        self.name, self.tbm_channel, self.fec, self.mfec, self.mfecchannel, self.hubaddress, self.portadd, self.rocid, self.fed, self.fedchannel, self.roc = line

    def __hash__(self):
        return hash(self.all)

    def __eq__(self, other):
        return self.all == other.all

class PixelNameTranslation:
    def __init__(self, fn):
        self.entries = []

        for line in open(fn):
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            self.entries.append(PixelNameTranslationEntry(line))

        self.entries.sort(key=lambda entry: entry.all)

        if len(set(entry.name for entry in self.entries)) != len(self.entries):
            raise ValueError('some name defined twice')

    def __eq__(self, other):
        return self.entries == other.entries

print PixelNameTranslation('plus') == PixelNameTranslation('translation.dat')
