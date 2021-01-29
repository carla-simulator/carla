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
/// @file    GNEPerson.cpp
/// @author  Pablo Alvarez Lopez
/// @date    May 2019
///
// Representation of persons in NETEDIT
/****************************************************************************/
#include <cmath>
#include <microsim/devices/MSDevice_BTreceiver.h>
#include <netedit/GNENet.h>
#include <netedit/GNEUndoList.h>
#include <netedit/GNEViewNet.h>
#include <netedit/changes/GNEChange_EnableAttribute.h>
#include <netedit/changes/GNEChange_Attribute.h>
#include <utils/gui/div/GLHelper.h>
#include <utils/gui/globjects/GLIncludes.h>
#include <utils/gui/windows/GUIAppEnum.h>
#include <utils/gui/div/GUIBasePersonHelper.h>

#include "GNEPerson.h"
#include "GNERouteHandler.h"


// ===========================================================================
// FOX callback mapping
// ===========================================================================
FXDEFMAP(GNEPerson::GNEPersonPopupMenu) personPopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_PERSON_TRANSFORM,   GNEPerson::GNEPersonPopupMenu::onCmdTransform),
};

FXDEFMAP(GNEPerson::GNESelectedPersonsPopupMenu) selectedPersonsPopupMenuMap[] = {
    FXMAPFUNC(SEL_COMMAND,  MID_GNE_PERSON_TRANSFORM,   GNEPerson::GNESelectedPersonsPopupMenu::onCmdTransform),
};

// Object implementation
FXIMPLEMENT(GNEPerson::GNEPersonPopupMenu,          GUIGLObjectPopupMenu, personPopupMenuMap,           ARRAYNUMBER(personPopupMenuMap))
FXIMPLEMENT(GNEPerson::GNESelectedPersonsPopupMenu, GUIGLObjectPopupMenu, selectedPersonsPopupMenuMap,  ARRAYNUMBER(selectedPersonsPopupMenuMap))

// ===========================================================================
// GNEPerson::GNEPersonPopupMenu
// ===========================================================================

