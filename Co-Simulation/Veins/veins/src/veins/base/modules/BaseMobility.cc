//
// Copyright (C) 2004 Telecommunication Networks Group (TKN) at Technische Universitaet Berlin, Germany.
// Copyright (C) 2005 Andras Varga
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

// author:      Daniel Willkomm, Andras Varga
// part of:     framework implementation developed by tkn

#include "veins/base/modules/BaseMobility.h"

#include <sstream>

#include "veins/base/utils/Coord.h"
#include "veins/base/messages/BorderMsg_m.h"
#include "veins/base/utils/FindModule.h"
#include "veins/base/modules/BaseWorldUtility.h"

using namespace veins;

Define_Module(veins::BaseMobility);

const simsignal_t BaseMobility::mobilityStateChangedSignal = registerSignal("org_car2x_veins_base_modules_mobilityStateChanged");

namespace {
bool isInBoundary(Coord c, Coord lowerBound, Coord upperBound)
{
    return lowerBound.x <= c.x && c.x <= upperBound.x && lowerBound.y <= c.y && c.y <= upperBound.y && lowerBound.z <= c.z && c.z <= upperBound.z;
}

} // namespace

BaseMobility::BaseMobility()
    : BatteryAccess()
    , move()
    , playgroundScaleX(1)
    , playgroundScaleY(1)
    , origDisplayWidth(0)
    , origDisplayHeight(0)
    , origIconSize(0)
{
}

BaseMobility::BaseMobility(unsigned stacksize)
    : BatteryAccess(stacksize)
    , move()
    , playgroundScaleX(1)
    , playgroundScaleY(1)
    , origDisplayWidth(0)
    , origDisplayHeight(0)
    , origIconSize(0)
{
}

void BaseMobility::initialize(int stage)
{
    BaseModule::initialize(stage);
    if (stage == 0) {
        EV_TRACE << "initializing BaseMobility stage " << stage << endl;

        hasPar("scaleNodeByDepth") ? scaleNodeByDepth = par("scaleNodeByDepth").boolValue() : scaleNodeByDepth = true;

        // get utility pointers (world and host)
        world = FindModule<BaseWorldUtility*>::findGlobalModule();
        if (world == nullptr) throw cRuntimeError("Could not find BaseWorldUtility module");

        EV_TRACE << "initializing BaseUtility stage " << stage << endl; // for node position

        if (hasPar("updateInterval")) {
            updateInterval = par("updateInterval");
        }
        else {
            updateInterval = 0;
        }

        // initialize Move parameter
        bool use2D = world->use2D();

        // initalize position with random values
        Coord pos = world->getRandomPosition();

        // read coordinates from parameters if available
        double x = hasPar("x") ? par("x").doubleValue() : -1;
        double y = hasPar("y") ? par("y").doubleValue() : -1;
        double z = hasPar("z") ? par("z").doubleValue() : -1;

        // set position with values from parameters if available
        if (x > -1) pos.x = x;
        if (y > -1) pos.y = y;
        if (!use2D && z > -1) pos.z = z;

        if (!hasPar("xOrientation") || !hasPar("yOrientation")) {
            throw cRuntimeError("Orientation coordinates in x and y direction have to specified (necessary for antenna gain calculation)");
        }
        else {
            double zOrient = hasPar("zOrientation") ? par("zOrientation").doubleValue() : 0;
            Coord orient(par("xOrientation").doubleValue(), par("yOrientation").doubleValue(), zOrient);
            move.setOrientationByVector(orient);
        }

        // set start-position and start-time (i.e. current simulation-time) of the Move
        move.setStart(pos);
        EV_TRACE << "start pos: " << move.getStartPos().info() << endl;

        // check whether position is within the playground
        if (!isInBoundary(move.getStartPos(), Coord::ZERO, *world->getPgs())) {
            throw cRuntimeError("node position specified in omnetpp.ini exceeds playgroundsize");
        }

        // set speed and direction of the Move
        move.setSpeed(0);
        move.setDirectionByVector(Coord::ZERO);
    }
    else if (stage == 1) {
        EV_TRACE << "initializing BaseMobility stage " << stage << endl;

        // get playground scaling
        if (world->getParentModule() != nullptr) {
            const cDisplayString& dispWorldOwner = world->getParentModule()->getDisplayString();

            if (dispWorldOwner.containsTag("bgb")) {
                double origPGWidth = 0.0;
                double origPGHeight = 0.0;
                // normally this should be equal to playground size
                std::istringstream(dispWorldOwner.getTagArg("bgb", 0)) >> origPGWidth;
                std::istringstream(dispWorldOwner.getTagArg("bgb", 1)) >> origPGHeight;

                // bgb of zero means size isn't set manually
                if (origPGWidth > 0) {
                    playgroundScaleX = origPGWidth / playgroundSizeX();
                }
                if (origPGHeight > 0) {
                    playgroundScaleY = origPGHeight / playgroundSizeY();
                }
            }
        }

        // get original display of host
        cDisplayString& disp = findHost()->getDisplayString();

        // get host width and height
        if (disp.containsTag("b")) {
            std::istringstream(disp.getTagArg("b", 0)) >> origDisplayWidth;
            std::istringstream(disp.getTagArg("b", 1)) >> origDisplayHeight;
        }
        // get hosts icon size
        if (disp.containsTag("i")) {
            // choose a appropriate icon size (only if a icon is specified)
            origIconSize = iconSizeTagToSize(disp.getTagArg("is", 0));
        }

        // print new host position on the screen and update bb info
        updatePosition();

        if (move.getSpeed() > 0 && updateInterval > 0) {
            EV_TRACE << "Host is moving, speed=" << move.getSpeed() << " updateInterval=" << updateInterval << endl;
            moveMsg = new cMessage("move", MOVE_HOST);
            // host moves the first time after some random delay to avoid synchronized movements
            scheduleAt(simTime() + uniform(0, updateInterval), moveMsg);
        }
    }
}

