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
/// @file    Traci.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.ws;

import javax.jws.WebMethod;
import javax.jws.WebParam;

import de.tudresden.sumo.cmd.Edge;
import de.tudresden.sumo.cmd.Gui;
import de.tudresden.sumo.cmd.Inductionloop;
import de.tudresden.sumo.cmd.Junction;
import de.tudresden.sumo.cmd.Lanearea;
import de.tudresden.sumo.cmd.Lane;
import de.tudresden.sumo.cmd.Multientryexit;
import de.tudresden.sumo.cmd.Person;
import de.tudresden.sumo.cmd.Poi;
import de.tudresden.sumo.cmd.Polygon;
import de.tudresden.sumo.cmd.Route;
import de.tudresden.sumo.cmd.Simulation;
import de.tudresden.sumo.cmd.Trafficlight;
import de.tudresden.sumo.cmd.Vehicle;
import de.tudresden.sumo.cmd.Vehicletype;
import de.tudresden.ws.container.SumoBoundingBox;
import de.tudresden.ws.container.SumoColor;
import de.tudresden.ws.container.SumoGeometry;
import de.tudresden.ws.container.SumoLinkList;
import de.tudresden.ws.container.SumoPosition2D;
import de.tudresden.ws.container.SumoPosition3D;
import de.tudresden.ws.container.SumoRoadPosition;
import de.tudresden.ws.container.SumoStopFlags;
import de.tudresden.ws.container.SumoStringList;
import de.tudresden.ws.container.SumoTLSProgram;
import de.tudresden.ws.container.SumoTLSController;
import de.tudresden.ws.container.SumoVehicleData;
import de.tudresden.ws.log.Log;
import de.tudresden.sumo.util.ConvertHelper;
import de.tudresden.sumo.util.Sumo;

/**
 *
 * @author Mario Krumnow
 *
 */

public class Traci {

    Sumo sumo;
    Log logger;
    ConvertHelper helper;

    public void init(Sumo sumo, Log logger, ConvertHelper helper) {
        this.sumo = sumo;
        this.logger = logger;
        this.helper = helper;
    }

    /*
     * Setter methods
     */

    @WebMethod(action = "Edge: adapt the Traveltime")
    public void Edge_adaptTraveltime(@WebParam(name = "edgeID") String edgeID, @WebParam(name = "time") int time) {
        this.sumo.set_cmd(Edge.adaptTraveltime(edgeID, time));
    }

    @WebMethod(action = "Edge: setEffort")
    public void Edge_setEffort(@WebParam(name = "edgeID") String edgeID, @WebParam(name = "effort") double effort) {
        this.sumo.set_cmd(Edge.setEffort(edgeID, effort));
    }

    @WebMethod(action = "Edge: Set a new maximum speed (in m/s) for all lanes of the edge..")
    public void Edge_setMaxSpeed(@WebParam(name = "edgeID") String edgeID, @WebParam(name = "speed") double speed) {
        this.sumo.set_cmd(Edge.setMaxSpeed(edgeID, speed));
    }

    @WebMethod(action = "Vehicle: add")
    public void Vehicle_add(@WebParam(name = "vehID") String vehID, @WebParam(name = "typeID") String typeID, @WebParam(name = "routeID") String routeID, @WebParam(name = "depart") int depart, @WebParam(name = "pos") double pos, @WebParam(name = "speed") double speed, @WebParam(name = "lane") byte lane) {
        this.sumo.set_cmd(Vehicle.add(vehID, typeID, routeID, depart, pos, speed, lane));
    }

    @WebMethod(action = "Vehicle: changeLane")
    public void Vehicle_changeLane(@WebParam(name = "vehID") String vehID, @WebParam(name = "laneIndex") byte laneIndex, @WebParam(name = "duration") double duration) {
        this.sumo.set_cmd(Vehicle.changeLane(vehID, laneIndex, duration));
    }

    @WebMethod(action = "Vehicle: changeTarget")
    public void Vehicle_changeTarget(@WebParam(name = "vehID") String vehID, @WebParam(name = "edgeID") String edgeID) {
        this.sumo.set_cmd(Vehicle.changeTarget(vehID, edgeID));
    }

    @WebMethod(action = "Vehicle: moveTo")
    public void Vehicle_moveTo(@WebParam(name = "vehID") String vehID, @WebParam(name = "laneID") String laneID, @WebParam(name = "pos") double pos) {
        this.sumo.set_cmd(Vehicle.moveTo(vehID, laneID, pos));
    }

    @WebMethod(action = "Vehicle: Remove vehicle with the given ID for the give reason.  Reasons are defined in module constants and start with REMOVE_")
    public void Vehicle_remove(@WebParam(name = "vehID") String vehID, @WebParam(name = "reason") byte reason) {
        this.sumo.set_cmd(Vehicle.remove(vehID, reason));
    }

    @WebMethod(action = "Vehicle: rerouteEffort")
    public void Vehicle_rerouteEffort(@WebParam(name = "vehID") String vehID) {
        this.sumo.set_cmd(Vehicle.rerouteEffort(vehID));
    }

    @WebMethod(action = "Vehicle: rerouteTraveltime")
    public void Vehicle_rerouteTraveltime(@WebParam(name = "vehID") String vehID) {
        this.sumo.set_cmd(Vehicle.rerouteTraveltime(vehID));
    }

    @WebMethod(action = "Vehicle: setAccel")
    public void Vehicle_setAccel(@WebParam(name = "vehID") String vehID, @WebParam(name = "accel") double accel) {
        this.sumo.set_cmd(Vehicle.setAccel(vehID, accel));
    }

    @WebMethod(action = "Vehicle: setAdaptedTraveltime")
    public void Vehicle_setAdaptedTraveltime(@WebParam(name = "vehID") String vehID, @WebParam(name = "begTime") int begTime, @WebParam(name = "endTime") int endTime, @WebParam(name = "edgeID") String edgeID, @WebParam(name = "time") double time) {
        this.sumo.set_cmd(Vehicle.setAdaptedTraveltime(vehID, begTime, endTime, edgeID, time));
    }

    @WebMethod(action = "Vehicle: setColor")
    public void Vehicle_setColor(@WebParam(name = "vehID") String vehID, @WebParam(name = "color") SumoColor color) {
        this.sumo.set_cmd(Vehicle.setColor(vehID, color));
    }

    @WebMethod(action = "Vehicle: setDecel")
    public void Vehicle_setDecel(@WebParam(name = "vehID") String vehID, @WebParam(name = "decel") double decel) {
        this.sumo.set_cmd(Vehicle.setDecel(vehID, decel));
    }

    @WebMethod(action = "Vehicle: setEffort")
    public void Vehicle_setEffort(@WebParam(name = "vehID") String vehID, @WebParam(name = "begTime") int begTime, @WebParam(name = "endTime") int endTime, @WebParam(name = "edgeID") String edgeID, @WebParam(name = "effort") double effort) {
        this.sumo.set_cmd(Vehicle.setEffort(vehID, begTime, endTime, edgeID, effort));
    }

    @WebMethod(action = "Vehicle: setEmissionClass")
    public void Vehicle_setEmissionClass(@WebParam(name = "vehID") String vehID, @WebParam(name = "clazz") String clazz) {
        this.sumo.set_cmd(Vehicle.setEmissionClass(vehID, clazz));
    }

    @WebMethod(action = "Vehicle: setImperfection")
    public void Vehicle_setImperfection(@WebParam(name = "vehID") String vehID, @WebParam(name = "imperfection") double imperfection) {
        this.sumo.set_cmd(Vehicle.setImperfection(vehID, imperfection));
    }

    @WebMethod(action = "Vehicle: setLength")
    public void Vehicle_setLength(@WebParam(name = "vehID") String vehID, @WebParam(name = "length") double length) {
        this.sumo.set_cmd(Vehicle.setLength(vehID, length));
    }

    @WebMethod(action = "Vehicle: setMaxSpeed")
    public void Vehicle_setMaxSpeed(@WebParam(name = "vehID") String vehID, @WebParam(name = "speed") double speed) {
        this.sumo.set_cmd(Vehicle.setMaxSpeed(vehID, speed));
    }

    @WebMethod(action = "Vehicle: setMinGap")
    public void Vehicle_setMinGap(@WebParam(name = "vehID") String vehID, @WebParam(name = "minGap") double minGap) {
        this.sumo.set_cmd(Vehicle.setMinGap(vehID, minGap));
    }

