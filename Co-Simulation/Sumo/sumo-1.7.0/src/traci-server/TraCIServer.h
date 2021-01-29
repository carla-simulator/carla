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
/// @file    TraCIServer.h
/// @author  Axel Wegener
/// @author  Friedemann Wesner
/// @author  Christoph Sommer
/// @author  Tino Morenz
/// @author  Daniel Krajzewicz
/// @author  Thimor Bohn
/// @author  Sascha Krieg
/// @author  Michael Behrisch
/// @author  Leonhard Luecken
/// @date    2007/10/24
///
// TraCI server used to control sumo by a remote TraCI client
/****************************************************************************/
#pragma once
#include <config.h>

#include <map>
#include <string>
#include <set>

#define BUILD_TCPIP
#include <foreign/tcpip/socket.h>
#include <foreign/tcpip/storage.h>
#include <utils/common/NamedRTree.h>
#include <utils/common/SUMOTime.h>
#include <utils/common/ToString.h>
#include <utils/geom/Boundary.h>
#include <utils/geom/Position.h>
#include <utils/geom/GeomHelper.h>
#include <utils/options/OptionsCont.h>
#include <microsim/MSNet.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <libsumo/TraCIConstants.h>
#include <libsumo/Subscription.h>
#include <libsumo/TraCIDefs.h>
#include "TraCIServerAPI_Lane.h"


// ===========================================================================
// class definitions
// ===========================================================================
/** @class TraCIServer
 * @brief TraCI server used to control sumo by a remote TraCI client
 */
class TraCIServer final : public MSNet::VehicleStateListener, public libsumo::VariableWrapper {
public:
    /// @brief Definition of a method to be called for serving an associated commandID
    typedef bool(*CmdExecutor)(TraCIServer& server, tcpip::Storage& inputStorage, tcpip::Storage& outputStorage);

    SUMOTime getTargetTime() const {
        return myTargetTime;
    }

    static TraCIServer* getInstance() {
        return myInstance;
    }

    /// @name Initialisation and Shutdown
    /// @{

    /** @brief Initialises the server
     * @param[in] execs The (additional) command executors to use
     */
    static void openSocket(const std::map<int, CmdExecutor>& execs);


    /// @brief request termination of connection
    static void close();


    /** @brief check whether close was requested
     * @return Whether the connection was closed
     */
    static bool wasClosed();
    /// @}


    /// @brief process all commands until the next SUMO simulation step.
    ///        It is guaranteed that t->getTargetTime() >= myStep after call
    ///        (except the case that a load or close command is received)s
    void processCommandsUntilSimStep(SUMOTime step);

    /// @brief clean up subscriptions
    void cleanup();


    void vehicleStateChanged(const SUMOVehicle* const vehicle, MSNet::VehicleState to, const std::string& info = "");

    /// @name Writing Status Messages
    /// @{

    /** @brief Writes a status command to the given storage
     * @param[in] commandId The id of the command to respond to
     * @param[in] status The status to send
     * @param[in] description The status description (error message, for example)
     * @param[in, filled] outputStorage The storage to write the status into
     */
    void writeStatusCmd(int commandId, int status, const std::string& description, tcpip::Storage& outputStorage);


    /** @brief Writes a status command to myOutputStorage
     * @param[in] commandId The id of the command to respond to
     * @param[in] status The status to send
     * @param[in] description The status description (error message, for example)
     */
    void writeStatusCmd(int commandId, int status, const std::string& description);


    /** @brief Writes a status command to the given storage with status = RTYPE_ERR
     * @param[in] commandId The id of the command to respond to
     * @param[in] description The status description (error message, for example)
     * @param[in, filled] outputStorage The storage to write the status into
     */
    bool writeErrorStatusCmd(int commandId, const std::string& description, tcpip::Storage& outputStorage);
    /// @}



    const std::map<MSNet::VehicleState, std::vector<std::string> >& getVehicleStateChanges() const {
        if (myCurrentSocket == mySockets.end()) {
            // Requested in context of a subscription update
            return myVehicleStateChanges;
        } else {
            // Requested in the context of a custom query by active client
            return myCurrentSocket->second->vehicleStateChanges;
        }
    }

    void writeResponseWithLength(tcpip::Storage& outputStorage, tcpip::Storage& tempMsg);

    void writePositionVector(tcpip::Storage& outputStorage, const libsumo::TraCIPositionVector& shape);


    /// @name Helpers for reading and checking values
    /// @{

    /** @brief Reads the value type and an int, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether an integer value was given (by data type)
     */
    bool readTypeCheckingInt(tcpip::Storage& inputStorage, int& into);


