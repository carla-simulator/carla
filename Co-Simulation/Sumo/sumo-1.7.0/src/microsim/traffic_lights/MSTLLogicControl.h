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
/// @file    MSTLLogicControl.h
/// @author  Daniel Krajzewicz
/// @author  Julia Ringel
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @author  Friedemann Wesner
/// @date    Sept 2002
///
// A class that stores and controls tls and switching of their programs
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <map>
#include <utils/common/Command.h>
#include <utils/common/StdDefs.h>
#include <utils/xml/SUMOXMLDefinitions.h>


// ===========================================================================
// class declarations
// ===========================================================================
class MSTrafficLightLogic;
class MSLink;
class MSLane;
class MSPhaseDefinition;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class MSTLLogicControl
 * @brief A class that stores and controls tls and switching of their programs
 *
 * This class holds all traffic light logics (programs) and their
 *  variants during the simulation. In addition, the schedule for switching
 *  between different tls programs are also stored.
 *
 * When a WAUT is forced to switch, for each TLS, a switching procedure
 *  derived from WAUTSwitchProcedure is initialised and is asked repeatedly
 *  whether a switch could be done until it returns true.
 */
class MSTLLogicControl {
public:
    /**
     * @class OnSwitchAction
     * @brief Base class for things to execute if a tls switches to a new phase
     */
    class OnSwitchAction {
    public:
        /// @brief Destructor.
        virtual ~OnSwitchAction() {};


        /** @brief Executes the action
         */
        virtual void execute() = 0;

    };



    /**
     * @class TLSLogicVariants
     * @brief Storage for all programs of a single tls
     *
     * This class joins all programs of a single tls.
     */
    class TLSLogicVariants {
    public:
        /// @brief Constructor
        TLSLogicVariants();


        /// @brief Destructor
        ~TLSLogicVariants();


        /** @brief Verifies traffic lights loaded from the network
         *
         * Compare the phase state sizes of each phase with the according tls' number
         *  of controlled links.
         * @return Whether all tls programs are valid
         */
        bool checkOriginalTLS() const;


        /** @brief Adds a logic (program)
         *
         * @param[in] programID The sub-id of this program
         * @param[in] logic The logic to add
         * @param[in] netWasLoaded Whether the network has already been loaded (the links have been assigned)
         * @param[in] isNewDefault Whether this logic shall be treated as the currently active logic
         */
        bool addLogic(const std::string& programID, MSTrafficLightLogic* logic, bool netWasLoaded,
                      bool isNewDefault = true);



        MSTrafficLightLogic* getLogic(const std::string& programID) const;
        void addSwitchCommand(OnSwitchAction* c);
        std::vector<MSTrafficLightLogic*> getAllLogics() const;
        void saveInitialStates();
        bool isActive(const MSTrafficLightLogic* tl) const;
        MSTrafficLightLogic* getActive() const;

        /// @brief return the default program (that last used program except TRACI_PROGRAM)
        MSTrafficLightLogic* getDefault() const;

        void switchTo(MSTLLogicControl& tlc, const std::string& programID);

        /* @brief get logic by programID. For the special case "off"
         * instantiate an MSOffTrafficLightLogic */
        MSTrafficLightLogic* getLogicInstantiatingOff(MSTLLogicControl& tlc,
                const std::string& programID);

        /* @brief sets the state to the given string get for the special program "online"
         * this program is instantiated only once */
        void setStateInstantiatingOnline(MSTLLogicControl& tlc,
                                         const std::string& state);


        void executeOnSwitchActions() const;
        void addLink(MSLink* link, MSLane* lane, int pos);
        void ignoreLinkIndex(int pos);


    private:
        /// @brief The currently used program
        MSTrafficLightLogic* myCurrentProgram;

        /// @brief The program that would be used in the absence of TraCI
        MSTrafficLightLogic* myDefaultProgram;

        /// @brief A map of subkeys to programs
        std::map<std::string, MSTrafficLightLogic*> myVariants;

