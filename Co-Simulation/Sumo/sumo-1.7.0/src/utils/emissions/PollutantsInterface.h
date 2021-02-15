/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2013-2020 German Aerospace Center (DLR) and others.
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
/// @file    PollutantsInterface.h
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    Mon, 19.08.2013
///
// Interface to capsulate different emission models
/****************************************************************************/
#pragma once
#include <config.h>

#include <cctype>  // defines std::tolower
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>
#include <utils/common/StdDefs.h>
#include <utils/common/SUMOVehicleClass.h>
#include "PHEMCEP.h"


// ===========================================================================
// class declarations
// ===========================================================================
class HelpersHBEFA;
class HelpersHBEFA3;
class HelpersPHEMlight;
class HelpersEnergy;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class PollutantsInterface
 * @brief Helper methods for PHEMlight-based emission computation
 */
class PollutantsInterface {

public:
    /// @brief Enumerating all emission types, including fuel
    enum EmissionType { CO2, CO, HC, FUEL, NO_X, PM_X, ELEC };

    /**
     * @struct Emissions
     * @brief Storage for collected values of all emission types
     */
    struct Emissions {
        double CO2;
        double CO;
        double HC;
        double fuel;
        double NOx;
        double PMx;
        double electricity;

        /** @brief Constructor, intializes all members
         * @param[in] co2 initial value for CO2, defaults to 0
         * @param[in] co  initial value for CO, defaults to 0
         * @param[in] hc  initial value for HC, defaults to 0
         * @param[in] f   initial value for fuel, defaults to 0
         * @param[in] nox initial value for NOx, defaults to 0
         * @param[in] pmx initial value for PMx, defaults to 0
         * @param[in] elec initial value for electricity, defaults to 0
         */
        Emissions(double co2 = 0, double co = 0, double hc = 0, double f = 0, double nox = 0, double pmx = 0, double elec = 0)
            : CO2(co2), CO(co), HC(hc), fuel(f), NOx(nox), PMx(pmx), electricity(elec) {
        }

        /** @brief Add the values of the other struct to this one, scaling the values if needed
         * @param[in] a the other emission valuess
         * @param[in] scale scaling factor, defaulting to 1 (no scaling)
         */
        void addScaled(const Emissions& a, const double scale = 1.) {
            CO2 += scale * a.CO2;
            CO += scale * a.CO;
            HC += scale * a.HC;
            fuel += scale * a.fuel;
            NOx += scale * a.NOx;
            PMx += scale * a.PMx;
            electricity += scale * a.electricity;
        }
    };

    /**
    * @class Helper
    * @brief zero emission model, used as superclass for the other model helpers
    */
    class Helper {
    public:
        /** @brief Constructor, intializes the name
         * @param[in] name the name of the model (string before the '/' in the emission class attribute)
         */
        Helper(std::string name, const int defaultClass = -1) : myName(name) {
            if (defaultClass != -1) {
                myEmissionClassStrings.insert("default", defaultClass);
                myEmissionClassStrings.addAlias("unknown", defaultClass);
            }
        }

        /** @brief Returns the name of the model
         * @return the name of the model (string before the '/' in the emission class attribute)
         */
        const std::string& getName() const {
            return myName;
        }

        /** @brief Returns the emission class associated with the given name, aliases are possible
         * If this method is asked for the "unknown" class it should return the default
         * (possibly depending on the given vehicle class).
         * The class name is case insensitive.
         *
         * @param[in] eClass the name of the emission class (string after the '/' in the emission class attribute)
         * @param[in] vc the vehicle class to use when determining default class
         * @return the name of the model (string before the '/' in the emission class)
         */
        virtual SUMOEmissionClass getClassByName(const std::string& eClass, const SUMOVehicleClass vc) {
            UNUSED_PARAMETER(vc);
            if (myEmissionClassStrings.hasString(eClass)) {
                return myEmissionClassStrings.get(eClass);
            }
            std::string eclower = eClass;
            /*
               For some compilers, std::tolower cannot be resolved correctly, resulting in error messages
               like "No matching function found ... unresolved overloaded function type.", see e.g.
               https://stackoverflow.com/questions/5539249. The problem may be fixed by specifying ::tolower,
               the global namespace version of the function that has no overloads.

               Similarly, https://en.cppreference.com/w/cpp/string/byte/tolower suggests that one should not
               use any of the functions defined in <cctype> with standard algorithms (like `transform`) when
               the iterator type is `char` or `signed char` -- we shall convert the value to `unsigned char`
               first:

               std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });

               This, however, still generates an ugly warning in VS2017. Go figure ...
            */
            std::transform(eclower.begin(), eclower.end(), eclower.begin(), [](unsigned char c) {
                return std::tolower(c);
            });
            return myEmissionClassStrings.get(eclower);
        }

