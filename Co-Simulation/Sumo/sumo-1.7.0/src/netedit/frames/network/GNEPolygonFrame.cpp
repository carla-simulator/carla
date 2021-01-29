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
/// @file    GNEPolygonFrame.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Aug 2017
///
// The Widget for add polygons
/****************************************************************************/
#include <config.h>

#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIDesigns.h>
#include <utils/gui/div/GUIUserIO.h>
#include <netedit/GNEViewParent.h>
#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/elements/additional/GNEPoly.h>
#include <netedit/elements/additional/GNEPOI.h>
#include <netedit/elements/network/GNELane.h>

#include "GNEPolygonFrame.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================

FXDEFMAP(GNEPolygonFrame::GEOPOICreator) GEOPOICreatorMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_SET_ATTRIBUTE,      GNEPolygonFrame::GEOPOICreator::onCmdSetCoordinates),
    FXMAPFUNC(SEL_COMMAND,  MID_CHOOSEN_OPERATION,      GNEPolygonFrame::GEOPOICreator::onCmdSetFormat),
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_CREATE,             GNEPolygonFrame::GEOPOICreator::onCmdCreateGEOPOI),
};

// Object implementation
FXIMPLEMENT(GNEPolygonFrame::GEOPOICreator,     FXGroupBox,     GEOPOICreatorMap,   ARRAYNUMBER(GEOPOICreatorMap))


// ===========================================================================
// method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEPolygonFrame::GEOPOICreator - methods
// ---------------------------------------------------------------------------

