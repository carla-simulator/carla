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
/// @file    NBPTPlatform.cpp
/// @author  Gregor Laemmel
/// @date    Tue, 24 Aug 2017
///
// The representation of a pt platform
/****************************************************************************/
#include "NBPTPlatform.h"
NBPTPlatform::NBPTPlatform(Position position, double d): myPos(position), myLength(d) {
}


const Position&
NBPTPlatform::getPos() const {
    return myPos;
}


void
NBPTPlatform::reshiftPosition(const double offsetX, const double offsetY) {
    myPos.add(offsetX, offsetY, 0);
}


double
NBPTPlatform::getLength() const {
    return myLength;
}