int BaseMobility::iconSizeTagToSize(const char* tag)
{
    if (strcmp(tag, "vs") == 0) {
        return 16;
    }
    else if (strcmp(tag, "s") == 0) {
        return 24;
    }
    else if (strcmp(tag, "n") == 0 || strcmp(tag, "") == 0) {
        return 40;
    }
    else if (strcmp(tag, "l") == 0) {
        return 60;
    }
    else if (strcmp(tag, "vl") == 0) {
        return 100;
    }

    return -1;
}

const char* BaseMobility::iconSizeToTag(double size)
{
    // returns the biggest icon smaller than the passed size (except sizes
    // smaller than the smallest icon
    if (size < 24) {
        return "vs";
    }
    else if (size < 40) {
        return "s";
    }
    else if (size < 60) {
        return "n";
    }
    else if (size < 100) {
        return "l";
    }
    else {
        return "vl";
    }
}

void BaseMobility::handleMessage(cMessage* msg)
{
    if (!msg->isSelfMessage()) throw cRuntimeError("mobility modules can only receive self messages");

    if (msg->getKind() == MOVE_TO_BORDER) {
        handleBorderMsg(msg);
    }
    else {
        handleSelfMsg(msg);
    }
}

void BaseMobility::handleSelfMsg(cMessage* msg)
{
    makeMove();
    updatePosition();

    if (!moveMsg->isScheduled() && move.getSpeed() > 0) {
        scheduleAt(simTime() + updateInterval, msg);
    }
    else {
        delete msg;
        moveMsg = nullptr;
    }
}

void BaseMobility::handleBorderMsg(cMessage* msg)
{
    EV_TRACE << "start MOVE_TO_BORDER:" << move.info() << endl;

    BorderMsg* bMsg = static_cast<BorderMsg*>(msg);

    switch (bMsg->getPolicy()) {
    case REFLECT:
        move.setStart(bMsg->getStartPos());
        move.setDirectionByVector(bMsg->getDirection());
        break;

    case WRAP:
        move.setStart(bMsg->getStartPos());
        break;

    case PLACERANDOMLY:
        move.setStart(bMsg->getStartPos());
        EV_TRACE << "new random position: " << move.getStartPos().info() << endl;
        break;

    case RAISEERROR:
        throw cRuntimeError("node moved outside the playground");
        break;

    default:
        throw cRuntimeError("Unknown BorderPolicy!");
        break;
    }

    fixIfHostGetsOutside();

    updatePosition();

    delete bMsg;

    EV_TRACE << "end MOVE_TO_BORDER:" << move.info() << endl;
}

