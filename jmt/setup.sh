#!/bin/bash

pushd ~/build/TriDAS/pixel/
ln -s trunk/Makefile
ln -s trunk/RPM.version
ln -s trunk/VERSION
ln -s PixelConfigDataExamples config
popd