        /// @brief Originally loaded link states
        std::map<MSLink*, LinkState> myOriginalLinkStates;

        /// @brief The list of actions/commands to execute on switch
        std::vector<OnSwitchAction*> mySwitchActions;


    private:
        /// @brief Invalidated copy constructor.
        TLSLogicVariants(const TLSLogicVariants&);

        /// @brief Invalidated assignment operator.
        TLSLogicVariants& operator=(const TLSLogicVariants&);


    };



    /// @brief Constructor
    MSTLLogicControl();


    /// @brief Destructor
    ~MSTLLogicControl();


    /** @brief Lets MSTLLogicControl know that the network has been loaded
     *
     * This method must be called after the network (including the initial tls
     *  definitions) was loaded.
     *
     * The originally loaded traffic lights are also verified herein by calling
     *  TLSLogicVariants::checkOriginalTLS, first.
     *
     * The MSTLLogicControl is informed in order to know that link information
     *  is known for the tls programs loaded afterwards so that it may be adapted
     *  from the previously loaded tls (a net may only contain one program per tls).
     *
     * The states of the links controlled by tls are saved for their potential later usage
     *  (if the tls is switched to off-mode).
     *
     * @return Whether the traffic lights could be initialised and are correct
     */
    bool closeNetworkReading();


    /** @brief Lets all running (current) tls programs apply their current signal states to links they control
     * @param[in] t The current time
     * @see MSTrafficLightLogic::setTrafficLightSignals
     * @see LinkState
     * @see MSLink::setTLState
     */
    void setTrafficLightSignals(SUMOTime t) const;


    /** @brief Returns a vector which contains all logics
     *
     * All logics are included, active (current) and non-active
     * @return A vector containing all loaded logics
     */
    std::vector<MSTrafficLightLogic*> getAllLogics() const;


    /** @brief Returns the variants of a named tls
     *
     * @param[in] id The id of the tls to get variants of
     * @return The variants of the named tls
     * @exception InvalidArgument
     */
    TLSLogicVariants& get(const std::string& id) const;


    /** @brief Returns a single program (variant) defined by the tls id and the program programID
     *
     * @param[in] id The id of the tls to get program of
     * @param[in] programID The program id of the tls program to get
     * @return The defined tls program if existing, 0 otherwise
     */
    MSTrafficLightLogic* get(const std::string& id, const std::string& programID) const;


    /** @brief Returns the active program of a named tls
     *
     * @param[in] id The id of the tls to get the active program of
     * @return The current program of the defined tls if existing, 0 otherwise
     */
    MSTrafficLightLogic* getActive(const std::string& id) const;


    /**
     * Returns the ids of all existing variants-structures, wich are the ids of their
     * contained tls logics (not the logic's programm-ids)
     * @return the list of ids
     */
    std::vector<std::string> getAllTLIds() const;


    /** @brief Adds a tls program to the container
     *
     * If a tls with the given id is not yet known, a TLSLogicVariants structure
     *  is built for this tls and added to the internal container and the tls
     *  program is used as the new default.
     *
     * If the tls to add is loaded from an additional file (indicated by myNetWasLoaded,
     *  see closeNetworkReading), links from previously loaded tls are adapted to the logic.
     *  This may throw a ProcessError in the case no tls program was loaded for this
     *  tls before (was not defined in the network).
     *
     * The parameter newDefault defines whether this program will be used as the new
     *  default program of this tls. This means that an existing tls program for this
     *  tls is replaced within the according TLSLogicVariants structure.
     *
     * @param[in] id The id of the tls (program) to add
     * @param[in] programID The program id of the tls (program) to add
     * @param[in] logic The tls logic to insert
     * @exception ProcessError In the case an additional tls program is loaded and no one for the tls existed in the network
     * @return true if the tls program could be added, false otherwise
     */
    bool add(const std::string& id, const std::string& programID,
             MSTrafficLightLogic* logic, bool newDefault = true);



