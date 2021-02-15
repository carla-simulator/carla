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
/// @file    GNEPolygonFrame.h
/// @author  Pablo Alvarez Lopez
/// @date    Aug 2017
///
// The Widget for add polygons
/****************************************************************************/
#pragma once
#include <netedit/frames/GNEFrame.h>

// ===========================================================================
// class definitions
// ===========================================================================
/**
* @class GNEPolygonFrame
* The Widget for setting internal attributes of shape elements
*/
class GNEPolygonFrame : public GNEFrame {

public:

    /// @brief enum with all possible values after try to create an shape using frame
    enum class AddShape {
        SUCCESS,                // Shape was successfully created
        UPDATEDTEMPORALSHAPE,   // Added or removed a new point to temporal shape
        INVALID,                // Shape wasn't created
        NOTHING                 // Nothing to do
    };

    // ===========================================================================
    // class GEOPOICreator
    // ===========================================================================

    class GEOPOICreator : protected FXGroupBox {
        /// @brief FOX-declaration
        FXDECLARE(GNEPolygonFrame::GEOPOICreator)

    public:
        /// @brief constructor
        GEOPOICreator(GNEPolygonFrame* polygonFrameParent);

        /// @brief destructor
        ~GEOPOICreator();

        /// @brief Show list of GEOPOICreator Modul
        void showGEOPOICreatorModul();

        /// @brief hide GEOPOICreator Modul
        void hideGEOPOICreatorModul();

        /// @name FOX-callbacks
        /// @{
        /// @brief called when user change the coordinates TextField
        long onCmdSetCoordinates(FXObject*, FXSelector, void*);

        /// @brief called when user select a format radio button
        long onCmdSetFormat(FXObject*, FXSelector, void*);

        /// @brief called when user type in search box
        long onCmdCreateGEOPOI(FXObject*, FXSelector, void*);
        /// @}

    protected:
        FOX_CONSTRUCTOR(GEOPOICreator)

    private:
        /// @brief pointer to Shape frame parent
        GNEPolygonFrame* myPolygonFrameParent;

        /// @brief radio button for the configuration lon-lat
        FXRadioButton* myLonLatRadioButton;

        /// @brief radio button for the configuration lat-lon
        FXRadioButton* myLatLonRadioButton;

        /// @brief text field for given geo coordinates
        FXTextField* myCoordinatesTextField;

        /// @brief button for enable or disable certer view after creation of GEO POI
        FXCheckButton* myCenterViewAfterCreationCheckButton;

        /// @brief button for create GEO Coordinates
        FXButton* myCreateGEOPOIButton;

        /// @brief FXLabel for the equivalent position of GEO Position in Cartesian Position
        FXLabel* myLabelCartesianPosition;
    };

    /**@brief Constructor
    * @brief parent FXHorizontalFrame in which this GNEFrame is placed
    * @brief viewNet viewNet that uses this GNEFrame
    */
    GNEPolygonFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet);

    /// @brief Destructor
    ~GNEPolygonFrame();

    /// @brief show Frame
    void show();

    /**@brief process click over Viewnet
    * @param[in] clickedPosition clicked position over ViewNet
    * @param[in] ObjectsUnderCursor objects under cursor after click over view
    * @return AddShapeStatus with the result of operation
    */
    GNEPolygonFrame::AddShape processClick(const Position& clickedPosition, const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor);

    /// @brief get list of selecte id's in string format
    static std::string getIdsSelected(const FXList* list);

    /// @brief get drawing mode editor
    GNEFrameModuls::DrawingShape* getDrawingShapeModul() const;

protected:
    /**@brief build a shaped element using the drawed shape
     * return true if was successfully created
     * @note called when user stop drawing polygon
     */
    bool shapeDrawed();

    /// @brief Tag selected in TagSelector
    void tagSelected();

    /// @brief add Polygon
    bool addPolygon(const std::map<SumoXMLAttr, std::string>& POIValues);

    /// @brief add POI
    bool addPOI(const std::map<SumoXMLAttr, std::string>& POIValues);

    /// @brief add POILane
    bool addPOILane(const std::map<SumoXMLAttr, std::string>& POIValues);

private:
    /// @brief shape tag selector
    GNEFrameModuls::TagSelector* myShapeTagSelector;

    /// @brief shape internal attributes
    GNEFrameAttributesModuls::AttributesCreator* myShapeAttributes;

    /// @brief Netedit parameter
    GNEFrameAttributesModuls::NeteditAttributes* myNeteditAttributes;

    /// @brief Drawing shape
    GNEFrameModuls::DrawingShape* myDrawingShape;

    /// @brief GEOPOICreator
    GEOPOICreator* myGEOPOICreator;
};
