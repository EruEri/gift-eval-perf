#!/usr/bin/env sh

BINARY=
HEX=
OUT=

help () {
    echo "usage: ./serial.sh -b <binary> -x <hex> -o <outfile>"
    if test -z "$1"
    then 
        exit 0
    else 
        exit $1
    fi
}

while getopts "hb:x:o:" option
do 
    case $option in
        b) BINARY=$OPTARG;;
        x) HEX=$OPTARG;;
        o) OUT=$OPTARG;;
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


ADDR_CYCLES=`arm-none-eabi-objdump $BINARY -t | grep bench_cycles | cut -f 1 -d  ' '`

openocd -l /dev/null \
        -f /usr/share/openocd/scripts/board/st_nucleo_f4.cfg \
        &>/dev/null &
# Get the pid of `openocd` (not `sudo opencd`)
OPENOCDPID=$!

sleep 1

TMP=`mktemp`

(echo "init; reset halt; flash write_image erase $HEX; reset run; sleep 2000"; echo "mdw 0x$ADDR_CYCLES 3"; echo "mdw 0x$ADDR_LENS 3"; echo 'exit') | netcat localhost 4444 > $TMP

grep -a -v '>' $TMP | grep -a "$ADDR_CYCLES" | cut -f 2-4 -d ' ' > $OUT


printf "Result to $OUT\n"

rm $TMP

# Kill openocd
kill $OPENOCDPID