    /** @brief Reads the value type and a double, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether a double value was given (by data type)
     */
    bool readTypeCheckingDouble(tcpip::Storage& inputStorage, double& into);


    /** @brief Reads the value type and a string, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether a string value was given (by data type)
     */
    bool readTypeCheckingString(tcpip::Storage& inputStorage, std::string& into);


    /** @brief Reads the value type and a string list, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether a double value was given (by data type)
     */
    bool readTypeCheckingStringList(tcpip::Storage& inputStorage, std::vector<std::string>& into);


    /** @brief Reads the value type and a double list, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether a double value was given (by data type)
     */
    bool readTypeCheckingDoubleList(tcpip::Storage& inputStorage, std::vector<double>& into);


    /** @brief Reads the value type and a color, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether a color was given (by data type)
     */
    bool readTypeCheckingColor(tcpip::Storage& inputStorage, libsumo::TraCIColor& into);


    /** @brief Reads the value type and a 2D position, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether a 2D position was given (by data type)
     */
    bool readTypeCheckingPosition2D(tcpip::Storage& inputStorage, libsumo::TraCIPosition& into);


    /** @brief Reads the value type and a byte, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether a byte was given (by data type)
     */
    bool readTypeCheckingByte(tcpip::Storage& inputStorage, int& into);


    /** @brief Reads the value type and an unsigned byte, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether an unsigned byte was given (by data type)
     */
    bool readTypeCheckingUnsignedByte(tcpip::Storage& inputStorage, int& into);


    /** @brief Reads the value type and a polygon, verifying the type
     *
     * @param[in, changed] inputStorage The storage to read from
     * @param[out] into Holder of the read value
     * @return Whether an unsigned byte was given (by data type)
     */
    bool readTypeCheckingPolygon(tcpip::Storage& inputStorage, PositionVector& into);
    /// @}


    /// @brief updates myTargetTime and resets vehicle state changes after loading a simulation state
    /// @note  Used in MSStateHandler to update the server's time after loading a state
    void stateLoaded(SUMOTime targetTime);

    std::vector<std::string>& getLoadArgs() {
        return myLoadArgs;
    }

    /// @name VariableWrapper interface
    /// @{
    void initWrapper(const int domainID, const int variable, const std::string& objID);
    bool wrapDouble(const std::string& objID, const int variable, const double value);
    bool wrapInt(const std::string& objID, const int variable, const int value);
    bool wrapString(const std::string& objID, const int variable, const std::string& value);
    bool wrapStringList(const std::string& objID, const int variable, const std::vector<std::string>& value);
    bool wrapPosition(const std::string& objID, const int variable, const libsumo::TraCIPosition& value);
    bool wrapColor(const std::string& objID, const int variable, const libsumo::TraCIColor& value);
    bool wrapRoadPosition(const std::string& objID, const int variable, const libsumo::TraCIRoadPosition& value);
    tcpip::Storage& getWrapperStorage();
    /// @}


private:
    /** @brief Constructor
     * @param[in] port The port to listen to (to open)
     */
    TraCIServer(const SUMOTime begin, const int port, const int numClients);


    /// @brief Destructor
    virtual ~TraCIServer();



    struct SocketInfo {
    public:
        /// @brief constructor
        SocketInfo(tcpip::Socket* socket, SUMOTime t)
            : targetTime(t), socket(socket) {}
        /// @brief destructor
        ~SocketInfo() {
            delete socket;
        }
        /// @brief Target time: next point of action for the client
        SUMOTime targetTime;
        /// @brief Socket object for this client
        tcpip::Socket* socket;
        /// @brief container for vehicle state changes since last step taken by this client
        std::map<MSNet::VehicleState, std::vector<std::string> > vehicleStateChanges;
    private:
        SocketInfo(const SocketInfo&);
    };

    /// @name Server-internal command handling
    /// @{

    /** @brief Returns the TraCI-version
     * @return Always true
     */
    bool commandGetVersion();


    /** @brief Handles subscriptions to send after a simstep2 command
     */
    void postProcessSimulationStep();
    /// @}


    /// @brief Reads the next command ID from the input storage
    /// @return the command ID
    /// @param[out] the version with reference parameters provides information on the command start position and length used in dispatchCommand for checking purposes
    int readCommandID(int& commandStart, int& commandLength);

    /// @brief Handles command, writes response to myOutputStorage
    int dispatchCommand();

    /// @brief Called once after connection of all clients for executing SET_ORDER (and possibly prior GET_VERSION) commands,
    ///        that should be executed before simulation starts (in processCommandsUntilNextSimStep()).
    void checkClientOrdering();

    /// @brief checks for and processes reordering requests (relevant for multiple clients)
    void processReorderingRequests();