        /** @brief Returns the complete name of the emission class including the model
         * @param[in] c the emission class
         * @return the name of the class (the complete emission class attribute)
         */
        const std::string getClassName(const SUMOEmissionClass c) const {
            return myName + "/" + myEmissionClassStrings.getString(c);
        }

        /** @brief Returns whether the class denotes a silent vehicle for interfacing with the noise model.
         * By default the first class in each model is the silent class.
         * @param[in] c the emission class
         * @return whether the class denotes a silent vehicle
         */
        virtual bool isSilent(const SUMOEmissionClass c) {
            return (c & 0xffffffff & ~HEAVY_BIT) == 0;
        }

        /// @name Methods for Amitran interfaces
        /// @{

        /** @brief Returns the emission class described by the given parameters.
         * The base is used to determine the model to use and as default return values.
         * Default implementation returns always base.
         * @param[in] base the base class giving the model and the default
         * @param[in] vClass the vehicle class as described in the Amitran interface (Passenger, ...)
         * @param[in] fuel the fuel type as described in the Amitran interface (Gasoline, Diesel, ...)
         * @param[in] eClass the emission class as described in the Amitran interface (Euro0, ...)
         * @param[in] weight the vehicle weight in kg as described in the Amitran interface
         * @return the class described by the parameters
         */
        virtual SUMOEmissionClass getClass(const SUMOEmissionClass base, const std::string& vClass,
                                           const std::string& fuel, const std::string& eClass, const double weight) const {
            UNUSED_PARAMETER(vClass);
            UNUSED_PARAMETER(fuel);
            UNUSED_PARAMETER(eClass);
            UNUSED_PARAMETER(weight);
            return base;
        }

        /** @brief Returns the vehicle class described by this emission class as described in the Amitran interface (Passenger, ...)
         * Default implementation returns always "Passenger".
         * @param[in] c the emission class
         * @return the name of the vehicle class
         */
        virtual std::string getAmitranVehicleClass(const SUMOEmissionClass c) const {
            UNUSED_PARAMETER(c);
            return "Passenger";
        }

        /** @brief Returns the fuel type described by this emission class as described in the Amitran interface (Gasoline, Diesel, ...)
         * Default implementation returns always "Gasoline".
         * @param[in] c the emission class
         * @return the fuel type
         */
        virtual std::string getFuel(const SUMOEmissionClass c) const {
            UNUSED_PARAMETER(c);
            return "Gasoline";
        }

        /** @brief Returns the Euro emission class described by this emission class as described in the Amitran interface (0, ..., 6)
         * Default implementation returns always 0.
         * @param[in] c the emission class
         * @return the Euro class
         */
        virtual int getEuroClass(const SUMOEmissionClass c) const {
            UNUSED_PARAMETER(c);
            return 0;
        }

        /** @brief Returns a reference weight in kg described by this emission class as described in the Amitran interface
        * It might return -1, if the weight is not important to distinguish different emission classes.
        * Default implementation returns always -1.
        * @param[in] c the emission class
        * @return a reference weight
        */
        virtual double getWeight(const SUMOEmissionClass c) const {
            UNUSED_PARAMETER(c);
            return -1.;
        }
        /// @}

