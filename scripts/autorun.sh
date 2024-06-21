#!/usr/bin/env sh

FIND_MODS_CMD="find_mods"
PROC_NAME_DEFAULT="ctf_io_server"
BAN_MODS_DEFAULT="IO,LOG,ALARM,HISTORY,RECIPE,CTC,GUI,ERRTOOL"
DISABLE_BAN_MODS=0

OK_CNT=0
FAIL_CNT=0
OK_MODS=""
FAIL_MODS=""

print_usage() {
    echo "Usage: $0 -i FILEPATH [-b MODNAME...] [-n IFNAME] [-d]"
    echo "Options:"
    echo "  -i  FILEPATH to the XML configuration file"
    echo "  -b  Comma-separated list of banned MODNAME"
    echo "  -n  Network IFNAME to get the IP addr (env: ETH_DEV | default: etheqos)"
    echo "  -d  Disable the list of banned MODNAME"
    echo "  -h  Show this help message"
    exit 1
}

if [ $# -eq 0 ]; then
    print_usage
fi

while getopts "n:i:b:dh" opt; do
    case $opt in
        i) XML_FILEPATH=$OPTARG ;;
        b) BAN_MODS=$OPTARG ;;
        n) CMD_ETH_DEV=$OPTARG ;;
        d) DISABLE_BAN_MODS=1 ;;
        h) print_usage ;;
        *) print_usage ;;
    esac
done

PROC_NAME=${PPROC:-$PROC_NAME_DEFAULT}
ETH_DEV=${ETH_DEV:-$CMD_ETH_DEV}
BAN_MODS=${BAN_MODS:-$BAN_MODS_DEFAULT}
if [ -z "$XML_FILEPATH" ]; then
    echo "Error: The -i option and corresponding XML file should be provided" >&2
    print_usage
fi

FIND_MODS_CMD="$FIND_MODS_CMD -i $XML_FILEPATH"
if [ $DISABLE_BAN_MODS -eq 0 ]; then
    FIND_MODS_CMD="$FIND_MODS_CMD -b $BAN_MODS"
fi

if [ -n "$ETH_DEV" ]; then
    FIND_MODS_CMD="$FIND_MODS_CMD -n $ETH_DEV"
fi

if ! pgrep -x "$PROC_NAME" > /dev/null; then
    echo "[$PROC_NAME] is not running ..." >&2
    exit 1
fi

$FIND_MODS_CMD | while read -r modname; do
    echo "Executing: runhalo -n $modname"
    runhalo -n "$modname" &
    MOD_PID=$!

    if wait $MOD_PID; then
        OK_CNT=$((OK_CNT + 1))
        OK_MODS="$OK_MODS\n$modname"
    else
        FAIL_CNT=$((FAIL_CNT + 1))
        FAIL_MODS="$FAIL_MODS\n$modname"
        echo "Warning: Failed to execute runhalo for $modname" >&2
    fi
done

echo "===================="
echo "Summary of Execution"
echo "===================="
echo "Total successful executions: $OK_CNT"
echo -e "Successful modules: $OK_MODS"
echo "--------------------"
echo "Total failed executions: $FAIL_CNT"
echo -e "Failed modules: $FAIL_MODS"