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
  multipass exec primary -- bash $CUR/make.sh
fi
