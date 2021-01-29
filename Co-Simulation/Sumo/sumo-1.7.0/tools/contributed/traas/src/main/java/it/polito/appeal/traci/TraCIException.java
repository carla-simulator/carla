/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraCI4J module
// Copyright (C) 2011 ApPeAL Group, Politecnico di Torino
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
/// @file    TraCIException.java
/// @author  Enrico Gueli
/// @author  Mario Krumnow
/// @date    2011
///
//
/****************************************************************************/
package it.polito.appeal.traci;

import java.io.IOException;

@SuppressWarnings("serial")
public class TraCIException extends IOException {

    public TraCIException() {
        super();
    }

    public TraCIException(String msg) {
        super(msg);
    }

    public static class UnexpectedData extends TraCIException {
        public UnexpectedData(String what, Object expected, Object got) {
            super("Unexpected " + what + ": expected " + expected + ", got " + got);
        }
    }

    public static class UnexpectedDatatype extends UnexpectedData {
        public UnexpectedDatatype(int expected, int got) {
            super("datatype", expected, got);
        }
    }

    public static class UnexpectedResponse extends UnexpectedData {
        public UnexpectedResponse(int expected, int got) {
            super("response", expected, got);
        }
    }

}
