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

#pragma once

#include "veins/veins.h"

namespace veins {

/**
 * @brief Provides method templates to find omnet modules.
 *
 * @ingroup baseUtils
 * @ingroup utils
 */
template <typename T = cModule* const>
class VEINS_API FindModule {
public:
    /**
     * @brief Returns a pointer to a sub module of the passed module with
     * the type of this template.
     *
     * Returns NULL if no matching submodule could be found.
     */
    static T findSubModule(const cModule* const top)
    {
        for (cModule::SubmoduleIterator i(top); !i.end(); i++) {
            cModule* const sub = *i;
            // this allows also a return type of read only pointer: const cModule *const
            T dCastRet = dynamic_cast<T>(sub);
            if (dCastRet != NULL) return dCastRet;
            // this allows also a return type of read only pointer: const cModule *const
            T recFnd = findSubModule(sub);
            if (recFnd != NULL) return recFnd;
        }
        return NULL;
    }

    /**
     * @brief Returns a pointer to the module with the type of this
     * template.
     *
     * Returns NULL if no module of this type could be found.
     */
    static T findGlobalModule()
    {
        return findSubModule(getSimulation()->getSystemModule());
    }

    /**
     * @brief Returns a pointer to the host module of the passed module.
     *
     * Assumes that every host module is a direct sub module of the
     * simulation.
     */
    static cModule* const findHost(cModule* const m)
    {
        cModule* parent = m != nullptr ? m->getParentModule() : nullptr;
        cModule* node = m;

        // all nodes should be a sub module of the simulation which has no parent module!!!
        while (parent != nullptr && parent->getParentModule() != nullptr) {
            node = parent;
            parent = node->getParentModule();
        }
        return node;
    }
    // the constness version
    static const cModule* const findHost(const cModule* const m)
    {
        const cModule* parent = m != nullptr ? m->getParentModule() : nullptr;
        const cModule* node = m;

        // all nodes should be a sub module of the simulation which has no parent module!!!
        while (parent != nullptr && parent->getParentModule() != nullptr) {
            node = parent;
            parent = node->getParentModule();
        }
        return node;
    }
};

/**
 * @brief Finds and returns the pointer to a module of type T.
 * Uses FindModule<>::findSubModule(), FindModule<>::findHost(). See usage e.g. at ChannelAccess.
 */
template <typename T = cModule>
class VEINS_API AccessModuleWrap {
public:
    using wrapType = T;

private:
    T* pModule;

public:
    AccessModuleWrap()
        : pModule(nullptr)
    {
    }

    T* const get(cModule* const from = nullptr)
    {
        if (!pModule) {
            pModule = FindModule<T*>::findSubModule(FindModule<>::findHost(from != nullptr ? from : getSimulation()->getContextModule()));
        }
        return pModule;
    }
};

/**
 * @brief Return a vector containing pointers to all submodules of parentModule of type T
 */
template <class T>
std::vector<T*> getSubmodulesOfType(cModule* parentModule, bool recurse = false)
{
    std::vector<T*> result;
    for (cModule::SubmoduleIterator iter(parentModule); !iter.end(); iter++) {
        auto mm = dynamic_cast<T*>(*iter);
        if (mm != nullptr) result.push_back(mm);
        if (recurse) {
            for (auto m : getSubmodulesOfType<T>(*iter, recurse)) {
                result.emplace_back(m);
            }
        }
    }
    return result;
}

} // namespace veins
