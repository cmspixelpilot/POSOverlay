#!/bin/bash

if [ "$#" -ne 1 ]
then
  echo Usage: phases_from_log.sh log_filename
  exit 1
fi

egrep 'phase|^   ' $1 | grep -v 'this is the only' | sed 's/ Init phase.*//' > ${1}.phases

