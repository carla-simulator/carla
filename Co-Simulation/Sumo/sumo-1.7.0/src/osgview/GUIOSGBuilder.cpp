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
/// @file    GUIOSGBuilder.cpp
/// @author  Daniel Krajzewicz
/// @author  Michael Behrisch
/// @date    19.01.2012
///
// Builds OSG nodes from microsim objects
/****************************************************************************/
#include <config.h>

#ifdef HAVE_OSG

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4127) // do not warn about constant conditional expression
#pragma warning(disable: 4275) // do not warn about the DLL interface for OSG
#endif
#include <osg/Version>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/ShapeDrawable>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Sequence>
#include <osg/Texture2D>
#include <osgViewer/Viewer>
#include <osgUtil/Tessellator>
#include <osg/PositionAttitudeTransform>
#include <osg/ShadeModel>
#include <osg/Light>
#include <osg/LightSource>
#include <osg/ComputeBoundsVisitor>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <microsim/MSNet.h>
#include <microsim/MSEdge.h>
#include <microsim/MSLane.h>
#include <microsim/MSEdgeControl.h>
#include <microsim/MSJunctionControl.h>
#include <microsim/MSJunction.h>
#include <microsim/MSVehicleType.h>
#include <microsim/traffic_lights/MSTLLogicControl.h>
#include <microsim/traffic_lights/MSTrafficLightLogic.h>
#include <guisim/GUIJunctionWrapper.h>
#include <guisim/GUINet.h>
#include <guisim/GUIEdge.h>
#include <guisim/GUILane.h>
#include <utils/common/MsgHandler.h>
#include <utils/gui/windows/GUISUMOAbstractView.h>
#include "GUIOSGView.h"
#include "GUIOSGBuilder.h"


//#define DEBUG_TESSEL

// ===========================================================================
// static member variables
// ===========================================================================
std::map<std::string, osg::ref_ptr<osg::Node> > GUIOSGBuilder::myCars;


// ===========================================================================
// member method definitions
// ===========================================================================
osg::Group*
GUIOSGBuilder::buildOSGScene(osg::Node* const tlg, osg::Node* const tly, osg::Node* const tlr, osg::Node* const tlu) {
    osgUtil::Tessellator tesselator;
    osg::Group* root = new osg::Group();
    GUINet* net = static_cast<GUINet*>(MSNet::getInstance());
    // build edges
    for (const MSEdge* e : net->getEdgeControl().getEdges()) {
        if (!e->isInternal()) {
            buildOSGEdgeGeometry(*e, *root, tesselator);
        }
    }
    // build junctions
    for (int index = 0; index < (int)net->myJunctionWrapper.size(); ++index) {
        buildOSGJunctionGeometry(*net->myJunctionWrapper[index], *root, tesselator);
    }
    // build traffic lights
    GUISUMOAbstractView::Decal d;
    const std::vector<std::string> tlids = net->getTLSControl().getAllTLIds();
    for (std::vector<std::string>::const_iterator i = tlids.begin(); i != tlids.end(); ++i) {
        MSTLLogicControl::TLSLogicVariants& vars = net->getTLSControl().get(*i);
        const MSTrafficLightLogic::LaneVectorVector& lanes = vars.getActive()->getLaneVectors();
        const MSLane* lastLane = 0;
        int idx = 0;
        for (MSTrafficLightLogic::LaneVectorVector::const_iterator j = lanes.begin(); j != lanes.end(); ++j, ++idx) {
            if ((*j).size() == 0) {
                continue;
            }
            const MSLane* const lane = (*j)[0];
            const Position pos = lane->getShape().back();
            const double angle =  osg::DegreesToRadians(lane->getShape().rotationDegreeAtOffset(-1.) + 90.);
            d.centerZ = pos.z() + 4.;
            if (lane == lastLane) {
                d.centerX += 1.2 * sin(angle);
                d.centerY += 1.2 * cos(angle);
            } else {
                d.centerX = pos.x() - 1.5 * sin(angle);
                d.centerY = pos.y() - 1.5 * cos(angle);
            }
            osg::Switch* switchNode = new osg::Switch();
            switchNode->addChild(getTrafficLight(d, tlg, osg::Vec4d(0.1, 0.5, 0.1, 1.0), .25), false);
            switchNode->addChild(getTrafficLight(d, tly, osg::Vec4d(0.5, 0.5, 0.1, 1.0), .25), false);
            switchNode->addChild(getTrafficLight(d, tlr, osg::Vec4d(0.5, 0.1, 0.1, 1.0), .25), false);
            switchNode->addChild(getTrafficLight(d, tlu, osg::Vec4d(0.8, 0.4, 0.0, 1.0), .25), false);
            root->addChild(switchNode);
            const MSLink* const l = vars.getActive()->getLinksAt(idx)[0];
            vars.addSwitchCommand(new GUIOSGView::Command_TLSChange(l, switchNode));
            lastLane = lane;
        }
    }
    return root;
}


