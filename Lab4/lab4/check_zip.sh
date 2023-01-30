#!/bin/bash

set -e

WORKDIR=$(pwd)
TEMPDIR=$(mktemp -d)

function _Exit() {
    cd $WORKDIR
    rm -rf $TEMPDIR
    exit $1
}

function _Fail() {
    echo -e "\e[1;31m ERROR: \e[0m"
    echo -e "\e[1;31m     $1 \e[0m"
    _Exit 1
}

############################################################
#       Initialisation                                     #
############################################################

# Check parameter

if [ "$#" -ne 1 ]; then
    echo "${0} <NUSNET ID>"
    _Fail "You should provide your NUSNET ID as a parameter and nothing else!"
fi

NUSNET_ID=${1}
if ! [[ $NUSNET_ID =~ ^E[0-9]{7}$ ]]; then
    _Fail "Your NUSNET ID cannot be recognised"
fi

if [[ -f $NUSNET_ID.zip ]]; then
    echo "You've got a $NUSNET_ID.zip modified at $(date -r $NUSNET_ID.zip)"
    echo "Do you want to delete it and create a new one?"
    select yn in "Yes" "No"; do
        case $yn in
        Yes)
            mv $NUSNET_ID.zip .$NUSNET_ID.zip
            break
            ;;
        No)
            _Exit 0
            ;;
        esac
    done
fi

# Detect bonus part
MANIFEST=(
    zc_io.c
)

cd $TEMPDIR

############################################################
#       Check solution compiles                            #
############################################################

echo "Checking solution compiles"
echo "Transferring necessary skeleton files"

# Setup temporary test directory

SK_FILENAME="lab4.tar.gz"
wget --no-check-certificate 'http://comp.nus.edu.sg/~e0446373/cs2106/lab4.tar.gz' -O $SK_FILENAME 2>/dev/null
tar -zxf $SK_FILENAME >/dev/null
mv lab4 $NUSNET_ID
cd $NUSNET_ID

# Copy solutions

for fn in ${MANIFEST[@]}; do
    cp "$WORKDIR/$fn" "$fn" || (_Fail "Fail to copy file $fn" && _Exit 1)
done

# Check compile

make || (_Fail "Fail to compile $ex" && _Exit 1)
make -s clean

cd $TEMPDIR
zip $WORKDIR/$NUSNET_ID.zip ${MANIFEST[@]/#/$NUSNET_ID/} >/dev/null

cd $WORKDIR

unzip -l $NUSNET_ID.zip
rm -f .$NUSNET_ID.zip

echo "============================================================"
echo " SUCCEED:"
echo "     $NUSNET_ID.zip is successfully generated."
echo "============================================================"

_Exit 0
