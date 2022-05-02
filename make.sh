CUR=`dirname $0`
CUR=`pushd $CUR > /dev/null; pwd`
do_make() {
    pushd $CUR > /dev/null
    make
}

which multipass &>/dev/null

if [ $? -ne 0 ]; then
  do_make
else
    docker run --network none --rm \
        -v /Volumes/dev/Github/mockos:/Volumes/dev/Github/mockos \
        -w /Volumes/dev/Github/mockos \
        gcc-3.4 make
fi
