#!/usr/bin/env bash
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	MACHINE_TYPE=`uname -m`
	if [[ "$MACHINE_TYPE" == "armv"* ]]; then
		MAKE=`dirname "$0"`/Tools/linux_arm/kmake
	elif [[ "$MACHINE_TYPE" == "aarch64"* ]]; then
		MAKE=`dirname "$0"`/Tools/linux_arm64/kmake
	else
		MAKE=`dirname "$0"`/Tools/linux_x64/kmake
	fi
elif [[ "$OSTYPE" == "darwin"* ]]; then
	MAKE=`dirname "$0"`/Tools/macos/kmake
elif [[ "$OSTYPE" == "FreeBSD"* ]]; then
	MAKE=`dirname "$0"`/Tools/freebsd_x64/kmake
fi

if [ -f "$MAKE" ]; then
	$MAKE "$@"
else 
	echo "kmake was not found, please run the get_dlc script."
fi