    /** @brief Returns the information whether the named tls is stored
     * @param[in] id The id of the tls to ask for
     * @return Whether a tls with the given id is known
     */
    bool knows(const std::string& id) const;


    /** @brief Returns whether the given tls program is the currently active for his tls
     * @param[in] tl The tls to ask for
     * @return Whether the given tl is currently active (or a different program is used)
     */
    bool isActive(const MSTrafficLightLogic* tl) const;


    /** @brief Switches the named (id) tls to the named (programID) program
     *
     * The program with the used programID must be previously added.
     * If the tls itself or the program to switch to is not known, false is returned.
     * @param[in] id The id of the tls to switch to
     * @param[in] programID The program id of the tls (program) to switch to
     * @exception ProcessError If either the tls or the program to switch to is not known
     */
    void switchTo(const std::string& id, const std::string& programID);



    /// @name WAUT definition methods
    /// @{

    /** @brief Adds a WAUT definition
     *
     * Throws an InvalidArgument if the given id is already in use.
     * @param[in] refTime The reference time of the WAUT
     * @param[in] id The ID of the WAUT
     * @param[in] startProg The begin program of the WAUT
     * @param[in] period The period with which to repeat the switches
     * @exception InvalidArgument If the id is already used by another WAUT
     */
    void addWAUT(SUMOTime refTime, const std::string& id,
                 const std::string& startProg, SUMOTime period);


    /** @brief Adds a WAUT switch step to a previously built WAUT
     *
     * Throws an InvalidArgument if the given WAUT id is not known.
     * @param[in] wautid The ID of the WAUT
     * @param[in] when The switch procedure begin
     * @param[in] to The program the WAUT shall start to switch to at the given time
     * @exception InvalidArgument If the named WAUT is not known
     */
    void addWAUTSwitch(const std::string& wautid, SUMOTime when,
                       const std::string& to);


    /** @brief Adds a tls to the list of tls to be switched by the named WAUT
     *
     * Passes the values directly to the used tls control. This throws an InvalidArgument
     *  if the given WAUT id or the given junction id is not known.
     * @param[in] wautid The ID of the WAUT
     * @param[in] tls The id of the tls to be switched
     * @param[in] proc The switching procedure to use
     * @param[in] synchron Whether the switching shall be done in synchron mode
     * @exception InvalidArgument If the named WAUT or the named tls are not known
     * @exception ProcessError If the initial switch fails
     */
    void addWAUTJunction(const std::string& wautid, const std::string& tls,
                         const std::string& proc, bool synchron);


    /** @brief Closes loading of a WAUT
     *
     * Instantiates the first switch ("SwitchInitCommand") for the WAUT into the
     *  network's simulation time step begin event control.
     * Throws an InvalidArgument if the given WAUT id is not known.
     *
     * @param[in] wautid The ID of the WAUT
     * @exception InvalidArgument If the named WAUT is not known
     * @see SwitchInitCommand
     */
    void closeWAUT(const std::string& wautid);
    /// @}



    /** @brief Checks whether any WAUT is trying to switch a tls into another program
     *
     * Called from MSNet::simulationStep
     */
    void check2Switch(SUMOTime step);


    /** @brief return the complete phase definition for a named traffic lights logic
     *
     * The phase definition will be the current of the currently active program of
     *  the named tls.
     * @param[in] tlid The id of the tls to get the current phases of
     * @return A pair containing the current time and the current phases of the named tls
     */
    std::pair<SUMOTime, MSPhaseDefinition> getPhaseDef(const std::string& tlid) const;

    /// @brief switch all logic variants to 'off'
    void switchOffAll();



protected:
    /**
     * @class SwitchInitCommand
     * @brief This event-class is used to initialise a WAUT switch at a certain time.
     *
     * This command is reused. The index of the WAUT-switch is incremented at each
     *  call to the control.
     */
    class SwitchInitCommand : public Command {
    public:
        /** @brief Constructor
         * @param[in] p The logic control
         * @param[in] wautid The id of the WAUT
         * @param[in] index The first position within the WAUT table
         */
        SwitchInitCommand(MSTLLogicControl& p, const std::string& wautid, int index)
            : myParent(p), myWAUTID(wautid), myIndex(index) { }


