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

#pragma once

#include <list>

#include "veins/veins.h"

#include "veins/base/utils/FindModule.h"
#include "veins/base/utils/Coord.h"

namespace veins {

/**
 * manages annotations on the OMNeT++ canvas.
 */
class VEINS_API AnnotationManager : public cSimpleModule {
public:
    class VEINS_API Group;

    class VEINS_API Annotation {
    public:
        Annotation()
            : group(nullptr)
            , figure(nullptr)
        {
        }
        virtual ~Annotation()
        {
        }

    protected:
        friend class AnnotationManager;

        Group* group;

        mutable cFigure* figure;

        mutable std::list<std::string> traciPoiIds;
        mutable std::list<std::string> traciLineIds;
        mutable std::list<std::string> traciPolygonsIds;
    };

    class VEINS_API Point : public Annotation {
    public:
        Point(Coord pos, std::string color, std::string text)
            : pos(pos)
            , color(color)
            , text(text)
        {
        }
        ~Point() override
        {
        }

    protected:
        friend class AnnotationManager;

        Coord pos;
        std::string color;
        std::string text;
    };

    class VEINS_API Line : public Annotation {
    public:
        Line(Coord p1, Coord p2, std::string color)
            : p1(p1)
            , p2(p2)
            , color(color)
        {
        }
        ~Line() override
        {
        }

    protected:
        friend class AnnotationManager;

        Coord p1;
        Coord p2;
        std::string color;
    };

    class VEINS_API Polygon : public Annotation {
    public:
        Polygon(std::list<Coord> coords, std::string color)
            : coords(coords)
            , color(color)
        {
        }
        ~Polygon() override
        {
        }

    protected:
        friend class AnnotationManager;

        std::list<Coord> coords;
        std::string color;
    };

    class VEINS_API Group {
    public:
        Group(std::string title)
            : title(title)
        {
        }
        virtual ~Group()
        {
        }

    protected:
        friend class AnnotationManager;

        std::string title;
    };

    ~AnnotationManager() override;
    void initialize() override;
    void finish() override;
    void handleMessage(cMessage* msg) override;
    void handleSelfMsg(cMessage* msg);
    void handleParameterChange(const char* parname) override;

    void addFromXml(cXMLElement* xml);
    Group* createGroup(std::string title = "untitled");
    Point* drawPoint(Coord p, std::string color, std::string text, Group* group = nullptr);
    Line* drawLine(Coord p1, Coord p2, std::string color, Group* group = nullptr);
    Polygon* drawPolygon(std::list<Coord> coords, std::string color, Group* group = nullptr);
    Polygon* drawPolygon(std::vector<Coord> coords, std::string color, Group* group = nullptr);
    void drawBubble(Coord p1, std::string text);
    void erase(const Annotation* annotation);
    void eraseAll(Group* group = nullptr);
    void scheduleErase(simtime_t deltaT, Annotation* annotation);

    void show(const Annotation* annotation);
    void hide(const Annotation* annotation);
    void showAll(Group* group = nullptr);
    void hideAll(Group* group = nullptr);

protected:
    using Annotations = std::list<Annotation*>;
    using Groups = std::list<Group*>;

    cXMLElement* annotationsXml; /**< annotations to add at startup */

    std::list<cMessage*> scheduledEraseEvts;

    Annotations annotations;
    Groups groups;

    cGroupFigure* annotationLayer;
};

class VEINS_API AnnotationManagerAccess {
public:
    AnnotationManager* getIfExists()
    {
        return FindModule<AnnotationManager*>::findGlobalModule();
    };
};

} // namespace veins
