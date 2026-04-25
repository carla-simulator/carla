#!/usr/bin/env bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -e
DEST="${XDG_DATA_HOME:-$HOME/.local/share}"
cp -r icons "$DEST/"
cp carla-studio.desktop "$DEST/applications/" 2>/dev/null || mkdir -p "$DEST/applications" && cp carla-studio.desktop "$DEST/applications/"
gtk-update-icon-cache -f -t "$DEST/icons/hicolor" 2>/dev/null || true
update-desktop-database "$DEST/applications" 2>/dev/null || true
echo "Installed Carla Studio."
