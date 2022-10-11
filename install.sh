#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 TARGET_DIRECTORY" >&2
    exit 1
fi

PREFIX=$1

# set up build variables
export PYTHONPATH=external:$PYTHONPATH

if [[ ! -z "${PYBIND11PATH}" ]]; then
    export DESRES_MODULE_CPPFLAGS="-I$PYBIND11PATH/include":$DESRES_MODULE_CPPFLAGS    
fi

PYTHONVER=37

# If we have homebrew, we're probably on a mac.  Use current defaults for PYTHONVER
if [[ -e /opt/homebrew/bin/pybind11-config ]]; then
    export DESRES_MODULE_CPPFLAGS=$(pybind11-config --includes)
    PYTHONVER=310
fi

# invoke build
. version.sh
scons -j4 install PREFIX=$PREFIX/ PYTHONVER=$PYTHONVER VERSION=$VERSION

# build documentation
. version.sh
mkdir -p $PREFIX/doc
(cd doc && LD_LIBRARY_PATH=$PREFIX/lib:$LD_LIBRARY_PATH PREFIX=$PREFIX VERSION=$VERSION make BUILDDIR=$PREFIX/doc clean html)

