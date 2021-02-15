/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraaS module
// Copyright (C) 2016-2017 Dresden University of Technology
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
/// @file    SubscriptionObject.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.subscription;

import de.tudresden.ws.container.SumoObject;

public class SubscriptionObject {

    public String id;
    public ResponseType response;
    public int domain;
    public String name;
    public int variable;
    public int status;
    public int return_type;
    public SumoObject object;


    //context
    public SubscriptionObject(String id, ResponseType response, int domain, String name, int variable, int status, int return_type, SumoObject object) {
        this.id = id;
        this.response = response;
        this.domain = domain;
        this.name = name;
        this.variable = variable;
        this.status = status;
        this.return_type = return_type;
        this.object = object;
    }

    //variable
    public SubscriptionObject(String id, ResponseType response, int variable, int status, int return_type, SumoObject object) {
        this.id = id;
        this.response = response;
        this.variable = variable;
        this.status = status;
        this.return_type = return_type;
        this.object = object;
    }

}
