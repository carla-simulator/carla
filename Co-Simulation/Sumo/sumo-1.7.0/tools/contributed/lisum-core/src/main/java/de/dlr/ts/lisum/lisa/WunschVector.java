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
/// @file    WunschVector.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
class WunschVector {
    private int ebene = 4;
    private int betriebsArt = 1;
    //private int signalProgram = 1;
    private int knotenEinAus = 0;

    private int vaEinAus = 0;
    private int ivEinAus = 0;
    private int oepnvEinAus = 0;

    private int coordinated = 0;

    private final LisaSignalPrograms signalPrograms;


    //1;1;0;2;1;0;0;0
    public String getVector() {
        int signalProg = 0;
        if (signalPrograms.getCurrentSignalProgram() != null) {
            signalProg = signalPrograms.getCurrentSignalProgram().getObjNr();
        }

        StringBuilder sb = new StringBuilder();
        //sb.append("{");
        sb.append(ebene).append(";");
        sb.append(betriebsArt).append(";");
        sb.append(signalProg).append(";");
        sb.append(knotenEinAus).append(";");
        sb.append(vaEinAus).append(";");
        sb.append(ivEinAus).append(";");
        sb.append(oepnvEinAus).append(";");
        sb.append(coordinated);
        //sb.append("}");

        return sb.toString();
    }

    /**
     *
     * @param signalPrograms
     */
    public WunschVector(LisaSignalPrograms signalPrograms) {
        this.signalPrograms = signalPrograms;
    }

    /**
     *
     * @return
     */
    public boolean isVA() {
        return this.vaEinAus == 2;
    }

    public void setVA(boolean va) {
        this.vaEinAus = va ? 2 : 1;
    }

    public boolean isIV() {
        return this.ivEinAus == 2;
    }

    public void setIV(boolean iv) {
        this.ivEinAus = iv ? 2 : 1;
    }

    public boolean isOV() {
        return this.oepnvEinAus == 2;
    }

    public void setOV(boolean ov) {
        this.oepnvEinAus = ov ? 2 : 1;
    }

    public void setEbene(int ebene) {
        this.ebene = ebene;
    }

    public int getEbene() {
        return ebene;
    }

    /**
     * 0 – keiner
     * 1 – Ein
     * 2 – AusDefault
     * 3 – AusBlinkenNR
     * 4 – AusDunkel
     * 5 – AusBlinkenAlle
     *
     * @param onOff
     */
    public void setKnotenEinAus(int onOff) {
        this.knotenEinAus = onOff;
    }

    /**
     *
     * @return
     */
    public int getKnotenEinAus() {
        return knotenEinAus;
    }

    /**
     *
     * @return
     */
    public int getBetriebsArt() {
        return betriebsArt;
    }

    public void setBetriebsArt(int betriebsArt) {
        this.betriebsArt = betriebsArt;
    }

    public String getSignalProgram() {
        return signalPrograms.getCurrentSignalProgram().getName();
    }

    public int getSignalProgramIndex() {
        return signalPrograms.getCurrentSignalProgramIndex();
    }

    public int getCoordinated() {
        return coordinated;
    }

    public void setCoordinated(int coordinated) {
        this.coordinated = coordinated;
    }
}
