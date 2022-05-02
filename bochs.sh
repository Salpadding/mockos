# build kernel
CUR=`dirname $0`

export OSLAB_PATH=$CUR/oslab

HOST=`hostname`
if [[ $HOST == LGR* ]]; then
    bochs -q -f $CUR/bochsrc.m1.txt
else
    bochs -q -f $CUR/bochsrc.intel.txt
fi

