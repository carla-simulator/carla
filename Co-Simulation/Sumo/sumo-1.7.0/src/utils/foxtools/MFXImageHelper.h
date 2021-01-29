/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2005-2020 German Aerospace Center (DLR) and others.
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
/// @file    MFXImageHelper.h
/// @author  Daniel Krajzewicz
/// @date    2005-05-04
///
// static helpers for loading and saving images in various formats
/****************************************************************************/
#ifndef MFXImageHelper_h
#define MFXImageHelper_h
#include <config.h>

#include <string>
#include <vector>
#include <fx.h>
#include <utils/common/UtilExceptions.h>

class MFXImageHelper {
public:
    static void checkSupported(FXString ext);
    static FXImage* loadImage(FXApp* a, const std::string& file);
    static FXbool scalePower2(FXImage* image, int maxSize = (2 << 29));
    static FXbool saveImage(const std::string& file,
                            int width, int height, FXColor* data);


};


#endif
