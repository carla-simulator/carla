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
/// @file    IstVektorType.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

import de.dlr.ts.commons.logger.ToString;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
class IstVektorType {

    private int sammelstoerung;
    private long zeitpunktLetzteAenderung;
    private int betriebsart;
    private int signalProgramm;
    private int knotenEinAus;
    private int sondereingriff;
    private int vaEinAus;
    private int ivEinAus;
    private int oepnvEinAus;
    private int koordinierung;

    public IstVektorType() {
    }

    public IstVektorType(String line) {
        this.parse(line);
    }

    @Override
    public String toString() {
        ToString tos = new ToString("IstVektorType");
        tos.add("sammelstoerung", sammelstoerung);
        tos.add("zeitpunktLetzteAenderung", zeitpunktLetzteAenderung);
        tos.add("betriebsart", betriebsart);
        tos.add("signalProgramm", signalProgramm);
        tos.add("knotenEinAus", knotenEinAus);
        tos.add("sondereingriff", sondereingriff);
        tos.add("vaEinAus", vaEinAus);
        tos.add("ivEinAus", ivEinAus);
        tos.add("oepnvEinAus", oepnvEinAus);
        tos.add("koordinierung", koordinierung);

        return tos.toString();
    }

    /**
     *
     * @param vector
     */
    public final void parse(String vector) {
        vector = vector.replaceAll("\\{", "");
        vector = vector.replaceAll("\\}", "");

        String[] split = vector.split(";");

        sammelstoerung = new Integer(split[0]);
        zeitpunktLetzteAenderung = new Integer(split[1]);
        betriebsart = new Integer(split[2]);
        signalProgramm = new Integer(split[3]);
        knotenEinAus = new Integer(split[4]);
        sondereingriff = new Integer(split[5]);
        vaEinAus = new Integer(split[6]);
        ivEinAus = new Integer(split[7]);
        oepnvEinAus = new Integer(split[8]);
        koordinierung = new Integer(split[9]);
    }

    public int getSammelstoerung() {
        return sammelstoerung;
    }

    public void setSammelstoerung(int sammelstoerung) {
        this.sammelstoerung = sammelstoerung;
    }

    public long getZeitpunktLetzteAenderung() {
        return zeitpunktLetzteAenderung;
    }

    public void setZeitpunktLetzteAenderung(long zeitpunktLetzteAenderung) {
        this.zeitpunktLetzteAenderung = zeitpunktLetzteAenderung;
    }

    public int getBetriebsart() {
        return betriebsart;
    }

    public void setBetriebsart(int betriebsart) {
        this.betriebsart = betriebsart;
    }

    public int getSignalProgramm() {
        return signalProgramm;
    }

    public void setSignalProgramm(int signalProgramm) {
        this.signalProgramm = signalProgramm;
    }

    public int getKnotenEinAus() {
        return knotenEinAus;
    }

    public void setKnotenEinAus(int knotenEinAus) {
        this.knotenEinAus = knotenEinAus;
    }

    public int getSondereingriff() {
        return sondereingriff;
    }

    public void setSondereingriff(int sondereingriff) {
        this.sondereingriff = sondereingriff;
    }

    public int getVaEinAus() {
        return vaEinAus;
    }

    public void setVaEinAus(int vaEinAus) {
        this.vaEinAus = vaEinAus;
    }

    public int getIvEinAus() {
        return ivEinAus;
    }

    public void setIvEinAus(int ivEinAus) {
        this.ivEinAus = ivEinAus;
    }

    public int getOepnvEinAus() {
        return oepnvEinAus;
    }

    public void setOepnvEinAus(int oepnvEinAus) {
        this.oepnvEinAus = oepnvEinAus;
    }

    public int getKoordinierung() {
        return koordinierung;
    }

    public void setKoordinierung(int koordinierung) {
        this.koordinierung = koordinierung;
    }

}
