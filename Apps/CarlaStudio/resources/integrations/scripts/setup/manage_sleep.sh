#!/bin/bash

# Copyright (C) 2026 Abdul, Hashim.
#
# This file is part of CARLA Studio.
# Licensed under the GNU Affero General Public License v3 or later —
# see <LICENSE> at the project root or
# <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
# SPDX-License-Identifier: AGPL-3.0-or-later

set -e

ACTION=${1:-status}

case "$ACTION" in
    disable)
        echo "════════════════════════════════════════════════════════════════"
        echo "Disabling System Sleep..."
        echo "════════════════════════════════════════════════════════════════"
        
        gsettings set org.gnome.desktop.session idle-delay 0 2>/dev/null || \
            echo "⚠ Could not disable GNOME idle delay"
        
        gsettings set org.gnome.desktop.screensaver idle-activation-enabled false 2>/dev/null || \
            echo "⚠ Could not disable GNOME screensaver"
        
        if command -v systemctl &> /dev/null; then
            echo "Attempting to prevent system sleep..."
            sudo systemctl mask sleep.target suspend.target hibernate.target hybrid-sleep.target 2>/dev/null || \
                echo "⚠ Could not mask system sleep targets (requires sudo)"
        fi
        
        if command -v xset &> /dev/null; then
            echo "Disabling screen blanking..."
            xset s off -dpms 2>/dev/null || true
        fi
        
        echo "✓ Sleep prevention settings applied"
        ;;
        
    enable)
        echo "════════════════════════════════════════════════════════════════"
        echo "Enabling System Sleep..."
        echo "════════════════════════════════════════════════════════════════"
        
        gsettings reset org.gnome.desktop.session idle-delay 2>/dev/null || \
            echo "⚠ Could not reset GNOME idle delay"
        
        gsettings reset org.gnome.desktop.screensaver idle-activation-enabled 2>/dev/null || \
            echo "⚠ Could not reset GNOME screensaver"
        
        if command -v systemctl &> /dev/null; then
            echo "Restoring system sleep..."
            sudo systemctl unmask sleep.target suspend.target hibernate.target hybrid-sleep.target 2>/dev/null || \
                echo "⚠ Could not unmask system sleep targets (requires sudo)"
        fi
        
        if command -v xset &> /dev/null; then
            echo "Enabling screen blanking..."
            xset s on +dpms 2>/dev/null || true
        fi
        
        echo "✓ Sleep enabled"
        ;;
        
    status)
        echo "════════════════════════════════════════════════════════════════"
        echo "System Sleep Status"
        echo "════════════════════════════════════════════════════════════════"
        
        idle_delay=$(gsettings get org.gnome.desktop.session idle-delay 2>/dev/null || echo "unknown")
        echo "GNOME Idle Delay: ${idle_delay}s"
        
        screensaver=$(gsettings get org.gnome.desktop.screensaver idle-activation-enabled 2>/dev/null || echo "unknown")
        echo "Screensaver Enabled: ${screensaver}"
        
        if command -v xset &> /dev/null; then
            echo -n "Display Power Management: "
            xset q 2>/dev/null | grep "DPMS is" || echo "unable to determine"
        fi
        
        if command -v acpi &> /dev/null; then
            echo -n "AC Power: "
            acpi -a 2>/dev/null || echo "unable to determine"
        fi
        ;;
        
    *)
        echo "Usage: $(basename "$0") [disable|enable|status]"
        echo ""
        echo "Commands:"
        echo "  disable - Disable all system sleep modes"
        echo "  enable  - Enable system sleep modes"
        echo "  status  - Show current sleep settings"
        echo ""
        echo "Note: Some commands require sudo privileges"
        ;;
esac
