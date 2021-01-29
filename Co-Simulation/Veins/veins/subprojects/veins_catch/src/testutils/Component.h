//
// Copyright (C) 2018 Christoph Sommer <sommer@ccs-labs.org>
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
#pragma once

#include "veins/veins.h"

#include "testutils/Simulation.h"

class DummyComponent : public cComponent {
public:
    DummyComponent(DummySimulation* ds)
    {
    }

    void callRefreshDisplay() override
    {
    }

    cProperties* getProperties() const override
    {
        return nullptr;
    }

    ComponentKind getComponentKind() const override
    {
        return KIND_OTHER;
    }

    cModule* getParentModule() const override
    {
        return nullptr;
    }

    void callInitialize() override
    {
    }

    bool callInitialize(int stage) override
    {
        return true;
    }

    void callFinish() override
    {
    }

private:
}; // end DummyComponent
