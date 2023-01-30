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
    echo "You've got a $NUSNET_ID.zip" modified at $(date -r $NUSNET_ID.zip)
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

HAS_BONUS=false
if [ -d "ex3" ]; then
    HAS_BONUS=true
fi

if $HAS_BONUS; then
    EXERCISES=(ex1 ex2 ex3)
    MANIFEST=(
        ex1/packer.c
        ex2/packer.c
        ex3/packer.c
    )
else
    EXERCISES=(ex1 ex2)
    MANIFEST=(
        ex1/packer.c
        ex2/packer.c
    )
fi

cd $TEMPDIR

############################################################
#       Check solution compiles                            #
############################################################

echo "Checking solution compiles"
echo "Transferring necessary skeleton files"

# Setup temporary test directory

SK_FILENAME="lab3.tar.gz"
wget --no-check-certificate 'https://docs.google.com/uc?export=download&id=1NHQG_iQ6iTyZNUmcZ48wbirdB31t9Mxp' -O $SK_FILENAME 2>/dev/null
tar -zxf $SK_FILENAME >/dev/null
mv lab3 $NUSNET_ID
cd $NUSNET_ID

# Copy solutions

for fn in ${MANIFEST[@]}; do
    cp "$WORKDIR/$fn" "$fn" || (_Fail "Fail to copy file $fn" && _Exit 1)
done

# Check compile

for ex in ${EXERCISES[@]}; do
    make -C $ex || (_Fail "Fail to compile $ex" && _Exit 1)
    make -s -C $ex clean
done

cd $TEMPDIR
zip $WORKDIR/$NUSNET_ID.zip ${MANIFEST[@]/#/$NUSNET_ID/} >/dev/null

cd $WORKDIR
unzip -l $NUSNET_ID.zip
rm -f .$NUSNET_ID.zip

_Exit 0