GNEPerson::GNEPersonPopupMenu::GNEPersonPopupMenu(GNEPerson* person, GUIMainWindow& app, GUISUMOAbstractView& parent) :
    GUIGLObjectPopupMenu(app, parent, *person),
    myPerson(person),
    myTransformToPerson(nullptr),
    myTransformToPersonFlow(nullptr) {
    // build header
    myPerson->buildPopupHeader(this, app);
    // build menu command for center button and copy cursor position to clipboard
    myPerson->buildCenterPopupEntry(this);
    myPerson->buildPositionCopyEntry(this, false);
    // buld menu commands for names
    new FXMenuCommand(this, ("Copy " + myPerson->getTagStr() + " name to clipboard").c_str(), nullptr, this, MID_COPY_NAME);
    new FXMenuCommand(this, ("Copy " + myPerson->getTagStr() + " typed name to clipboard").c_str(), nullptr, this, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(this);
    // build selection and show parameters menu
    myPerson->getNet()->getViewNet()->buildSelectionACPopupEntry(this, myPerson);
    myPerson->buildShowParamsPopupEntry(this);
    // add transform functions only in demand mode
    if (myPerson->getNet()->getViewNet()->getEditModes().isCurrentSupermodeDemand()) {
        // create menu pane for transform operations
        FXMenuPane* transformOperation = new FXMenuPane(this);
        this->insertMenuPaneChild(transformOperation);
        new FXMenuCascade(this, "transform to", nullptr, transformOperation);
        // Create menu comands for all transformations
        myTransformToPerson = new FXMenuCommand(transformOperation, "Person", GUIIconSubSys::getIcon(GUIIcon::PERSON), this, MID_GNE_PERSON_TRANSFORM);
        myTransformToPersonFlow = new FXMenuCommand(transformOperation, "Person (embedded route)", GUIIconSubSys::getIcon(GUIIcon::PERSONFLOW), this, MID_GNE_PERSON_TRANSFORM);
        // check what menu command has to be disabled
        if (myPerson->getTagProperty().getTag() == SUMO_TAG_PERSON) {
            myTransformToPerson->disable();
        } else if (myPerson->getTagProperty().getTag() == SUMO_TAG_PERSONFLOW) {
            myTransformToPersonFlow->disable();
        }
    }
}


GNEPerson::GNEPersonPopupMenu::~GNEPersonPopupMenu() {}


long
GNEPerson::GNEPersonPopupMenu::onCmdTransform(FXObject* obj, FXSelector, void*) {
    if (obj == myTransformToPerson) {
        GNERouteHandler::transformToPerson(myPerson);
    } else if (obj == myTransformToPersonFlow) {
        GNERouteHandler::transformToPersonFlow(myPerson);
    }
    return 1;
}


// ===========================================================================
// GNEPerson::GNESelectedPersonsPopupMenu
// ===========================================================================

GNEPerson::GNESelectedPersonsPopupMenu::GNESelectedPersonsPopupMenu(GNEPerson* person, const std::vector<GNEPerson*>& selectedPerson, GUIMainWindow& app, GUISUMOAbstractView& parent) :
    GUIGLObjectPopupMenu(app, parent, *person),
    myPersonTag(person->getTagProperty().getTag()),
    mySelectedPersons(selectedPerson),
    myTransformToPerson(nullptr),
    myTransformToPersonFlow(nullptr) {
    // build header
    person->buildPopupHeader(this, app);
    // build menu command for center button and copy cursor position to clipboard
    person->buildCenterPopupEntry(this);
    person->buildPositionCopyEntry(this, false);
    // buld menu commands for names
    new FXMenuCommand(this, ("Copy " + person->getTagStr() + " name to clipboard").c_str(), nullptr, this, MID_COPY_NAME);
    new FXMenuCommand(this, ("Copy " + person->getTagStr() + " typed name to clipboard").c_str(), nullptr, this, MID_COPY_TYPED_NAME);
    new FXMenuSeparator(this);
    // build selection and show parameters menu
    person->getNet()->getViewNet()->buildSelectionACPopupEntry(this, person);
    person->buildShowParamsPopupEntry(this);
    // add transform functions only in demand mode
    if (person->getNet()->getViewNet()->getEditModes().isCurrentSupermodeDemand()) {
        // create menu pane for transform operations
        FXMenuPane* transformOperation = new FXMenuPane(this);
        this->insertMenuPaneChild(transformOperation);
        new FXMenuCascade(this, "transform to", nullptr, transformOperation);
        // Create menu comands for all transformations
        myTransformToPerson = new FXMenuCommand(transformOperation, "Person", GUIIconSubSys::getIcon(GUIIcon::PERSON), this, MID_GNE_PERSON_TRANSFORM);
        myTransformToPersonFlow = new FXMenuCommand(transformOperation, "PersonFlow", GUIIconSubSys::getIcon(GUIIcon::PERSONFLOW), this, MID_GNE_PERSON_TRANSFORM);
    }
}


GNEPerson::GNESelectedPersonsPopupMenu::~GNESelectedPersonsPopupMenu() {}


long
GNEPerson::GNESelectedPersonsPopupMenu::onCmdTransform(FXObject* obj, FXSelector, void*) {
    // iterate over all selected persons
    for (const auto& i : mySelectedPersons) {
        if ((obj == myTransformToPerson) &&
                (i->getTagProperty().getTag() == myPersonTag)) {
            GNERouteHandler::transformToPerson(i);
        } else if ((obj == myTransformToPersonFlow) &&
                   (i->getTagProperty().getTag() == myPersonTag)) {
            GNERouteHandler::transformToPerson(i);
        }
    }
    return 1;
}

// ===========================================================================
// member method definitions
// ===========================================================================

GNEPerson::GNEPerson(SumoXMLTag tag, GNENet* net, GNEDemandElement* pType, const SUMOVehicleParameter& personparameters) :
    GNEDemandElement(personparameters.id, net, (tag == SUMO_TAG_PERSONFLOW) ? GLO_PERSONFLOW : GLO_PERSON, tag,
{}, {}, {}, {}, {}, {}, {pType}, {}),
SUMOVehicleParameter(personparameters) {
    // set manually vtypeID (needed for saving)
    vtypeid = pType->getID();
}


GNEPerson::~GNEPerson() {}


std::string
GNEPerson::getBegin() const {
    // obtain depart depending if is a Person, trip or routeFlow
    std::string departStr;
    if (myTagProperty.getTag() == SUMO_TAG_PERSONFLOW) {
        departStr = toString(depart);
    } else {
        departStr = getDepart();
    }
    // we need to handle depart as a tuple of 20 numbers (format: 000000...00<departTime>)
    departStr.reserve(20 - departStr.size());
    // add 0s at the beginning of departStr until we have 20 numbers
    for (int i = (int)departStr.size(); i < 20; i++) {
        departStr.insert(departStr.begin(), '0');
    }
    return departStr;
}


void
GNEPerson::writeDemandElement(OutputDevice& device) const {
    // obtain tag depending if tagProperty has a synonym
    SumoXMLTag synonymTag = myTagProperty.hasTagSynonym() ? myTagProperty.getTagSynonym() : myTagProperty.getTag();
    // attribute VType musn't be written if is DEFAULT_PEDTYPE_ID
    if (getParentDemandElements().at(0)->getID() == DEFAULT_PEDTYPE_ID) {
        // unset VType parameter
        parametersSet &= ~VEHPARS_VTYPE_SET;
        // write person attributes (VType will not be written)
        write(device, OptionsCont::getOptions(), synonymTag);
        // set VType parameter again
        parametersSet |= VEHPARS_VTYPE_SET;
    } else {
        // write person attributes, including VType
        write(device, OptionsCont::getOptions(), synonymTag, getParentDemandElements().at(0)->getID());
    }
    // write specific flow attributes
    if (myTagProperty.getTag() == SUMO_TAG_PERSONFLOW) {
        // write routeFlow values depending if it was set
        if (isAttributeEnabled(SUMO_ATTR_END)) {
            device.writeAttr(SUMO_ATTR_END,  time2string(repetitionEnd));
        }
        if (isAttributeEnabled(SUMO_ATTR_NUMBER)) {
            device.writeAttr(SUMO_ATTR_NUMBER, repetitionNumber);
        }
        if (isAttributeEnabled(SUMO_ATTR_PERSONSPERHOUR)) {
            device.writeAttr(SUMO_ATTR_PERSONSPERHOUR, 3600. / STEPS2TIME(repetitionOffset));
        }
        if (isAttributeEnabled(SUMO_ATTR_PERIOD)) {
            device.writeAttr(SUMO_ATTR_PERIOD, time2string(repetitionOffset));
        }
        if (isAttributeEnabled(SUMO_ATTR_PROB)) {
            device.writeAttr(SUMO_ATTR_PROB, repetitionProbability);
        }
    }
    // write parameters
    writeParams(device);
    // write child demand elements associated to this person (Rides, Walks...)
    for (const auto& i : getChildDemandElements()) {
        i->writeDemandElement(device);
    }
    // close person tag
    device.closeTag();
}


bool
GNEPerson::isDemandElementValid() const {
    // a single person is always valid
    return true;
}


std::string
GNEPerson::getDemandElementProblem() const {
    // A single person cannot habe problem (but their children)
    return "";
}


void
GNEPerson::fixDemandElementProblem() {
    // nothing to fix
}


SUMOVehicleClass
GNEPerson::getVClass() const {
    return getParentDemandElements().front()->getVClass();
}


const RGBColor&
GNEPerson::getColor() const {
    return color;
}


void
GNEPerson::startGeometryMoving() {
    // Persons cannot be moved
}


void
GNEPerson::endGeometryMoving() {
    // Persons cannot be moved
}


void
GNEPerson::moveGeometry(const Position&) {
    // Persons cannot be moved
}


void
GNEPerson::commitGeometryMoving(GNEUndoList*) {
    // Persons cannot be moved
}


void
GNEPerson::updateGeometry() {
    // only update geometry of childrens
    for (const auto& i : getChildDemandElements()) {
        i->updateGeometry();
    }
}


void
GNEPerson::computePath() {
    // nothing to compute
}


void
GNEPerson::invalidatePath() {
    // nothing to invalidate
}


Position
GNEPerson::getPositionInView() const {
    // Position in view depend of first child element
    if (getChildDemandElements().size() > 0) {
        if (getChildDemandElements().at(0)->getTagProperty().isPersonStop()) {
            return getChildDemandElements().at(0)->getDemandElementGeometry().getShape().getLineCenter();
        } else {
            // obtain lane (special case for rides)
            SUMOVehicleClass vClassEdgeFrom = getChildDemandElements().front()->getTagProperty().isRide() ? SVC_PASSENGER : SVC_PEDESTRIAN;
            GNELane* lane = getChildDemandElements().at(0)->getParentEdges().at(0)->getLaneByAllowedVClass(vClassEdgeFrom);
            // return position in view depending of lane
            if (lane->getLaneShape().length() < 2.5) {
                return lane->getLaneShape().front();
            } else {
                Position A = lane->getLaneShape().positionAtOffset(2.5);
                Position B = lane->getLaneShape().positionAtOffset(2.5);
                // return Middle point
                return Position((A.x() + B.x()) / 2, (A.y() + B.y()) / 2);
            }
        }
    } else {
        return Position(0, 0);
    }
}


GUIGLObjectPopupMenu*
GNEPerson::getPopUpMenu(GUIMainWindow& app, GUISUMOAbstractView& parent) {
    // return a GNEPersonPopupMenu
    return new GNEPersonPopupMenu(this, app, parent);
}


std::string
GNEPerson::getParentName() const {
    return getParentDemandElements().front()->getID();
}


Boundary
GNEPerson::getCenteringBoundary() const {
    Boundary personBoundary;
    if (getChildDemandElements().size() > 0) {
        if (getChildDemandElements().front()->getTagProperty().isPersonStop()) {
            // use boundary of stop center
            return getChildDemandElements().front()->getCenteringBoundary();
        } else {
            // obtain boundary of first position over edge
            personBoundary.add(getChildDemandElements().front()->getDemandElementSegmentGeometry().getFirstPosition());
        }
    } else {
        personBoundary = Boundary(-0.1, -0.1, 0.1, 0.1);
    }
    personBoundary.grow(20);
    return personBoundary;
}


void
GNEPerson::splitEdgeGeometry(const double /*splitPosition*/, const GNENetworkElement* /*originalElement*/, const GNENetworkElement* /*newElement*/, GNEUndoList* /*undoList*/) {
    // geometry of this element cannot be splitted
}


void
GNEPerson::drawGL(const GUIVisualizationSettings& s) const {
    bool drawPerson = true;
    // check if person can be drawn
    if (!myNet->getViewNet()->getNetworkViewOptions().showDemandElements()) {
        drawPerson = false;
    } else if (!myNet->getViewNet()->getDataViewOptions().showDemandElements()) {
        drawPerson = false;
    } else if (!myNet->getViewNet()->getDemandViewOptions().showNonInspectedDemandElements(this)) {
        drawPerson = false;
    } else if (getChildDemandElements().empty()) {
        drawPerson = false;
    }
    // continue if person can be drawn
    if (drawPerson) {
        // obtain exaggeration (and add the special personExaggeration)
        const double exaggeration = s.personSize.getExaggeration(s, this, 80) + s.detailSettings.personExaggeration;
        // obtain width and length
        const double length = getParentDemandElements().at(0)->getAttributeDouble(SUMO_ATTR_LENGTH);
        const double width = getParentDemandElements().at(0)->getAttributeDouble(SUMO_ATTR_WIDTH);
        // obtain diameter around person (used to calculate distance bewteen cursor and person)
        const double distanceSquared = pow(exaggeration * std::max(length, width), 2);
        // obtain img file
        const std::string file = getParentDemandElements().at(0)->getAttribute(SUMO_ATTR_IMGFILE);
        Position personPosition;
        // obtain position depending of first PersonPlan child
        if (getChildDemandElements().front()->getTagProperty().isPersonStop()) {
            // obtain position of stop center
            personPosition = getChildDemandElements().front()->getPositionInView();
        } else {
            // obtain position of first edge
            personPosition = getChildDemandElements().front()->getDemandElementSegmentGeometry().getFirstPosition();
        }
        // check that position is valid and person can be drawn
        if ((personPosition != Position::INVALID) &&
                !(s.drawForPositionSelection && (personPosition.distanceSquaredTo(myNet->getViewNet()->getPositionInformation()) > distanceSquared))) {
            // push GL ID
            glPushName(getGlID());
            // push draw matrix
            glPushMatrix();
            // Start with the drawing of the area traslating matrix to origin
            myNet->getViewNet()->drawTranslateFrontAttributeCarrier(this, getType());
            // translate and rotate
            glTranslated(personPosition.x(), personPosition.y(), 0);
            glRotated(90, 0, 0, 1);
            // set person color
            setColor(s);
            // set scale
            glScaled(exaggeration, exaggeration, 1);
            // draw person depending of detail level
            if (s.drawDetail(s.detailSettings.personShapes, exaggeration)) {
                GUIBasePersonHelper::drawAction_drawAsImage(0, length, width, file, SVS_PEDESTRIAN, exaggeration);
            } else if (s.drawDetail(s.detailSettings.personCircles, exaggeration)) {
                GUIBasePersonHelper::drawAction_drawAsCircle(length, width);
            } else if (s.drawDetail(s.detailSettings.personTriangles, exaggeration)) {
                GUIBasePersonHelper::drawAction_drawAsTriangle(0, length, width);
            }
            // pop matrix
            glPopMatrix();
            // pop name
            glPopName();
            // draw name
            drawName(personPosition, s.scale, s.personName, s.angle);
            if (s.personValue.show) {
                Position personValuePosition = personPosition + Position(0, 0.6 * s.personName.scaledSize(s.scale));
                const double value = getColorValue(s, s.personColorer.getActive());
                GLHelper::drawTextSettings(s.personValue, toString(value), personValuePosition, s.scale, s.angle, GLO_MAX - getType());
            }
            // check if dotted contours has to be drawn
            if (s.drawDottedContour() || myNet->getViewNet()->getInspectedAttributeCarrier() == this) {
                // draw using drawDottedSquaredShape
                GNEGeometry::drawDottedSquaredShape(true, s, personPosition, 0.5, 0.5, 0, 0, 0, exaggeration);
            }
            if (s.drawDottedContour() || myNet->getViewNet()->getFrontAttributeCarrier() == this) {
                // draw using drawDottedSquaredShape
                GNEGeometry::drawDottedSquaredShape(false, s, personPosition, 0.5, 0.5, 0, 0, 0, exaggeration);
            }
        }
    }
}


void
GNEPerson::drawPartialGL(const GUIVisualizationSettings& /*s*/, const GNELane* /*lane*/, const double /*offsetFront*/) const {
    // Stops don't use drawPartialGL
}


void
GNEPerson::drawPartialGL(const GUIVisualizationSettings& /*s*/, const GNELane* /*fromLane*/, const GNELane* /*toLane*/, const double /*offsetFront*/) const {
    // Stops don't use drawPartialGL
}


std::string
GNEPerson::getAttribute(SumoXMLAttr key) const {
    // declare string error
    std::string error;
    switch (key) {
        case SUMO_ATTR_ID:
            return getID();
        case SUMO_ATTR_TYPE:
            return getParentDemandElements().at(0)->getID();
        case SUMO_ATTR_COLOR:
            if (wasSet(VEHPARS_COLOR_SET)) {
                return toString(color);
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_COLOR);
            }
        case SUMO_ATTR_DEPARTPOS:
            if (wasSet(VEHPARS_DEPARTPOS_SET)) {
                return getDepartPos();
            } else {
                return myTagProperty.getDefaultValue(SUMO_ATTR_DEPARTPOS);
            }
        // Specific of persons
        case SUMO_ATTR_DEPART:
            return toString(depart);
        // Specific of personFlows
        case SUMO_ATTR_BEGIN:
            return time2string(depart);
        case SUMO_ATTR_END:
            return time2string(repetitionEnd);
        case SUMO_ATTR_PERSONSPERHOUR:
            return toString(3600 / STEPS2TIME(repetitionOffset));
        case SUMO_ATTR_PERIOD:
            return time2string(repetitionOffset);
        case SUMO_ATTR_PROB:
            return toString(repetitionProbability);
        case SUMO_ATTR_NUMBER:
            return toString(repetitionNumber);
        //
        case GNE_ATTR_SELECTED:
            return toString(isAttributeCarrierSelected());
        case GNE_ATTR_PARAMETERS:
            return getParametersStr();
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


double
GNEPerson::getAttributeDouble(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_DEPARTPOS:
            if (departPosProcedure == DepartPosDefinition::GIVEN) {
                return departPos;
            } else {
                return 0;
            }
        default:
            throw InvalidArgument(getTagStr() + " doesn't have a double attribute of type '" + toString(key) + "'");
    }
}


void
GNEPerson::setAttribute(SumoXMLAttr key, const std::string& value, GNEUndoList* undoList) {
    if (value == getAttribute(key)) {
        return; //avoid needless changes, later logic relies on the fact that attributes have changed
    }
    switch (key) {
        case SUMO_ATTR_ID:
        case SUMO_ATTR_TYPE:
        case SUMO_ATTR_COLOR:
        case SUMO_ATTR_DEPARTPOS:
        // Specific of persons
        case SUMO_ATTR_DEPART:
        // Specific of personFlows
        case SUMO_ATTR_BEGIN:
        case SUMO_ATTR_END:
        case SUMO_ATTR_NUMBER:
        case SUMO_ATTR_PERSONSPERHOUR:
        case SUMO_ATTR_PERIOD:
        case SUMO_ATTR_PROB:
        //
        case GNE_ATTR_PARAMETERS:
        case GNE_ATTR_SELECTED:
            undoList->p_add(new GNEChange_Attribute(this, key, value));
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


bool
GNEPerson::isValid(SumoXMLAttr key, const std::string& value) {
    // declare string error
    std::string error;
    switch (key) {
        case SUMO_ATTR_ID:
            // Persons and personflows share namespace
            if (SUMOXMLDefinitions::isValidVehicleID(value) &&
                    (myNet->retrieveDemandElement(SUMO_TAG_PERSON, value, false) == nullptr) &&
                    (myNet->retrieveDemandElement(SUMO_TAG_PERSONFLOW, value, false) == nullptr)) {
                return true;
            } else {
                return false;
            }
        case SUMO_ATTR_TYPE:
            return SUMOXMLDefinitions::isValidTypeID(value) && (myNet->retrieveDemandElement(SUMO_TAG_VTYPE, value, false) != nullptr);
        case SUMO_ATTR_COLOR:
            return canParse<RGBColor>(value);
        case SUMO_ATTR_DEPARTPOS: {
            double dummyDepartPos;
            DepartPosDefinition dummyDepartPosProcedure;
            parseDepartPos(value, toString(SUMO_TAG_VEHICLE), id, dummyDepartPos, dummyDepartPosProcedure, error);
            // if error is empty, given value is valid
            return error.empty();
        }
        // Specific of persons
        case SUMO_ATTR_DEPART: {
            if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        }
        // Specific of personflows
        case SUMO_ATTR_BEGIN:
            if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_END:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_PERSONSPERHOUR:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) > 0);
            } else {
                return false;
            }
        case SUMO_ATTR_PERIOD:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) > 0);
            } else {
                return false;
            }
        case SUMO_ATTR_PROB:
            if (value.empty()) {
                return true;
            } else if (canParse<double>(value)) {
                return (parse<double>(value) >= 0);
            } else {
                return false;
            }
        case SUMO_ATTR_NUMBER:
            if (canParse<int>(value)) {
                return (parse<int>(value) >= 0);
            } else {
                return false;
            }
        //
        case GNE_ATTR_SELECTED:
            return canParse<bool>(value);
        case GNE_ATTR_PARAMETERS:
            return Parameterised::areParametersValid(value);
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPerson::enableAttribute(SumoXMLAttr key, GNEUndoList* undoList) {
    // obtain a copy of parameter sets
    int newParametersSet = parametersSet;
    // modify newParametersSet
    GNERouteHandler::setFlowParameters(key, newParametersSet);
    // add GNEChange_EnableAttribute
    undoList->add(new GNEChange_EnableAttribute(this, parametersSet, newParametersSet), true);
}


