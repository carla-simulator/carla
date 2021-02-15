//
// Copyright (C) 2010 Christoph Sommer <christoph.sommer@informatik.uni-erlangen.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// AnnotationManager - manages annotations on the OMNeT++ canvas

#include <sstream>
#include <cmath>

#include "veins/modules/world/annotations/AnnotationManager.h"
#include "veins/modules/mobility/traci/TraCIScenarioManager.h"
#include "veins/modules/mobility/traci/TraCICommandInterface.h"

Define_Module(veins::AnnotationManager);

using veins::AnnotationManager;
using veins::TraCIScenarioManager;
using veins::TraCIScenarioManagerAccess;

namespace {
const short EVT_SCHEDULED_ERASE = 3;
}

void AnnotationManager::initialize()
{
    scheduledEraseEvts.clear();

    annotations.clear();
    groups.clear();

    annotationLayer = new cGroupFigure();
    cCanvas* canvas = getParentModule()->getCanvas();
    canvas->addFigure(annotationLayer, canvas->findFigure("submodules"));

    annotationsXml = par("annotations");
    addFromXml(annotationsXml);
}

void AnnotationManager::finish()
{
    hideAll();
}

AnnotationManager::~AnnotationManager()
{
    while (scheduledEraseEvts.begin() != scheduledEraseEvts.end()) {
        cancelAndDelete(*scheduledEraseEvts.begin());
        scheduledEraseEvts.erase(scheduledEraseEvts.begin());
    }
    scheduledEraseEvts.clear();

    while (annotations.begin() != annotations.end()) {
        delete *annotations.begin();
        annotations.erase(annotations.begin());
    }
    annotations.clear();

    while (groups.begin() != groups.end()) {
        delete *groups.begin();
        groups.erase(groups.begin());
    }
    groups.clear();
}

void AnnotationManager::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
        return;
    }
    throw cRuntimeError("AnnotationManager doesn't handle messages from other modules");
}

void AnnotationManager::handleSelfMsg(cMessage* msg)
{

    if (msg->getKind() == EVT_SCHEDULED_ERASE) {
        Annotation* a = static_cast<Annotation*>(msg->getContextPointer());
        ASSERT(a);

        erase(a);

        scheduledEraseEvts.remove(msg);
        delete msg;
        return;
    }

    throw cRuntimeError("unknown self message type");
}

void AnnotationManager::handleParameterChange(const char* parname)
{
    if (parname && (std::string(parname) == "draw")) {
        if (par("draw")) {
            showAll();
        }
        else {
            hideAll();
        }
    }
}

/**
 * adds Annotations from an XML document; example below.
 *
 * ```
 * <annotations>
 *   <line color="#F00" shape="16,0 8,13.8564" />
 *   <poly color="#0F0" shape="16,64 8,77.8564 -8,77.8564 -16,64 -8,50.1436 8,50.1436" />
 * </annotations>
 * ```
 */
void AnnotationManager::addFromXml(cXMLElement* xml)
{
    std::string rootTag = xml->getTagName();
    ASSERT(rootTag == "annotations");

    cXMLElementList list = xml->getChildren();
    for (cXMLElementList::const_iterator i = list.begin(); i != list.end(); ++i) {
        cXMLElement* e = *i;

        std::string tag = e->getTagName();

        if (tag == "point") {
            ASSERT(e->getAttribute("text"));
            std::string text = e->getAttribute("text");
            ASSERT(e->getAttribute("color"));
            std::string color = e->getAttribute("color");
            ASSERT(e->getAttribute("shape"));
            std::string shape = e->getAttribute("shape");
            std::vector<std::string> points = cStringTokenizer(shape.c_str(), " ").asVector();
            ASSERT(points.size() == 2);

            std::vector<double> p1a = cStringTokenizer(points[0].c_str(), ",").asDoubleVector();
            ASSERT(p1a.size() == 2);

            drawPoint(Coord(p1a[0], p1a[1]), color, text);
        }
        else if (tag == "line") {
            ASSERT(e->getAttribute("color"));
            std::string color = e->getAttribute("color");
            ASSERT(e->getAttribute("shape"));
            std::string shape = e->getAttribute("shape");
            std::vector<std::string> points = cStringTokenizer(shape.c_str(), " ").asVector();
            ASSERT(points.size() == 2);

            std::vector<double> p1a = cStringTokenizer(points[0].c_str(), ",").asDoubleVector();
            ASSERT(p1a.size() == 2);
            std::vector<double> p2a = cStringTokenizer(points[1].c_str(), ",").asDoubleVector();
            ASSERT(p2a.size() == 2);

            drawLine(Coord(p1a[0], p1a[1]), Coord(p2a[0], p2a[1]), color);
        }
        else if (tag == "poly") {
            ASSERT(e->getAttribute("color"));
            std::string color = e->getAttribute("color");
            ASSERT(e->getAttribute("shape"));
            std::string shape = e->getAttribute("shape");
            std::vector<std::string> points = cStringTokenizer(shape.c_str(), " ").asVector();
            ASSERT(points.size() >= 2);

            std::vector<Coord> coords;
            for (std::vector<std::string>::const_iterator i = points.begin(); i != points.end(); ++i) {
                std::vector<double> pa = cStringTokenizer(i->c_str(), ",").asDoubleVector();
                ASSERT(pa.size() == 2);
                coords.push_back(Coord(pa[0], pa[1]));
            }

            drawPolygon(coords, color);
        }
        else {
            throw cRuntimeError("while reading annotations xml: expected 'line' or 'poly', but got '%s'", tag.c_str());
        }
    }
}

