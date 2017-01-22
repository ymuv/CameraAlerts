#!/bin/bash

while [ 1 ] ; do
    echo run ${EXE_NAME} $*
    nice -n 5 ${EXE_NAME} $* \
    || mpg123 ${CMAKE_INSTALL_PREFIX}/${INSTALL_BIN_DIR}/crash.mp3 > /dev/null 2> /dev/null

    echo "rerun loop"
    echo
    echo
    echo
    sleep 1
done
