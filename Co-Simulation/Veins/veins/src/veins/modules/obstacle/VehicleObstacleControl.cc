//
// Copyright (C) 2010-2018 Christoph Sommer <sommer@ccs-labs.org>
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

#include <sstream>
#include <map>
#include <set>

#include <limits>
#include <cmath>

#include "veins/modules/obstacle/VehicleObstacleControl.h"
#include "veins/base/modules/BaseMobility.h"
#include "veins/base/connectionManager/ChannelAccess.h"
#include "veins/base/toolbox/Signal.h"

using veins::MobileHostObstacle;
using veins::Signal;
using veins::VehicleObstacleControl;

Define_Module(veins::VehicleObstacleControl);

VehicleObstacleControl::~VehicleObstacleControl() = default;

void VehicleObstacleControl::initialize(int stage)
{
    if (stage == 1) {
        annotations = AnnotationManagerAccess().getIfExists();
        if (annotations) {
            vehicleAnnotationGroup = annotations->createGroup("vehicleObstacles");
        }
    }
}

void VehicleObstacleControl::finish()
{
}

void VehicleObstacleControl::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMsg(msg);
        return;
    }
    throw cRuntimeError("VehicleObstacleControl doesn't handle messages from other modules");
}

void VehicleObstacleControl::handleSelfMsg(cMessage* msg)
{
    throw cRuntimeError("VehicleObstacleControl doesn't handle self-messages");
}

const MobileHostObstacle* VehicleObstacleControl::add(MobileHostObstacle obstacle)
{
    auto* o = new MobileHostObstacle(obstacle);
    vehicleObstacles.push_back(o);

    return o;
}

void VehicleObstacleControl::erase(const MobileHostObstacle* obstacle)
{
    bool erasedOne = false;
    for (auto k = vehicleObstacles.begin(); k != vehicleObstacles.end();) {
        MobileHostObstacle* o = *k;
        if (o == obstacle) {
            erasedOne = true;
            k = vehicleObstacles.erase(k);
        }
        else {
            ++k;
        }
    }
    ASSERT(erasedOne);
    delete obstacle;
}

Signal VehicleObstacleControl::getVehicleAttenuationSingle(double h1, double h2, double h, double d, double d1, Signal attenuationPrototype)
{
    Signal attenuation = Signal(attenuationPrototype.getSpectrum());

    for (uint16_t i = 0; i < attenuation.getNumValues(); i++) {
        double freq = attenuation.getSpectrum().freqAt(i);
        double lambda = BaseWorldUtility::speedOfLight() / freq;
        double d2 = d - d1;
        double y = (h2 - h1) / d * d1 + h1;
        double H = h - y;
        double r1 = sqrt(lambda * d1 * d2 / d);
        double V0 = sqrt(2) * H / r1;

        if (V0 <= -0.7) {
            attenuation.at(i) = 0;
        }
        else {
            attenuation.at(i) = 6.9 + 20 * log10(sqrt(pow((V0 - 0.1), 2) + 1) + V0 - 0.1);
        }
    }

    return attenuation;
}

