#!/usr/bin/env bash
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	MACHINE_TYPE=`uname -m`
	if [[ "$MACHINE_TYPE" == "armv"* ]]; then
		`dirname "$0"`/Tools/kmake/kmake-linuxarm "$@"
	elif [[ "$MACHINE_TYPE" == "aarch64"* ]]; then
		`dirname "$0"`/Tools/kmake/kmake-linuxaarch64 "$@"
	else
		`dirname "$0"`/Tools/kmake/kmake-linux64 "$@"
	fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
	`dirname "$0"`/Tools/kmake/kmake-osx "$@"
elif [[ "$OSTYPE" == "FreeBSD"* ]]; then
	`dirname "$0"`/Tools/kmake/kmake-freebsd "$@"
fi