    @WebMethod(action = "Vehicle: setRouteID")
    public void Vehicle_setRouteID(@WebParam(name = "vehID") String vehID, @WebParam(name = "routeID") String routeID) {
        this.sumo.set_cmd(Vehicle.setRouteID(vehID, routeID));
    }

    @WebMethod(action = "Vehicle: setShapeClass")
    public void Vehicle_setShapeClass(@WebParam(name = "vehID") String vehID, @WebParam(name = "clazz") String clazz) {
        this.sumo.set_cmd(Vehicle.setShapeClass(vehID, clazz));
    }

    @WebMethod(action = "Vehicle: setSignals")
    public void Vehicle_setSignals(@WebParam(name = "vehID") String vehID, @WebParam(name = "signals") int signals) {
        this.sumo.set_cmd(Vehicle.setSignals(vehID, signals));
    }

    @WebMethod(action = "Vehicle: setSpeed")
    public void Vehicle_setSpeed(@WebParam(name = "vehID") String vehID, @WebParam(name = "speed") double speed) {
        this.sumo.set_cmd(Vehicle.setSpeed(vehID, speed));
    }

    @WebMethod(action = "Vehicle: setSpeedDeviation")
    public void Vehicle_setSpeedDeviation(@WebParam(name = "vehID") String vehID, @WebParam(name = "deviation") double deviation) {
        this.sumo.set_cmd(Vehicle.setSpeedDeviation(vehID, deviation));
    }

    @WebMethod(action = "Vehicle: setSpeedFactor")
    public void Vehicle_setSpeedFactor(@WebParam(name = "vehID") String vehID, @WebParam(name = "factor") double factor) {
        this.sumo.set_cmd(Vehicle.setSpeedFactor(vehID, factor));
    }

    @WebMethod(action="Vehicle: setStop")
    public void Vehicle_setStop(@WebParam(name = "vehID") String vehID, 
            @WebParam(name = "edgeID") String edgeID, 
            @WebParam(name = "pos") double pos, 
            @WebParam(name = "laneIndex") byte laneIndex, 
            @WebParam(name = "duration") double duration,
            @WebParam(name = "stopType") SumoStopFlags stopType,
            @WebParam(name = "startPos") double startPos,
            @WebParam(name = "until") double until){
        this.sumo.set_cmd(Vehicle.setStop(vehID, edgeID, pos, laneIndex, duration,stopType, startPos, until));
    }

    @WebMethod(action = "Vehicle: setResume")
    public void Vehicle_resume(@WebParam(name = "vehID") String vehID) {
        this.sumo.set_cmd(Vehicle.resume(vehID));
    }

    @WebMethod(action = "Vehicle: setTau")
    public void Vehicle_setTau(@WebParam(name = "vehID") String vehID, @WebParam(name = "tau") double tau) {
        this.sumo.set_cmd(Vehicle.setTau(vehID, tau));
    }

    @WebMethod(action = "Vehicle: setVehicleClass")
    public void Vehicle_setVehicleClass(@WebParam(name = "vehID") String vehID, @WebParam(name = "clazz") String clazz) {
        this.sumo.set_cmd(Vehicle.setVehicleClass(vehID, clazz));
    }

    @WebMethod(action = "Vehicle: setWidth")
    public void Vehicle_setWidth(@WebParam(name = "vehID") String vehID, @WebParam(name = "width") double width) {
        this.sumo.set_cmd(Vehicle.setWidth(vehID, width));
    }

    @WebMethod(action = "Trafficlights: setCompleteRedYellowGreenDefinition")
    public void Trafficlights_setCompleteRedYellowGreenDefinition(@WebParam(name = "tlsID") String tlsID, @WebParam(name = "tls") SumoTLSProgram tls) {
        this.sumo.set_cmd(Trafficlight.setCompleteRedYellowGreenDefinition(tlsID, tls));
    }

    @WebMethod(action = "Trafficlights: setPhase")
    public void Trafficlights_setPhase(@WebParam(name = "tlsID") String tlsID, @WebParam(name = "index") int index) {
        this.sumo.set_cmd(Trafficlight.setPhase(tlsID, index));
    }

    @WebMethod(action = "Trafficlights: setPhaseDuration")
    public void Trafficlights_setPhaseDuration(@WebParam(name = "tlsID") String tlsID, @WebParam(name = "phaseDuration") int phaseDuration) {
        this.sumo.set_cmd(Trafficlight.setPhaseDuration(tlsID, phaseDuration));
    }

    @WebMethod(action = "Trafficlights: setProgram")
    public void Trafficlights_setProgram(@WebParam(name = "tlsID") String tlsID, @WebParam(name = "programID") String programID) {
        this.sumo.set_cmd(Trafficlight.setProgram(tlsID, programID));
    }

    @WebMethod(action = "Trafficlights: setRedYellowGreenState")
    public void Trafficlights_setRedYellowGreenState(@WebParam(name = "tlsID") String tlsID, @WebParam(name = "state") String state) {
        this.sumo.set_cmd(Trafficlight.setRedYellowGreenState(tlsID, state));
    }

    @WebMethod(action = "Vehicletype: setAccel")
    public void Vehicletype_setAccel(@WebParam(name = "typeID") String typeID, @WebParam(name = "accel") double accel) {
        this.sumo.set_cmd(Vehicletype.setAccel(typeID, accel));
    }

    @WebMethod(action = "Vehicletype: setColor")
    public void Vehicletype_setColor(@WebParam(name = "typeID") String typeID, @WebParam(name = "color") SumoColor color) {
        this.sumo.set_cmd(Vehicletype.setColor(typeID, color));
    }

    @WebMethod(action = "Vehicletype: setDecel")
    public void Vehicletype_setDecel(@WebParam(name = "typeID") String typeID, @WebParam(name = "decel") double decel) {
        this.sumo.set_cmd(Vehicletype.setDecel(typeID, decel));
    }

    @WebMethod(action = "Vehicletype: setEmissionClass")
    public void Vehicletype_setEmissionClass(@WebParam(name = "typeID") String typeID, @WebParam(name = "clazz") String clazz) {
        this.sumo.set_cmd(Vehicletype.setEmissionClass(typeID, clazz));
    }

    @WebMethod(action = "Vehicletype: setImperfection")
    public void Vehicletype_setImperfection(@WebParam(name = "typeID") String typeID, @WebParam(name = "imperfection") double imperfection) {
        this.sumo.set_cmd(Vehicletype.setImperfection(typeID, imperfection));
    }

    @WebMethod(action = "Vehicletype: setLength")
    public void Vehicletype_setLength(@WebParam(name = "typeID") String typeID, @WebParam(name = "length") double length) {
        this.sumo.set_cmd(Vehicletype.setLength(typeID, length));
    }

    @WebMethod(action = "Vehicletype: setMaxSpeed")
    public void Vehicletype_setMaxSpeed(@WebParam(name = "typeID") String typeID, @WebParam(name = "speed") double speed) {
        this.sumo.set_cmd(Vehicletype.setMaxSpeed(typeID, speed));
    }

    @WebMethod(action = "Vehicletype: setMinGap")
    public void Vehicletype_setMinGap(@WebParam(name = "typeID") String typeID, @WebParam(name = "minGap") double minGap) {
        this.sumo.set_cmd(Vehicletype.setMinGap(typeID, minGap));
    }

    @WebMethod(action = "Vehicletype: setShapeClass")
    public void Vehicletype_setShapeClass(@WebParam(name = "typeID") String typeID, @WebParam(name = "clazz") String clazz) {
        this.sumo.set_cmd(Vehicletype.setShapeClass(typeID, clazz));
    }

    @WebMethod(action = "Vehicletype: setSpeedDeviation")
    public void Vehicletype_setSpeedDeviation(@WebParam(name = "typeID") String typeID, @WebParam(name = "deviation") double deviation) {
        this.sumo.set_cmd(Vehicletype.setSpeedDeviation(typeID, deviation));
    }

    @WebMethod(action = "Vehicletype: setSpeedFactor")
    public void Vehicletype_setSpeedFactor(@WebParam(name = "typeID") String typeID, @WebParam(name = "factor") double factor) {
        this.sumo.set_cmd(Vehicletype.setSpeedFactor(typeID, factor));
    }

    @WebMethod(action = "Vehicletype: setTau")
    public void Vehicletype_setTau(@WebParam(name = "typeID") String typeID, @WebParam(name = "tau") double tau) {
        this.sumo.set_cmd(Vehicletype.setTau(typeID, tau));
    }

