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
/// @file    LisaSignalPrograms.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

import de.dlr.ts.lisum.interfaces.SignalProgramInterface;
import java.util.ArrayList;
import java.util.List;



/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
class LisaSignalPrograms {
    private final List<LisaSignalProgram> signalPrograms = new ArrayList<>();
    private LisaSignalProgram currentSignalProgram = null;


    /**
     *
     */
    public LisaSignalPrograms() {
    }

    /**
     *
     * @param signalProgramName

    public void setCurrentSignalProgram(String signalProgramName)
    {
        this.currentSignalProgram = get(signalProgramName);
    }
    * */

    /**
     *
     * @param signalProgramNameIndex
     */
    public void setCurrentSignalProgram(int signalProgramNameIndex) {
        this.currentSignalProgram = signalPrograms.get(signalProgramNameIndex);
    }

    /**
     *
     * @param name
     * @return
     */
    public LisaSignalProgram get(String name) {
        if (name == null) {
            return null;
        }

        for (LisaSignalProgram signalProgram : signalPrograms)
            if (signalProgram.getName().equals(name)) {
                return signalProgram;
            }

        return null;
    }

    /**
     *
     * @param index
     * @return
     */
    public LisaSignalProgram get(int index) {
        if (index >= signalPrograms.size() || index < 0) {
            return null;
        }

        return signalPrograms.get(index);
    }

    /**
     *
     * @return
     */
    public LisaSignalProgram getCurrentSignalProgram() {
        return currentSignalProgram;
    }

    /**
     *
     * @return
     */
    public int getCurrentSignalProgramIndex() {
        return signalPrograms.indexOf(currentSignalProgram);
    }

    /**
     *
     * @return
     */
    public SignalProgramInterface[] getSignalProgramsArray() {
        return signalPrograms.toArray(new SignalProgramInterface[signalPrograms.size()]);
    }

    /**
     *
     * @param controlUnitConf
     */
    public void load(LisaConfigurationFiles.ControlUnit controlUnitConf) {
        LisaSignalProgram sp = new LisaSignalProgram("Off", 0);
        signalPrograms.add(sp);

        for (LisaConfigurationFiles.ControlUnit.SignalProgram signalProgram : controlUnitConf.signalPrograms) {
            sp = new LisaSignalProgram(signalProgram.bezeichnung, signalProgram.objNr);
            signalPrograms.add(sp);
        }
    }
}