void
GUIOSGBuilder::buildLight(const GUISUMOAbstractView::Decal& d, osg::Group& addTo) {
    // each light must have a unique number
    osg::Light* light = new osg::Light(d.filename[5] - '0');
    // we set the light's position via a PositionAttitudeTransform object
    light->setPosition(osg::Vec4(0.0, 0.0, 0.0, 1.0));
    light->setDiffuse(osg::Vec4(1.0, 1.0, 1.0, 1.0));
    light->setSpecular(osg::Vec4(1.0, 1.0, 1.0, 1.0));
    light->setAmbient(osg::Vec4(1.0, 1.0, 1.0, 1.0));

    osg::LightSource* lightSource = new osg::LightSource();
    lightSource->setLight(light);
    lightSource->setLocalStateSetModes(osg::StateAttribute::ON);
    lightSource->setStateSetModes(*addTo.getOrCreateStateSet(), osg::StateAttribute::ON);

    osg::PositionAttitudeTransform* lightTransform = new osg::PositionAttitudeTransform();
    lightTransform->addChild(lightSource);
    lightTransform->setPosition(osg::Vec3d(d.centerX, d.centerY, d.centerZ));
    lightTransform->setScale(osg::Vec3d(0.1, 0.1, 0.1));
    addTo.addChild(lightTransform);
}


void
GUIOSGBuilder::buildOSGEdgeGeometry(const MSEdge& edge,
                                    osg::Group& addTo,
                                    osgUtil::Tessellator& tessellator) {
    const std::vector<MSLane*>& lanes = edge.getLanes();
    for (std::vector<MSLane*>::const_iterator j = lanes.begin(); j != lanes.end(); ++j) {
        MSLane* l = (*j);
        const PositionVector& shape = l->getShape();
        osg::Geode* geode = new osg::Geode();
        osg::Geometry* geom = new osg::Geometry();
        geode->addDrawable(geom);
        addTo.addChild(geode);
        const int shapeSize = (int)(edge.isWalkingArea() ? shape.size() : shape.size() * 2);
        const float zOffset = edge.isWalkingArea() || edge.isCrossing() ? 0.01f : 0.f;
        osg::Vec3Array* osg_coords = new osg::Vec3Array(shapeSize);
        geom->setVertexArray(osg_coords);
        if (edge.isWalkingArea()) {
            int index = 0;
            for (int k = 0; k < (int)shape.size(); ++k, ++index) {
                (*osg_coords)[index].set((float)shape[k].x(), (float)shape[k].y(), (float)shape[k].z() + zOffset);
            }
        } else {
            PositionVector rshape = shape;
            rshape.move2side(l->getWidth() / 2);
            int index = 0;
            for (int k = 0; k < (int)rshape.size(); ++k, ++index) {
                (*osg_coords)[index].set((float)rshape[k].x(), (float)rshape[k].y(), (float)rshape[k].z() + zOffset);
            }
            PositionVector lshape = shape;
            lshape.move2side(-l->getWidth() / 2);
            for (int k = (int) lshape.size() - 1; k >= 0; --k, ++index) {
                (*osg_coords)[index].set((float)lshape[k].x(), (float)lshape[k].y(), (float)lshape[k].z() + zOffset);
            }
        }
        osg::Vec3Array* osg_normals = new osg::Vec3Array(1);
        (*osg_normals)[0] = osg::Vec3(0, 0, 1);
#if OSG_MIN_VERSION_REQUIRED(3,2,0)
        geom->setNormalArray(osg_normals, osg::Array::BIND_PER_PRIMITIVE_SET);
#else
        geom->setNormalArray(osg_normals);
        geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
#endif
        osg::Vec4ubArray* osg_colors = new osg::Vec4ubArray(1);
        (*osg_colors)[0].set(128, 128, 128, 255);
#if OSG_MIN_VERSION_REQUIRED(3,2,0)
        geom->setColorArray(osg_colors, osg::Array::BIND_OVERALL);
#else
        geom->setColorArray(osg_colors);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);
#endif
        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, shapeSize));

        osg::ref_ptr<osg::StateSet> ss = geode->getOrCreateStateSet();
        ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        ss->setMode(GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);

        if (shape.size() > 2) {
            tessellator.retessellatePolygons(*geom);
#ifdef DEBUG_TESSEL
            std::cout << "l=" << l->getID() << " origPoints=" << shape.size() << " geomSize=" << geom->getVertexArray()->getNumElements() << " points=";
            for (int i = 0; i < (int)geom->getVertexArray()->getNumElements(); i++) {
                const osg::Vec3& p = (*((osg::Vec3Array*)geom->getVertexArray()))[i];
                std::cout << p.x() << "," << p.y() << "," << p.z() << " ";
            }
            std::cout << "\n";
#endif
        }
        static_cast<GUILane*>(l)->setGeometry(geom);
    }
}


