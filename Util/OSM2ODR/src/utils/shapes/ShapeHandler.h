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
/// @file    ShapeHandler.h
/// @author  Jakob Erdmann
/// @date    Feb 2015
///
// The XML-Handler for network loading
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/common/RGBColor.h>
#include <utils/geom/Position.h>
#include <utils/xml/SUMOSAXHandler.h>


// ===========================================================================
// class declarations
// ===========================================================================
class ShapeContainer;
class Parameterised;
class GeoConvHelper;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class ShapeHandler
 * @brief The XML-Handler for network loading
 *
 * The SAX2-handler responsible for parsing networks and routes to load.
 * This is an extension of the MSRouteHandler as routes and vehicles may also
 *  be loaded from network descriptions.
 */
class ShapeHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     * @param[in] file Name of the parsed file
     * @param[in, out] net The network to fill
     * @param[in] detBuilder The detector builder to use
     * @param[in] triggerBuilder The trigger builder to use
     * @param[in] edgeBuilder The builder of edges to use
     * @param[in] junctionBuilder The builder of junctions to use
     */
    ShapeHandler(const std::string& file, ShapeContainer& sc, const GeoConvHelper* = nullptr);

    /// @brief Destructor
    virtual ~ShapeHandler();

    /// @brief loads all of the given files
    static bool loadFiles(const std::vector<std::string>& files, ShapeHandler& sh);

protected:
    /// @name inherited from GenericSAXHandler
    //@{
    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     * @todo Refactor/describe
     */
    virtual void myStartElement(int element, const SUMOSAXAttributes& attrs);

    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     * @todo Refactor/describe
     */
    virtual void myEndElement(int element);
    //@}

    /// @brief get position for a given laneID (Has to be implemented in all child)
    virtual Position getLanePos(const std::string& poiID, const std::string& laneID, double lanePos, double lanePosLat) = 0;

    /// @brief Whether some input attributes shall be automatically added as params  (Can be implemented in all child)
    virtual bool addLanePosParams();

protected:
    /// @brief set default values
    void setDefaults(const std::string& prefix, const RGBColor& color, const double layer, const bool fill = false);

    /// @brief adds a POI
    void addPOI(const SUMOSAXAttributes& attrs, const bool ignorePruning, const bool useProcessing);

    /// @brief adds a polygon
    void addPoly(const SUMOSAXAttributes& attrs, const bool ignorePruning, const bool useProcessing);

    /// @brief get last parameterised object
    Parameterised* getLastParameterised() const;

protected:
    /// @brief reference to shape container in which all Shares are being added
    ShapeContainer& myShapeContainer;

    /// @brief The prefix to use
    std::string myPrefix;

    /// @brief The default color to use
    RGBColor myDefaultColor;

    /// @brief The default layer to use
    double myDefaultLayer;

    /// @brief Information whether polygons should be filled
    bool myDefaultFill;

    /// @brief element to receive parameters
    Parameterised* myLastParameterised;

    /// @brief geo-conversion to use during loading
    const GeoConvHelper* myGeoConvHelper;

    /// @brief invalidate copy constructor
    ShapeHandler(const ShapeHandler& s) = delete;

    /// @brief invalidate assignment operator
    ShapeHandler& operator=(const ShapeHandler& s) = delete;
};