Signal VehicleObstacleControl::getVehicleAttenuationDZ(const std::vector<std::pair<double, double>>& dz_vec, Signal attenuationPrototype)
{

    // basic sanity check
    ASSERT(dz_vec.size() >= 2);

    // make sure the list of x coordinates is sorted
    for (size_t i = 0; i < dz_vec.size() - 1; i++) {
        ASSERT(dz_vec[i].first < dz_vec[i + 1].first);
    }

    // find "major obstacles" (MOs) between sender and receiver via rope-stretching algorithm
    /*
     *      |
     *      |         |
     *      |   :     |
     *  |   |   :  :  |    |
     * mo0 mo1       mo2  mo3
     * snd                rcv
     */
    std::vector<size_t> mo; ///< indices of MOs (this includes the sender and receiver)
    mo.push_back(0);
    for (size_t i = 0;;) {
        double max_slope = -std::numeric_limits<double>::infinity();
        size_t max_slope_index;
        bool have_max_slope_index = false;

        for (size_t j = i + 1; j < dz_vec.size(); ++j) {
            double slope = (dz_vec[j].second - dz_vec[i].second) / (dz_vec[j].first - dz_vec[i].first);

            if (slope > max_slope) {
                max_slope = slope;
                max_slope_index = j;
                have_max_slope_index = true;
            }
        }

        // Sanity check
        ASSERT(have_max_slope_index);

        if (max_slope_index >= dz_vec.size() - 1) break;

        mo.push_back(max_slope_index);

        i = max_slope_index;
    }
    mo.push_back(dz_vec.size() - 1);

    // calculate attenuation due to MOs
    Signal attenuation_mo(attenuationPrototype.getSpectrum());
    for (size_t mm = 0; mm < mo.size() - 2; ++mm) {
        size_t tx = mo[mm];
        size_t ob = mo[mm + 1];
        size_t rx = mo[mm + 2];

        double h1 = dz_vec[tx].second;
        double h2 = dz_vec[rx].second;
        double d = dz_vec[rx].first - dz_vec[tx].first;
        double d1 = dz_vec[ob].first - dz_vec[tx].first;
        double h = dz_vec[ob].second;

        Signal ad_mo = getVehicleAttenuationSingle(h1, h2, h, d, d1, attenuationPrototype);

        attenuation_mo += ad_mo;
    }

    // calculate attenuation due to "small obstacles" (i.e. the ones in-between MOs)
    Signal attenuation_so(attenuationPrototype.getSpectrum());
    for (size_t i = 0; i < mo.size() - 1; ++i) {
        size_t delta = mo[i + 1] - mo[i];

        if (delta == 1) {
            // no obstacle in-between these two MOs
        }
        else if (delta == 2) {
            // one obstacle in-between these two MOs
            size_t tx = mo[i];
            size_t ob = mo[i] + 1;
            size_t rx = mo[i + 1];

            double h1 = dz_vec[tx].second;
            double h2 = dz_vec[rx].second;
            double d = dz_vec[rx].first - dz_vec[tx].first;
            double d1 = dz_vec[ob].first - dz_vec[tx].first;
            double h = dz_vec[ob].second;

            Signal ad_mo = getVehicleAttenuationSingle(h1, h2, h, d, d1, attenuationPrototype);
            attenuation_so += ad_mo;
        }
        else {
            // multiple obstacles in-between these two MOs -- use the one closest to their line of sight
            double x1 = dz_vec[mo[i]].first;
            double y1 = dz_vec[mo[i]].second;
            double x2 = dz_vec[mo[i + 1]].first;
            double y2 = dz_vec[mo[i + 1]].second;

            double min_delta_h = std::numeric_limits<float>::infinity();
            size_t min_delta_h_index;
            bool have_min_delta_h_index = false;
            for (size_t j = mo[i] + 1; j < mo[i + 1]; ++j) {
                double h = (y2 - y1) / (x2 - x1) * (dz_vec[j].first - x1) + y1;
                double delta_h = h - dz_vec[j].second;

                if (delta_h < min_delta_h) {
                    min_delta_h = delta_h;
                    min_delta_h_index = j;
                    have_min_delta_h_index = true;
                }
            }

            // Sanity check
            ASSERT(have_min_delta_h_index);

            size_t tx = mo[i];
            size_t ob = min_delta_h_index;
            size_t rx = mo[i + 1];

            double h1 = dz_vec[tx].second;
            double h2 = dz_vec[rx].second;
            double d = dz_vec[rx].first - dz_vec[tx].first;
            double d1 = dz_vec[ob].first - dz_vec[tx].first;
            double h = dz_vec[ob].second;

            Signal ad_mo = getVehicleAttenuationSingle(h1, h2, h, d, d1, attenuationPrototype);
            attenuation_so += ad_mo;
        }
    }

    double c;
    {
        double prodS = 1;
        double sumS = 0;
        double prodSsum = 1;
        double firstS = 0;
        double lastS = 0;

        double s_old = 0;
        for (size_t jj = 0; jj < mo.size() - 1; ++jj) {
            double s = dz_vec[mo[jj + 1]].first - dz_vec[mo[jj]].first; ///< distance between two MOs

            prodS *= s;
            sumS += s;
            if (jj == 0)
                firstS = s;
            else if (jj > 0)
                prodSsum *= (s + s_old);
            if (jj == mo.size() - 2) lastS = s;
            s_old = s;
        }

        c = -10 * log10((prodS * sumS) / (prodSsum * firstS * lastS));
    }

    return attenuation_mo + attenuation_so + c;
}