        /** @brief Returns the amount of the emitted pollutant given the vehicle type and state (in mg/s or ml/s for fuel)
         * @param[in] c The vehicle emission class
         * @param[in] e the type of emission (CO, CO2, ...)
         * @param[in] v The vehicle's current velocity
         * @param[in] a The vehicle's current acceleration
         * @param[in] slope The road's slope at vehicle's position [deg]
         * @return The amount emitted by the given emission class when moving with the given velocity and acceleration [mg/s or ml/s]
         */
        virtual double compute(const SUMOEmissionClass c, const EmissionType e, const double v, const double a, const double slope, const std::map<int, double>* param) const {
            UNUSED_PARAMETER(c);
            UNUSED_PARAMETER(e);
            UNUSED_PARAMETER(v);
            UNUSED_PARAMETER(a);
            UNUSED_PARAMETER(slope);
            UNUSED_PARAMETER(param);
            return 0.;
        }

        /** @brief Returns the adapted acceleration value, useful for comparing with external PHEMlight references.
         * Default implementation returns always the input accel.
         * @param[in] c the emission class
         * @param[in] v the speed value
         * @param[in] a the acceleration value
         * @param[in] slope The road's slope at vehicle's position [deg]
         * @return the modified acceleration
         */
        virtual double getModifiedAccel(const SUMOEmissionClass c, const double v, const double a, const double slope) const {
            UNUSED_PARAMETER(c);
            UNUSED_PARAMETER(v);
            UNUSED_PARAMETER(slope);
            return a;
        }

        /** @brief Add all known emission classes of this model to the given container
         * @param[in] list the vector to add to
         */
        void addAllClassesInto(std::vector<SUMOEmissionClass>& list) const {
            myEmissionClassStrings.addKeysInto(list);
        }

    protected:
        /// @brief the name of the model
        const std::string myName;

        /// @brief Mapping between emission class names and integer representations
        StringBijection<SUMOEmissionClass> myEmissionClassStrings;

    private:
        /// @brief invalidate copy constructor
        Helper& operator=(const Helper&) = delete;
    };

    /// @brief the first class in each model representing a zero emission vehicle
    static const int ZERO_EMISSIONS = 0;

    /// @brief the bit to set for denoting heavy vehicles
    static const int HEAVY_BIT = 1 << 15;

    /** @brief Checks whether the string describes a known vehicle class
     * @param[in] eClass The string describing the vehicle emission class
     * @return whether it describes a valid emission class
     */
    static SUMOEmissionClass getClassByName(const std::string& eClass, const SUMOVehicleClass vc = SVC_IGNORING);

    /** @brief Checks whether the string describes a known vehicle class
     * @param[in] eClass The string describing the vehicle emission class
     * @return whether it describes a valid emission class
     */
    static const std::vector<SUMOEmissionClass> getAllClasses();

    /// @brief Get all SUMOEmissionClass in string format
    static const std::vector<std::string>& getAllClassesStr();

    /** @brief Checks whether the string describes a known vehicle class
     * @param[in] eClass The string describing the vehicle emission class
     * @return whether it describes a valid emission class
     */
    static std::string getName(const SUMOEmissionClass c);

    /// @brief return the name for the given emission type
    static std::string getPollutantName(const EmissionType e);

    /** @brief Checks whether the emission class describes a bus, truck or similar vehicle
     * @param[in] c The vehicle emission class
     * @return whether it describes a heavy vehicle
     */
    static bool isHeavy(const SUMOEmissionClass c);

    /** @brief Checks whether the emission class describes an electric or similar silent vehicle
     * @param[in] c The vehicle emission class
     * @return whether it describes a silent vehicle
     */
    static bool isSilent(const SUMOEmissionClass c);

    /** @brief Returns the emission class fittig the given parameters
     * @param[in] base The base emission class to derive from
     * @param[in] vClass The vehicle class description (like "truck")
     * @param[in] eClass The emission class description (like "Euro5")
     * @param[in] fuel The fuel type (like "Diesel")
     * @param[in] weight The weight in kg
     * @return The best fitting emission class related to the base
     */
    static SUMOEmissionClass getClass(const SUMOEmissionClass base, const std::string& vClass, const std::string& fuel, const std::string& eClass, const double weight);

