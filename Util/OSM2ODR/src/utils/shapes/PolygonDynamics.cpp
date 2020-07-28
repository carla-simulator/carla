/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
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
/// @file    PolygonDynamics.cpp
/// @author  Leonhard Luecken
/// @date    Apr 2019
///
// A polygon, which holds a timeSpan for displaying dynamic properties
/****************************************************************************/


#include "PolygonDynamics.h"

#include <assert.h>
#include "utils/common/StdDefs.h"
#include "utils/common/SUMOTime.h"
#include "utils/vehicle/SUMOTrafficObject.h"


//#define DEBUG_DYNAMIC_SHAPES

PolygonDynamics::PolygonDynamics(double creationTime,
                                 SUMOPolygon* p,
                                 SUMOTrafficObject* trackedObject,
                                 const std::vector<double>& timeSpan,
                                 const std::vector<double>& alphaSpan,
                                 bool looped,
                                 bool rotate) :
    myPolygon(p),
    myCurrentTime(0),
    myLastUpdateTime(creationTime),
    animated(!timeSpan.empty()),
    looped(looped),
    tracking(trackedObject != nullptr),
    rotate(rotate),
    myTrackedObject(trackedObject),
    myTrackedObjectID(""),
    myTrackedObjectsInitialPositon(nullptr),
    myTrackedObjectsInitialAngle(-1),
    myOriginalShape(nullptr),
    myTimeSpan(nullptr),
    myAlphaSpan(nullptr),
    myVis(nullptr) {
    // Check for consistency
    if (animated) {
        myTimeSpan = std::unique_ptr<std::vector<double> >(new std::vector<double>(timeSpan));
        assert(myTimeSpan->size() >= 2);
        assert((*myTimeSpan)[0] == 0.0);
        assert(myAlphaSpan == nullptr || myAlphaSpan->size() >= 2);
#ifdef DEBUG_DYNAMIC_SHAPES
        if (myTimeSpan->size() >= 2) {
            for (unsigned int i = 1; i < myTimeSpan->size(); ++i) {
                assert((*myTimeSpan)[i - 1] <= (*myTimeSpan)[i]);
            }
        }
#endif
        myPrevTime = myTimeSpan->begin();
        myNextTime = ++myTimeSpan->begin();
    }
#ifdef DEBUG_DYNAMIC_SHAPES
    else  {
        assert(myAlphaSpan == nullptr);
    }
#endif

    myOriginalShape = std::unique_ptr<PositionVector>(new PositionVector(p->getShape()));

    if (tracking) {
        // Try initializing the tracked position (depends on whether object is already on the road)
        initTrackedPosition();
        myTrackedObjectID = myTrackedObject->getID();
    }

    if (!alphaSpan.empty()) {
        myAlphaSpan = std::unique_ptr<std::vector<double> >(new std::vector<double>(alphaSpan));
        assert(myAlphaSpan->size() >= 2);
        assert(myAlphaSpan->size() == myTimeSpan->size());
        myPrevAlpha = myAlphaSpan->begin();
        myNextAlpha = ++myAlphaSpan->begin();
    }
}

PolygonDynamics::~PolygonDynamics()
{}


