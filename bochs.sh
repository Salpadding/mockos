# build kernel
CUR=`dirname $0`

export OSLAB_PATH=$CUR/oslab

if [[ $OSTYPE == darwin* ]]; then
    ~/opt/bochs/bin/bochs -q -f $CUR/bochsrc.mac.txt
else
    bochs -q -f $CUR/bochsrc.linux.txt
fi