void
GUIOSGBuilder::buildOSGJunctionGeometry(GUIJunctionWrapper& junction,
                                        osg::Group& addTo,
                                        osgUtil::Tessellator& tessellator) {
    const PositionVector& shape = junction.getJunction().getShape();
    osg::Geode* geode = new osg::Geode();
    osg::Geometry* geom = new osg::Geometry();
    geode->addDrawable(geom);
    addTo.addChild(geode);
    osg::Vec3Array* osg_coords = new osg::Vec3Array((int)shape.size());
    geom->setVertexArray(osg_coords);
    for (int k = 0; k < (int)shape.size(); ++k) {
        (*osg_coords)[k].set((float)shape[k].x(), (float)shape[k].y(), (float)shape[k].z());
    }
    osg::Vec3Array* osg_normals = new osg::Vec3Array(1);
    (*osg_normals)[0] = osg::Vec3(0, 0, 1);
#if OSG_MIN_VERSION_REQUIRED(3,2,0)
    geom->setNormalArray(osg_normals, osg::Array::BIND_PER_PRIMITIVE_SET);
#else
    geom->setNormalArray(osg_normals);
    geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
#endif
    osg::Vec4ubArray* osg_colors = new osg::Vec4ubArray(1);
    (*osg_colors)[0].set(128, 128, 128, 255);
#if OSG_MIN_VERSION_REQUIRED(3,2,0)
    geom->setColorArray(osg_colors, osg::Array::BIND_OVERALL);
#else
    geom->setColorArray(osg_colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
#endif
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON, 0, (int)shape.size()));

    osg::ref_ptr<osg::StateSet> ss = geode->getOrCreateStateSet();
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setMode(GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);

    if (shape.size() > 4) {
        tessellator.retessellatePolygons(*geom);
    }
    junction.setGeometry(geom);
}