void
GNEPerson::disableAttribute(SumoXMLAttr /*key*/, GNEUndoList* /*undoList*/) {
    // nothing to disable
}


bool
GNEPerson::isAttributeEnabled(SumoXMLAttr key) const {
    switch (key) {
        case SUMO_ATTR_END:
            return (parametersSet & VEHPARS_END_SET) != 0;
        case SUMO_ATTR_NUMBER:
            return (parametersSet & VEHPARS_NUMBER_SET) != 0;
        case SUMO_ATTR_PERSONSPERHOUR:
            return (parametersSet & VEHPARS_VPH_SET) != 0;
        case SUMO_ATTR_PERIOD:
            return (parametersSet & VEHPARS_PERIOD_SET) != 0;
        case SUMO_ATTR_PROB:
            return (parametersSet & VEHPARS_PROB_SET) != 0;
        default:
            return true;
    }
}


std::string
GNEPerson::getPopUpID() const {
    return getTagStr();
}


std::string
GNEPerson::getHierarchyName() const {
    // special case for Trips and flow
    if ((myTagProperty.getTag() == SUMO_TAG_TRIP) || (myTagProperty.getTag() == SUMO_TAG_FLOW)) {
        // check if we're inspecting a Edge
        if (myNet->getViewNet()->getInspectedAttributeCarrier() &&
                myNet->getViewNet()->getInspectedAttributeCarrier()->getTagProperty().getTag() == SUMO_TAG_EDGE) {
            // check if edge correspond to a "from", "to" or "via" edge
            if (getParentEdges().front() == myNet->getViewNet()->getInspectedAttributeCarrier()) {
                return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID) + " (from)";
            } else if (getParentEdges().front() == myNet->getViewNet()->getInspectedAttributeCarrier()) {
                return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID) + " (to)";
            } else {
                // iterate over via
                for (const auto& i : via) {
                    if (i == myNet->getViewNet()->getInspectedAttributeCarrier()->getID()) {
                        return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID) + " (via)";
                    }
                }
            }
        }
    }
    return getTagStr() + ": " + getAttribute(SUMO_ATTR_ID);
}

