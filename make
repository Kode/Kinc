#!/bin/bash
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	`dirname "$0"`/Tools/kmake/kmake-linux64 "$@"
elif [[ "$OSTYPE" == "darwin"* ]]; then
	`dirname "$0"`/Tools/kmake/kmake-osx "$@"
fi
