//
// Copyright (C) 2007 Technische Universitaet Berlin (TUB), Germany, Telecommunication Networks Group
// Copyright (C) 2007 Technische Universiteit Delft (TUD), Netherlands
// Copyright (C) 2007 Universitaet Paderborn (UPB), Germany
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

#include "veins/base/connectionManager/BaseConnectionManager.h"

#include "veins/base/connectionManager/NicEntryDebug.h"
#include "veins/base/connectionManager/NicEntryDirect.h"
#include "veins/base/modules/BaseWorldUtility.h"
#include "veins/base/utils/FindModule.h"

using namespace veins;

namespace {
/**
 * On a torus the end and the begin of the axes are connected so you
 * get a circle. On a circle the distance between two points can't be greater
 * than half of the circumference.
 * If the normal distance between two points on one axis is bigger than
 * half of the size there must be a "shorter way" over the border on this axis
 */
static double dist(double coord1, double coord2, double size)
{
    double difference = fabs(coord1 - coord2);
    if (difference == 0)
        // NOTE: event if size is zero
        return 0;
    else {
        ASSERT(size != 0);
        double dist = FWMath::modulo(difference, size);
        return std::min(dist, size - dist);
    }
}

double sqrTorusDist(Coord c, Coord b, Coord size)
{
    double xDist = dist(c.x, b.x, size.x);
    double yDist = dist(c.y, b.y, size.y);
    double zDist = dist(c.z, b.z, size.z);
    return xDist * xDist + yDist * yDist + zDist * zDist;
}
} // namespace

void BaseConnectionManager::initialize(int stage)
{
    // BaseModule::initialize(stage);

    if (stage == 0) {
        drawMIR = hasPar("drawMaxIntfDist") ? par("drawMaxIntfDist").boolValue() : false;

        EV_TRACE << "initializing BaseConnectionManager\n";

        BaseWorldUtility* world = FindModule<BaseWorldUtility*>::findGlobalModule();

        ASSERT(world != nullptr);

        playgroundSize = world->getPgs();
        useTorus = world->useTorus();

        if (hasPar("sendDirect"))
            sendDirect = par("sendDirect").boolValue();
        else
            sendDirect = false;

        maxInterferenceDistance = calcInterfDist();
        maxDistSquared = maxInterferenceDistance * maxInterferenceDistance;

        // ----initialize node grid-----
        // step 1 - calculate dimension of grid
        // one cell should have at least the size of maxInterferenceDistance
        // but also should divide the playground in equal parts
        Coord dim((*playgroundSize) / maxInterferenceDistance);
        gridDim = GridCoord(dim);

        // A grid smaller or equal to 3x3 would mean that every cell has every
        // other cell as direct neighbor (if our playground is a torus, even if
        // not the most of the cells are direct neighbors of each other. So we
        // reduce the grid size to 1x1.
        if ((gridDim.x <= 3) && (gridDim.y <= 3) && (gridDim.z <= 3)) {
            gridDim.x = 1;
            gridDim.y = 1;
            gridDim.z = 1;
        }
        else {
            gridDim.x = std::max(1, gridDim.x);
            gridDim.y = std::max(1, gridDim.y);
            gridDim.z = std::max(1, gridDim.z);
        }

        // step 2 - initialize the matrix which represents our grid
        NicEntries entries;
        RowVector row;
        NicMatrix matrix;

        for (int i = 0; i < gridDim.z; ++i) {
            row.push_back(entries); // copy empty NicEntries to RowVector
        }
        for (int i = 0; i < gridDim.y; ++i) { // fill the ColVector with copies of
            matrix.push_back(row); // the RowVector.
        }
        for (int i = 0; i < gridDim.x; ++i) { // fill the grid with copies of
            nicGrid.push_back(matrix); // the matrix.
        }
        EV_TRACE << " using " << gridDim.x << "x" << gridDim.y << "x" << gridDim.z << " grid" << endl;

        // step 3 -    calculate the factor which maps the coordinate of a node
        //            to the grid cell
        // if we use a 1x1 grid every coordinate is mapped to (0,0, 0)
        findDistance = Coord(std::max(playgroundSize->x, maxInterferenceDistance), std::max(playgroundSize->y, maxInterferenceDistance), std::max(playgroundSize->z, maxInterferenceDistance));
        // otherwise we divide the playground into cells of size of the maximum
        // interference distance
        if (gridDim.x != 1) findDistance.x = playgroundSize->x / gridDim.x;
        if (gridDim.y != 1) findDistance.y = playgroundSize->y / gridDim.y;
        if (gridDim.z != 1) findDistance.z = playgroundSize->z / gridDim.z;

        // since the upper playground borders (at pg-size) are part of the
        // playground we have to assure that they are mapped to a valid
        // (the last) grid cell we do this by increasing the find distance
        // by a small value.
        // This also assures that findDistance is never zero.
        const auto epsilon = 0.001;
        findDistance += Coord(epsilon, epsilon, epsilon);

        // findDistance (equals cell size) has to be greater or equal
        // maxInt-distance
        ASSERT(findDistance.x >= maxInterferenceDistance);
        ASSERT(findDistance.y >= maxInterferenceDistance);
        ASSERT(findDistance.z >= maxInterferenceDistance);

        // playGroundSize has to be part of the playGround
        ASSERT(GridCoord(*playgroundSize, findDistance).x == gridDim.x - 1);
        ASSERT(GridCoord(*playgroundSize, findDistance).y == gridDim.y - 1);
        ASSERT(GridCoord(*playgroundSize, findDistance).z == gridDim.z - 1);
        EV_TRACE << "findDistance is " << findDistance.info() << endl;
    }
    else if (stage == 1) {
    }
}