        /// @brief Destructor
        ~SwitchInitCommand() { }



        /// @name Derived from Command
        /// @{

        /** @brief Begins a WAUT switch by executing the command.
         *
         * The parent's "initWautSwitch" method is called supporting
         *  this command as an argument. The result of "initWautSwitch"
         *  is returned.
         *
         * "initWautSwitch" may throw an ProcessError if the program
         *  to switch to is not known.
         *
         * @param[in] currentTime The current simulation time (unused)
         * @return The time after which the command shall be executed again
         * @exception ProcessError If the program to switch to does not exist
         * @see MSTLLogicControl::initWautSwitch
         */
        SUMOTime execute(SUMOTime) {
            return myParent.initWautSwitch(*this);
        }
        /// @}



        /** @brief Returns the WAUT-id
         * @return The WAUT id
         */
        const std::string& getWAUTID() const {
            return myWAUTID;
        }


        /** @brief Returns a reference to the index
         * @return A reference to the index
         */
        int& getIndex() {
            return myIndex;
        }


    protected:
        /// @brief The control to call
        MSTLLogicControl& myParent;

        /// @brief The id of the WAUT that shall switch
        std::string myWAUTID;

        /// @brief The current index within the WAUT switch table
        int myIndex;


    private:
        /// @brief Invalidated copy constructor.
        SwitchInitCommand(const SwitchInitCommand&);

        /// @brief Invalidated assignment operator.
        SwitchInitCommand& operator=(const SwitchInitCommand&);

    };



public:
    /** @brief Initialises switching a WAUT
     *
     * This method is called from a previously built SwitchInitCommand
     * @param[in] The command which initialises the switch
     * @return The time offset to next call
     */
    SUMOTime initWautSwitch(SwitchInitCommand& cmd);


protected:
    /** @struct WAUTSwitch
     * @brief Storage for a WAUTs switch point
     */
    struct WAUTSwitch {
        /// @brief The time the WAUT shall switch the TLS
        SUMOTime when;
        /// @brief The program name the WAUT shall switch the TLS to
        std::string to;
    };


    /** @struct WAUTJunction
     * @brief Storage for a junction assigned to a WAUT
     */
    struct WAUTJunction {
        /// @brief The junction name
        std::string junction;
        /// @brief The procedure to switch the junction with
        std::string procedure;
        /// @brief Information whether this junction shall be switched synchron
        bool synchron;
    };


    /** @struct WAUT
     * @brief A WAUT definition
     */
    struct WAUT {
        /// @brief The id of the WAUT
        std::string id;
        /// @brief The name of the start program
        std::string startProg;
        /// @brief The reference time (offset to the switch times)
        SUMOTime refTime;
        /// @brief The period with which to repeat switches
        SUMOTime period;
        /// @brief The list of switches to be done by the WAUT
        std::vector<WAUTSwitch> switches;
        /// @brief The list of switches assigned to the WAUT
        std::vector<WAUTJunction> junctions;
    };


    /** @class WAUTSwitchProcedure
     * @brief This is the abstract base class for switching from one tls program to another.
     */
    class WAUTSwitchProcedure {
    public:
        /** @brief Constructor
         * @param[in] control The responsible tls control
         * @param[in] waut The WAUT to switch
         * @param[in] from The original tls program
         * @param[in] to The destination tls program
         * @param[in] synchron Whether the switch shall be done in synchronuous mode
         */
        WAUTSwitchProcedure(MSTLLogicControl& control, WAUT& waut,
                            MSTrafficLightLogic* from, MSTrafficLightLogic* to,
                            bool synchron)
            : myFrom(from), myTo(to), mySwitchSynchron(synchron), myWAUT(waut), myControl(control) { }


