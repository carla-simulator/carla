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
/// @file    ControlUnitInterface.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.interfaces;

import de.dlr.ts.lisum.enums.LightColor;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public interface ControlUnitInterface {
    String getFullName();

    int signalGroupsCount();

    LightColor getLightColor(int signalGroupIndex);
    LightColor getLightColor(String signalGroupName);

    SignalProgramInterface[] getSignalPrograms();
    void setCurrentSignalProgram(int index);
    SignalProgramInterface getCurrentSignalProgram();

    DetectorInterface getDetector(String name);

    boolean isVA();
    void setVA(boolean va);

    boolean isIV();
    void setIV(boolean iv);

    boolean isOV();
    void setOV(boolean ov);

    int getEbene();
    void setEbene(int ebene);

    int getKnotenEinAus();
    void setKnotenEinAus(int einAus);

    void storePersistent();

    void setEnabled(boolean enabled);
    boolean isEnabled();

    void setCoordinated(int coordinated);
    int getCoordinated();

}