SUMOTime
PolygonDynamics::update(SUMOTime t) {
#ifdef DEBUG_DYNAMIC_SHAPES
    std::cout << t << " PolygonDynamics::update() for polygon '" << myPolygon->getID() << "'" << std::endl;
#endif
    const double simtime = STEPS2TIME(t);
    const double dt = simtime - myLastUpdateTime;
    myLastUpdateTime = simtime;

    SUMOTime ret = DELTA_T;

    if (tracking) {
        if (myTrackedObjectsInitialPositon == nullptr) {
            // Tracked object hasn't entered the network, until now.
            // Continuously try to obtain its initial position
            initTrackedPosition();
        }
        if (myTrackedObjectsInitialPositon != nullptr) {
            // Initial position was initialized, relative tracking is possible
            const Position& objPos = myTrackedObject->getPosition();
            const bool onRoad = objPos != Position::INVALID;
            if (onRoad) {
#ifdef DEBUG_DYNAMIC_SHAPES
                std::cout << " Tracked object '" << myTrackedObject->getID() << "' is on the road. Tracked position=" << objPos << std::endl;
#endif
                // Update polygon's shape
                PositionVector newShape(*myOriginalShape);
                if (rotate) {
                    const double relRotation = myTrackedObject->getAngle() - myTrackedObjectsInitialAngle;
                    newShape.rotate2D(relRotation);
#ifdef DEBUG_DYNAMIC_SHAPES
                    std::cout << " Relative rotation wrt original rotation: " << relRotation << std::endl;
#endif
                }
                newShape.add(objPos);
                myPolygon->setShape(newShape);
            }
#ifdef DEBUG_DYNAMIC_SHAPES
            else {
                // tracked object is off road
                std::cout << " Tracked object '" << myTrackedObject->getID() << "' is off road." << std::endl;
            }
#endif
        }
#ifdef DEBUG_DYNAMIC_SHAPES
        else {
            // Initial position was not initialized, yet
            std::cout << " Tracked object '" << myTrackedObject->getID() << "' hasn't entered the network since tracking was started." << std::endl;
        }
#endif
    }

    if (animated) {
        // Continue animation
        myCurrentTime += dt;
        while (myCurrentTime >= *myNextTime) {
            // step forward along time lines to appropriate anchor points
            ++myPrevTime;
            ++myNextTime;
            if (myNextTime == myTimeSpan->end()) {
                // Set iterators back to point to valid positions
                --myPrevTime;
                --myNextTime;
                break;
            } else {
                // Forward corresponding iterators for property time lines
                if (myAlphaSpan != nullptr) {
                    ++myPrevAlpha;
                    ++myNextAlpha;
                }
            }
        }

        // Linear interpolation factor between previous and next time
        double theta = 1.0;
#ifdef DEBUG_DYNAMIC_SHAPES
        std::cout << " animation: dt=" << dt
                  << ", current animation time: " << myCurrentTime
                  << ", previous anchor time: " << *myPrevTime
                  << ", next anchor time: " << *myNextTime;
#endif
        if (looped) {
            const bool resetAnimation = myCurrentTime >= *myNextTime;
#ifdef DEBUG_DYNAMIC_SHAPES
            if (resetAnimation) {
                std::cout << " (resetting animation!)";
            }
#endif
            if (resetAnimation) {
                // Reset animation time line to start, if appropriate
                while (myCurrentTime >= *myNextTime) {
                    myCurrentTime -= *myNextTime;
                }
                myCurrentTime = MAX2(myCurrentTime, 0.0);
                myPrevTime = myTimeSpan->begin();
                myNextTime = ++myTimeSpan->begin();
                if (myAlphaSpan != nullptr) {
                    myPrevAlpha = myAlphaSpan->begin();
                    myNextAlpha = ++myAlphaSpan->begin();
                }
            }
        }
        if (myCurrentTime >= *myNextTime) {
            assert(!looped);
            // Reached the end of the dynamics, indicate expiration by returning zero
            // and set all properties to the final state (theta remains one)
            ret = 0;
#ifdef DEBUG_DYNAMIC_SHAPES
            std::cout << " (animation elapsed!)";
#endif
        } else {
            // Animation is still going on, schedule next update
            if (*myNextTime - *myPrevTime != 0) {
                theta = (myCurrentTime - *myPrevTime) / (*myNextTime - *myPrevTime);
            }
        }
        if (myAlphaSpan != nullptr) {
            // Interpolate values of properties
            setAlpha(*myPrevAlpha + theta * (*myNextAlpha - *myPrevAlpha));
#ifdef DEBUG_DYNAMIC_SHAPES
            std::cout << ", previous anchor alpha: " << *myPrevAlpha
                      << ", next anchor alpha: " << *myNextAlpha;
#endif
        }
#ifdef DEBUG_DYNAMIC_SHAPES
        std::cout << ", theta=" << theta << std::endl;
#endif
    }
    return ret;
}

void
PolygonDynamics::initTrackedPosition() {
    const Position& objPos = myTrackedObject->getPosition();
    if (objPos != Position::INVALID) {
        // Initialize Position of tracked object
        myTrackedObjectsInitialPositon = std::unique_ptr<Position>(new Position(objPos));
        myTrackedObjectsInitialAngle = myTrackedObject->getAngle();
        // Store original polygon shape relative to the tracked object's original position
        myOriginalShape->sub(*myTrackedObjectsInitialPositon);
#ifdef DEBUG_DYNAMIC_SHAPES
        std::cout << " Tracking object '" << myTrackedObject->getID() << "' at initial positon: " << *myTrackedObjectsInitialPositon << std::endl;
#endif
    }
}

void
PolygonDynamics::setAlpha(double alpha) {
    int a = (int) alpha;
    myPolygon->setShapeAlpha((unsigned char) a);
#ifdef DEBUG_DYNAMIC_SHAPES
    std::cout << "\n   DynamicPolygon::setAlpha() Converted alpha=" << alpha << " into myAlpha=" << a << std::endl;
#endif
}
