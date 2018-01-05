#!/bin/bash
APP=mqttClient

DEBUG_LIB_DIR="3rdParty/localhost/lib"
export LD_LIBRARY_PATH=${DEBUG_LIB_DIR}:$LD_LIBRARY_PATH

export MYMAKE=make

# build for localhost
alias l="$MYMAKE localhost"

# build for WP85
alias w="$MYMAKE wp85"

# cleans files
alias c="$MYMAKE clean"

# install
alias i="$MYMAKE install"

# Debug
alias D="_build_$APP/localhost/app/$APP/staging/read-only/bin/$APP"

# the end
