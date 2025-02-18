#!/usr/bin/env sh

BINARY=
HEX=
OUT=
CIPHER=

help () {
    echo "usage: ./serial.sh [-c <cipher-result>] -b <binary> -x <hex> -o <outfile>"
    if test -z "$1"
    then 
        exit 0
    else 
        exit $1
    fi
}

while getopts "hb:x:o:c:" option
do 
    case $option in
        b) BINARY=$OPTARG;;
        x) HEX=$OPTARG;;
        o) OUT=$OPTARG;;
        c) CIPHER=$OPTARG;;
        h) help
    esac
done

if test -z "$BINARY"
then 
    echo "missing -b option"
    help 1
elif test -z "$HEX"
then
    echo "missing -x option"
    help 1
elif test -z "$OUT"
then
    echo "missing -o option"
    help 1
fi


ADDR_CYCLES=`arm-none-eabi-objdump -t $BINARY | grep bench_cycles | cut -f 1 -d  ' '`
ADDR_CIPHER=`arm-none-eabi-objdump -t $BINARY | grep ciphertexts | cut -f 1 -d  ' '`

openocd -l /dev/null \
        -f board/st_nucleo_f4.cfg \
        &>/dev/null &
# Get the pid of `openocd` (not `sudo opencd`)
OPENOCDPID=$!

sleep 1

TMP=`mktemp`

(echo "init; reset halt; flash write_image erase $HEX; reset run; sleep 2000"; echo "mdw 0x$ADDR_CYCLES 3"; echo 'exit') | netcat localhost 4444 > $TMP

grep -a -v '>' $TMP | grep -a "$ADDR_CYCLES" | cut -f 2-4 -d ' ' > $OUT
printf "Result to $OUT\n"
rm $TMP

if test -n "$CIPHER"
then
    TMP_CIPHER=`mktemp`
    (echo "mdw 0x$ADDR_CIPHER 4"; echo 'exit') | netcat localhost 4444 > $TMP_CIPHER
    grep -a -v '>' $TMP_CIPHER | grep -a "$ADDR_CIPHER" | cut -f 2-5 -d ' ' > $CIPHER
    rm $TMP_CIPHER
fi


# Kill openocd
kill $OPENOCDPID