AnnotationManager::Group* AnnotationManager::createGroup(std::string title)
{
    Group* group = new Group(title);
    groups.push_back(group);

    return group;
}

AnnotationManager::Point* AnnotationManager::drawPoint(Coord p, std::string color, std::string text, Group* group)
{
    Point* o = new Point(p, color, text);
    o->group = group;

    annotations.push_back(o);

    if (par("draw")) show(o);

    return o;
}

AnnotationManager::Line* AnnotationManager::drawLine(Coord p1, Coord p2, std::string color, Group* group)
{
    Line* l = new Line(p1, p2, color);
    l->group = group;

    annotations.push_back(l);

    if (par("draw")) show(l);

    return l;
}

AnnotationManager::Polygon* AnnotationManager::drawPolygon(std::list<Coord> coords, std::string color, Group* group)
{
    Polygon* p = new Polygon(coords, color);
    p->group = group;

    annotations.push_back(p);

    if (par("draw")) show(p);

    return p;
}

AnnotationManager::Polygon* AnnotationManager::drawPolygon(std::vector<Coord> coords, std::string color, Group* group)
{
    return drawPolygon(std::list<Coord>(coords.begin(), coords.end()), color, group);
}

void AnnotationManager::drawBubble(Coord p1, std::string text)
{
    std::string pxOld = getDisplayString().getTagArg("p", 0);
    std::string pyOld = getDisplayString().getTagArg("p", 1);

    std::string px;
    {
        std::stringstream ss;
        ss << p1.x;
        px = ss.str();
    }
    std::string py;
    {
        std::stringstream ss;
        ss << p1.x;
        py = ss.str();
    }

    getDisplayString().setTagArg("p", 0, px.c_str());
    getDisplayString().setTagArg("p", 1, py.c_str());

    bubble(text.c_str());

    getDisplayString().setTagArg("p", 0, pxOld.c_str());
    getDisplayString().setTagArg("p", 1, pyOld.c_str());
}

void AnnotationManager::erase(const Annotation* annotation)
{
    hide(annotation);
    annotations.remove(const_cast<Annotation*>(annotation));
    delete annotation;
}

void AnnotationManager::eraseAll(Group* group)
{
    for (Annotations::iterator i = annotations.begin(); i != annotations.end();) {
        if ((!group) || ((*i)->group == group)) {
            erase(*i++);
        }
        else {
            i++;
        }
    }
}

void AnnotationManager::scheduleErase(simtime_t deltaT, Annotation* annotation)
{
    Enter_Method_Silent();

    cMessage* evt = new cMessage("erase", EVT_SCHEDULED_ERASE);
    evt->setContextPointer(annotation);

    scheduleAt(simTime() + deltaT, evt);

    scheduledEraseEvts.push_back(evt);
}

