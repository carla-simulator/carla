//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package org.car2x.veins.base.modules;

// Basic utility module for the whole network.
// Provides utility methods and information used by
// the whole network as well as simulation wide
// black board functionality.
simple BaseWorldUtility
{
    parameters:
        @class(veins::BaseWorldUtility);
        double playgroundSizeX @unit(m);    // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m);    // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m);    // z size of the area the nodes are in (in meters)
        bool   useTorus = default(false);   // use the playground as torus?
        bool   use2D    = default(false);   // use a 2-dimensional world?
        @display("i=misc/globe");
}