BaseConnectionManager::GridCoord BaseConnectionManager::getCellForCoordinate(const Coord& c)
{
    return GridCoord(c, findDistance);
}

void BaseConnectionManager::updateConnections(int nicID, Coord oldPos, Coord newPos)
{
    GridCoord oldCell = getCellForCoordinate(oldPos);
    GridCoord newCell = getCellForCoordinate(newPos);

    checkGrid(oldCell, newCell, nicID);
}

BaseConnectionManager::NicEntries& BaseConnectionManager::getCellEntries(BaseConnectionManager::GridCoord& cell)
{
    return nicGrid[cell.x][cell.y][cell.z];
}

void BaseConnectionManager::registerNicExt(int nicID)
{
    NicEntries::mapped_type nicEntry = nics[nicID];

    GridCoord cell = getCellForCoordinate(nicEntry->pos);

    EV_TRACE << " registering (ext) nic at loc " << cell.info() << std::endl;

    // add to matrix
    NicEntries& cellEntries = getCellEntries(cell);
    cellEntries[nicID] = nicEntry;
}

void BaseConnectionManager::checkGrid(BaseConnectionManager::GridCoord& oldCell, BaseConnectionManager::GridCoord& newCell, int id)

{

    // structure to find union of grid squares
    CoordSet gridUnion(74);

    // find nic at old position
    NicEntries& oldCellEntries = getCellEntries(oldCell);
    NicEntries::iterator it = oldCellEntries.find(id);
    NicEntries::mapped_type nic = it->second;

    // move nic to a new position in matrix
    if (oldCell != newCell) {
        oldCellEntries.erase(it);
        getCellEntries(newCell)[id] = nic;
    }

    if ((gridDim.x == 1) && (gridDim.y == 1) && (gridDim.z == 1)) {
        gridUnion.add(oldCell);
    }
    else {
        // add grid around oldPos
        fillUnionWithNeighbors(gridUnion, oldCell);

        if (oldCell != newCell) {
            // add grid around newPos
            fillUnionWithNeighbors(gridUnion, newCell);
        }
    }

    GridCoord* c = gridUnion.next();
    while (c != nullptr) {
        EV_TRACE << "Update cons in [" << c->info() << "]" << endl;
        updateNicConnections(getCellEntries(*c), nic);
        c = gridUnion.next();
    }
}

int BaseConnectionManager::wrapIfTorus(int value, int max)
{
    if (value < 0) {
        if (useTorus) {
            return max + value;
        }
        else {
            return -1;
        }
    }
    else if (value >= max) {
        if (useTorus) {
            return value - max;
        }
        else {
            return -1;
        }
    }
    else {
        return value;
    }
}

void BaseConnectionManager::fillUnionWithNeighbors(CoordSet& gridUnion, GridCoord cell)
{
    for (int iz = (int) cell.z - 1; iz <= (int) cell.z + 1; iz++) {
        int cz = wrapIfTorus(iz, gridDim.z);
        if (cz == -1) {
            continue;
        }
        for (int ix = (int) cell.x - 1; ix <= (int) cell.x + 1; ix++) {
            int cx = wrapIfTorus(ix, gridDim.x);
            if (cx == -1) {
                continue;
            }
            for (int iy = (int) cell.y - 1; iy <= (int) cell.y + 1; iy++) {
                int cy = wrapIfTorus(iy, gridDim.y);
                if (cy != -1) {
                    gridUnion.add(GridCoord(cx, cy, cz));
                }
            }
        }
    }
}

bool BaseConnectionManager::isInRange(BaseConnectionManager::NicEntries::mapped_type pFromNic, BaseConnectionManager::NicEntries::mapped_type pToNic)
{
    double dDistance = 0.0;

    if (useTorus) {
        dDistance = sqrTorusDist(pFromNic->pos, pToNic->pos, *playgroundSize);
    }
    else {
        dDistance = pFromNic->pos.sqrdist(pToNic->pos);
    }
    return (dDistance <= maxDistSquared);
}