    @WebMethod(action = "Vehicletype: setVehicleClass")
    public void Vehicletype_setVehicleClass(@WebParam(name = "typeID") String typeID, @WebParam(name = "clazz") String clazz) {
        this.sumo.set_cmd(Vehicletype.setVehicleClass(typeID, clazz));
    }

    @WebMethod(action = "Vehicletype: setWidth")
    public void Vehicletype_setWidth(@WebParam(name = "typeID") String typeID, @WebParam(name = "width") double width) {
        this.sumo.set_cmd(Vehicletype.setWidth(typeID, width));
    }

    @WebMethod(action = "Lane: setAllowed")
    public void Lane_setAllowed(@WebParam(name = "laneID") String laneID, @WebParam(name = "allowedClasses") SumoStringList allowedClasses) {
        this.sumo.set_cmd(Lane.setAllowed(laneID, allowedClasses));
    }

    @WebMethod(action = "Lane: setDisallowed")
    public void Lane_setDisallowed(@WebParam(name = "laneID") String laneID, @WebParam(name = "disallowedClasses") SumoStringList disallowedClasses) {
        this.sumo.set_cmd(Lane.setDisallowed(laneID, disallowedClasses));
    }

    @WebMethod(action = "Lane: setLength")
    public void Lane_setLength(@WebParam(name = "laneID") String laneID, @WebParam(name = "length") double length) {
        this.sumo.set_cmd(Lane.setLength(laneID, length));
    }

    @WebMethod(action = "Lane: setMaxSpeed")
    public void Lane_setMaxSpeed(@WebParam(name = "laneID") String laneID, @WebParam(name = "speed") double speed) {
        this.sumo.set_cmd(Lane.setMaxSpeed(laneID, speed));
    }

    @WebMethod(action = "Polygon: add")
    public void Polygon_add(@WebParam(name = "polygonID") String polygonID, @WebParam(name = "shape") SumoGeometry shape, @WebParam(name = "color") SumoColor color, @WebParam(name = "fill") boolean fill, @WebParam(name = "polygonType") String polygonType, @WebParam(name = "layer") int layer) {
        this.sumo.set_cmd(Polygon.add(polygonID, shape, color, fill, polygonType, layer));
    }

    @WebMethod(action = "Polygon: remove")
    public void Polygon_remove(@WebParam(name = "polygonID") String polygonID, @WebParam(name = "layer") int layer) {
        this.sumo.set_cmd(Polygon.remove(polygonID, layer));
    }

    @WebMethod(action = "Polygon: setColor")
    public void Polygon_setColor(@WebParam(name = "polygonID") String polygonID, @WebParam(name = "color") SumoColor color) {
        this.sumo.set_cmd(Polygon.setColor(polygonID, color));
    }

    @WebMethod(action = "Polygon: setShape")
    public void Polygon_setShape(@WebParam(name = "polygonID") String polygonID, @WebParam(name = "shape") SumoStringList shape) {
        this.sumo.set_cmd(Polygon.setShape(polygonID, shape));
    }

    @WebMethod(action = "Polygon: setType")
    public void Polygon_setType(@WebParam(name = "polygonID") String polygonID, @WebParam(name = "polygonType") String polygonType) {
        this.sumo.set_cmd(Polygon.setType(polygonID, polygonType));
    }

    @WebMethod(action = "Poi: remove")
    public void Poi_remove(@WebParam(name = "poiID") String poiID, @WebParam(name = "layer") int layer) {
        this.sumo.set_cmd(Poi.remove(poiID, layer));
    }

    @WebMethod(action = "Poi: setColor")
    public void Poi_setColor(@WebParam(name = "poiID") String poiID, @WebParam(name = "color") SumoColor color) {
        this.sumo.set_cmd(Poi.setColor(poiID, color));
    }

    @WebMethod(action = "Poi: setPosition")
    public void Poi_setPosition(@WebParam(name = "poiID") String poiID, @WebParam(name = "x") double x, @WebParam(name = "y") double y) {
        this.sumo.set_cmd(Poi.setPosition(poiID, x, y));
    }

    @WebMethod(action = "Poi: setType")
    public void Poi_setType(@WebParam(name = "poiID") String poiID, @WebParam(name = "poiType") String poiType) {
        this.sumo.set_cmd(Poi.setType(poiID, poiType));
    }

    @WebMethod(action = "Gui: screenshot")
    public void GUI_screenshot(@WebParam(name = "viewID") String viewID, @WebParam(name = "filename") String filename) {
        this.sumo.set_cmd(Gui.screenshot(viewID, filename));
    }

    @WebMethod(action = "Gui: Set the current boundary for the given view (see getBoundary()).")
    public void GUI_setBoundary(@WebParam(name = "viewID") String viewID, @WebParam(name = "xmin") double xmin, @WebParam(name = "ymin") double ymin, @WebParam(name = "xmax") double xmax, @WebParam(name = "ymax") double ymax) {
        this.sumo.set_cmd(Gui.setBoundary(viewID, xmin, ymin, xmax, ymax));
    }

    @WebMethod(action = "Gui: Set the current offset for the given view.")
    public void GUI_setOffset(@WebParam(name = "viewID") String viewID, @WebParam(name = "x") double x, @WebParam(name = "y") double y) {
        this.sumo.set_cmd(Gui.setOffset(viewID, x, y));
    }

    @WebMethod(action = "Gui: Set the current coloring scheme for the given view.")
    public void GUI_setSchema(@WebParam(name = "viewID") String viewID, @WebParam(name = "schemeName") String schemeName) {
        this.sumo.set_cmd(Gui.setSchema(viewID, schemeName));
    }

    @WebMethod(action = "Gui: Set the current zoom factor for the given view.")
    public void GUI_setZoom(@WebParam(name = "viewID") String viewID, @WebParam(name = "zoom") double zoom) {
        this.sumo.set_cmd(Gui.setZoom(viewID, zoom));
    }

    @WebMethod(action = "Gui: trackVehicle")
    public void GUI_trackVehicle(@WebParam(name = "viewID") String viewID, @WebParam(name = "vehID") String vehID) {
        this.sumo.set_cmd(Gui.trackVehicle(viewID, vehID));
    }

    @WebMethod(action = "Route: add")
    public void Route_add(@WebParam(name = "routeID") String routeID, @WebParam(name = "edges") SumoStringList edges) {
        this.sumo.set_cmd(Route.add(routeID, edges));
    }

    /*
     * Getter methods
     */

