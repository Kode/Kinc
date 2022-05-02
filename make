#!/usr/bin/env bash
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	MACHINE_TYPE=`uname -m`
	if [[ "$MACHINE_TYPE" == "armv"* ]]; then
		`dirname "$0"`/Tools/linux_arm/kmake "$@"
	elif [[ "$MACHINE_TYPE" == "aarch64"* ]]; then
		`dirname "$0"`/Tools/linux_arm64/kmake "$@"
	else
		`dirname "$0"`/Tools/linux_x64/kmake "$@"
	fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
	`dirname "$0"`/Tools/macos/kmake "$@"
elif [[ "$OSTYPE" == "FreeBSD"* ]]; then
	`dirname "$0"`/Tools/freebsd_x64/kmake "$@"
fi