void BaseMobility::updatePosition()
{
    EV_DEBUG << "updatePosition: " << move.info() << endl;

    // publish the the new move
    emit(mobilityStateChangedSignal, this);

    if (hasGUI()) {
        std::ostringstream osDisplayTag;
#ifdef __APPLE__
        const int iPrecis = 0;
#else
        const int iPrecis = 5;
#endif
        cDisplayString& disp = findHost()->getDisplayString();

        // setup output stream
        osDisplayTag << std::fixed;
        osDisplayTag.precision(iPrecis);

        if (playgroundScaleX != 1.0) {
            osDisplayTag << (move.getStartPos().x * playgroundScaleX);
        }
        else {
            osDisplayTag << (move.getStartPos().x);
        }
        disp.setTagArg("p", 0, osDisplayTag.str().data());

        osDisplayTag.str(""); // reset
        if (playgroundScaleY != 1.0) {
            osDisplayTag << (move.getStartPos().y * playgroundScaleY);
        }
        else {
            osDisplayTag << (move.getStartPos().y);
        }
        disp.setTagArg("p", 1, osDisplayTag.str().data());

        if (!world->use2D() && scaleNodeByDepth) {
            const double minScale = 0.25;
            const double maxScale = 1.0;

            // scale host dependent on their z coordinate to simulate a depth
            // effect
            // z-coordinate of zero maps to a scale of maxScale (very close)
            // z-coordinate of playground size z maps to size of minScale (far away)
            double depthScale = minScale + (maxScale - minScale) * (1.0 - move.getStartPos().z / playgroundSizeZ());

            if (origDisplayWidth > 0.0 && origDisplayHeight > 0.0) {
                osDisplayTag.str(""); // reset
                osDisplayTag << (origDisplayWidth * depthScale);
                disp.setTagArg("b", 0, osDisplayTag.str().data());

                osDisplayTag.str(""); // reset
                osDisplayTag << (origDisplayHeight * depthScale);
                disp.setTagArg("b", 1, osDisplayTag.str().data());
            }

            if (origIconSize > 0) {
                // choose a appropriate icon size (only if a icon is specified)
                disp.setTagArg("is", 0, iconSizeToTag(origIconSize * depthScale));
            }
        }
    }
}

void BaseMobility::reflectCoordinate(BorderHandling border, Coord& c)
{
    switch (border) {
    case X_SMALLER:
        c.x = (-c.x);
        break;
    case X_BIGGER:
        c.x = (2 * playgroundSizeX() - c.x);
        break;

    case Y_SMALLER:
        c.y = (-c.y);
        break;
    case Y_BIGGER:
        c.y = (2 * playgroundSizeY() - c.y);
        break;

    case Z_SMALLER:
        c.z = (-c.z);
        break;
    case Z_BIGGER:
        c.z = (2 * playgroundSizeZ() - c.z);
        break;

    case NOWHERE:
    default:
        throw cRuntimeError("wrong border handling case!");
        break;
    }
}

void BaseMobility::reflectIfOutside(BorderHandling wo, Coord& stepTarget, Coord& targetPos, Coord& step, double& angle)
{

    reflectCoordinate(wo, targetPos);
    reflectCoordinate(wo, stepTarget);

    switch (wo) {
    case X_SMALLER:
    case X_BIGGER:
        step.x = (-step.x);
        angle = 180 - angle;
        break;

    case Y_SMALLER:
    case Y_BIGGER:
        step.y = (-step.y);
        angle = -angle;
        break;

    case Z_SMALLER:
    case Z_BIGGER:
        step.z = (-step.z);
        angle = -angle;
        break;

    case NOWHERE:
    default:
        throw cRuntimeError("wrong border handling case!");
        break;
    }
}

void BaseMobility::wrapIfOutside(BorderHandling wo, Coord& stepTarget, Coord& targetPos)
{
    switch (wo) {
    case X_SMALLER:
    case X_BIGGER:
        targetPos.x = (FWMath::modulo(targetPos.x, playgroundSizeX()));
        stepTarget.x = (FWMath::modulo(stepTarget.x, playgroundSizeX()));
        break;

    case Y_SMALLER:
    case Y_BIGGER:
        targetPos.y = (FWMath::modulo(targetPos.y, playgroundSizeY()));
        stepTarget.y = (FWMath::modulo(stepTarget.y, playgroundSizeY()));
        break;

    case Z_SMALLER:
    case Z_BIGGER:
        targetPos.z = (FWMath::modulo(targetPos.z, playgroundSizeZ()));
        stepTarget.z = (FWMath::modulo(stepTarget.z, playgroundSizeZ()));
        break;

    case NOWHERE:
    default:
        throw cRuntimeError("wrong border handling case!");
        break;
    }
}

void BaseMobility::placeRandomlyIfOutside(Coord& targetPos)
{
    targetPos = world->getRandomPosition();
}

