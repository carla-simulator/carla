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
/// @file    ConvertHelper.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.util;

/**
 *
 * @author Mario Krumnow
 *
 */

import de.tudresden.ws.container.SumoBoundingBox;
import de.tudresden.ws.container.SumoColor;
import de.tudresden.ws.container.SumoGeometry;
import de.tudresden.ws.container.SumoLinkList;
import de.tudresden.ws.container.SumoPosition2D;
import de.tudresden.ws.container.SumoPosition3D;
import de.tudresden.ws.container.SumoRoadPosition;
import de.tudresden.ws.container.SumoStringList;
import de.tudresden.ws.container.SumoTLSProgram;
import de.tudresden.ws.container.SumoTLSController;
import de.tudresden.ws.container.SumoVehicleData;
import de.tudresden.ws.log.Log;

public class ConvertHelper {

    Log logger;

    public ConvertHelper(Log logger) {
        this.logger = logger;
    }

    public byte getByte(Object obj) {

        byte output = -1;

        try {

            if (obj.getClass().equals(Short.class)) {
                short helpVariable = (short) obj;
                output = (byte) helpVariable;
            }


        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;
    }

    public int getInt(Object obj) {

        int output = -1;

        try {

            if (obj.getClass().equals(Integer.class)) {
                output = (Integer) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;
    }

    public double getDouble(Object obj) {

        double output = -1;

        try {

            if (obj.getClass().equals(Double.class)) {
                output = (Double) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public SumoStringList getStringList(Object obj) {

        SumoStringList output = new SumoStringList();

        try {

            if (obj.getClass().equals(SumoStringList.class)) {
                output = (SumoStringList) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;
    }

    public SumoColor getColor(Object obj) {

        SumoColor output = new SumoColor(0, 0, 0, 0);

        try {

            if (obj.getClass().equals(SumoColor.class)) {
                output = (SumoColor) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public String getString(Object obj) {

        String output = "";

        try {

            if (obj.getClass().equals(String.class)) {
                output = (String) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public SumoPosition2D getPosition2D(Object obj) {

        SumoPosition2D output = new SumoPosition2D(0, 0);

        try {
            if (obj.getClass().equals(SumoPosition2D.class)) {
                output = (SumoPosition2D) obj;
            }
        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public SumoPosition3D getPosition3D(Object obj) {

        SumoPosition3D output = new SumoPosition3D(0, 0, 0);

        try {
            if (obj.getClass().equals(SumoPosition3D.class)) {
                output = (SumoPosition3D) obj;
            }
        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public SumoRoadPosition getRoadPosition(Object obj) {

        SumoRoadPosition output = new SumoRoadPosition();

        try {
            if (obj.getClass().equals(SumoRoadPosition.class)) {
                output = (SumoRoadPosition) obj;
            }
        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public SumoGeometry getPolygon(Object obj) {

        SumoGeometry output = new SumoGeometry();

        try {

            if (obj.getClass().equals(SumoGeometry.class)) {
                output = (SumoGeometry) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public SumoBoundingBox getBoundingBox(Object obj) {

        SumoBoundingBox output = new SumoBoundingBox(0, 0, 0, 0);

        try {

            if (obj.getClass().equals(SumoBoundingBox.class)) {
                output = (SumoBoundingBox) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

    public SumoLinkList getLaneLinks(Object obj) {
        //build an empty list
        SumoLinkList output = new SumoLinkList();

        try {

            if (obj.getClass().equals(SumoLinkList.class)) {
                output = (SumoLinkList) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;
    }

    public SumoVehicleData getVehicleData(Object obj) {

        //build an empty list
        SumoVehicleData output = new SumoVehicleData();

        try {

            if (obj.getClass().equals(SumoVehicleData.class)) {
                output = (SumoVehicleData) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;
    }

    public SumoTLSController getTLSProgram(Object obj) {

        SumoTLSController output = new SumoTLSController();

        try {

            if (obj.getClass().equals(SumoTLSProgram.class)) {
                output = (SumoTLSController) obj;
            }

        } catch (Exception ex) {
            this.logger.write(ex.getStackTrace());
        }

        return output;

    }

}