        /// @brief Destructor
        virtual ~WAUTSwitchProcedure() { }


        /** @brief Determines whether a switch is possible.
         * @param[in] step The current simulation step
         * @return If a switch shall be done, this method should return true.
         */
        virtual bool trySwitch(SUMOTime step);


    protected:
        /** @brief Checks, whether the position of a signal programm is at the GSP ("Good Switching Point")
         *
         * The GSP must be given as a logic's parameter ("GSP"). Not the simulation second,
         *  but the phase the GSP lies within is used. If the phase the GSP lies within is
         *  the same as the logic's current phase, the result is true.
         * @param[in] currentTime The current time step
         * @param[in] logic The logic for which this should be examined
         * @return Whether the current step is the GSP
         * @see getGSPValue
         */
        bool isPosAtGSP(SUMOTime currentTime, const MSTrafficLightLogic& logic);

        /** @brief Returns the difference between a given time and the start of the phase
         * @param[in] logic The logic to consider
         * @param[in] toTime The time to ask for
         * @return How much time elapsed between the last pahse start and the given time
         */
        SUMOTime getDiffToStartOfPhase(MSTrafficLightLogic& logic, SUMOTime toTime);

        /** @brief switches the given logic directly to the given position
         * @param[in] simStep The current simulation time
         * @param[in] logic The logic to switch
         * @param[in] toTime The time offset within the logic's phases to switch to
         */
        void switchToPos(SUMOTime simStep, MSTrafficLightLogic& logic, SUMOTime toTime);

        /** @brief Returns the GSP-value
         *
         * The GSP must be given as a logic's parameter ("GSP").
         * @param[in] logic The logic to retrieve the GSP from
         * @return The GSP value; 0 if not given.
         * @see MSTrafficLightLogic::getParameterValue
         */
        SUMOTime getGSPTime(const MSTrafficLightLogic& logic) const;

        /** @brief Changes the destination program's phase to which the tls was switched
         *
         * Default does nothing, implemented only in the subclasses.
         * @param[in] step The current simulation time
         */
        virtual void adaptLogic(SUMOTime step) {
            UNUSED_PARAMETER(step);
        }

    protected:
        /// @brief The current program of the tls to switch
        MSTrafficLightLogic* myFrom;

        /// @brief The program to switch the tls to
        MSTrafficLightLogic* myTo;

        /// @brief Information whether to switch synchron (?)
        bool mySwitchSynchron;

        /// @brief The WAUT responsible for switching
        WAUT& myWAUT;

        /// @brief The control the logic belongs to
        MSTLLogicControl& myControl;


    private:
        /// @brief Invalidated copy constructor.
        WAUTSwitchProcedure(const WAUTSwitchProcedure&);

        /// @brief Invalidated assignment operator.
        WAUTSwitchProcedure& operator=(const WAUTSwitchProcedure&);

    };


    /**
     * @class WAUTSwitchProcedure_JustSwitch
     * @brief This class simply switches to the next program
     */
    class WAUTSwitchProcedure_JustSwitch : public WAUTSwitchProcedure {
    public:
        /** @brief Constructor
         * @param[in] control The responsible tls control
         * @param[in] waut The WAUT to switch
         * @param[in] from The original tls program
         * @param[in] to The destination tls program
         * @param[in] synchron Whether the switch shall be done in synchronuous mode
         */
        WAUTSwitchProcedure_JustSwitch(MSTLLogicControl& control, WAUT& waut,
                                       MSTrafficLightLogic* from, MSTrafficLightLogic* to,
                                       bool synchron);

        /// @brief Destructor
        ~WAUTSwitchProcedure_JustSwitch();

        /** @brief Determines whether a switch is possible.
         * @param[in] step The current simulation step
         * @return This implementation alsways returns true
         */
        bool trySwitch(SUMOTime step);

    };