void
GUIOSGBuilder::buildDecal(const GUISUMOAbstractView::Decal& d, osg::Group& addTo) {
    osg::Node* pLoadedModel = osgDB::readNodeFile(d.filename);
    if (pLoadedModel == nullptr) {
        WRITE_ERROR("Could not load '" + d.filename + "'.");
        return;
    }
    osg::ShadeModel* sm = new osg::ShadeModel();
    sm->setMode(osg::ShadeModel::FLAT);
    pLoadedModel->getOrCreateStateSet()->setAttribute(sm);
    osg::PositionAttitudeTransform* base = new osg::PositionAttitudeTransform();
    base->addChild(pLoadedModel);
    osg::ComputeBoundsVisitor bboxCalc;
    pLoadedModel->accept(bboxCalc);
    const osg::BoundingBox& bbox = bboxCalc.getBoundingBox();
    WRITE_MESSAGE("Loaded decal '" + d.filename + "' with bounding box " + toString(Position(bbox.xMin(), bbox.yMin(), bbox.zMin())) + " " + toString(Position(bbox.xMax(), bbox.yMax(), bbox.zMax())) + ".");
    double xScale = d.width > 0 ? d.width / (bbox.xMax() - bbox.xMin()) : 1.;
    double yScale = d.height > 0 ? d.height / (bbox.yMax() - bbox.yMin()) : 1.;
    const double zScale = d.altitude > 0 ? d.altitude / (bbox.zMax() - bbox.zMin()) : 1.;
    if (d.width < 0 && d.height < 0 && d.altitude > 0) {
        xScale = yScale = zScale;
    }
    base->setScale(osg::Vec3d(xScale, yScale, zScale));
    base->setPosition(osg::Vec3d(d.centerX, d.centerY, d.centerZ));
    base->setAttitude(osg::Quat(osg::DegreesToRadians(d.roll), osg::Vec3d(1, 0, 0),
                                osg::DegreesToRadians(d.tilt), osg::Vec3d(0, 1, 0),
                                osg::DegreesToRadians(d.rot), osg::Vec3d(0, 0, 1)));
    addTo.addChild(base);
}


