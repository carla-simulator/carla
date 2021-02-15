/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
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
/// @file    LisaSignalProgram.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

import de.dlr.ts.lisum.interfaces.SignalProgramInterface;
import java.util.Objects;

/**
 *
 */
class LisaSignalProgram implements SignalProgramInterface {
    private String name;
    private int objNr;
    private final boolean fz = false;
    private final boolean va = false;


    /**
     *
     * @param name
     * @param objNr
     */
    public LisaSignalProgram(String name, int objNr) {
        this.name = name;
        this.objNr = objNr;
    }

    @Override
    public String getName() {
        return name;
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (getClass() != obj.getClass()) {
            return false;
        }
        final LisaSignalProgram other = (LisaSignalProgram) obj;
        return (this.name == null ? other.name == null : this.name.equals(other.name));
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 59 * hash + Objects.hashCode(this.name);
        return hash;
    }

    @Override
    public void setName(String name) {
        this.name = name;
    }

    @Override
    public void setObjNr(int objNr) {
        this.objNr = objNr;
    }

    @Override
    public int getObjNr() {
        return objNr;
    }

    @Override
    public boolean isVA() {
        return va;
    }

    @Override
    public boolean isFZ() {
        return fz;
    }

}