    /**
     * @class WAUTSwitchProcedure_GSP
     * @brief This class switches using the GSP algorithm.
     */
    class WAUTSwitchProcedure_GSP : public WAUTSwitchProcedure {
    public:
        /** @brief Constructor
         * @param[in] control The responsible tls control
         * @param[in] waut The WAUT to switch
         * @param[in] from The original tls program
         * @param[in] to The destination tls program
         * @param[in] synchron Whether the switch shall be done in synchronuous mode
         */
        WAUTSwitchProcedure_GSP(MSTLLogicControl& control, WAUT& waut,
                                MSTrafficLightLogic* from, MSTrafficLightLogic* to,
                                bool synchron);

        /// @brief Destructor
        ~WAUTSwitchProcedure_GSP();

    protected:
        /** @brief Stretches the destination program's phase to which the tls was switched
         */
        void adaptLogic(SUMOTime step);

    };


    /**
     * @class WAUTSwitchProcedure_Stretch
     * @brief This class switches using the Stretch algorithm.
     */
    class WAUTSwitchProcedure_Stretch : public WAUTSwitchProcedure {
    public:
        /** @brief Constructor
         * @param[in] control The responsible tls control
         * @param[in] waut The WAUT to switch
         * @param[in] from The original tls program
         * @param[in] to The destination tls program
         * @param[in] synchron Whether the switch shall be done in synchronuous mode
         */
        WAUTSwitchProcedure_Stretch(MSTLLogicControl& control, WAUT& waut,
                                    MSTrafficLightLogic* from, MSTrafficLightLogic* to,
                                    bool synchron);

        /// @brief Destructor
        ~WAUTSwitchProcedure_Stretch();

    protected:
        /** @brief Determines the destination program's changes and applies them
         * @param[in] step The current simulation step
         * @see cutLogic
         * @see stretchLogic
         */
        void adaptLogic(SUMOTime step);

        /** @brief Stretches the logic to synchronize
         * @param[in] step The current simulation step
         * @param[in] startPos The position in the destination program to switch to
         * @param[in] allStretchTime The amount by which the logic shall be streched
         */
        void stretchLogic(SUMOTime step, SUMOTime startPos, SUMOTime allStretchTime);

        /** @brief Cuts the logic to synchronize
         * @param[in] step The current simulation step
         * @param[in] startPos The position in the destination program to switch to
         * @param[in] allCutTime The amount by which the logic shall be cut
         */
        void cutLogic(SUMOTime step, SUMOTime startPos, SUMOTime allCutTime);

    protected:
        /** @struct StretchRange
         * @brief A definition of a stretch - Bereich
         */
        struct StretchRange {
            /// @brief The begin of a stretch/cut area
            SUMOTime begin;
            /// @brief The end of a stretch/cut area
            SUMOTime end;
            /// @brief The weight factor of a stretch/cut area
            double fac;
        };

    protected:
        /// @brief the given Stretch-areas for the "to" program, this is 0-based indexed, while the input is 1-based
        std::vector<StretchRange> myStretchRanges;
    };


    /**
     * @struct WAUTSwitchProcess
     * @brief An initialised switch process
     */
    struct WAUTSwitchProcess {
        /// @brief The id of the junction to switch
        std::string junction;
        /// @brief The current program of the tls
        MSTrafficLightLogic* from;
        /// @brief The program to switch the tls to
        MSTrafficLightLogic* to;
        /// @brief The used procedure
        WAUTSwitchProcedure* proc;
    };


    /// @brief A map of ids to corresponding WAUTs
    std::map<std::string, WAUT*> myWAUTs;

    /// @brief A list of currently running switching procedures
    std::vector<WAUTSwitchProcess> myCurrentlySwitched;

    /// @brief A map from ids to the corresponding variants
    std::map<std::string, TLSLogicVariants*> myLogics;

    /// @brief Information whether the net was completely loaded
    bool myNetWasLoaded;


private:
    /// @brief Invalidated copy constructor.
    MSTLLogicControl(const MSTLLogicControl&);

    /// @brief Invalidated assignment operator.
    MSTLLogicControl& operator=(const MSTLLogicControl&);

};