BaseMobility::BorderHandling BaseMobility::checkIfOutside(Coord targetPos, Coord& borderStep)
{
    BorderHandling outside = NOWHERE;

    // Testing x-value
    if (targetPos.x < 0) {
        borderStep.x = (-move.getStartPos().x);
        outside = X_SMALLER;
    }
    else if (targetPos.x >= playgroundSizeX()) {
        borderStep.x = (playgroundSizeX() - move.getStartPos().x);
        outside = X_BIGGER;
    }

    // Testing y-value
    if (targetPos.y < 0) {
        borderStep.y = (-move.getStartPos().y);

        if (outside == NOWHERE || fabs(borderStep.x / move.getDirection().x) > fabs(borderStep.y / move.getDirection().y)) {
            outside = Y_SMALLER;
        }
    }
    else if (targetPos.y >= playgroundSizeY()) {
        borderStep.y = (playgroundSizeY() - move.getStartPos().y);

        if (outside == NOWHERE || fabs(borderStep.x / move.getDirection().x) > fabs(borderStep.y / move.getDirection().y)) {
            outside = Y_BIGGER;
        }
    }

    // Testing z-value
    if (!world->use2D()) {
        // going to reach the lower z-border
        if (targetPos.z < 0) {
            borderStep.z = (-move.getStartPos().z);

            // no border reached so far
            if (outside == NOWHERE) {
                outside = Z_SMALLER;
            }
            // an y-border is reached earliest so far, test whether z-border
            // is reached even earlier
            else if ((outside == Y_SMALLER || outside == Y_BIGGER) && fabs(borderStep.y / move.getDirection().y) > fabs(borderStep.z / move.getDirection().z)) {
                outside = Z_SMALLER;
            }
            // an x-border is reached earliest so far, test whether z-border
            // is reached even earlier
            else if ((outside == X_SMALLER || outside == X_BIGGER) && fabs(borderStep.x / move.getDirection().x) > fabs(borderStep.z / move.getDirection().z)) {
                outside = Z_SMALLER;
            }
        }
        // going to reach the upper z-border
        else if (targetPos.z >= playgroundSizeZ()) {
            borderStep.z = (playgroundSizeZ() - move.getStartPos().z);

            // no border reached so far
            if (outside == NOWHERE) {
                outside = Z_BIGGER;
            }
            // an y-border is reached earliest so far, test whether z-border
            // is reached even earlier
            else if ((outside == Y_SMALLER || outside == Y_BIGGER) && fabs(borderStep.y / move.getDirection().y) > fabs(borderStep.z / move.getDirection().z)) {
                outside = Z_BIGGER;
            }
            // an x-border is reached earliest so far, test whether z-border
            // is reached even earlier
            else if ((outside == X_SMALLER || outside == X_BIGGER) && fabs(borderStep.x / move.getDirection().x) > fabs(borderStep.z / move.getDirection().z)) {
                outside = Z_BIGGER;
            }
        }
    }

    EV_TRACE << "checkIfOutside, outside=" << outside << " borderStep: " << borderStep.info() << endl;

    return outside;
}

