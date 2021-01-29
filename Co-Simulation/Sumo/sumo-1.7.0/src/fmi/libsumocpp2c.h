/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2020-2020 German Aerospace Center (DLR) and others.
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
/// @file    libsumocpp2c.h
/// @author  Robert Hilbrich
/// @date    Mon, 15 Aug 2020
///
// Declaration of the libsumo c++ to c wrapper functions
/****************************************************************************/

#ifndef LIBSUMOCPP2C_H
#define LIBSUMOCPP2C_H


#ifdef __cplusplus
extern "C" {
#endif

void libsumo_load();
void libsumo_close();
int libsumo_vehicle_getIDCount();
void libsumo_step(double time);

#ifdef __cplusplus
}
#endif

#endif /* LIBSUMOCPP2C_H */

