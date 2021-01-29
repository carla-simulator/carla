/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraaS module
// Copyright (C) 2016-2019 Dresden University of Technology
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    SumoRoadPosition.java
/// @author  Jakob Erdmann
/// @date    2019
///
//
/****************************************************************************/
package de.tudresden.ws.container;

/**
 *
 * @author Jakob Erdmann
 *
 */

public class SumoRoadPosition implements SumoObject {

    public String edgeID;
    public double pos;
    public int laneIndex;

    public SumoRoadPosition() { }
}
