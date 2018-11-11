#!/bin/bash
export MROOT=`readlink -f maple`
cd maple/core
make rs
cd ../../
cp maple/core/maplesat bin/maplesat
