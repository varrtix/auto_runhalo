#!/usr/bin/env sh

GREENC='\033[0;32m'
REDC='\033[0;31m'
NC='\033[0m'


FIND_MODS_CMD="find_mods"
PROC_NAME_DEFAULT="ctf_io_server"
BAN_MODS_DEFAULT="IO,LOG,ALARM,HISTORY,RECIPE,CTC,GUI,ERRTOOL"
DISABLE_BAN_MODS=0

OK_CNT=0
OK_MODS=""
FAIL_CNT=0
FAIL_MODS=""

print_usage() {
    printf "Usage: $0 -i FILEPATH [-b MODNAME...] [-n IFNAME] [-d]\n"
    printf "Options:\n"
    printf "  -i  FILEPATH to the XML configuration file\n"
    printf "  -b  Comma-separated list of banned MODNAME\n"
    printf "  -n  Network IFNAME to get the IP addr (env: ETH_DEV | default: etheqos)\n"
    printf "  -d  Disable the list of banned MODNAME\n"
    printf "  -h  Show this help message\n"
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
    printf "Error: The -i option and corresponding XML file should be provided\n" >&2
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
    printf "[$PROC_NAME] is not running ...\n" >&2
    exit 1
fi

TMPFILE=$(mktemp)
$FIND_MODS_CMD > "$TMPFILE"
while read -r modname; do
    runhalo -n "$modname" &
    wait $!
    if [ $? -eq 0 ]; then
        OK_CNT=$((OK_CNT + 1))
        OK_MODS="$OK_MODS\n$modname"
        printf "[$modname] Executing ... [${GREENC}OK${NC}]\n"
    else
        FAIL_CNT=$((FAIL_CNT + 1))
        FAIL_MODS="$FAIL_MODS\n$modname"
        printf "[$modname] Executing ... [${REDC}FAIL${NC}]\n"
    fi
done < "$TMPFILE"
rm -f "$TMPFILE"

printf "\n========================================\n"
printf "          Summary of Execution\n"
printf "========================================\n"
printf "Executed modules: $OK_MODS\n\n"
printf "Failed modules: $FAIL_MODS\n\n"
printf "Total ok executions: $OK_CNT\n"
printf "Total failed executions: $FAIL_CNT\n"