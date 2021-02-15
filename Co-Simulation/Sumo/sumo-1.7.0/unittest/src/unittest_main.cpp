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
/// @file    unittest_main.cpp
/// @author  Matthias Heppner
/// @author  Michael Behrisch
/// @date    Sept 2009
///
// main for the unit tests
/****************************************************************************/


// ===========================================================================
// included modules
// ===========================================================================
#include <config.h>

#include <iostream>
#include <gtest/gtest.h>


// ===========================================================================
// method definitions
// ===========================================================================
int main(int argc, char** argv) {
    std::cout << "Running unit tests\n";

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
