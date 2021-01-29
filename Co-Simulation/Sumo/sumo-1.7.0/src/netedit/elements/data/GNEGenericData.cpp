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
/// @file    GNEGenericData.cpp
/// @author  Pablo Alvarez Lopez
/// @date    Jan 2020
///
// A abstract class for data sets
/****************************************************************************/
#include <config.h>

#include <netedit/GNENet.h>
#include <netedit/GNEViewNet.h>
#include <netedit/GNEViewParent.h>
#include <netedit/elements/data/GNEGenericData.h>
#include <netedit/frames/data/GNEEdgeDataFrame.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/div/GUIParameterTableWindow.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/globjects/GUIGLObjectPopupMenu.h>

#include "GNEGenericData.h"
#include "GNEDataInterval.h"


// ===========================================================================
// member method definitions
// ===========================================================================

// ---------------------------------------------------------------------------
// GNEGenericData - methods
// ---------------------------------------------------------------------------

GNEGenericData::GNEGenericData(const SumoXMLTag tag, const GUIGlObjectType type, GNEDataInterval* dataIntervalParent,
                               const std::map<std::string, std::string>& parameters,
                               const std::vector<GNEJunction*>& junctionParents,
                               const std::vector<GNEEdge*>& edgeParents,
                               const std::vector<GNELane*>& laneParents,
                               const std::vector<GNEAdditional*>& additionalParents,
                               const std::vector<GNEShape*>& shapeParents,
                               const std::vector<GNETAZElement*>& TAZElementParents,
                               const std::vector<GNEDemandElement*>& demandElementParents,
                               const std::vector<GNEGenericData*>& genericDataParents) :
    GUIGlObject(type, dataIntervalParent->getID()),
    Parameterised(ParameterisedAttrType::DOUBLE, parameters),
    GNEHierarchicalElement(dataIntervalParent->getNet(), tag, junctionParents, edgeParents, laneParents, additionalParents, shapeParents, TAZElementParents, demandElementParents, genericDataParents),
    GNEPathElements(this),
    myDataIntervalParent(dataIntervalParent) {
}


GNEGenericData::~GNEGenericData() {}


const std::string&
GNEGenericData::getID() const {
    return getMicrosimID();
}


GUIGlObject*
GNEGenericData::getGUIGlObject() {
    return this;
}


GNEDataInterval*
GNEGenericData::getDataIntervalParent() const {
    return myDataIntervalParent;
}


void
GNEGenericData::drawAttribute(const PositionVector& shape) const {
    if ((myTagProperty.getTag() == SUMO_TAG_MEANDATA_EDGE) && (shape.length() > 0)) {
        // obtain pointer to edge data frame (only for code legibly)
        const GNEEdgeDataFrame* edgeDataFrame = myDataIntervalParent->getNet()->getViewNet()->getViewParent()->getEdgeDataFrame();
        // check if we have to filter generic data
        if (edgeDataFrame->shown()) {
            // check attribute
            if ((edgeDataFrame->getAttributeSelector()->getFilteredAttribute().size() > 0) &&
                    (getParametersMap().count(edgeDataFrame->getAttributeSelector()->getFilteredAttribute()) > 0)) {
                // get value
                const std::string value = getParametersMap().at(edgeDataFrame->getAttributeSelector()->getFilteredAttribute());
                // calculate center position
                const Position centerPosition = shape.positionAtOffset2D(shape.length2D() / 2);
                // Add a draw matrix
                glPushMatrix();
                GLHelper::drawText(value, centerPosition, GLO_MAX, 2, RGBColor::BLUE);
                // pop draw matrix
                glPopMatrix();
            }
        }
    }
}


bool
GNEGenericData::isGenericDataValid() const {
    return true;
}


std::string
GNEGenericData::getGenericDataProblem() const {
    return "";
}


void
GNEGenericData::fixGenericDataProblem() {
    throw InvalidArgument(getTagStr() + " cannot fix any problem");
}


GUIGLObjectPopupMenu*
GNEGenericData::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    GUIGLObjectPopupMenu* ret = new GUIGLObjectPopupMenu(app, parent, *this);
    // build header
    buildPopupHeader(ret, app);
    // build menu command for center button and copy cursor position to clipboard
    buildCenterPopupEntry(ret);
    buildPositionCopyEntry(ret, false);
    // buld menu commands for names
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " name to clipboard").c_str(), nullptr, ret, MID_COPY_NAME);
    new FXMenuCommand(ret, ("Copy " + getTagStr() + " typed name to clipboard").c_str(), nullptr, ret, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(ret);
    // build selection and show parameters menu
    myDataIntervalParent->getNet()->getViewNet()->buildSelectionACPopupEntry(ret, this);
    buildShowParamsPopupEntry(ret);
    // show option to open additional dialog
    if (myTagProperty.hasDialog()) {
        new FXMenuCommand(ret, ("Open " + getTagStr() + " Dialog").c_str(), getIcon(), &parent, MID_OPEN_ADDITIONAL_DIALOG);
        new FXMenuSeparator(ret);
    } else {
        new FXMenuCommand(ret, ("Cursor position in view: " + toString(getPositionInView().x()) + "," + toString(getPositionInView().y())).c_str(), nullptr, nullptr, 0);
    }
    return ret;
}