void AnnotationManager::show(const Annotation* annotation)
{
    if (annotation->figure) return;

    if (const Point* o = dynamic_cast<const Point*>(annotation)) {

        if (hasGUI()) {
            // no corresponding TkEnv representation
        }

        TraCIScenarioManager* traci = TraCIScenarioManagerAccess().get();
        if (traci && traci->isConnected()) {
            std::stringstream nameBuilder;
            nameBuilder << o->text << " " << getEnvir()->getUniqueNumber();
            traci->getCommandInterface()->addPoi(nameBuilder.str(), "Annotation", TraCIColor::fromTkColor(o->color), 6, o->pos);
            annotation->traciPoiIds.push_back(nameBuilder.str());
        }
    }
    else if (const Line* l = dynamic_cast<const Line*>(annotation)) {

        if (hasGUI()) {
            cLineFigure* figure = new cLineFigure();
            figure->setStart(cFigure::Point(l->p1.x, l->p1.y));
            figure->setEnd(cFigure::Point(l->p2.x, l->p2.y));
            figure->setLineColor(cFigure::Color(l->color.c_str()));
            annotation->figure = figure;
            annotationLayer->addFigure(annotation->figure);
        }

        TraCIScenarioManager* traci = TraCIScenarioManagerAccess().get();
        if (traci && traci->isConnected()) {
            std::list<Coord> coords;
            coords.push_back(l->p1);
            coords.push_back(l->p2);
            std::stringstream nameBuilder;
            nameBuilder << "Annotation" << getEnvir()->getUniqueNumber();
            traci->getCommandInterface()->addPolygon(nameBuilder.str(), "Annotation", TraCIColor::fromTkColor(l->color), false, 5, coords);
            annotation->traciLineIds.push_back(nameBuilder.str());
        }
    }
    else if (const Polygon* p = dynamic_cast<const Polygon*>(annotation)) {

        ASSERT(p->coords.size() >= 2);

        if (hasGUI()) {
            cPolygonFigure* figure = new cPolygonFigure();
            std::vector<cFigure::Point> points;
            for (std::list<Coord>::const_iterator i = p->coords.begin(); i != p->coords.end(); ++i) {
                points.push_back(cFigure::Point(i->x, i->y));
            }
            figure->setPoints(points);
            figure->setLineColor(cFigure::Color(p->color.c_str()));
            figure->setFilled(false);
            annotation->figure = figure;
            annotationLayer->addFigure(annotation->figure);
        }

        TraCIScenarioManager* traci = TraCIScenarioManagerAccess().get();
        if (traci && traci->isConnected()) {
            std::stringstream nameBuilder;
            nameBuilder << "Annotation" << getEnvir()->getUniqueNumber();
            traci->getCommandInterface()->addPolygon(nameBuilder.str(), "Annotation", TraCIColor::fromTkColor(p->color), false, 4, p->coords);
            annotation->traciPolygonsIds.push_back(nameBuilder.str());
        }
    }
    else {
        throw cRuntimeError("unknown Annotation type");
    }
}

void AnnotationManager::hide(const Annotation* annotation)
{
    if (annotation->figure) {
        delete annotationLayer->removeFigure(annotation->figure);
        annotation->figure = nullptr;
    }

    TraCIScenarioManager* traci = TraCIScenarioManagerAccess().get();
    if (traci && traci->isConnected()) {
        for (std::list<std::string>::const_iterator i = annotation->traciPolygonsIds.begin(); i != annotation->traciPolygonsIds.end(); ++i) {
            std::string id = *i;
            traci->getCommandInterface()->polygon(id).remove(3);
        }
        annotation->traciPolygonsIds.clear();
        for (std::list<std::string>::const_iterator i = annotation->traciLineIds.begin(); i != annotation->traciLineIds.end(); ++i) {
            std::string id = *i;
            traci->getCommandInterface()->polygon(id).remove(4);
        }
        annotation->traciLineIds.clear();
        for (std::list<std::string>::const_iterator i = annotation->traciPoiIds.begin(); i != annotation->traciPoiIds.end(); ++i) {
            std::string id = *i;
            traci->getCommandInterface()->poi(id).remove(5);
        }
        annotation->traciPoiIds.clear();
    }
}

void AnnotationManager::showAll(Group* group)
{
    for (Annotations::const_iterator i = annotations.begin(); i != annotations.end(); ++i) {
        if ((!group) || ((*i)->group == group)) show(*i);
    }
}

void AnnotationManager::hideAll(Group* group)
{
    for (Annotations::const_iterator i = annotations.begin(); i != annotations.end(); ++i) {
        if ((!group) || ((*i)->group == group)) hide(*i);
    }
}
