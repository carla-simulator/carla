#!/bin/bash

#
# Copyright (C) 2019 Julian Heinovski <heinovski@ccs-labs.org>
#
# Documentation for these modules is at http://veins.car2x.org/
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# option parsing
export STRICT=0
while test $# -gt 0; do
    case "$1" in
        -h|--help)
            echo "format-code.sh - format code"
            echo " "
            echo "./format-code.sh [options] [path]"
            echo " "
            echo "options:"
            echo "-h, --help                show this text"
            echo "-s, --strict              be more strict about formatting"
            exit 0
            ;;
        -s|--strict)
            export STRICT=1
            shift
            ;;
        *)
            break
            ;;
    esac
done

if [ "$STRICT" != "0" ] ;then

    # check (minimum) version of clang-format
    MINIMUM_VERSION_CF="6.0.0"
    CF=$(clang-format --version)
    if [[ ($? -eq 0) && ($CF =~ ^clang-format) ]]; then
        VERSION_CF=$(echo $CF | sed -n 's/clang-format version \([0-9\.]\+\).*$/\1/p')
        printf "%s\n%s" "$MINIMUM_VERSION_CF" "$VERSION_CF" | sort -C -V
        if [[ $? -eq 1 ]]; then
            >&2 echo "Your version of clang-format is too old!"
            exit 1
        fi
    else
        >&2 echo "Cannot find clang-format or version check does not work!"
        exit 1
    fi

    # actually format the code
    find "$@" \( -name "*.cc" -or -name "*.h" \) -exec clang-format -style=file -i "{}" \;

fi

# check (minimum) version of uncrustify
MINIMUM_VERSION_UF="0.68"
UF=$(uncrustify --version)
if [[ ($? -eq 0) && ($UF =~ ^Uncrustify-) ]]; then
    VERSION_UF=$(echo $UF | sed -n 's/Uncrustify-\([0-9\.]\+\).*$/\1/p')
    printf "%s\n%s" "$MINIMUM_VERSION_UF" "$VERSION_UF" | sort -C -V
    if [[ $? -eq 1 ]]; then
        >&2 echo "Your version of uncrustify is too old!"
        exit 1
    fi
else
    >&2 echo "Cannot find clang-format or version check does not work!"
    exit 1
fi

# actually format the code
find "$@" \( -name "*.cc" -or -name "*.h" \) -exec uncrustify -q --replace --no-backup -c "$(dirname "$0")"/.uncrustify.cfg "{}" \;