GUIParameterTableWindow*
GNEGenericData::getParameterWindow(GUIMainWindow& app, GUISUMOAbstractView& /* parent */) {
    // Create table
    GUIParameterTableWindow* ret = new GUIParameterTableWindow(app, *this);
    // Iterate over attributes
    for (const auto& tagProperty : myTagProperty) {
        // Add attribute and set it dynamic if aren't unique
        if (tagProperty.isUnique()) {
            ret->mkItem(tagProperty.getAttrStr().c_str(), false, getAttribute(tagProperty.getAttr()));
        } else {
            ret->mkItem(tagProperty.getAttrStr().c_str(), true, getAttribute(tagProperty.getAttr()));
        }
    }
    // close building
    ret->closeBuilding();
    return ret;
}


void
GNEGenericData::drawFilteredAttribute(const GUIVisualizationSettings& s, const PositionVector& laneShape, const std::string& attribute) const {
    if (getParametersMap().count(attribute) > 0) {
        const Position pos = laneShape.positionAtOffset2D(laneShape.length2D() * 0.5);
        const double rot = laneShape.rotationDegreeAtOffset(laneShape.length2D() * 0.5);
        // Add a draw matrix for details
        glPushMatrix();
        // draw value
        GLHelper::drawText(getParameter(attribute), pos, GLO_MAX - 1, 2, RGBColor::BLACK, s.getTextAngle(rot + 90));
        // pop draw matrix
        glPopMatrix();
    }
}


bool
GNEGenericData::isVisibleInspectDeleteSelect() const {
    // get toolbar
    const GNEViewNetHelper::IntervalBar& toolBar = myNet->getViewNet()->getIntervalBar();
    // declare flag
    bool draw = true;
    // check filter by generic data type
    if ((toolBar.getGenericDataTypeStr().size() > 0) && (toolBar.getGenericDataTypeStr() != myTagProperty.getTagStr())) {
        draw = false;
    }
    // check filter by data set
    if ((toolBar.getDataSetStr().size() > 0) && (toolBar.getDataSetStr() != myDataIntervalParent->getID())) {
        draw = false;
    }
    // check filter by begin
    if ((toolBar.getBeginStr().size() > 0) && (parse<double>(toolBar.getBeginStr()) > myDataIntervalParent->getAttributeDouble(SUMO_ATTR_BEGIN))) {
        draw = false;
    }
    // check filter by end
    if ((toolBar.getEndStr().size() > 0) && (parse<double>(toolBar.getEndStr()) < myDataIntervalParent->getAttributeDouble(SUMO_ATTR_END))) {
        draw = false;
    }
    // check filter by attribute
    if ((toolBar.getAttributeStr().size() > 0) && (getParametersMap().count(toolBar.getAttributeStr()) == 0)) {
        draw = false;
    }
    // return flag
    return draw;
}

void
GNEGenericData::replaceFirstParentEdge(const std::string& value) {
    std::vector<GNEEdge*> parentEdges = getParentEdges();
    parentEdges[0] = myNet->retrieveEdge(value);
    // replace parent edges
    replaceParentElements(this, parentEdges);
}


void
GNEGenericData::replaceLastParentEdge(const std::string& value) {
    std::vector<GNEEdge*> parentEdges = getParentEdges();
    parentEdges[(int)parentEdges.size() - 1] = myNet->retrieveEdge(value);
    // replace parent edges
    replaceParentElements(this, parentEdges);
}


void
GNEGenericData::replaceFirstParentTAZElement(SumoXMLTag tag, const std::string& value) {
    std::vector<GNETAZElement*> parentTAZElements = getParentTAZElements();
    parentTAZElements[0] = myNet->retrieveTAZElement(tag, value);
    // replace parent TAZElements
    replaceParentElements(this, parentTAZElements);
}


void
GNEGenericData::replaceLastParentTAZElement(SumoXMLTag tag, const std::string& value) {
    std::vector<GNETAZElement*> parentTAZElements = getParentTAZElements();
    parentTAZElements[(int)parentTAZElements.size() - 1] = myNet->retrieveTAZElement(tag, value);
    // replace parent TAZElements
    replaceParentElements(this, parentTAZElements);
}

/****************************************************************************/