osg::PositionAttitudeTransform*
GUIOSGBuilder::getTrafficLight(const GUISUMOAbstractView::Decal& d, osg::Node* tl, const osg::Vec4& color, const double size) {
    osg::PositionAttitudeTransform* ret = new osg::PositionAttitudeTransform();
    if (tl != nullptr) {
        osg::PositionAttitudeTransform* base = new osg::PositionAttitudeTransform();
        base->addChild(tl);
        osg::ComputeBoundsVisitor bboxCalc;
        tl->accept(bboxCalc);
        const osg::BoundingBox& bbox = bboxCalc.getBoundingBox();
        double xScale = d.width > 0 ? d.width / (bbox.xMax() - bbox.xMin()) : 1.;
        double yScale = d.height > 0 ? d.height / (bbox.yMax() - bbox.yMin()) : 1.;
        const double zScale = d.altitude > 0 ? d.altitude / (bbox.zMax() - bbox.zMin()) : 1.;
        if (d.width < 0 && d.height < 0 && d.altitude > 0) {
            xScale = yScale = zScale;
        }
        base->setScale(osg::Vec3d(xScale, yScale, zScale));
        base->setPosition(osg::Vec3d(d.centerX, d.centerY, d.centerZ));
        base->setAttitude(osg::Quat(osg::DegreesToRadians(d.roll), osg::Vec3(1, 0, 0),
                                    osg::DegreesToRadians(d.tilt), osg::Vec3(0, 1, 0),
                                    osg::DegreesToRadians(d.rot), osg::Vec3(0, 0, 1)));
        ret->addChild(base);
    }
    osg::Geode* geode = new osg::Geode();
    osg::Vec3d center(d.centerX, d.centerY, d.centerZ);
    osg::ShapeDrawable* shape = new osg::ShapeDrawable(new osg::Sphere(center, (float)size));
    geode->addDrawable(shape);
    osg::ref_ptr<osg::StateSet> ss = shape->getOrCreateStateSet();
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setMode(GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
    osg::PositionAttitudeTransform* ellipse = new osg::PositionAttitudeTransform();
    ellipse->addChild(geode);
    ellipse->setPivotPoint(center);
    ellipse->setPosition(center);
    ellipse->setScale(osg::Vec3d(4., 4., 2.5 * d.altitude + 1.1));
    shape->setColor(color);
    ret->addChild(ellipse);
    return ret;
}


void
GUIOSGBuilder::setShapeState(osg::ref_ptr<osg::ShapeDrawable> shape) {
    osg::ref_ptr<osg::StateSet> ss = shape->getOrCreateStateSet();
    ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    ss->setMode(GL_BLEND, osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED | osg::StateAttribute::ON);
}


GUIOSGView::OSGMovable
GUIOSGBuilder::buildMovable(const MSVehicleType& type) {
    GUIOSGView::OSGMovable m;
    m.pos = new osg::PositionAttitudeTransform();
    double enlarge = 0.;
    const std::string& osgFile = type.getOSGFile();
    if (myCars.find(osgFile) == myCars.end()) {
        myCars[osgFile] = osgDB::readNodeFile(osgFile);
        if (myCars[osgFile] == 0) {
            WRITE_ERROR("Could not load '" + osgFile + "'.");
        }
    }
    osg::Node* carNode = myCars[osgFile];
    if (carNode != nullptr) {
        osg::ComputeBoundsVisitor bboxCalc;
        carNode->accept(bboxCalc);
        const osg::BoundingBox& bbox = bboxCalc.getBoundingBox();
        osg::PositionAttitudeTransform* base = new osg::PositionAttitudeTransform();
        base->addChild(carNode);
        base->setPivotPoint(osg::Vec3d((bbox.xMin() + bbox.xMax()) / 2., bbox.yMin(), bbox.zMin()));
        base->setScale(osg::Vec3d(type.getWidth() / (bbox.xMax() - bbox.xMin()),
                                  type.getLength() / (bbox.yMax() - bbox.yMin()),
                                  type.getHeight() / (bbox.zMax() - bbox.zMin())));
        m.pos->addChild(base);
        enlarge = type.getMinGap() / 2.;
    }
    m.lights = new osg::Switch();
    for (double offset = -0.3; offset < 0.5; offset += 0.6) {
        osg::Geode* geode = new osg::Geode();
        osg::ShapeDrawable* right = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3d(offset, (type.getLength() - .9) / 2., (type.getHeight() - .5) / 2.), .1f));
        geode->addDrawable(right);
        setShapeState(right);
        right->setColor(osg::Vec4(1.f, .5f, 0.f, .8f));
        osg::Sequence* seq = new osg::Sequence();
        // Wikipedia says about 1.5Hz
        seq->addChild(geode, .33);
        seq->addChild(new osg::Geode(), .33);
        // loop through all children
        seq->setInterval(osg::Sequence::LOOP, 0, -1);
        // real-time playback, repeat indefinitely
        seq->setDuration(1.0f, -1);
        // must be started explicitly
        seq->setMode(osg::Sequence::START);
        m.lights->addChild(seq);
    }

    osg::Geode* geode = new osg::Geode();
    osg::CompositeShape* comp = new osg::CompositeShape();
    comp->addChild(new osg::Sphere(osg::Vec3d(-0.3, (type.getLength() + .8) / 2., (type.getHeight() - .5) / 2.), .1f));
    comp->addChild(new osg::Sphere(osg::Vec3d(0.3, (type.getLength() + .8) / 2., (type.getHeight() - .5) / 2.), .1f));
    osg::ShapeDrawable* brake = new osg::ShapeDrawable(comp);
    brake->setColor(osg::Vec4(1.f, 0.f, 0.f, .8f));
    geode->addDrawable(brake);
    setShapeState(brake);
    m.lights->addChild(geode);

    geode = new osg::Geode();
    osg::Vec3d center(0, type.getLength() / 2., type.getHeight() / 2.);
    m.geom = new osg::ShapeDrawable(new osg::Sphere(center, .5f));
    geode->addDrawable(m.geom);
    setShapeState(m.geom);
    osg::PositionAttitudeTransform* ellipse = new osg::PositionAttitudeTransform();
    ellipse->addChild(geode);
    ellipse->addChild(m.lights);
    ellipse->setPivotPoint(center);
    ellipse->setPosition(center);
    ellipse->setScale(osg::Vec3d(type.getWidth() + enlarge, type.getLength() + enlarge, type.getHeight() + enlarge));
    m.pos->addChild(ellipse);
    m.active = true;
    return m;
}

#endif


/****************************************************************************/
