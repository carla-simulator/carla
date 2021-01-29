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
/// @file    Gui.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.cmd;
import de.tudresden.sumo.config.Constants;
import de.tudresden.sumo.util.SumoCommand;

/**
 *
 * @author Mario Krumnow
 * @author Evamarie Wiessner
 *
 */

public class Gui {

    //getter methods

    /**
     * Returns the coordinates of the lower left and the upper right corner of
     * the currently visible view.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @return coordinates of the lower left and the upper right corner of the
     *         visible network
     */

    public static SumoCommand getBoundary(String viewID) {
        return new SumoCommand(Constants.CMD_GET_GUI_VARIABLE, Constants.VAR_VIEW_BOUNDARY, viewID, Constants.RESPONSE_GET_GUI_VARIABLE, Constants.TYPE_POLYGON);
    }

    /**
     * Returns the list of available views (open windows).
     *
     * @return list of available views
     */

    public static SumoCommand getIDList() {
        return new SumoCommand(Constants.CMD_GET_GUI_VARIABLE, Constants.TRACI_ID_LIST, "", Constants.RESPONSE_GET_GUI_VARIABLE, Constants.TYPE_STRINGLIST);
    }

    /**
     * Returns the x and y offset of the center of the current view.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @return the x and y offset of the center of the visible network
     */

    public static SumoCommand getOffset(String viewID) {
        return new SumoCommand(Constants.CMD_GET_GUI_VARIABLE, Constants.VAR_VIEW_OFFSET, viewID, Constants.RESPONSE_GET_GUI_VARIABLE, Constants.POSITION_2D);
    }

    /**
     * Returns the name of the current coloring scheme.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @return the visualization scheme (e.g. "standard")
     */

    public static SumoCommand getSchema(String viewID) {
        return new SumoCommand(Constants.CMD_GET_GUI_VARIABLE, Constants.VAR_VIEW_SCHEMA, viewID, Constants.RESPONSE_GET_GUI_VARIABLE, Constants.TYPE_STRING);
    }

    /**
     * Returns the current zoom factor.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @return the current zoom level (in %)
     */

    public static SumoCommand getZoom(String viewID) {
        return new SumoCommand(Constants.CMD_GET_GUI_VARIABLE, Constants.VAR_VIEW_ZOOM, viewID, Constants.RESPONSE_GET_GUI_VARIABLE, Constants.TYPE_DOUBLE);
    }

    //setter methods

    /**
     * Save a screenshot for the given view to the given filename.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @param filename
     *            a string to name the file, the fileformat is guessed from the
     *            extension, the available formats differ from platform to
     *            platform but should at least include ps, svg and pdf, on linux
     *            probably gif, png and jpg as well
     * @return SumoCommand
     */

    public static SumoCommand screenshot(String viewID, String filename) {

        return new SumoCommand(Constants.CMD_SET_GUI_VARIABLE, Constants.VAR_SCREENSHOT, viewID, filename);
    }

    /**
     * Set the current boundary for the given view (see getBoundary()).
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @param xmin
     *            x-coordinate (double) of the lower left corner of the current
     *            view
     * @param ymin
     *            y-coordinate (double) of the lower left corner of the current
     *            view
     * @param xmax
     *            x-coordinate (double) of the upper right corner of the current
     *            view
     * @param ymax
     *            y-coordinate (double) of the upper right corner of the current
     *            view
     * @return SumoCommand
     */

    public static SumoCommand setBoundary(String viewID, double xmin, double ymin, double xmax, double ymax) {

        Object[] array = new Object[] {xmin, ymin, xmax, ymax};
        return new SumoCommand(Constants.CMD_SET_GUI_VARIABLE, Constants.VAR_VIEW_BOUNDARY, viewID, array);
    }

    /**
     * Set the current offset for the given view.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @param x
     *            x-offset of the center of the visible network
     * @param y
     *            y-offset of the center of the visible network
     * @return SumoCommand
     */

    public static SumoCommand setOffset(String viewID, double x, double y) {

        Object[] array = new Object[] {x, y};
        return new SumoCommand(Constants.CMD_SET_GUI_VARIABLE, Constants.VAR_VIEW_OFFSET, viewID, array);
    }

    /**
     * Set the current coloring scheme for the given view.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @param schemeName
     *            the coloring scheme
     * @return SumoCommand
     */

    public static SumoCommand setSchema(String viewID, String schemeName) {
        return new SumoCommand(Constants.CMD_SET_GUI_VARIABLE, Constants.VAR_VIEW_SCHEMA, viewID, schemeName);
    }

    /**
     * Set the current zoom factor for the given view.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @param zoom
     *            desired zoom factor
     * @return SumoCommand
     */

    public static SumoCommand setZoom(String viewID, double zoom) {
        return new SumoCommand(Constants.CMD_SET_GUI_VARIABLE, Constants.VAR_VIEW_ZOOM, viewID, zoom);
    }

    /**
     * Start visually tracking the given vehicle on the given view.
     *
     * @param viewID
     *            a string identifying the view in the simulation GUI
     * @param vehID
     *            a string identifying the vehicle
     * @return SumoCommand
     */

    public static SumoCommand trackVehicle(String viewID, String vehID) {
        return new SumoCommand(Constants.CMD_SET_GUI_VARIABLE, Constants.VAR_TRACK_VEHICLE, viewID, vehID);
    }


}