std::vector<std::pair<double, double>> VehicleObstacleControl::getPotentialObstacles(const AntennaPosition& senderPos_, const AntennaPosition& receiverPos_, const Signal& s) const
{
    Enter_Method_Silent();

    auto senderPos = senderPos_.getPositionAt();
    auto receiverPos = receiverPos_.getPositionAt();

    double senderHeight = senderPos.z;
    double receiverHeight = receiverPos.z;

    ASSERT(senderHeight > 0);
    ASSERT(receiverHeight > 0);

    std::vector<std::pair<double, double>> potentialObstacles; /**< linear position of each obstructing vehicle along (senderPos--receiverPos) */

    simtime_t sStart = s.getSendingStart();

    EV << "searching candidates for transmission from " << senderPos.info() << " -> " << receiverPos.info() << " (" << senderPos.distance(receiverPos) << "meters total)" << std::endl;

    if (hasGUI() && annotations) {
        annotations->eraseAll(vehicleAnnotationGroup);
        drawVehicleObstacles(sStart);
        annotations->drawLine(senderPos, receiverPos, "blue", vehicleAnnotationGroup);
    }

    double x1 = std::min(senderPos.x, receiverPos.x);
    double x2 = std::max(senderPos.x, receiverPos.x);
    double y1 = std::min(senderPos.y, receiverPos.y);
    double y2 = std::max(senderPos.y, receiverPos.y);

    for (auto o : vehicleObstacles) {
        auto obstacleAntennaPositions = o->getInitialAntennaPositions();
        double l = o->getLength();
        double w = o->getWidth();
        double h = o->getHeight();

        auto hp_approx = o->getMobility()->getPositionAt(simTime());

        EV << "checking vehicle in proximity of " << hp_approx.info() << " with height: " << h << " width: " << w << " length: " << l << endl;

        if (!o->maybeInBounds(x1, y1, x2, y2, sStart)) {
            EV_TRACE << "bounding boxes don't overlap: ignore" << std::endl;
            continue;
        }

        // check if this is either the sender or the receiver
        bool ignoreMe = false;
        for (auto obstacleAntenna : obstacleAntennaPositions) {
            if (obstacleAntenna.isSameAntenna(senderPos_)) {
                EV_TRACE << "...this is the sender: ignore" << std::endl;
                ignoreMe = true;
            }
            if (obstacleAntenna.isSameAntenna(receiverPos_)) {
                EV_TRACE << "...this is the receiver: ignore" << std::endl;
                ignoreMe = true;
            }
        }
        if (ignoreMe) continue;

        // this is a potential obstacle
        double p1d = o->getIntersectionPoint(senderPos, receiverPos, sStart);
        double maxd = senderPos.distance(receiverPos);
        if (!std::isnan(p1d) && p1d > 0 && p1d < maxd) {
            auto it = potentialObstacles.begin();
            while (true) {
                if (it == potentialObstacles.end()) {
                    potentialObstacles.emplace_back(p1d, h);
                    break;
                }
                if (it->first == p1d) { // omit double entries
                    EV << "two obstacles at same distance " << it->first << " == " << p1d << " height: " << it->second << " =? " << h << std::endl;
                    break;
                }
                if (it->first > p1d) {
                    potentialObstacles.insert(it, std::make_pair(p1d, h));
                    break;
                }
                ++it;
            }
            EV << "\tgot obstacle in 2d-LOS, " << p1d << " meters away from sender" << std::endl;
            Coord hitPos = senderPos + (receiverPos - senderPos) / senderPos.distance(receiverPos) * p1d;
            if (hasGUI() && annotations) {
                annotations->drawLine(senderPos, hitPos, "red", vehicleAnnotationGroup);
            }
        }
    }

    return potentialObstacles;
}

void VehicleObstacleControl::drawVehicleObstacles(const simtime_t& t) const
{
    for (auto o : vehicleObstacles) {
        annotations->drawPolygon(o->getShape(t), "black", vehicleAnnotationGroup);
    }
}