GNEPolygonFrame::GEOPOICreator::GEOPOICreator(GNEPolygonFrame* polygonFrameParent) :
    FXGroupBox(polygonFrameParent->myContentFrame, "GEO POI Creator", GUIDesignGroupBoxFrame),
    myPolygonFrameParent(polygonFrameParent) {
    // create RadioButtons for formats
    myLonLatRadioButton = new FXRadioButton(this, "Format: Lon-Lat", this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    myLatLonRadioButton = new FXRadioButton(this, "Format: Lat-Lon", this, MID_CHOOSEN_OPERATION, GUIDesignRadioButton);
    // set lat-lon as default
    myLatLonRadioButton->setCheck(TRUE);
    // create text field for coordinates
    myCoordinatesTextField = new FXTextField(this, GUIDesignTextFieldNCol, this, MID_GNE_SET_ATTRIBUTE, GUIDesignTextField);
    // create checkBox
    myCenterViewAfterCreationCheckButton = new FXCheckButton(this, "Center View after creation", this, MID_GNE_SET_ATTRIBUTE, GUIDesignCheckButton);
    // create button for create GEO POIs
    myCreateGEOPOIButton = new FXButton(this, "Create GEO POI (clipboard)", nullptr, this, MID_GNE_CREATE, GUIDesignButton);
    // create information label
    myLabelCartesianPosition = new FXLabel(this, "Cartesian equivalence:\n- X = give valid longitude\n- Y = give valid latitude", 0, GUIDesignLabelFrameInformation);
}


GNEPolygonFrame::GEOPOICreator::~GEOPOICreator() {}


void
GNEPolygonFrame::GEOPOICreator::showGEOPOICreatorModul() {
    // check if there is an GEO Proj string is defined
    if (GeoConvHelper::getFinal().getProjString() != "!") {
        myCoordinatesTextField->enable();
        myCoordinatesTextField->setText("");
        myCoordinatesTextField->enable();
        myCreateGEOPOIButton->enable();
    } else  {
        myCoordinatesTextField->setText("No geo-conversion defined");
        myCoordinatesTextField->disable();
        myCreateGEOPOIButton->disable();
    }
    show();
}


void
GNEPolygonFrame::GEOPOICreator::hideGEOPOICreatorModul() {
    hide();
}


long
GNEPolygonFrame::GEOPOICreator::onCmdSetCoordinates(FXObject*, FXSelector, void*) {
    // check if input contains spaces
    std::string input = myCoordinatesTextField->getText().text();
    std::string inputWithoutSpaces;
    for (const auto& i : input) {
        if (i != ' ') {
            inputWithoutSpaces.push_back(i);
        }
    }
    // if input contains spaces, call this function again, and in other case set red text color
    if (input.size() != inputWithoutSpaces.size()) {
        myCoordinatesTextField->setText(inputWithoutSpaces.c_str());
    }
    if (inputWithoutSpaces.size() > 0) {
        myCreateGEOPOIButton->setText("Create GEO POI");
    } else {
        myCreateGEOPOIButton->setText("Create GEO POI (clipboard)");
    }
    // simply check if given value can be parsed to Position
    if (GNEAttributeCarrier::canParse<Position>(myCoordinatesTextField->getText().text())) {
        myCoordinatesTextField->setTextColor(FXRGB(0, 0, 0));
        myCoordinatesTextField->killFocus();
        // convert coordinates into lon-lat
        Position geoPos = GNEAttributeCarrier::parse<Position>(myCoordinatesTextField->getText().text());
        if (myLatLonRadioButton->getCheck() == TRUE) {
            geoPos.swapXY();
        }
        GeoConvHelper::getFinal().x2cartesian_const(geoPos);
        // check if GEO Position has to be swapped
        // update myLabelCartesianPosition
        myLabelCartesianPosition->setText(("Cartesian equivalence:\n- X = " + toString(geoPos.x()) + "\n- Y = " + toString(geoPos.y())).c_str());
    } else {
        myCoordinatesTextField->setTextColor(FXRGB(255, 0, 0));
        myLabelCartesianPosition->setText("Cartesian equivalence:\n- X = give valid longitude\n- Y = give valid latitude");
    };
    return 1;
}


long
GNEPolygonFrame::GEOPOICreator::onCmdSetFormat(FXObject* obj, FXSelector, void*) {
    //disable other radio button depending of selected option
    if (obj == myLonLatRadioButton) {
        myLonLatRadioButton->setCheck(TRUE);
        myLatLonRadioButton->setCheck(FALSE);
    } else if (obj == myLatLonRadioButton) {
        myLonLatRadioButton->setCheck(FALSE);
        myLatLonRadioButton->setCheck(TRUE);
    }
    // in both cases call onCmdSetCoordinates(0,0,0) to set new cartesian equivalence
    onCmdSetCoordinates(0, 0, 0);
    return 1;
}


long
GNEPolygonFrame::GEOPOICreator::onCmdCreateGEOPOI(FXObject*, FXSelector, void*) {
    // first check if current GEO Position is valid
    if (myPolygonFrameParent->myShapeAttributes->areValuesValid()) {
        std::string geoPosStr = myCoordinatesTextField->getText().text();
        if (geoPosStr.empty()) {
            // use clipboard
            WRITE_WARNING("Using clipboard");
            geoPosStr = GUIUserIO::copyFromClipboard(*getApp());
            myCoordinatesTextField->setText(geoPosStr.c_str());
            // remove spaces, update cartesian value
            onCmdSetCoordinates(0, 0, 0);
            geoPosStr = myCoordinatesTextField->getText().text();
            myCoordinatesTextField->setText("");
            myCreateGEOPOIButton->setText("Create GEO POI (clipboard)");
        }
        if (GNEAttributeCarrier::canParse<Position>(geoPosStr)) {
            // obtain shape attributes and values
            auto valuesMap = myPolygonFrameParent->myShapeAttributes->getAttributesAndValues(true);
            // obtain netedit attributes and values
            myPolygonFrameParent->myNeteditAttributes->getNeteditAttributesAndValues(valuesMap, nullptr);
            // Check if ID has to be generated
            if (valuesMap.count(SUMO_ATTR_ID) == 0) {
                valuesMap[SUMO_ATTR_ID] = myPolygonFrameParent->myViewNet->getNet()->generateShapeID(myPolygonFrameParent->myShapeTagSelector->getCurrentTagProperties().getTag());
            }
            // force GEO attribute to true and obain position
            valuesMap[SUMO_ATTR_GEO] = "true";
            Position geoPos = GNEAttributeCarrier::parse<Position>(geoPosStr);
            // convert coordinates into lon-lat
            if (myLatLonRadioButton->getCheck() == TRUE) {
                geoPos.swapXY();
            }
            GeoConvHelper::getFinal().x2cartesian_const(geoPos);
            valuesMap[SUMO_ATTR_POSITION] = toString(geoPos);
            // return AddShape::SUCCESS if POI was sucesfully created
            if (myPolygonFrameParent->addPOI(valuesMap)) {
                // check if view has to be centered over created GEO POI
                if (myCenterViewAfterCreationCheckButton->getCheck() == TRUE) {
                    // create a boundary over given GEO Position and center view over it
                    Boundary centerPosition;
                    centerPosition.add(geoPos);
                    centerPosition = centerPosition.grow(10);
                    myPolygonFrameParent->myViewNet->getViewParent()->getView()->centerTo(centerPosition);
                }
            } else {
                WRITE_WARNING("Could not create GEO POI");
            }
        }
        // refresh shape attributes
        myPolygonFrameParent->myShapeAttributes->refreshRows();
    }
    return 1;
}


// ---------------------------------------------------------------------------
// GNEPolygonFrame - methods
// ---------------------------------------------------------------------------

GNEPolygonFrame::GNEPolygonFrame(FXHorizontalFrame* horizontalFrameParent, GNEViewNet* viewNet) :
    GNEFrame(horizontalFrameParent, viewNet, "Shapes") {

    // create item Selector modul for shapes
    myShapeTagSelector = new GNEFrameModuls::TagSelector(this, GNETagProperties::TagType::SHAPE);

    // Create shape parameters
    myShapeAttributes = new GNEFrameAttributesModuls::AttributesCreator(this);

    // Create Netedit parameter
    myNeteditAttributes = new GNEFrameAttributesModuls::NeteditAttributes(this);

    // Create drawing controls
    myDrawingShape = new GNEFrameModuls::DrawingShape(this);

    /// @brief create GEOPOICreator
    myGEOPOICreator = new GEOPOICreator(this);

    // set polygon as default shape
    myShapeTagSelector->setCurrentTag(SUMO_TAG_POLY);
}


GNEPolygonFrame::~GNEPolygonFrame() {
}


void
GNEPolygonFrame::show() {
    // refresh item selector
    myShapeTagSelector->refreshTagProperties();
    // show frame
    GNEFrame::show();
}


GNEPolygonFrame::AddShape
GNEPolygonFrame::processClick(const Position& clickedPosition, const GNEViewNetHelper::ObjectsUnderCursor& objectsUnderCursor) {
    // Declare map to keep values
    std::map<SumoXMLAttr, std::string> valuesMap;
    // check if current selected shape is valid
    if (myShapeTagSelector->getCurrentTagProperties().getTag() == SUMO_TAG_POI) {
        // show warning dialogbox and stop if input parameters are invalid
        if (myShapeAttributes->areValuesValid() == false) {
            myShapeAttributes->showWarningMessage();
            return AddShape::INVALID;
        }
        // obtain shape attributes and values
        valuesMap = myShapeAttributes->getAttributesAndValues(true);
        // obtain netedit attributes and values
        myNeteditAttributes->getNeteditAttributesAndValues(valuesMap, objectsUnderCursor.getLaneFront());
        // Check if ID has to be generated
        if (valuesMap.count(SUMO_ATTR_ID) == 0) {
            valuesMap[SUMO_ATTR_ID] = myViewNet->getNet()->generateShapeID(myShapeTagSelector->getCurrentTagProperties().getTag());
        }
        // obtain position
        valuesMap[SUMO_ATTR_POSITION] = toString(clickedPosition);
        // set GEO Position as false (because we have created POI clicking over View
        valuesMap[SUMO_ATTR_GEO] = "false";
        // return AddShape::SUCCESS if POI was sucesfully created
        if (addPOI(valuesMap)) {
            // refresh shape attributes
            myShapeAttributes->refreshRows();
            return AddShape::SUCCESS;
        } else {
            return AddShape::INVALID;
        }
    } else if (myShapeTagSelector->getCurrentTagProperties().getTag() == SUMO_TAG_POILANE) {
        // abort if lane is nullptr
        if (objectsUnderCursor.getLaneFront() == nullptr) {
            WRITE_WARNING(toString(SUMO_TAG_POILANE) + " can be only placed over lanes");
            return AddShape::INVALID;
        }
        // show warning dialogbox and stop if input parameters are invalid
        if (myShapeAttributes->areValuesValid() == false) {
            myShapeAttributes->showWarningMessage();
            return AddShape::INVALID;
        }
        // obtain shape attributes and values
        valuesMap = myShapeAttributes->getAttributesAndValues(true);
        // obtain netedit attributes and values
        myNeteditAttributes->getNeteditAttributesAndValues(valuesMap, objectsUnderCursor.getLaneFront());
        // Check if ID has to be generated
        if (valuesMap.count(SUMO_ATTR_ID) == 0) {
            valuesMap[SUMO_ATTR_ID] = myViewNet->getNet()->generateShapeID(myShapeTagSelector->getCurrentTagProperties().getTag());
        }
        // obtain Lane
        valuesMap[SUMO_ATTR_LANE] = objectsUnderCursor.getLaneFront()->getID();
        // obtain position over lane
        valuesMap[SUMO_ATTR_POSITION] = toString(objectsUnderCursor.getLaneFront()->getLaneShape().nearest_offset_to_point2D(clickedPosition));
        // return AddShape::SUCCESS if POI was sucesfully created
        if (addPOILane(valuesMap)) {
            // refresh shape attributes
            myShapeAttributes->refreshRows();
            return AddShape::SUCCESS;
        } else {
            return AddShape::INVALID;
        }
    } else if (myShapeTagSelector->getCurrentTagProperties().getTag() == SUMO_TAG_POLY) {
        if (myDrawingShape->isDrawing()) {
            // add or delete a new point depending of flag "delete last created point"
            if (myDrawingShape->getDeleteLastCreatedPoint()) {
                myDrawingShape->removeLastPoint();
            } else {
                myDrawingShape->addNewPoint(clickedPosition);
            }
            return AddShape::UPDATEDTEMPORALSHAPE;
        } else {
            // return AddShape::NOTHING if is drawing isn't enabled
            return AddShape::NOTHING;
        }
    } else {
        myViewNet->setStatusBarText("Current selected shape isn't valid.");
        return AddShape::INVALID;
    }
}


std::string
GNEPolygonFrame::getIdsSelected(const FXList* list) {
    // Obtain Id's of list
    std::string vectorOfIds;
    for (int i = 0; i < list->getNumItems(); i++) {
        if (list->isItemSelected(i)) {
            if (vectorOfIds.size() > 0) {
                vectorOfIds += " ";
            }
            vectorOfIds += (list->getItem(i)->getText()).text();
        }
    }
    return vectorOfIds;
}


GNEFrameModuls::DrawingShape*
GNEPolygonFrame::getDrawingShapeModul() const {
    return myDrawingShape;
}


bool
GNEPolygonFrame::shapeDrawed() {
    // show warning dialogbox and stop check if input parameters are valid
    if (!myShapeAttributes->areValuesValid()) {
        myShapeAttributes->showWarningMessage();
        return false;
    } else if (myDrawingShape->getTemporalShape().size() == 0) {
        WRITE_WARNING("Polygon shape cannot be empty");
        return false;
    } else {
        // Declare map to keep values
        std::map<SumoXMLAttr, std::string> valuesMap = myShapeAttributes->getAttributesAndValues(true);
        // obtain netedit attributes and values
        myNeteditAttributes->getNeteditAttributesAndValues(valuesMap, nullptr);
        // Check if ID has to be generated
        if (valuesMap.count(SUMO_ATTR_ID) == 0) {
            valuesMap[SUMO_ATTR_ID] = myViewNet->getNet()->generateShapeID(SUMO_TAG_POLY);
        }
        // obtain shape and check if has to be closed
        PositionVector temporalShape = myDrawingShape->getTemporalShape();
        if (GNEAttributeCarrier::parse<bool>(valuesMap[GNE_ATTR_CLOSE_SHAPE])) {
            temporalShape.closePolygon();
        }
        valuesMap[SUMO_ATTR_SHAPE] = toString(temporalShape);
        // obtain geo (by default false)
        valuesMap[SUMO_ATTR_GEO] = "false";
        // return true if polygon was successfully created
        if (addPolygon(valuesMap)) {
            // refresh shape attributes
            myShapeAttributes->refreshRows();
            return true;
        }
    }
    return false;
}


void
GNEPolygonFrame::tagSelected() {
    if (myShapeTagSelector->getCurrentTagProperties().getTag() != SUMO_TAG_NOTHING) {
        // if there are parmeters, show and Recalc groupBox
        myShapeAttributes->showAttributesCreatorModul(myShapeTagSelector->getCurrentTagProperties(), {});
        // show netedit attributes
        myNeteditAttributes->showNeteditAttributesModul(myShapeTagSelector->getCurrentTagProperties());
        // Check if drawing mode has to be shown
        if (myShapeTagSelector->getCurrentTagProperties().getTag() == SUMO_TAG_POLY) {
            myDrawingShape->showDrawingShape();
        } else {
            myDrawingShape->hideDrawingShape();
        }
        // Check if GEO POI Creator has to be shown
        if (myShapeTagSelector->getCurrentTagProperties().getTag() == SUMO_TAG_POI) {
            myGEOPOICreator->showGEOPOICreatorModul();
        } else {
            myGEOPOICreator->hideGEOPOICreatorModul();
        }
    } else {
        // hide all widgets
        myShapeAttributes->hideAttributesCreatorModul();
        myNeteditAttributes->hideNeteditAttributesModul();
        myDrawingShape->hideDrawingShape();
        myGEOPOICreator->hideGEOPOICreatorModul();
    }
}


bool
GNEPolygonFrame::addPolygon(const std::map<SumoXMLAttr, std::string>& polyValues) {
    // parse attributes from polyValues
    std::string id = polyValues.at(SUMO_ATTR_ID);
    std::string type = polyValues.at(SUMO_ATTR_TYPE);
    RGBColor color = RGBColor::parseColor(polyValues.at(SUMO_ATTR_COLOR));
    std::string layerStr = polyValues.at(SUMO_ATTR_LAYER);
    double angle = GNEAttributeCarrier::parse<double>(polyValues.at(SUMO_ATTR_ANGLE));
    std::string imgFile = polyValues.at(SUMO_ATTR_IMGFILE);
    bool relativePath = GNEAttributeCarrier::parse<bool>(polyValues.at(SUMO_ATTR_RELATIVEPATH));
    PositionVector shape = GNEAttributeCarrier::parse<PositionVector>(polyValues.at(SUMO_ATTR_SHAPE));
    bool fill = GNEAttributeCarrier::parse<bool>(polyValues.at(SUMO_ATTR_FILL));
    double lineWidth = GNEAttributeCarrier::parse<double>(polyValues.at(SUMO_ATTR_LINEWIDTH));
    // parse layer
    double layer = GNEAttributeCarrier::canParse<double>(layerStr) ? GNEAttributeCarrier::parse<double>(layerStr) : Shape::DEFAULT_LAYER;
    // create new Polygon only if number of shape points is greather than 2
    myViewNet->getUndoList()->p_begin("add " + toString(SUMO_TAG_POLY));
    if ((shape.size() > 0) && myViewNet->getNet()->getAttributeCarriers()->addPolygon(id, type, color, layer, angle, imgFile, relativePath, shape, false, fill, lineWidth)) {
        // set manually attributes use GEO, block movement and block shape
        GNEShape* poly = myViewNet->getNet()->retrieveShape(SUMO_TAG_POLY, id);
        poly->setAttribute(GNE_ATTR_BLOCK_MOVEMENT, polyValues.at(GNE_ATTR_BLOCK_MOVEMENT), myViewNet->getUndoList());
        poly->setAttribute(GNE_ATTR_BLOCK_SHAPE, polyValues.at(GNE_ATTR_BLOCK_SHAPE), myViewNet->getUndoList());
        myViewNet->getUndoList()->p_end();
        return true;
    } else {
        // abort creation
        myViewNet->getUndoList()->p_abort();
        return false;
    }
}


bool
GNEPolygonFrame::addPOI(const std::map<SumoXMLAttr, std::string>& POIValues) {
    // parse attributes from POIValues
    std::string id = POIValues.at(SUMO_ATTR_ID);
    std::string type = POIValues.at(SUMO_ATTR_TYPE);
    RGBColor color = RGBColor::parseColor(POIValues.at(SUMO_ATTR_COLOR));
    std::string layerStr = POIValues.at(SUMO_ATTR_LAYER);
    Position pos = GNEAttributeCarrier::parse<Position>(POIValues.at(SUMO_ATTR_POSITION));
    double angle = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_ANGLE));
    std::string imgFile = POIValues.at(SUMO_ATTR_IMGFILE);
    bool relativePath = GNEAttributeCarrier::parse<bool>(POIValues.at(SUMO_ATTR_RELATIVEPATH));
    double widthPOI = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_WIDTH));
    double heightPOI = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_HEIGHT));
    double layer = GNEAttributeCarrier::canParse<double>(layerStr) ? GNEAttributeCarrier::parse<double>(layerStr) : Shape::DEFAULT_LAYER_POI;
    bool geo = GNEAttributeCarrier::parse<bool>(POIValues.at(SUMO_ATTR_GEO));
    // create new POI
    myViewNet->getUndoList()->p_begin("add " + toString(SUMO_TAG_POI));
    if (myViewNet->getNet()->getAttributeCarriers()->addPOI(id, type, color, pos, geo, "", 0, 0, layer, angle, imgFile, relativePath, widthPOI, heightPOI)) {
        // Set manually the attribute block movement
        GNEShape* POI = myViewNet->getNet()->retrieveShape(SUMO_TAG_POI, id);
        POI->setAttribute(GNE_ATTR_BLOCK_MOVEMENT, POIValues.at(GNE_ATTR_BLOCK_MOVEMENT), myViewNet->getUndoList());
        myViewNet->getUndoList()->p_end();
        return true;
    } else {
        // abort creation
        myViewNet->getUndoList()->p_abort();
        return false;
    }
}


