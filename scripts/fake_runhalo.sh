#!/usr/bin/env sh

sleep 1
exit $(($(awk 'BEGIN { srand(); print int(1 + rand() * 100) }') > 25))