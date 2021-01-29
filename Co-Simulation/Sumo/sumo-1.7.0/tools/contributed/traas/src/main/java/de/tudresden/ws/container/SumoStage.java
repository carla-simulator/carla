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
/// @file    SumoStage.java
/// @author  Jakob Erdmann
/// @date    2019
///
//
/****************************************************************************/
package de.tudresden.ws.container;
import de.tudresden.ws.container.SumoStringList;

/**
 *
 * @author Jakob Erdmann
 *
 */

public class SumoStage implements SumoObject {

    public int type;
    public String vType;
    public String line;
    public String destStop;
    public SumoStringList edges;
    public double travelTime;
    public double cost;
    public double length;
    public String intended;
    public double depart;
    public double departPos;
    public double arrivalPos;
    public String description;

    public SumoStage() {
        edges = new SumoStringList();
    }
}