    /// @brief get the minimal next target time among all clients
    SUMOTime nextTargetTime() const;

    /// @brief send out subscription results (actually just the content of myOutputStorage) to clients which will act in this step (i.e. with client target time <= myTargetTime)
    void sendOutputToAll() const;

    /// @brief sends an empty response to a simstep command to the current client. (This applies to a situation where the TraCI step frequency is higher than the SUMO step frequency)
    void sendSingleSimStepResponse();

    /// @brief removes myCurrentSocket from mySockets and returns an iterator pointing to the next member according to the ordering
    std::map<int, SocketInfo*>::iterator removeCurrentSocket();


private:
    /// @brief Singleton instance of the server
    static TraCIServer* myInstance;

    /// @brief Whether the connection was set to be to close
    static bool myDoCloseConnection;

    /// @brief The server socket
    tcpip::Socket* myServerSocket;

    /// @brief The socket connections to the clients
    /// the first component (index) determines the client's order (lowest index's commands are processed first), @see CMD_SETORDER
    std::map<int, SocketInfo*> mySockets;

    /// @brief This stores the setOrder(int) requests of the clients.
    std::map<int, SocketInfo*> mySocketReorderRequests;

    /// @brief The currently active client socket
    std::map<int, SocketInfo*>::iterator myCurrentSocket;

    /// @brief The time step to reach until processing the next commands
    SUMOTime myTargetTime;

    /// @brief The storage to read from
    tcpip::Storage myInputStorage;

    /// @brief The storage to write to
    tcpip::Storage myOutputStorage;

    /// @brief A temporary storage to let the wrapper write to
    tcpip::Storage myWrapperStorage;

    /// @brief The last timestep's subscription results
    tcpip::Storage mySubscriptionCache;

    /// @brief Map of commandIds -> their executors; applicable if the executor applies to the method footprint
    std::map<int, CmdExecutor> myExecutors;

    /// @brief Map of variable ids to the size of the parameter in bytes
    std::map<int, int> myParameterSizes;

    std::vector<std::string> myLoadArgs;

    /// @brief The list of known, still valid subscriptions
    std::vector<libsumo::Subscription> mySubscriptions;

    /// @brief The last modified context subscription (the one to add a filter to, see @addSubscriptionFilter(), currently only for vehicle to vehicle context)
    libsumo::Subscription* myLastContextSubscription;

    /// @brief Changes in the states of simulated vehicles
    /// @note
    /// Server cache myVehicleStateChanges is used for managing last steps subscription updates
    /// and for client information in case that myAmEmbedded==true, which implies a single client.
    /// For the potential multiclient case (myAmEmbedded==false), each socket in mySockets is associated
    /// with a proper vehicleStateChanges container mySockets[...].second->vehicleStateChanges
    /// Performance could be improved if for a single client, myVehicleStateChanges is used only.
    std::map<MSNet::VehicleState, std::vector<std::string> > myVehicleStateChanges;

private:
    bool addObjectVariableSubscription(const int commandId, const bool hasContext);
    void initialiseSubscription(libsumo::Subscription& s);
    void removeSubscription(int commandId, const std::string& identity, int domain);
    bool processSingleSubscription(const libsumo::Subscription& s, tcpip::Storage& writeInto,
                                   std::string& errors);


    bool addSubscriptionFilter();
    void removeFilters();
    void addSubscriptionFilterLanes(std::vector<int> lanes);
    void addSubscriptionFilterNoOpposite();
    void addSubscriptionFilterDownstreamDistance(double dist);
    void addSubscriptionFilterUpstreamDistance(double dist);
    void addSubscriptionFilterLeadFollow();
    // TODO: for libsumo, implement convenience definitions present in python client:
    //    void addSubscriptionFilterCF();
    //    void addSubscriptionFilterLC(int direction);
    void addSubscriptionFilterTurn();
    void addSubscriptionFilterVClass(SVCPermissions vClasses);
    void addSubscriptionFilterVType(std::set<std::string> vTypes);
    /** @brief Filter only vehicles within field of vision
     *
     * @param[in] openingAngle The opening angle of the circle sector
     */
    void addSubscriptionFilterFieldOfVision(double openingAngle);
    /** @brief Filter only vehicles within the given lateral distance
     *
     * @param[in] dist The lateral distance
     */
    void addSubscriptionFilterLateralDistance(double dist);

    bool findObjectShape(int domain, const std::string& id, PositionVector& shape);

    /// @brief check whether a found objID refers to the central object of a context subscription
    bool centralObject(const libsumo::Subscription& s, const std::string& objID);


private:
    /// @brief Invalidated assignment operator
    TraCIServer& operator=(const TraCIServer& s);

};


