CUR=`dirname $0`
do_make() {
    pushd $CUR
    make
}

which multipass &>/dev/null

if [ $? -ne 0 ]; then
  do_make
else
  multipass exec primary -- bash /home/ubuntu/Github/mockos/make.sh
fi
