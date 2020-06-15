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
/// @file    NIXMLTrafficLightsHandler.h
/// @author  Jakob Erdmann
/// @date    2011-10-05
///
// Importer for traffic lights stored in XML
/****************************************************************************/
#pragma once
#include <config.h>

#include <utils/xml/SUMOSAXHandler.h>
#include <netbuild/NBConnection.h>


// ===========================================================================
// class declarations
// ===========================================================================
class NBEdge;
class NBEdgeCont;
class MsgHandler;
class NBLoadedSUMOTLDef;
class NBTrafficLightLogicCont;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class NIXMLTrafficLightsHandler
 * @brief Importer for edge connections stored in XML
 *
 * This importer parses connections, and prohibitions, and is able
 *  to reset connections between edges.
 */
class NIXMLTrafficLightsHandler : public SUMOSAXHandler {
public:
    /** @brief Constructor
     * @param[in] ec The traffic light container into which to load logics
     */
    NIXMLTrafficLightsHandler(NBTrafficLightLogicCont& tlCont, NBEdgeCont& ec, bool ignoreUnknown = false);


    /// @brief Destructor
    ~NIXMLTrafficLightsHandler();


protected:
    /// @name inherited from GenericSAXHandler
    //@{

    /** @brief Called on the opening of a tag;
     *
     * @param[in] element ID of the currently opened element
     * @param[in] attrs Attributes within the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myStartElement
     */
    void myStartElement(int element, const SUMOSAXAttributes& attrs);


    /** @brief Called when a closing tag occurs
     *
     * @param[in] element ID of the currently opened element
     * @exception ProcessError If something fails
     * @see GenericSAXHandler::myEndElement
     */
    void myEndElement(int element);
    //@}



private:
    /// @brief The traffic light container to fill
    NBTrafficLightLogicCont& myTLLCont;

    /// @brief The edge container for retrieving edges
    NBEdgeCont& myEdgeCont;

    /// @brief The currently parsed traffic light
    NBLoadedSUMOTLDef* myCurrentTL;

    /// @brief whether phases of a previously loaded traffic light must be reset
    bool myResetPhases;

    /** begins the reading of a traffic lights logic
     * This differs from NIImporter_SUMO::initTrafficLightLogic insofar as
     * partial definitions (diffs) are allowed in an xml file
     */
    NBLoadedSUMOTLDef* initTrafficLightLogic(const SUMOSAXAttributes& attrs, NBLoadedSUMOTLDef* currentTL);

    /// @brief reads and adds tl-controlled connection
    void addTlConnection(const SUMOSAXAttributes& attrs);

    /// @brief reads and removes tl-controlled connection
    void removeTlConnection(const SUMOSAXAttributes& attrs);

    /// parses and edge id an returns an existing edge
    NBEdge* retrieveEdge(const SUMOSAXAttributes& attrs, SumoXMLAttr attr, bool& ok);

    /// parses a lane index and verifies its correctness
    int retrieveLaneIndex(const SUMOSAXAttributes& attrs, SumoXMLAttr attr, NBEdge* edge, bool& ok, bool isDelete = false);

    /// @brief whether definitions for unknown traffic lights shall be silently ignored
    bool myIgnoreUnknown;


private:
    /// @brief invalidated copy constructor
    NIXMLTrafficLightsHandler(const NIXMLTrafficLightsHandler& s);

    /// @brief invalidated assignment operator
    NIXMLTrafficLightsHandler& operator=(const NIXMLTrafficLightsHandler& s);


};