void BaseMobility::goToBorder(BorderPolicy policy, BorderHandling wo, Coord& borderStep, Coord& borderStart)
{
    double factor;

    EV_TRACE << "goToBorder: startPos: " << move.getStartPos().info() << " borderStep: " << borderStep.info() << " BorderPolicy: " << policy << " BorderHandling: " << wo << endl;

    switch (wo) {
    case X_SMALLER:
        factor = borderStep.x / move.getDirection().x;
        borderStep.y = (factor * move.getDirection().y);
        if (!world->use2D()) {
            borderStep.z = (factor * move.getDirection().z); // 3D case
        }

        if (policy == WRAP) {
            borderStart.x = (playgroundSizeX());
            borderStart.y = (move.getStartPos().y + borderStep.y);
            if (!world->use2D()) {
                borderStart.z = (move.getStartPos().z + borderStep.z); // 3D case
            }
        }
        break;

    case X_BIGGER:
        factor = borderStep.x / move.getDirection().x;
        borderStep.y = (factor * move.getDirection().y);
        if (!world->use2D()) {
            borderStep.z = (factor * move.getDirection().z); // 3D case
        }

        if (policy == WRAP) {
            borderStart.x = (0);
            borderStart.y = (move.getStartPos().y + borderStep.y);
            if (!world->use2D()) {
                borderStart.z = (move.getStartPos().z + borderStep.z); // 3D case
            }
        }
        break;

    case Y_SMALLER:
        factor = borderStep.y / move.getDirection().y;
        borderStep.x = (factor * move.getDirection().x);
        if (!world->use2D()) {
            borderStep.z = (factor * move.getDirection().z); // 3D case
        }

        if (policy == WRAP) {
            borderStart.y = (playgroundSizeY());
            borderStart.x = (move.getStartPos().x + borderStep.x);
            if (!world->use2D()) {
                borderStart.z = (move.getStartPos().z + borderStep.z); // 3D case
            }
        }
        break;

    case Y_BIGGER:
        factor = borderStep.y / move.getDirection().y;
        borderStep.x = (factor * move.getDirection().x);
        if (!world->use2D()) {
            borderStep.z = (factor * move.getDirection().z); // 3D case
        }

        if (policy == WRAP) {
            borderStart.y = (0);
            borderStart.x = (move.getStartPos().x + borderStep.x);
            if (!world->use2D()) {
                borderStart.z = (move.getStartPos().z + borderStep.z); // 3D case
            }
        }
        break;

    case Z_SMALLER: // here we are definitely in 3D
        factor = borderStep.z / move.getDirection().z;
        borderStep.x = (factor * move.getDirection().x);
        borderStep.y = (factor * move.getDirection().y);

        if (policy == WRAP) {
            borderStart.z = (playgroundSizeZ());
            borderStart.x = (move.getStartPos().x + borderStep.x);
            borderStart.y = (move.getStartPos().y + borderStep.y);
        }
        break;

    case Z_BIGGER: // here we are definitely in 3D
        factor = borderStep.z / move.getDirection().z;
        borderStep.x = (factor * move.getDirection().x);
        borderStep.y = (factor * move.getDirection().y);

        if (policy == WRAP) {
            borderStart.z = (0);
            borderStart.x = (move.getStartPos().x + borderStep.x);
            borderStart.y = (move.getStartPos().y + borderStep.y);
        }
        break;

    default:
        throw cRuntimeError("invalid state in goToBorder switch!");
        break;
    }

    EV_TRACE << "goToBorder: startPos: " << move.getStartPos().info() << " borderStep: " << borderStep.info() << " borderStart: " << borderStart.info() << " factor: " << factor << endl;
}

bool BaseMobility::handleIfOutside(BorderPolicy policy, Coord& stepTarget, Coord& targetPos, Coord& step, double& angle)
{
    // where did the host leave the playground?
    BorderHandling wo;

    // step to reach the border
    Coord borderStep;

    wo = checkIfOutside(stepTarget, borderStep);

    // just return if the next step is not outside the playground
    if (wo == NOWHERE) return false;

    EV_TRACE << "handleIfOutside:stepTarget = " << stepTarget.info() << endl;

    // new start position after the host reaches the border
    Coord borderStart;
    // new direction the host has to move to
    Coord borderDirection;
    // time to reach the border
    simtime_t borderInterval;

    EV_TRACE << "old values: stepTarget: " << stepTarget.info() << " step: " << step.info() << " targetPos: " << targetPos.info() << " angle: " << angle << endl;

    // which border policy is to be followed
    switch (policy) {
    case REFLECT:
        reflectIfOutside(wo, stepTarget, targetPos, step, angle);
        break;
    case WRAP:
        wrapIfOutside(wo, stepTarget, targetPos);
        break;
    case PLACERANDOMLY:
        placeRandomlyIfOutside(targetPos);
        break;
    case RAISEERROR:
        break;
    }

    EV_TRACE << "new values: stepTarget: " << stepTarget.info() << " step: " << step.info() << " angle: " << angle << endl;

    // calculate the step to reach the border
    goToBorder(policy, wo, borderStep, borderStart);

    // calculate the time to reach the border
    borderInterval = (borderStep.length()) / move.getSpeed();

    // calculate new start position
    // NOTE: for WRAP this is done in goToBorder
    switch (policy) {
    case REFLECT: {
        borderStart = move.getStartPos() + borderStep;
        double d = stepTarget.distance(borderStart);
        borderDirection = (stepTarget - borderStart) / d;
        break;
    }
    case PLACERANDOMLY:
        borderStart = targetPos;
        stepTarget = targetPos;
        break;

    case WRAP:
    case RAISEERROR:
        break;

    default:
        throw cRuntimeError("unknown BorderPolicy");
        break;
    }

    EV_DEBUG << "border handled, borderStep: " << borderStep.info() << "borderStart: " << borderStart.info() << " stepTarget " << stepTarget.info() << endl;

    // create a border self message and schedule it appropriately
    BorderMsg* bMsg = new BorderMsg("borderMove", MOVE_TO_BORDER);
    bMsg->setPolicy(policy);
    bMsg->setStartPos(borderStart);
    bMsg->setDirection(borderDirection);

    scheduleAt(simTime() + borderInterval, bMsg);

    return true;
}