void BaseConnectionManager::updateNicConnections(NicEntries& nmap, BaseConnectionManager::NicEntries::mapped_type nic)
{
    int id = nic->nicId;

    for (NicEntries::iterator i = nmap.begin(); i != nmap.end(); ++i) {
        NicEntries::mapped_type nic_i = i->second;

        // no recursive connections
        if (nic_i->nicId == id) continue;

        bool inRange = isInRange(nic, nic_i);
        bool connected = nic->isConnected(nic_i);

        if (inRange && !connected) {
            // nodes within communication range: connect
            // nodes within communication range && not yet connected
            EV_TRACE << "nic #" << id << " and #" << nic_i->nicId << " are in range" << endl;
            nic->connectTo(nic_i);
            nic_i->connectTo(nic);
        }
        else if (!inRange && connected) {
            // out of range: disconnect
            // out of range, and still connected
            EV_TRACE << "nic #" << id << " and #" << nic_i->nicId << " are NOT in range" << endl;
            nic->disconnectFrom(nic_i);
            nic_i->disconnectFrom(nic);
        }
    }
}

bool BaseConnectionManager::registerNic(cModule* nic, ChannelAccess* chAccess, Coord nicPos, Heading heading)
{
    ASSERT(nic != nullptr);

    int nicID = nic->getId();
    EV_TRACE << " registering nic #" << nicID << endl;

    // create new NicEntry
    NicEntries::mapped_type nicEntry;

    if (sendDirect)
        nicEntry = new NicEntryDirect(this);
    else
        nicEntry = new NicEntryDebug(this);

    // fill nicEntry
    nicEntry->nicPtr = nic;
    nicEntry->nicId = nicID;
    nicEntry->hostId = nic->getParentModule()->getId();
    nicEntry->pos = nicPos;
    nicEntry->heading = heading;
    nicEntry->chAccess = chAccess;

    // add to map
    nics[nicID] = nicEntry;

    registerNicExt(nicID);

    updateConnections(nicID, nicPos, nicPos);

    if (drawMIR) {
        nic->getParentModule()->getDisplayString().setTagArg("r", 0, maxInterferenceDistance);
    }

    return sendDirect;
}

bool BaseConnectionManager::unregisterNic(cModule* nicModule)
{
    ASSERT(nicModule != nullptr);

    // find nicEntry
    int nicID = nicModule->getId();
    EV_TRACE << " unregistering nic #" << nicID << endl;

    // we assume that the module was previously registered with this CM
    // TODO: maybe change this to an omnet-error instead of an assertion
    ASSERT(nics.find(nicID) != nics.end());
    NicEntries::mapped_type nicEntry = nics[nicID];

    // get all affected grid squares
    CoordSet gridUnion(74);
    GridCoord cell = getCellForCoordinate(nicEntry->pos);
    if ((gridDim.x == 1) && (gridDim.y == 1) && (gridDim.z == 1)) {
        gridUnion.add(cell);
    }
    else {
        fillUnionWithNeighbors(gridUnion, cell);
    }

    // disconnect from all NICs in these grid squares
    GridCoord* c = gridUnion.next();
    while (c != nullptr) {
        EV_TRACE << "Update cons in [" << c->info() << "]" << endl;
        NicEntries& nmap = getCellEntries(*c);
        for (NicEntries::iterator i = nmap.begin(); i != nmap.end(); ++i) {
            NicEntries::mapped_type other = i->second;
            if (other == nicEntry) continue;
            if (!other->isConnected(nicEntry)) continue;
            other->disconnectFrom(nicEntry);
            nicEntry->disconnectFrom(other);
        }
        c = gridUnion.next();
    }

    // erase from grid
    NicEntries& cellEntries = getCellEntries(cell);
    cellEntries.erase(nicID);

    // erase from list of known nics
    nics.erase(nicID);

    delete nicEntry;

    return true;
}

void BaseConnectionManager::updateNicPos(int nicID, Coord newPos, Heading heading)
{
    NicEntries::iterator ItNic = nics.find(nicID);
    if (ItNic == nics.end()) throw cRuntimeError("No nic with this ID (%d) is registered with this ConnectionManager.", nicID);

    Coord oldPos = ItNic->second->pos;
    ItNic->second->pos = newPos;
    ItNic->second->heading = heading;

    updateConnections(nicID, oldPos, newPos);
}

const NicEntry::GateList& BaseConnectionManager::getGateList(int nicID) const
{
    NicEntries::const_iterator ItNic = nics.find(nicID);
    if (ItNic == nics.end()) throw cRuntimeError("No nic with this ID (%d) is registered with this ConnectionManager.", nicID);

    return ItNic->second->getGateList();
}

const cGate* BaseConnectionManager::getOutGateTo(const NicEntry* nic, const NicEntry* targetNic) const
{
    NicEntries::const_iterator ItNic = nics.find(nic->nicId);
    if (ItNic == nics.end()) throw cRuntimeError("No nic with this ID (%d) is registered with this ConnectionManager.", nic->nicId);

    return ItNic->second->getOutGateTo(targetNic);
}

BaseConnectionManager::~BaseConnectionManager()
{
    for (NicEntries::iterator ne = nics.begin(); ne != nics.end(); ne++) {
        delete ne->second;
    }
}
