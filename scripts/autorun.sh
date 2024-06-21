#!/usr/bin/env sh

PROC_NAME=${PPROC:-ctf_io_server}
if ! pgrep -x "$PROC_NAME" > /dev/null; then
    echo "[${PROC_NAME}] is not running ..." >&2
    exit 1
fi

find_mods | xargs -I {} runhalo -n "{}"