bool
GNEPolygonFrame::addPOILane(const std::map<SumoXMLAttr, std::string>& POIValues) {
    // parse attributes from POIValues
    std::string id = POIValues.at(SUMO_ATTR_ID);
    std::string type = POIValues.at(SUMO_ATTR_TYPE);
    RGBColor color = RGBColor::parseColor(POIValues.at(SUMO_ATTR_COLOR));
    std::string layerStr = POIValues.at(SUMO_ATTR_LAYER);
    double angle = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_ANGLE));
    std::string imgFile = POIValues.at(SUMO_ATTR_IMGFILE);
    bool relativePath = GNEAttributeCarrier::parse<bool>(POIValues.at(SUMO_ATTR_RELATIVEPATH));
    GNELane* lane = myViewNet->getNet()->retrieveLane(POIValues.at(SUMO_ATTR_LANE));
    double posLane = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_POSITION));
    double posLat = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_POSITION_LAT));
    double widthPOI = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_WIDTH));
    double heightPOI = GNEAttributeCarrier::parse<double>(POIValues.at(SUMO_ATTR_HEIGHT));
    // parse layer
    double layer = GNEAttributeCarrier::canParse<double>(layerStr) ? GNEAttributeCarrier::parse<double>(layerStr) : Shape::DEFAULT_LAYER_POI;
    // create new POILane
    myViewNet->getUndoList()->p_begin("add " + toString(SUMO_TAG_POILANE));
    if (myViewNet->getNet()->getAttributeCarriers()->addPOI(id, type, color, Position(), false, lane->getID(), posLane, posLat, layer, angle, imgFile, relativePath, widthPOI, heightPOI)) {
        // Set manually the attribute block movement
        GNEShape* POILane = myViewNet->getNet()->retrieveShape(SUMO_TAG_POILANE, id);
        POILane->setAttribute(GNE_ATTR_BLOCK_MOVEMENT, POIValues.at(GNE_ATTR_BLOCK_MOVEMENT), myViewNet->getUndoList());
        myViewNet->getUndoList()->p_end();
        return true;
    } else {
        // abort creation
        myViewNet->getUndoList()->p_abort();
        return false;
    }
}


/****************************************************************************/
