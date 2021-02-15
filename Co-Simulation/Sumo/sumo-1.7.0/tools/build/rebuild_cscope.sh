#!/bin/bash
# script for building ctags/cscope index files 
# to be run from the root directory

export FOXDIR=~/programs/fox-1.6.43
export CSTMP=~/tmp/cscope.files
export CSOUT=cscope.out
export TAGSOUT=tags

cd src
rm $CSTMP
rm $CSOUT
rm $TAGSOUT
#find $FOXDIR/src -name '*.cpp' > $CSTMP
#find $FOXDIR/include -name '*.h' >> $CSTMP
find -name '*.cpp' >> $CSTMP
find -name '*.h' >> $CSTMP
#find ../unittest -name '*.cpp' >> $CSTMP
#find ../unittest -name '*.h' >> $CSTMP
cscope -i $CSTMP -b -f $CSOUT
ctags -L $CSTMP -f $TAGSOUT