// ===========================================================================
// protected
// ===========================================================================

void
GNEPerson::setColor(const GUIVisualizationSettings& s) const {
    const GUIColorer& c = s.personColorer;
    if (!setFunctionalColor(c.getActive())) {
        GLHelper::setColor(c.getScheme().getColor(getColorValue(s, c.getActive())));
    }
}


bool
GNEPerson::setFunctionalColor(int /* activeScheme */) const {
    /*
    switch (activeScheme) {
        case 0: {
            if (getParameter().wasSet(VEHPARS_COLOR_SET)) {
                GLHelper::setColor(getParameter().color);
                return true;
            }
            if (getVehicleType().wasSet(VTYPEPARS_COLOR_SET)) {
                GLHelper::setColor(getVehicleType().getColor());
                return true;
            }
            return false;
        }
        case 2: {
            if (getParameter().wasSet(VEHPARS_COLOR_SET)) {
                GLHelper::setColor(getParameter().color);
                return true;
            }
            return false;
        }
        case 3: {
            if (getVehicleType().wasSet(VTYPEPARS_COLOR_SET)) {
                GLHelper::setColor(getVehicleType().getColor());
                return true;
            }
            return false;
        }
        case 8: { // color by angle
            double hue = GeomHelper::naviDegree(getAngle());
            GLHelper::setColor(RGBColor::fromHSV(hue, 1., 1.));
            return true;
        }
        case 9: { // color randomly (by pointer)
            const double hue = (long)this % 360; // [0-360]
            const double sat = (((long)this / 360) % 67) / 100.0 + 0.33; // [0.33-1]
            GLHelper::setColor(RGBColor::fromHSV(hue, sat, 1.));
            return true;
        }
        default:
            return false;
    }
    */
    return false;
}

