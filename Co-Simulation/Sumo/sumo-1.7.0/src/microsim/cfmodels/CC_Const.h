/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2001-2020 German Aerospace Center (DLR) and others.
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
/// @file    CC_Const.h
/// @author  Michele Segata
/// @date    Fri, 11 Apr 2014
///
// File defining constants, structs, and enums for cruise controllers
/****************************************************************************/
#pragma once

#include <string>
#include <sstream>

namespace Plexe {

/**
 * @brief action that might be requested by the platooning management
 */
enum PLATOONING_LANE_CHANGE_ACTION {
    DRIVER_CHOICE = 0,        //the platooning management is not active, so just let the driver choose the lane
    STAY_IN_CURRENT_LANE = 3, //the car is part of a platoon, so it has to stay on the dedicated platooning lane
    MOVE_TO_FIXED_LANE = 4    //move the car to a specific lane
};

/**
 * @brief TraCI modes for lane changing
 */
#define FIX_LC 0b1000000000
#define DEFAULT_NOTRACI_LC 0b1010101010

/** @enum ACTIVE_CONTROLLER
 * @brief Determines the currently active controller, i.e., ACC, CACC, or the
 * driver. In future we might need to switch off the automatic controller and
 * leave the control to the mobility model which reproduces a human driver
 */
enum ACTIVE_CONTROLLER
{DRIVER = 0, ACC = 1, CACC = 2, FAKED_CACC = 3, PLOEG = 4, CONSENSUS = 5, FLATBED = 6};

/**
 * @brief struct used as header for generic data passing to this model through
 * traci
 */
struct CCDataHeader {
    int type;    //type of message. indicates what comes after the header
    int size;    //size of message. indicates how many bytes comes after the header
};

/**
 * Struct defining data passed about a vehicle
 */
struct VEHICLE_DATA {
    int index;           //position in the platoon (0 = first)
    double speed;        //vehicle speed
    double acceleration; //vehicle acceleration
    double positionX;    //position of the vehicle in the simulation
    double positionY;    //position of the vehicle in the simulation
    double time;         //time at which such information was read from vehicle's sensors
    double length;       //vehicle length
    double u;            //controller acceleration
    double speedX;       //vehicle speed on the X axis
    double speedY;       //vehicle speed on the Y axis
    double angle;        //vehicle angle in radians
};

#define MAX_N_CARS 8

#define CC_ENGINE_MODEL_FOLM             0x00    //first order lag model
#define CC_ENGINE_MODEL_REALISTIC        0x01    //the detailed and realistic engine model

//parameter names for engine models
#define FOLM_PAR_TAU                     "tau_s"
#define FOLM_PAR_DT                      "dt_s"

#define ENGINE_PAR_VEHICLE               "vehicle"
#define ENGINE_PAR_XMLFILE               "xmlFile"
#define ENGINE_PAR_DT                    "dt_s"

#define CC_PAR_VEHICLE_DATA              "ccvd"   //data about a vehicle, like position, speed, acceleration, etc
#define CC_PAR_VEHICLE_POSITION          "ccvp"   //position of the vehicle in the platoon (0 based)
#define CC_PAR_PLATOON_SIZE              "ccps"   //number of cars in the platoon

//set of controller-related constants
#define CC_PAR_CACC_XI                   "ccxi"    //xi
#define CC_PAR_CACC_OMEGA_N              "ccon"    //omega_n
#define CC_PAR_CACC_C1                   "ccc1"    //C1
#define CC_PAR_ENGINE_TAU                "cctau"   //engine time constant

#define CC_PAR_UMIN                      "ccumin"  //lower saturation for u
#define CC_PAR_UMAX                      "ccumax"  //upper saturation for u

#define CC_PAR_PLOEG_H                   "ccph"    //time headway of ploeg's CACC
#define CC_PAR_PLOEG_KP                  "ccpkp"   //kp parameter of ploeg's CACC
#define CC_PAR_PLOEG_KD                  "ccpkd"   //kd parameter of ploeg's CACC

#define CC_PAR_FLATBED_KA                "ccfka"   //ka parameter of flatbed CACC
#define CC_PAR_FLATBED_KV                "ccfkv"   //kv parameter of flatbed CACC
#define CC_PAR_FLATBED_KP                "ccfkp"   //kp parameter of flatbed CACC
#define CC_PAR_FLATBED_H                 "ccfh"    //h parameter of flatbed CACC
#define CC_PAR_FLATBED_D                 "ccfd"    //distance parameter of flatbed CACC

#define CC_PAR_VEHICLE_ENGINE_MODEL      "ccem"    //set the engine model for a vehicle

#define CC_PAR_VEHICLE_MODEL             "ccvm"    //set the vehicle model, i.e., engine characteristics
#define CC_PAR_VEHICLES_FILE             "ccvf"    //set the location of the vehicle parameters file

// set CACC constant spacing
#define PAR_CACC_SPACING                 "ccsp"

// get ACC computed acceleration when faked CACC controller is enabled
#define PAR_ACC_ACCELERATION             "ccacc"

// determine whether a vehicle has crashed or not
#define PAR_CRASHED                      "cccr"

// set a fixed acceleration to a vehicle controlled by CC/ACC/CACC
#define PAR_FIXED_ACCELERATION           "ccfa"

// get vehicle speed and acceleration, needed for example by the platoon leader (get: vehicle)
#define PAR_SPEED_AND_ACCELERATION       "ccsa"

// set speed and acceleration of the platoon leader
#define PAR_LEADER_SPEED_AND_ACCELERATION "cclsa"

// set whether CACCs should use real or controller acceleration
#define PAR_USE_CONTROLLER_ACCELERATION "ccca"

// get lane count for the street the vehicle is currently traveling
#define PAR_LANES_COUNT                  "cclc"

// set the cruise control desired speed
#define PAR_CC_DESIRED_SPEED             "ccds"

// set the currently active vehicle controller which can be either the driver, or the ACC or the CACC
#define PAR_ACTIVE_CONTROLLER            "ccac"

// get radar data from the car
#define PAR_RADAR_DATA                   "ccrd"

// communicate with the cruise control to give him fake indications. this can be useful when you want
// to advance a vehicle to a certain position, for example, for joining a platoon. clearly the ACC
// must always take into consideration both fake and real data
#define PAR_LEADER_FAKE_DATA             "cclfd"
#define PAR_FRONT_FAKE_DATA              "ccffd"

// get the distance that a car has to travel until it reaches the end of its route
#define PAR_DISTANCE_TO_END              "ccdte"

// get the distance from the beginning of the route
#define PAR_DISTANCE_FROM_BEGIN          "ccdfb"

// set speed and acceleration of preceding vehicle
#define PAR_PRECEDING_SPEED_AND_ACCELERATION "ccpsa"

// set ACC headway time
#define PAR_ACC_HEADWAY_TIME             "ccaht"

// return engine information (for the realistic engine model)
#define PAR_ENGINE_DATA                  "cced"

// enabling/disabling auto feeding
#define PAR_USE_AUTO_FEEDING             "ccaf"

// enabling/disabling data prediction
#define PAR_USE_PREDICTION               "ccup"

// add/remove members from own platoon
#define PAR_ADD_MEMBER                   "ccam"
#define PAR_REMOVE_MEMBER                "ccrm"

// let the leader automatically change lane for the whole platoon if there is a speed advantage
#define PAR_ENABLE_AUTO_LANE_CHANGE      "ccalc"

}

