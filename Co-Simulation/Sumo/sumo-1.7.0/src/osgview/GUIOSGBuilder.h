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
/// @file    GUIOSGBuilder.h
/// @author  Daniel Krajzewicz
/// @date    19.01.2012
///
// Builds OSG nodes from microsim objects
/****************************************************************************/
#pragma once
#include <config.h>

#ifdef HAVE_OSG

#include <map>
#include "GUIOSGView.h"


// ===========================================================================
// class declarations
// ===========================================================================
namespace osg {
class Node;
class Group;
class PositionAttitudeTransform;
}
namespace osgUtil {
class Tessellator;
}
class MSVehicleType;
class MSEdge;
class GUIJunctionWrapper;


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class GUIOSGBuilder
 * @brief Builds OSG nodes from microsim objects
 */
class GUIOSGBuilder {
public:
    static osg::Group* buildOSGScene(osg::Node* const tlg, osg::Node* const tly, osg::Node* const tlr, osg::Node* const tlu);

    static void buildDecal(const GUISUMOAbstractView::Decal& d, osg::Group& addTo);

    static void buildLight(const GUISUMOAbstractView::Decal& d, osg::Group& addTo);

    static osg::PositionAttitudeTransform* getTrafficLight(const GUISUMOAbstractView::Decal& d, osg::Node* tl, const osg::Vec4& color, const double size = 0.5);

    static GUIOSGView::OSGMovable buildMovable(const MSVehicleType& type);

private:
    static void buildOSGEdgeGeometry(const MSEdge& edge,
                                     osg::Group& addTo, osgUtil::Tessellator& tessellator);

    static void buildOSGJunctionGeometry(GUIJunctionWrapper& junction,
                                         osg::Group& addTo, osgUtil::Tessellator& tessellator);

    static void setShapeState(osg::ref_ptr<osg::ShapeDrawable> shape);

private:
    static std::map<std::string, osg::ref_ptr<osg::Node> > myCars;

};


#endif
