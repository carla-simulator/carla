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
/// @file    MFXMenuHeader.cpp
/// @author  Daniel Krajzewicz
/// @date    2004-07-02
///
// Header for menu commands
/****************************************************************************/
#include <config.h>

#include "MFXMenuHeader.h"

MFXMenuHeader::MFXMenuHeader(FXComposite* p, FXFont* fnt, const FXString& text, FXIcon* ic, FXObject* tgt, FXSelector sel, FXuint opts) :
    FXMenuCommand(p, text, ic, tgt, sel, opts) {
    setFont(fnt);
}


MFXMenuHeader::~MFXMenuHeader() {}


/****************************************************************************/