    @WebMethod(action = "Multientryexit: Returns a list of all e3 detectors in the network.")
    public SumoStringList Multientryexit_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Multientryexit.getIDList()));
    }

    @WebMethod(action = "Multientryexit: Returns the number of all e3 detectors in the network.")
    public int Multientryexit_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Multientryexit.getIDCount()));
    }

    @WebMethod(action = "Multientryexit: Returns the subscription results for the last time step and the given detector.")
    public int Multientryexit_getLastStepHaltingNumber(@WebParam(name = "detID") String detID) {
        return this.helper.getInt(this.sumo.get_cmd(Multientryexit.getLastStepHaltingNumber(detID)));
    }

    @WebMethod(action = "Multientryexit: getLastStepMeanSpeed")
    public double Multientryexit_getLastStepMeanSpeed(@WebParam(name = "detID") String detID) {
        return this.helper.getDouble(this.sumo.get_cmd(Multientryexit.getLastStepMeanSpeed(detID)));
    }

    @WebMethod(action = "Multientryexit: getLastStepVehicleIDs")
    public SumoStringList Multientryexit_getLastStepVehicleIDs(@WebParam(name = "detID") String detID) {
        return this.helper.getStringList(this.sumo.get_cmd(Multientryexit.getLastStepVehicleIDs(detID)));
    }

    @WebMethod(action = "Multientryexit: getLastStepVehicleNumber")
    public int Multientryexit_getLastStepVehicleNumber(@WebParam(name = "detID") String detID) {
        return this.helper.getInt(this.sumo.get_cmd(Multientryexit.getLastStepVehicleNumber(detID)));
    }

    @WebMethod(action = "Edge: Returns the travel time value (in s) used for (re-)routing")
    public double Edge_getAdaptedTraveltime(@WebParam(name = "edgeID") String edgeID, @WebParam(name = "time") int time) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getAdaptedTraveltime(edgeID, time)));
    }

    @WebMethod(action = "Edge: Returns the CO2 emission in mg for the last time step on the given edge.")
    public double Edge_getCO2Emission(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getCO2Emission(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the CO emission in mg for the last time step on the given edge.")
    public double Edge_getCOEmission(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getCOEmission(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the effort value used for (re-)routing")
    public double Edge_getEffort(@WebParam(name = "edgeID") String edgeID, @WebParam(name = "time") int time) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getEffort(edgeID, time)));
    }

    @WebMethod(action = "Edge: getElectricityConsumption")
    public double Edge_getElectricityConsumption(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getElectricityConsumption(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the fuel consumption in ml for the last time step on the given edge.")
    public double Edge_getFuelConsumption(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getFuelConsumption(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the HC emission in mg for the last time step on the given edge.")
    public double Edge_getHCEmission(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getHCEmission(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the number of edges in the network.")
    public int Edge_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Edge.getIDCount()));
    }

    @WebMethod(action = "Edge: Returns a list of all edges in the network.")
    public SumoStringList Edge_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Edge.getIDList()));
    }

    @WebMethod(action = "Edge: Returns the total number of halting vehicles for the last time step on the given edge.")
    public int Edge_getLastStepHaltingNumber(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getInt(this.sumo.get_cmd(Edge.getLastStepHaltingNumber(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the total vehicle length in m for the last time step on the given edge.")
    public double Edge_getLastStepLength(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getLastStepLength(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the average speed in m/s for the last time step on the given edge.")
    public double Edge_getLastStepMeanSpeed(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getLastStepMeanSpeed(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the occupancy in % for the last time step on the given edge.")
    public double Edge_getLastStepOccupancy(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getLastStepOccupancy(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the subscription results for the last time step and the given edge.")
    public SumoStringList Edge_getLastStepVehicleIDs(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getStringList(this.sumo.get_cmd(Edge.getLastStepVehicleIDs(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the total number of vehicles for the last time step on the given edge.")
    public int Edge_getLastStepVehicleNumber(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getInt(this.sumo.get_cmd(Edge.getLastStepVehicleNumber(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the NOx emission in mg for the last time step on the given edge.")
    public double Edge_getNOxEmission(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getNOxEmission(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the noise emission in db for the last time step on the given edge.")
    public double Edge_getNoiseEmission(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getNoiseEmission(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the particular matter emission in mg for the last time step on the given edge.")
    public double Edge_getPMxEmission(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getPMxEmission(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the estimated travel time in s for the last time step on the given edge.")
    public double Edge_getTraveltime(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getTraveltime(edgeID)));
    }

    @WebMethod(action = "Edge: Returns the waiting time in s for the last time step on the given edge.")
    public double Edge_getWaitingTime(@WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Edge.getWaitingTime(edgeID)));
    }


    @WebMethod(action = "Lanearea: getIDList")
    public SumoStringList Lanearea_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Lanearea.getIDList()));
    }

    @WebMethod(action = "Lanearea: getIDCount")
    public int Lanearea_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Lanearea.getIDCount()));
    }

    @WebMethod(action = "Lanearea: getJamLengthVehicle")
    public int Lanearea_getJamLengthVehicle(@WebParam(name = "loopID") String loopID) {
        return this.helper.getInt(this.sumo.get_cmd(Lanearea.getJamLengthVehicle(loopID)));
    }

    @WebMethod(action = "Lanearea: getJamLengthMeters")
    public double Lanearea_getJamLengthMeters(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lanearea.getJamLengthMeters(loopID)));
    }

    @WebMethod(action = "Lanearea: getLastStepMeanSpeed")
    public double Lanearea_getLastStepMeanSpeed(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lanearea.getLastStepMeanSpeed(loopID)));
    }

    @WebMethod(action = "Lanearea: getLastStepOccupancy")
    public double Lanearea_getLastStepOccupancy(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lanearea.getLastStepOccupancy(loopID)));
    }

    @WebMethod(action = "Person: getIDList")
    public SumoStringList Person_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Person.getIDList()));
    }

    @WebMethod(action = "Person: getIDCount")
    public int Person_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Person.getIDCount()));
    }

    @WebMethod(action = "Person: getSpeed")
    public double Person_getSpeed(@WebParam(name = "personID") String personID) {
        return this.helper.getDouble(this.sumo.get_cmd(Person.getSpeed(personID)));
    }

    @WebMethod(action = "Person: getPosition")
    public SumoPosition2D Person_getPosition(@WebParam(name = "personID") String personID) {
        return this.helper.getPosition2D(this.sumo.get_cmd(Person.getPosition(personID)));
    }

    @WebMethod(action = "Person: getPosition3D")
    public SumoPosition3D Person_getPosition3D(@WebParam(name = "personID") String personID) {
        return this.helper.getPosition3D(this.sumo.get_cmd(Person.getPosition3D(personID)));
    }

    @WebMethod(action = "Person: getAngle")
    public int Person_getAngle(@WebParam(name = "vehID") String personID) {
        return this.helper.getInt(this.sumo.get_cmd(Person.getAngle(personID)));
    }

    @WebMethod(action = "Person: getRoadID")
    public String Person_getRoadID(@WebParam(name = "personID") String personID) {
        return this.helper.getString(this.sumo.get_cmd(Person.getRoadID(personID)));
    }

    @WebMethod(action = "Person: getTypeID")
    public String Person_getTypeID(@WebParam(name = "personID") String personID) {
        return this.helper.getString(this.sumo.get_cmd(Person.getTypeID(personID)));
    }

    @WebMethod(action = "Person: getLanePosition")
    public double Person_getLanePosition(@WebParam(name = "personID") String personID) {
        return this.helper.getDouble(this.sumo.get_cmd(Person.getLanePosition(personID)));
    }

    @WebMethod(action = "Person: getColor")
    public SumoColor Person_getColor(@WebParam(name = "personID") String personID) {
        return this.helper.getColor(this.sumo.get_cmd(Person.getColor(personID)));
    }

    @WebMethod(action = "Person: getPersonNumber")
    public int Person_getPersonNumber(@WebParam(name = "personID") String personID) {
        return this.helper.getInt(this.sumo.get_cmd(Person.getPersonNumber(personID)));
    }

    @WebMethod(action = "Person: getLength")
    public double Person_getLength(@WebParam(name = "personID") String personID) {
        return this.helper.getDouble(this.sumo.get_cmd(Person.getLength(personID)));
    }

    @WebMethod(action = "Person: getWaitingTime")
    public double Person_getWaitingTime(@WebParam(name = "personID") String personID) {
        return this.helper.getDouble(this.sumo.get_cmd(Person.getWaitingTime(personID)));
    }

    @WebMethod(action = "Person: getMinGap")
    public double Person_getMinGap(@WebParam(name = "personID") String personID) {
        return this.helper.getDouble(this.sumo.get_cmd(Person.getMinGap(personID)));
    }

    @WebMethod(action = "Vehicle: getAccel")
    public double Vehicle_getAccel(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getAccel(vehID)));
    }

    @WebMethod(action = "Vehicle: getAdaptedTraveltime")
    public double Vehicle_getAdaptedTraveltime(@WebParam(name = "vehID") String vehID, @WebParam(name = "time") int time, @WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getAdaptedTraveltime(vehID, time, edgeID)));
    }

    @WebMethod(action = "Vehicle: getAngle")
    public double Vehicle_getAngle(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getAngle(vehID)));
    }

    @WebMethod(action = "Vehicle: getBestLanes")
    public SumoStringList Vehicle_getBestLanes(@WebParam(name = "vehID") String vehID) {
        return this.helper.getStringList(this.sumo.get_cmd(Vehicle.getBestLanes(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the CO2 emission in mg for the last time step.")
    public double Vehicle_getCO2Emission(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getCO2Emission(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the CO emission in mg for the last time step.")
    public double Vehicle_getCOEmission(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getCOEmission(vehID)));
    }

    @WebMethod(action = "Vehicle: getColor")
    public SumoColor Vehicle_getColor(@WebParam(name = "vehID") String vehID) {
        return this.helper.getColor(this.sumo.get_cmd(Vehicle.getColor(vehID)));
    }

    @WebMethod(action = "Vehicle: getDecel")
    public double Vehicle_getDecel(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getDecel(vehID)));
    }

    @WebMethod(action = "Vehicle: getDrivingDistance")
    public double Vehicle_getDrivingDistance(@WebParam(name = "vehID") String vehID, @WebParam(name = "edgeID") String edgeID, @WebParam(name = "pos") double pos, @WebParam(name = "laneID") byte laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getDrivingDistance(vehID, edgeID, pos, laneID)));
    }

    @WebMethod(action = "Vehicle: Returns the subscription results for the last time step and the given vehicle.")
    public double Vehicle_getDrivingDistance2D(@WebParam(name = "vehID") String vehID, @WebParam(name = "x") double x, @WebParam(name = "y") double y) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getDrivingDistance2D(vehID, x, y)));
    }

    @WebMethod(action = "Vehicle: getEffort")
    public double Vehicle_getEffort(@WebParam(name = "vehID") String vehID, @WebParam(name = "time") int time, @WebParam(name = "edgeID") String edgeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getEffort(vehID, time, edgeID)));
    }

    @WebMethod(action = "Vehicle: getEmissionClass")
    public String Vehicle_getEmissionClass(@WebParam(name = "vehID") String vehID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getEmissionClass(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the fuel consumption in ml for the last time step.")
    public double Vehicle_getFuelConsumption(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getFuelConsumption(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the HC emission in mg for the last time step.")
    public double Vehicle_getHCEmission(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getHCEmission(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns a list of all known vehicles.")
    public SumoStringList Vehicle_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Vehicle.getIDList()));
    }

    @WebMethod(action = "Vehicle: Returns the number of all known vehicles.")
    public int Vehicle_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Vehicle.getIDCount()));
    }

    @WebMethod(action = "Vehicle: getImperfection")
    public double Vehicle_getImperfection(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getImperfection(vehID)));
    }

    @WebMethod(action = "Vehicle: getAllowedSpeed")
    public double Vehicle_getAllowedSpeed(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getAllowedSpeed(vehID)));
    }

    @WebMethod(action = "Vehicle: getPersonNumber")
    public int Vehicle_getPersonNumber(@WebParam(name = "vehID") String vehID) {
        return this.helper.getInt(this.sumo.get_cmd(Vehicle.getPersonNumber(vehID)));
    }

    @WebMethod(action = "Vehicle: getDistance")
    public double Vehicle_getDistance(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getDistance(vehID)));
    }

    @WebMethod(action = "Vehicle: getElectricityConsumption")
    public double Vehicle_getElectricityConsumption(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getElectricityConsumption(vehID)));
    }
    @WebMethod(action = "Vehicle: getWaitingTime")
    public double Vehicle_getWaitingTime(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getWaitingTime(vehID)));
    }

    @WebMethod(action = "Vehicle: getLaneID")
    public String Vehicle_getLaneID(@WebParam(name = "vehID") String vehID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getLaneID(vehID)));
    }

    @WebMethod(action = "Vehicle: getLaneIndex")
    public int Vehicle_getLaneIndex(@WebParam(name = "vehID") String vehID) {
        return this.helper.getInt(this.sumo.get_cmd(Vehicle.getLaneIndex(vehID)));
    }

    @WebMethod(action = "Vehicle: getLanePosition")
    public double Vehicle_getLanePosition(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getLanePosition(vehID)));
    }

    @WebMethod(action = "Vehicle: getLeader")
    public String Vehicle_getLeader(@WebParam(name = "vehID") String vehID, @WebParam(name = "dist")  double dist) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getLeader(vehID, dist)));
    }

    @WebMethod(action = "Vehicle: getLength")
    public double Vehicle_getLength(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getLength(vehID)));
    }

    @WebMethod(action = "Vehicle: getMaxSpeed")
    public double Vehicle_getMaxSpeed(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getMaxSpeed(vehID)));
    }

    @WebMethod(action = "Vehicle: getMinGap")
    public double Vehicle_getMinGap(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getMinGap(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the NOx emission in mg for the last time step.")
    public double Vehicle_getNOxEmission(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getNOxEmission(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the noise emission in db for the last time step.")
    public double Vehicle_getNoiseEmission(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getNoiseEmission(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the particular matter emission in mg for the last time step.")
    public double Vehicle_getPMxEmission(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getPMxEmission(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the position of the named vehicle within the last step [m,m].")
    public SumoPosition2D Vehicle_getPosition(@WebParam(name = "vehID") String vehID) {
        return this.helper.getPosition2D(this.sumo.get_cmd(Vehicle.getPosition(vehID)));
    }

    @WebMethod(action = "Vehicle: Returns the position of the named vehicle within the last step [m,m].")
    public SumoPosition3D Vehicle_getPosition3D(@WebParam(name = "vehID") String vehID) {
        return this.helper.getPosition3D(this.sumo.get_cmd(Vehicle.getPosition3D(vehID)));
    }

    @WebMethod(action = "Vehicle: getRoadID")
    public String Vehicle_getRoadID(@WebParam(name = "vehID") String vehID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getRoadID(vehID)));
    }

    @WebMethod(action = "Vehicle: getRoute")
    public SumoStringList Vehicle_getRoute(@WebParam(name = "vehID") String vehID) {
        return this.helper.getStringList(this.sumo.get_cmd(Vehicle.getRoute(vehID)));
    }

    @WebMethod(action = "Vehicle: getRouteID")
    public String Vehicle_getRouteID(@WebParam(name = "vehID") String vehID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getRouteID(vehID)));
    }

    @WebMethod(action = "Vehicle: getShapeClass")
    public String Vehicle_getShapeClass(@WebParam(name = "vehID") String vehID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getShapeClass(vehID)));
    }

    @WebMethod(action = "Vehicle: getSignals")
    public int Vehicle_getSignals(@WebParam(name = "vehID") String vehID) {
        return this.helper.getInt(this.sumo.get_cmd(Vehicle.getSignals(vehID)));
    }

    @WebMethod(action = "Vehicle: getSlope")
    public double Vehicle_getSlope(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getSlope(vehID)));
    }

    @WebMethod(action = "Vehicle: getSpeed")
    public double Vehicle_getSpeed(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getSpeed(vehID)));
    }

    @WebMethod(action = "Vehicle: getSpeedDeviation")
    public double Vehicle_getSpeedDeviation(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getSpeedDeviation(vehID)));
    }

    @WebMethod(action = "Vehicle: getSpeedFactor")
    public double Vehicle_getSpeedFactor(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getSpeedFactor(vehID)));
    }

    @WebMethod(action = "Vehicle: getSpeedMode")
    public double Vehicle_getSpeedMode(@WebParam(name = "vehID") String vehID) {
        return this.helper.getInt(this.sumo.get_cmd(Vehicle.getSpeedMode(vehID)));
    }

    @WebMethod(action = "Vehicle: getSpeedWithoutTraCI")
    public double Vehicle_getSpeedWithoutTraCI(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getSpeedWithoutTraCI(vehID)));
    }

    @WebMethod(action = "Vehicle: getTau")
    public double Vehicle_getTau(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getTau(vehID)));
    }

    @WebMethod(action = "Vehicle: getTypeID")
    public String Vehicle_getTypeID(@WebParam(name = "vehID") String vehID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getTypeID(vehID)));
    }

    @WebMethod(action = "Vehicle: getVehicleClass")
    public String Vehicle_getVehicleClass(@WebParam(name = "vehID") String vehID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicle.getVehicleClass(vehID)));
    }

    @WebMethod(action = "Vehicle: getWidth")
    public double Vehicle_getWidth(@WebParam(name = "vehID") String vehID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicle.getWidth(vehID)));
    }

    @WebMethod(action = "Vehicle: isRouteValid")
    public int Vehicle_isRouteValid(@WebParam(name = "vehID") String vehID) {
        return this.helper.getInt(this.sumo.get_cmd(Vehicle.isRouteValid(vehID)));
    }

    @WebMethod(action = "Vehicle: setRoute")
    public void Vehicle_setRoute(@WebParam(name = "vehID") String vehID, @WebParam(name = "edgeList") SumoStringList edgeList) {
        this.sumo.set_cmd(Vehicle.setRoute(vehID, edgeList));
    }

    @WebMethod(action = "Vehicle: setLaneChangeMode")
    public void Vehicle_setLaneChangeMode(@WebParam(name = "vehID") String vehID, @WebParam(name = "lcm") int lcm) {
        this.sumo.set_cmd(Vehicle.setLaneChangeMode(vehID, lcm));
    }

    @WebMethod(action = "Vehicle: setType")
    public void Vehicle_setType(@WebParam(name = "vehID") String vehID, @WebParam(name = "typeID") String typeID) {
        this.sumo.get_cmd(Vehicle.setType(vehID, typeID));
    }

    @WebMethod(action = "Vehicle: slowDown")
    public void Vehicle_slowDown(@WebParam(name = "vehID") String vehID, @WebParam(name = "speed") double speed, @WebParam(name = "duration") double duration) {
        this.sumo.set_cmd(Vehicle.slowDown(vehID, speed, duration));
    }

    @WebMethod(action = "Simulation: clearPending")
    public void GUI_clearPending(@WebParam(name = "routeID") String routeID) {
        this.sumo.set_cmd(Simulation.clearPending(routeID));
    }

    @WebMethod(action = "Simulation: convert2D")
    public SumoStringList Simulation_convert2D(@WebParam(name = "edgeID") String edgeID, @WebParam(name = "pos") double pos, @WebParam(name = "laneIndex") byte laneIndex, @WebParam(name = "toGeo") String toGeo) {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.convert2D(edgeID, pos, laneIndex, toGeo)));
    }

    @WebMethod(action = "Simulation: convert3D")
    public SumoStringList Simulation_convert3D(@WebParam(name = "edgeID") String edgeID, @WebParam(name = "pos") double pos, @WebParam(name = "laneIndex") byte laneIndex, @WebParam(name = "toGeo") String toGeo) {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.convert3D(edgeID, pos, laneIndex, toGeo)));
    }

    @WebMethod(action = "Simulation: convertGeo")
    public SumoStringList Simulation_convertGeo(@WebParam(name = "x") double x, @WebParam(name = "y") double y, @WebParam(name = "fromGeo") boolean fromGeo) {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.convertGeo(x, y, fromGeo)));
    }

    @WebMethod(action = "Simulation: convertRoad")
    public SumoRoadPosition Simulation_convertRoad(@WebParam(name = "x") double x, @WebParam(name = "y") double y, @WebParam(name = "isGeo") boolean isGeo, @WebParam(name = "vClass") String vClass) {
        return this.helper.getRoadPosition(this.sumo.get_cmd(Simulation.convertRoad(x, y, isGeo, vClass)));
    }

    @WebMethod(action = "Simulation: getArrivedIDList")
    public SumoStringList Simulation_getArrivedIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getArrivedIDList()));
    }

    @WebMethod(action = "Simulation: getArrivedNumber")
    public int Simulation_getArrivedNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getArrivedNumber()));
    }

    @WebMethod(action = "Simulation: getCurrentTime")
    public int Simulation_getCurrentTime() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getCurrentTime()));
    }


    @WebMethod(action = "Simulation: getBusStopWaiting")
    public int Simulation_getBusStopWaiting() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getBusStopWaiting()));
    }

    @WebMethod(action = "Simulation: getParkingEndingVehiclesIDList")
    public SumoStringList Simulation_getParkingEndingVehiclesIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getParkingEndingVehiclesIDList()));
    }


    @WebMethod(action = "Simulation: getParkingEndingVehiclesNumber")
    public int Simulation_getParkingEndingVehiclesNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getParkingEndingVehiclesNumber()));
    }

    @WebMethod(action = "Simulation: getParkingStartingVehiclesIDList")
    public SumoStringList Simulation_getParkingStartingVehiclesIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getParkingStartingVehiclesIDList()));
    }

    @WebMethod(action = "Simulation: getParkingStartingVehiclesNumber")
    public int Simulation_getParkingStartingVehiclesNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getParkingStartingVehiclesNumber()));
    }

    @WebMethod(action = "Simulation: getStopEndingVehiclesIDList")
    public SumoStringList Simulation_getStopEndingVehiclesIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getStopEndingVehiclesIDList()));
    }

    @WebMethod(action = "Simulation: getStopEndingVehiclesNumber")
    public int Simulation_getStopEndingVehiclesNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getStopEndingVehiclesNumber()));
    }

    @WebMethod(action = "Simulation: getStopStartingVehiclesIDList")
    public SumoStringList Simulation_getStopStartingVehiclesIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getStopStartingVehiclesIDList()));
    }

    @WebMethod(action = "Simulation: getStopStartingVehiclesNumber")
    public int Simulation_getStopStartingVehiclesNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getStopStartingVehiclesNumber()));
    }


    @WebMethod(action = "Simulation: getDeltaT")
    public int Simulation_getDeltaT() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getDeltaT()));
    }

    @WebMethod(action = "Simulation: getDepartedIDList")
    public SumoStringList Simulation_getDepartedIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getDepartedIDList()));
    }

    @WebMethod(action = "Simulation: getDepartedNumber")
    public int Simulation_getDepartedNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getDepartedNumber()));
    }

    @WebMethod(action = "Simulation: getDistance2D")
    public double Simulation_getDistance2D(@WebParam(name = "x1") double x1, @WebParam(name = "y1") double y1, @WebParam(name = "x2") double x2, @WebParam(name = "y2") double y2, @WebParam(name = "isGeo") boolean isGeo, @WebParam(name = "isDriving") boolean isDriving) {
        return this.helper.getDouble(this.sumo.get_cmd(Simulation.getDistance2D(x1, y1, x2, y2, isGeo, isDriving)));
    }

    @WebMethod(action = "Simulation: Returns the subscription results for the last time step.")
    public double Simulation_getDistanceRoad(@WebParam(name = "edgeID1") String edgeID1, @WebParam(name = "pos1") double pos1, @WebParam(name = "edgeID2") String edgeID2, @WebParam(name = "pos2") double pos2, @WebParam(name = "isDriving") boolean isDriving) {
        return this.helper.getDouble(this.sumo.get_cmd(Simulation.getDistanceRoad(edgeID1, pos1, edgeID2, pos2, isDriving)));
    }

    @WebMethod(action = "Simulation: getEndingTeleportIDList")
    public SumoStringList Simulation_getEndingTeleportIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getEndingTeleportIDList()));
    }

    @WebMethod(action = "Simulation: getEndingTeleportNumber")
    public int Simulation_getEndingTeleportNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getEndingTeleportNumber()));
    }

    @WebMethod(action = "Simulation: getLoadedIDList")
    public SumoStringList Simulation_getLoadedIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getLoadedIDList()));
    }

    @WebMethod(action = "Simulation: getLoadedNumber")
    public int Simulation_getLoadedNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getLoadedNumber()));
    }

    @WebMethod(action = "Simulation: getMinExpectedNumber")
    public int Simulation_getMinExpectedNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getMinExpectedNumber()));
    }

    @WebMethod(action = "Simulation: getNetBoundary")
    public SumoStringList Simulation_getNetBoundary() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getNetBoundary()));
    }

    @WebMethod(action = "Simulation: getStartingTeleportIDList")
    public SumoStringList Simulation_getStartingTeleportIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Simulation.getStartingTeleportIDList()));
    }

    @WebMethod(action = "Simulation: getStartingTeleportNumber")
    public int Simulation_getStartingTeleportNumber() {
        return this.helper.getInt(this.sumo.get_cmd(Simulation.getStartingTeleportNumber()));
    }

    @WebMethod(action = "Trafficlights: getCompleteRedYellowGreenDefinition")
    public SumoTLSController Trafficlights_getCompleteRedYellowGreenDefinition(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getTLSProgram(this.sumo.get_cmd(Trafficlight.getCompleteRedYellowGreenDefinition(tlsID)));
    }

    @WebMethod(action = "Trafficlights: getControlledLanes")
    public SumoStringList Trafficlights_getControlledLanes(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getStringList(this.sumo.get_cmd(Trafficlight.getControlledLanes(tlsID)));
    }

    @WebMethod(action = "Trafficlights: getControlledLinks")
    public SumoStringList Trafficlights_getControlledLinks(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getStringList(this.sumo.get_cmd(Trafficlight.getControlledLinks(tlsID)));
    }

    @WebMethod(action = "Trafficlights: Returns a list of all traffic lights in the network.")
    public SumoStringList Trafficlights_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Trafficlight.getIDList()));
    }

    @WebMethod(action = "Trafficlights: Returns the number of all traffic lights in the network.")
    public int Trafficlights_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Trafficlight.getIDCount()));
    }

    @WebMethod(action = "Trafficlights: Returns the next switch")
    public int Trafficlights_getNextSwitch(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getInt(this.sumo.get_cmd(Trafficlight.getNextSwitch(tlsID)));
    }

    @WebMethod(action = "Trafficlights: Returns the phase duration")
    public int Trafficlights_getPhaseDuration(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getInt(this.sumo.get_cmd(Trafficlight.getPhaseDuration(tlsID)));
    }

    @WebMethod(action = "Trafficlights: getPhase")
    public int Trafficlights_getPhase(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getInt(this.sumo.get_cmd(Trafficlight.getPhase(tlsID)));
    }

    @WebMethod(action = "Trafficlights: getProgram")
    public String Trafficlights_getProgram(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getString(this.sumo.get_cmd(Trafficlight.getProgram(tlsID)));
    }

    @WebMethod(action = "Trafficlights: getRedYellowGreenState")
    public String Trafficlights_getRedYellowGreenState(@WebParam(name = "tlsID") String tlsID) {
        return this.helper.getString(this.sumo.get_cmd(Trafficlight.getRedYellowGreenState(tlsID)));
    }

    @WebMethod(action = "Vehicletype: getAccel")
    public double Vehicletype_getAccel(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getAccel(typeID)));
    }

    @WebMethod(action = "Vehicletype: Returns the subscription results for the last time step and the given vehicle type.")
    public SumoColor Vehicletype_getColor(@WebParam(name = "typeID") String typeID) {
        return this.helper.getColor(this.sumo.get_cmd(Vehicletype.getColor(typeID)));
    }

    @WebMethod(action = "Vehicletype: getDecel")
    public double Vehicletype_getDecel(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getDecel(typeID)));
    }

    @WebMethod(action = "Vehicletype: getEmissionClass")
    public String Vehicletype_getEmissionClass(@WebParam(name = "typeID") String typeID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicletype.getEmissionClass(typeID)));
    }

    @WebMethod(action = "Vehicletype: Returns a list of all known vehicle types.")
    public SumoStringList Vehicletype_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Vehicletype.getIDList()));
    }

    @WebMethod(action = "Vehicletype: Returns the number of all known vehicle types.")
    public int Vehicletype_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Vehicletype.getIDCount()));
    }

    @WebMethod(action = "Vehicletype: getImperfection")
    public double Vehicletype_getImperfection(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getImperfection(typeID)));
    }

    @WebMethod(action = "Vehicletype: getLength")
    public double Vehicletype_getLength(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getLength(typeID)));
    }

    @WebMethod(action = "Vehicletype: getMaxSpeed")
    public double Vehicletype_getMaxSpeed(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getMaxSpeed(typeID)));
    }

    @WebMethod(action = "Vehicletype: getMinGap")
    public double Vehicletype_getMinGap(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getMinGap(typeID)));
    }

    @WebMethod(action = "Vehicletype: getShapeClass")
    public String Vehicletype_getShapeClass(@WebParam(name = "typeID") String typeID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicletype.getShapeClass(typeID)));
    }

    @WebMethod(action = "Vehicletype: getSpeedDeviation")
    public double Vehicletype_getSpeedDeviation(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getSpeedDeviation(typeID)));
    }

    @WebMethod(action = "Vehicletype: getSpeedFactor")
    public double Vehicletype_getSpeedFactor(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getSpeedFactor(typeID)));
    }

    @WebMethod(action = "Vehicletype: getTau")
    public double Vehicletype_getTau(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getTau(typeID)));
    }

    @WebMethod(action = "Vehicletype: getVehicleClass")
    public String Vehicletype_getVehicleClass(@WebParam(name = "typeID") String typeID) {
        return this.helper.getString(this.sumo.get_cmd(Vehicletype.getVehicleClass(typeID)));
    }

    @WebMethod(action = "Vehicletype: getWidth")
    public double Vehicletype_getWidth(@WebParam(name = "typeID") String typeID) {
        return this.helper.getDouble(this.sumo.get_cmd(Vehicletype.getWidth(typeID)));
    }

    @WebMethod(action = "Lane: getAllowed")
    public SumoStringList Lane_getAllowed(@WebParam(name = "laneID") String laneID) {
        return this.helper.getStringList(this.sumo.get_cmd(Lane.getAllowed(laneID)));
    }

    @WebMethod(action = "Lane: Returns the CO2 emission in mg for the last time step on the given lane.")
    public double Lane_getCO2Emission(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getCO2Emission(laneID)));
    }

    @WebMethod(action = "Lane: Returns the CO emission in mg for the last time step on the given lane.")
    public double Lane_getCOEmission(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getCOEmission(laneID)));
    }

    @WebMethod(action = "Lane: getDisallowed")
    public SumoStringList Lane_getDisallowed(@WebParam(name = "laneID") String laneID) {
        return this.helper.getStringList(this.sumo.get_cmd(Lane.getDisallowed(laneID)));
    }

    @WebMethod(action = "Lane: getEdgeID")
    public String Lane_getEdgeID(@WebParam(name = "laneID") String laneID) {
        return this.helper.getString(this.sumo.get_cmd(Lane.getEdgeID(laneID)));
    }

    @WebMethod(action = "Lane: getElectricityConsumption")
    public String Lane_getElectricityConsumption(@WebParam(name = "laneID") String laneID) {
        return this.helper.getString(this.sumo.get_cmd(Lane.getElectricityConsumption(laneID)));
    }

    @WebMethod(action = "Lane: Returns the fuel consumption in ml for the last time step on the given lane.")
    public double Lane_getFuelConsumption(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getFuelConsumption(laneID)));
    }

    @WebMethod(action = "Lane: Returns the HC emission in mg for the last time step on the given lane.")
    public double Lane_getHCEmission(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getHCEmission(laneID)));
    }

    @WebMethod(action = "Lane: Returns a list of all lanes in the network.")
    public SumoStringList Lane_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Lane.getIDList()));
    }

    @WebMethod(action = "Lane: Returns the number of all lanes in the network.")
    public int Lane_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Lane.getIDCount()));
    }


    @WebMethod(action = "Lane: Returns the total number of halting vehicles for the last time step on the given lane.")
    public int Lane_getLastStepHaltingNumber(@WebParam(name = "laneID") String laneID) {
        return this.helper.getInt(this.sumo.get_cmd(Lane.getLastStepHaltingNumber(laneID)));
    }

    @WebMethod(action = "Lane: Returns the total vehicle length in m for the last time step on the given lane.")
    public double Lane_getLastStepLength(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getLastStepLength(laneID)));
    }

    @WebMethod(action = "Lane: Returns the average speed in m/s for the last time step on the given lane.")
    public double Lane_getLastStepMeanSpeed(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getLastStepMeanSpeed(laneID)));
    }

    @WebMethod(action = "Lane: Returns the occupancy in % for the last time step on the given lane.")
    public double Lane_getLastStepOccupancy(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getLastStepOccupancy(laneID)));
    }

    @WebMethod(action = "Lane: Returns the subscription results for the last time step and the given lane.")
    public SumoStringList Lane_getLastStepVehicleIDs(@WebParam(name = "laneID") String laneID) {
        return this.helper.getStringList(this.sumo.get_cmd(Lane.getLastStepVehicleIDs(laneID)));
    }

    @WebMethod(action = "Lane: Returns the total number of vehicles for the last time step on the given lane.")
    public int Lane_getLastStepVehicleNumber(@WebParam(name = "laneID") String laneID) {
        return this.helper.getInt(this.sumo.get_cmd(Lane.getLastStepVehicleNumber(laneID)));
    }

    @WebMethod(action = "Lane: getLength")
    public double Lane_getLength(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getLength(laneID)));
    }

    @WebMethod(action = "Lane: getLinkNumber")
    public byte Lane_getLinkNumber(@WebParam(name = "laneID") String laneID) {
        return this.helper.getByte(this.sumo.get_cmd(Lane.getLinkNumber(laneID)));
    }

    @WebMethod(action = "Lane: getLinks")
    public SumoLinkList Lane_getLinks(@WebParam(name = "laneID") String laneID) {
        return this.helper.getLaneLinks(this.sumo.get_cmd(Lane.getLinks(laneID)));
    }

    @WebMethod(action = "Lane: getMaxSpeed")
    public double Lane_getMaxSpeed(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getMaxSpeed(laneID)));
    }

    @WebMethod(action = "Lane: getWaitingTime")
    public double Lane_getWaitingTime(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getWaitingTime(laneID)));
    }

    @WebMethod(action = "Lane: Returns the NOx emission in mg for the last time step on the given lane.")
    public double Lane_getNOxEmission(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getNOxEmission(laneID)));
    }

    @WebMethod(action = "Lane: Returns the noise emission in db for the last time step on the given lane.")
    public double Lane_getNoiseEmission(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getNoiseEmission(laneID)));
    }

    @WebMethod(action = "Lane: Returns the particular matter emission in mg for the last time step on the given lane.")
    public double Lane_getPMxEmission(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getPMxEmission(laneID)));
    }

    @WebMethod(action = "Lane: getShape")
    public SumoGeometry Lane_getShape(@WebParam(name = "laneID") String laneID) {
        return this.helper.getPolygon(this.sumo.get_cmd(Lane.getShape(laneID)));
    }

    @WebMethod(action = "Lane: Returns the estimated travel time in s for the last time step on the given lane.")
    public double Lane_getTraveltime(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getTraveltime(laneID)));
    }

    @WebMethod(action = "Lane: getWidth")
    public double Lane_getWidth(@WebParam(name = "laneID") String laneID) {
        return this.helper.getDouble(this.sumo.get_cmd(Lane.getWidth(laneID)));
    }

    @WebMethod(action = "Polygon: Returns the subscription results for the last time step and the given poi.")
    public SumoColor Polygon_getColor(@WebParam(name = "polygonID") String polygonID) {
        return this.helper.getColor(this.sumo.get_cmd(Polygon.getColor(polygonID)));
    }

    @WebMethod(action = "Polygon: Returns a list of all polygons in the network.")
    public SumoStringList Polygon_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Polygon.getIDList()));
    }

    @WebMethod(action = "Polygon: Returns the number of all polygons in the network.")
    public int Polygon_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Polygon.getIDCount()));
    }

    @WebMethod(action = "Polygon: getShape")
    public SumoGeometry Polygon_getShape(@WebParam(name = "polygonID") String polygonID) {
        return this.helper.getPolygon(this.sumo.get_cmd(Polygon.getShape(polygonID)));
    }

    @WebMethod(action = "Polygon: getType")
    public String Polygon_getType(@WebParam(name = "polygonID") String polygonID) {
        return this.helper.getString(this.sumo.get_cmd(Polygon.getType(polygonID)));
    }

    @WebMethod(action = "Poi: add")
    public void Poi_add(@WebParam(name = "poiID") String poiID, @WebParam(name = "x") double x, @WebParam(name = "y") double y, @WebParam(name = "color") SumoColor color, @WebParam(name = "poiType") String poiType, @WebParam(name = "layer") int layer) {
        this.sumo.get_cmd(Poi.add(poiID, x, y, color, poiType, layer));
    }

    @WebMethod(action = "Poi: Returns the color of the given poi.")
    public SumoColor Poi_getColor(@WebParam(name = "poiID") String poiID) {
        return this.helper.getColor(this.sumo.get_cmd(Poi.getColor(poiID)));
    }

    @WebMethod(action = "Poi: Returns a list of all pois in the network.")
    public SumoStringList Poi_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Poi.getIDList()));
    }

    @WebMethod(action = "Poi: Returns the number of all pois in the network.")
    public int Poi_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Poi.getIDCount()));
    }

    @WebMethod(action = "Poi: getPosition")
    public SumoPosition2D Poi_getPosition(@WebParam(name = "poiID") String poiID) {
        return this.helper.getPosition2D(this.sumo.get_cmd(Poi.getPosition(poiID)));
    }

    @WebMethod(action = "Poi: getType")
    public String Poi_getType(@WebParam(name = "poiID") String poiID) {
        return this.helper.getString(this.sumo.get_cmd(Poi.getType(poiID)));
    }

    @WebMethod(action = "Junction: Returns a list of all junctions in the network.")
    public SumoStringList Junction_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Junction.getIDList()));
    }

    @WebMethod(action = "Junction: getShape")
    public SumoGeometry Junction_getShape(@WebParam(name = "junctionID") String junctionID) {
        return this.helper.getPolygon(this.sumo.get_cmd(Junction.getShape(junctionID)));
    }

    @WebMethod(action = "Junction: Returns the number of all junctions in the network.")
    public int Junction_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Junction.getIDCount()));
    }

    @WebMethod(action = "Junction: Returns the subscription results for the last time step and the given junction.")
    public SumoPosition2D Junction_getPosition(@WebParam(name = "junctionID") String junctionID) {
        return this.helper.getPosition2D(this.sumo.get_cmd(Junction.getPosition(junctionID)));
    }

    @WebMethod(action = "Gui: Returns the subscription results for the last time step and the given view.")
    public SumoBoundingBox GUI_getBoundary(@WebParam(name = "viewID") String viewID) {
        return this.helper.getBoundingBox(this.sumo.get_cmd(Gui.getBoundary(viewID)));
    }

    @WebMethod(action = "Gui: Returns the list of available views (open windows).")
    public SumoStringList GUI_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Gui.getIDList()));
    }

    @WebMethod(action = "Gui: Returns the x and y offset of the center of the current view.")
    public SumoPosition2D GUI_getOffset(@WebParam(name = "viewID") String viewID) {
        return this.helper.getPosition2D(this.sumo.get_cmd(Gui.getOffset(viewID)));
    }

    @WebMethod(action = "Gui: Returns the name of the current coloring scheme.")
    public String GUI_getSchema(@WebParam(name = "viewID") String viewID) {
        return this.helper.getString(this.sumo.get_cmd(Gui.getSchema(viewID)));
    }

    @WebMethod(action = "Gui: Returns the current zoom factor.")
    public double GUI_getZoom(@WebParam(name = "viewID") String viewID) {
        return this.helper.getDouble(this.sumo.get_cmd(Gui.getZoom(viewID)));
    }

    @WebMethod(action = "Route: Returns the subscription results for the last time step and the given route.")
    public SumoStringList Route_getEdges(@WebParam(name = "routeID") String routeID) {
        return this.helper.getStringList(this.sumo.get_cmd(Route.getEdges(routeID)));
    }

    @WebMethod(action = "Route: Returns a list of all routes in the network.")
    public SumoStringList Route_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Route.getIDList()));
    }

    @WebMethod(action = "Route: Returns the number of all routes in the network.")
    public int Route_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Route.getIDCount()));
    }

    @WebMethod(action = "Inductionloop: Returns a list of all induction loops in the network.")
    public SumoStringList Inductionloop_getIDList() {
        return this.helper.getStringList(this.sumo.get_cmd(Inductionloop.getIDList()));
    }

    @WebMethod(action = "Inductionloop: Returns the number of induction loops in the network.")
    public int Inductionloop_getIDCount() {
        return this.helper.getInt(this.sumo.get_cmd(Inductionloop.getIDCount()));
    }

    @WebMethod(action = "Inductionloop: Returns the id of the lane the loop is on.")
    public String Inductionloop_getLaneID(@WebParam(name = "loopID") String loopID) {
        return this.helper.getString(this.sumo.get_cmd(Inductionloop.getLaneID(loopID)));
    }

    @WebMethod(action = "Inductionloop: getLastStepMeanLength")
    public double Inductionloop_getLastStepMeanLength(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Inductionloop.getLastStepMeanLength(loopID)));
    }

    @WebMethod(action = "Inductionloop: getLastStepMeanSpeed")
    public double Inductionloop_getLastStepMeanSpeed(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Inductionloop.getLastStepMeanSpeed(loopID)));
    }

    @WebMethod(action = "Inductionloop: getLastStepOccupancy")
    public double Inductionloop_getLastStepOccupancy(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Inductionloop.getLastStepOccupancy(loopID)));
    }

    @WebMethod(action = "Inductionloop: getLastStepVehicleIDs")
    public SumoStringList Inductionloop_getLastStepVehicleIDs(@WebParam(name = "loopID") String loopID) {
        return this.helper.getStringList(this.sumo.get_cmd(Inductionloop.getLastStepVehicleIDs(loopID)));
    }

    @WebMethod(action = "Inductionloop: getLastStepVehicleNumber")
    public int Inductionloop_getLastStepVehicleNumber(@WebParam(name = "loopID") String loopID) {
        return this.helper.getInt(this.sumo.get_cmd(Inductionloop.getLastStepVehicleNumber(loopID)));
    }

    @WebMethod(action = "Inductionloop: Returns the position measured from the beginning of the lane.")
    public double Inductionloop_getPosition(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Inductionloop.getPosition(loopID)));
    }

    @WebMethod(action = "Inductionloop: getTimeSinceDetection")
    public double Inductionloop_getTimeSinceDetection(@WebParam(name = "loopID") String loopID) {
        return this.helper.getDouble(this.sumo.get_cmd(Inductionloop.getTimeSinceDetection(loopID)));
    }

    @WebMethod(action = "Inductionloop: Returns the subscription results for the last time step and the given loop.")
    public SumoVehicleData Inductionloop_getVehicleData(@WebParam(name = "loopID") String loopID) {
        return this.helper.getVehicleData(this.sumo.get_cmd(Inductionloop.getVehicleData(loopID)));
    }


}
