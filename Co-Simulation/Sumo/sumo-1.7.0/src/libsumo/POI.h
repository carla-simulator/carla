/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2012-2020 German Aerospace Center (DLR) and others.
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
/// @file    POI.h
/// @author  Daniel Krajzewicz
/// @author  Mario Krumnow
/// @author  Michael Behrisch
/// @author  Robert Hilbrich
/// @date    30.05.2012
///
// C++ TraCI client API implementation
/****************************************************************************/
#pragma once
#include <config.h>

#include <vector>
#include <libsumo/TraCIDefs.h>
#include <utils/shapes/Shape.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NamedRTree;
class PointOfInterest;
class PositionVector;
namespace libsumo {
class VariableWrapper;
}


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class POI
 * @brief C++ TraCI client API implementation
 */
namespace libsumo {
class POI {
public:
    static std::vector<std::string> getIDList();
    static int getIDCount();
    static std::string getType(const std::string& poiID);
    static TraCIPosition getPosition(const std::string& poiID, const bool includeZ = false);
    static TraCIColor getColor(const std::string& poiID);
    static double getWidth(const std::string& poiID);
    static double getHeight(const std::string& poiID);
    static double getAngle(const std::string& poiID);
    static std::string getImageFile(const std::string& poiID);
    static std::string getParameter(const std::string& poiID, const std::string& key);
    LIBSUMO_GET_PARAMETER_WITH_KEY_API

    static void setType(const std::string& poiID, const std::string& setType);
    static void setColor(const std::string& poiID, const TraCIColor& c);
    static void setPosition(const std::string& poiID, double x, double y);
    static void setWidth(const std::string& poiID, double width);
    static void setHeight(const std::string& poiID, double height);
    static void setAngle(const std::string& poiID, double angle);
    static void setImageFile(const std::string& poiID, const std::string& imageFile);
    static bool add(const std::string& poiID, double x, double y, const TraCIColor& color, const std::string& poiType = "", int layer = 0, const std::string& imgFile = Shape::DEFAULT_IMG_FILE, double width = Shape::DEFAULT_IMG_WIDTH, double height = Shape::DEFAULT_IMG_HEIGHT, double angle = Shape::DEFAULT_ANGLE);
    static bool remove(const std::string& poiID, int layer = 0);
    static void highlight(const std::string& poiID, const TraCIColor& col = TraCIColor(255, 0, 0, 255), double size = -1, const int alphaMax = -1, const double duration = -1, const int type = 0);

    static void setParameter(const std::string& poiID, const std::string& key, const std::string& value);

    LIBSUMO_SUBSCRIPTION_API

    /** @brief Returns a tree filled with PoI instances
     *  @return The rtree of PoIs
     */
    static NamedRTree* getTree();
    static void cleanup();

    /** @brief Saves the shape of the requested object in the given container
    *  @param id The id of the poi to retrieve
    *  @param shape The container to fill
    */
    static void storeShape(const std::string& id, PositionVector& shape);

    static std::shared_ptr<VariableWrapper> makeWrapper();

    static bool handleVariable(const std::string& objID, const int variable, VariableWrapper* wrapper);

private:
    static PointOfInterest* getPoI(const std::string& id);


private:
    static SubscriptionResults mySubscriptionResults;
    static ContextSubscriptionResults myContextSubscriptionResults;
    static NamedRTree* myTree;

    /// @brief invalidated standard constructor
    POI() = delete;
};


}
