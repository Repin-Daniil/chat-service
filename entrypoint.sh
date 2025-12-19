#!/bin/sh
set -e

export CPU_LIMIT="$(nproc).0c"

exec "$@"