    /** @brief Returns the vehicle class described by the given emission class
     * @param[in] c The vehicle emission class
     * @return The Amitran string describing the vehicle class
     */
    static std::string getAmitranVehicleClass(const SUMOEmissionClass c);

    /** @brief Returns the fuel type of the given emission class
     * @param[in] c The vehicle emission class
     * @return "Diesel", "Gasoline", "HybridDiesel", or "HybridGasoline"
     */
    static std::string getFuel(const SUMOEmissionClass c);

    /** @brief Returns the Euro norm described by the given emission class
     * @param[in] c The vehicle emission class
     * @return A value between 0 and 6 (inclusive)
     */
    static int getEuroClass(const SUMOEmissionClass c);

    /** @brief Returns a representative weight for the given emission class
     * see http://colombo-fp7.eu/deliverables/COLOMBO_D4.2_ExtendedPHEMSUMO_v1.7.pdf
     * @param[in] c The vehicle emission class
     * @return the weight in kg if it matters, 0 otherwise
     */
    static double getWeight(const SUMOEmissionClass c);

    /** @brief Returns the amount of the emitted pollutant given the vehicle type and state (in mg/s or ml/s for fuel)
     * @param[in] c The vehicle emission class
     * @param[in] e the type of emission (CO, CO2, ...)
     * @param[in] v The vehicle's current velocity
     * @param[in] a The vehicle's current acceleration
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return The amount emitted by the given vehicle class when moving with the given velocity and acceleration [mg/s]
     */
    static double compute(const SUMOEmissionClass c, const EmissionType e, const double v, const double a, const double slope, const std::map<int, double>* param = 0);

    /** @brief Returns the amount of all emitted pollutants given the vehicle type and state (in mg/s or ml/s for fuel)
     * @param[in] c The vehicle emission class
     * @param[in] v The vehicle's current velocity
     * @param[in] a The vehicle's current acceleration
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return The amount emitted by the given vehicle class when moving with the given velocity and acceleration [mg/s]
     */
    static Emissions computeAll(const SUMOEmissionClass c, const double v, const double a, const double slope, const std::map<int, double>* param = 0);

    /** @brief Returns the amount of emitted pollutant given the vehicle type and default values for the state (in mg)
     * @param[in] c The vehicle emission class
     * @param[in] e the type of emission (CO, CO2, ...)
     * @param[in] v The vehicle's average velocity
     * @param[in] a The vehicle's average acceleration
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @param{in] tt the time the vehicle travels
     * @return The amount emitted by the given vehicle class [mg]
     */
    static double computeDefault(const SUMOEmissionClass c, const EmissionType e, const double v, const double a, const double slope, const double tt, const std::map<int, double>* param = 0);

    /** @brief Returns the adapted acceleration value, useful for comparing with external PHEMlight references.
     * @param[in] c the emission class
     * @param[in] v the speed value
     * @param[in] a the acceleration value
     * @param[in] slope The road's slope at vehicle's position [deg]
     * @return the modified acceleration
     */
    static double getModifiedAccel(const SUMOEmissionClass c, const double v, const double a, const double slope);

    /// @brief get energy helper
    static const HelpersEnergy& getEnergyHelper() {
        return myEnergyHelper;
    }

private:
    /// @brief Instance of Helper which gets cleaned up automatically
    static Helper myZeroHelper;

    /// @brief Instance of HBEFA2Helper which gets cleaned up automatically
    static HelpersHBEFA myHBEFA2Helper;

    /// @brief Instance of HBEFA3Helper which gets cleaned up automatically
    static HelpersHBEFA3 myHBEFA3Helper;

    /// @brief Instance of PHEMlightHelper which gets cleaned up automatically
    static HelpersPHEMlight myPHEMlightHelper;

    /// @brief Instance of EnergyHelper which gets cleaned up automatically
    static HelpersEnergy myEnergyHelper;

    /// @brief the known model helpers
    static Helper* myHelpers[];

    /// @brief get all emission classes in string format
    static std::vector<std::string> myAllClassesStr;
};