// ===========================================================================
// private
// ===========================================================================

GNEPerson::personPlanSegment::personPlanSegment(GNEDemandElement* _personPlan) :
    personPlan(_personPlan),
    edge(nullptr),
    arrivalPos(-1) {
}


GNEPerson::personPlanSegment::personPlanSegment() :
    personPlan(nullptr),
    edge(nullptr),
    arrivalPos(-1) {
}


void
GNEPerson::setAttribute(SumoXMLAttr key, const std::string& value) {
    // declare string error
    std::string error;
    switch (key) {
        case SUMO_ATTR_ID:
            myNet->getAttributeCarriers()->updateID(this, value);
            // Change IDs of all person plans children
            for (const auto& personPlans : getChildDemandElements()) {
                personPlans->setMicrosimID(getID());
            }
            break;
        case SUMO_ATTR_TYPE:
            replaceDemandElementParent(SUMO_TAG_PTYPE, value, 0);
            // set manually vtypeID (needed for saving)
            vtypeid = value;
            break;
        case SUMO_ATTR_COLOR:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                color = parse<RGBColor>(value);
                // mark parameter as set
                parametersSet |= VEHPARS_COLOR_SET;
            } else {
                // set default value
                color = parse<RGBColor>(myTagProperty.getDefaultValue(key));
                // unset parameter
                parametersSet &= ~VEHPARS_COLOR_SET;
            }
            break;
        case SUMO_ATTR_DEPARTPOS:
            if (!value.empty() && (value != myTagProperty.getDefaultValue(key))) {
                parseDepartPos(value, toString(SUMO_TAG_VEHICLE), id, departPos, departPosProcedure, error);
                // mark parameter as set
                parametersSet |= VEHPARS_DEPARTPOS_SET;
            } else {
                // set default value
                parseDepartPos(myTagProperty.getDefaultValue(key), toString(SUMO_TAG_VEHICLE), id, departPos, departPosProcedure, error);
                // unset parameter
                parametersSet &= ~VEHPARS_DEPARTPOS_SET;
            }
            // compute person
            updateGeometry();
            break;
        // Specific of persons
        case SUMO_ATTR_DEPART: {
            parseDepart(value, toString(SUMO_TAG_VEHICLE), id, depart, departProcedure, error);
            break;
        }
        // Specific of personFlows
        case SUMO_ATTR_BEGIN: {
            depart = string2time(value);
            break;
        }
        case SUMO_ATTR_END:
            repetitionEnd = string2time(value);
            break;
        case SUMO_ATTR_PERSONSPERHOUR:
            repetitionOffset = TIME2STEPS(3600 / parse<double>(value));
            break;
        case SUMO_ATTR_PERIOD:
            repetitionOffset = string2time(value);
            break;
        case SUMO_ATTR_PROB:
            repetitionProbability = parse<double>(value);
            break;
        case SUMO_ATTR_NUMBER:
            repetitionNumber = parse<int>(value);
            break;
        //
        case GNE_ATTR_SELECTED:
            if (parse<bool>(value)) {
                selectAttributeCarrier();
            } else {
                unselectAttributeCarrier();
            }
            break;
        case GNE_ATTR_PARAMETERS:
            setParametersStr(value);
            break;
        default:
            throw InvalidArgument(getTagStr() + " doesn't have an attribute of type '" + toString(key) + "'");
    }
}


void
GNEPerson::setEnabledAttribute(const int enabledAttributes) {
    parametersSet = enabledAttributes;
}


/****************************************************************************/
