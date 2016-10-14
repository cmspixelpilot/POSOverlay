function pathadd() {
  # TODO add check for empty path
  # and what happens if $1 == $2
  # Copy into temp variables
  PATH_NAME=$1
  PATH_VAL=${!1}
  if [[ ":$PATH_VAL:" != *":$2:"* ]]; then
    PATH_VAL="$2${PATH_VAL:+":$PATH_VAL"}"
    echo "- $1 += $2"

    # use eval to reset the target
    eval "$PATH_NAME=$PATH_VAL"
  fi

}


CACTUS_ROOT=/opt/cactus
PYCHIPS_ROOT=$( readlink -f $(dirname $BASH_SOURCE)/.. )/pychips
FC7_ROOT=$( readlink -f $(dirname $BASH_SOURCE)/.. )/fc7


# add to path
pathadd PATH "${CACTUS_ROOT}/bin"


# add python path
pathadd PYTHONPATH "${PYCHIPS_ROOT}/src"


# add libary path
pathadd LD_LIBRARY_PATH "${FC7_ROOT}/tests/lib"
pathadd LD_LIBRARY_PATH "${FC7_ROOT}/fc7/lib"
pathadd LD_LIBRARY_PATH "${CACTUS_ROOT}/lib"
pathadd LD_LIBRARY_PATH "${XDAQ_ROOT}/lib"

export CACTUS_ROOT FC7_ROOT PATH LD_LIBRARY_PATH PYTHONPATH

ln -s tests/bin/fc7firmwarer.exe
