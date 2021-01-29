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
/// @file    NIVissimElements.h
/// @author  Daniel Krajzewicz
/// @date    Sept 2002
///
// -------------------
/****************************************************************************/
#pragma once
#include <config.h>


enum NIVissimElement {
    VE_Kennungszeile,
    VE_Startzufallszahl,
    VE_Simdauer,
    VE_Startuhrzeit,
    VE_SimRate,
    VE_Zeitschrittfaktor,
    VE_Linksverkehr,
    VE_DynUml,
    VE_Stauparameterdefinition,
    VE_Gelbverhaltendefinition,
    VE_Streckendefinition,
    VE_Verbindungsdefinition,
    VE_Richtungsentscheidungsdefinition,
    VE_Routenentscheidungsdefinition,
    VE_VWunschentscheidungsdefinition,
    VE_Langsamfahrbereichdefinition,
    VE_Zuflussdefinition,
    VE_Fahrzeugtypdefinition,
    VE_Fahrzeugklassendefinition,
    VE_Verkehrszusammensetzungsdefinition,
    VE_Geschwindigkeitsverteilungsdefinition,
    VE_Laengenverteilungsdefinition,
    VE_Zeitenverteilungsdefinition,
    VE_Baujahrverteilungsdefinition,
    VE_Laufleistungsverteilungsdefinition,
    VE_Massenverteilungsdefinition,
    VE_Leistungsverteilungsdefinition,
    VE_Maxbeschleunigungskurvedefinition,
    VE_Wunschbeschleunigungskurvedefinition,
    VE_Maxverzoegerungskurvedefinition,
    VE_Wunschverzoegerungskurvedefinition,
    VE_Querverkehrsstoerungsdefinition,
    VE_Lichtsignalanlagendefinition,
    VE_Signalgruppendefinition,
    VE_Signalgeberdefinition,
    VE_LSAKopplungdefinition,
    VE_Detektorendefinition,
    VE_Haltestellendefinition,
    VE_Liniendefinition,
    VE_Stopschilddefinition,
    VE_Messungsdefinition,
    VE_Reisezeitmessungsdefinition,
    VE_Verlustzeitmessungsdefinition,
    VE_Querschnittsmessungsdefinition,
    VE_Stauzaehlerdefinition,
    VE_Auswertungsdefinition,
    VE_Fensterdefinition,
    VE_Gefahrenwarnsystemdefinition,
    VE_Parkplatzdefinition,
    VE_Knotendefinition,
    VE_TEAPACdefinition,
    VE_Netzobjektdefinition,
    VE_Richtungspfeildefinition,
    VE_Rautedefinition,
    VE_Fahrverhaltendefinition,
    VE_Fahrtverlaufdateien,
    VE_Emission,
    VE_Einheitendefinition,
    VE_Streckentypdefinition,
    VE_Kantensperrung,
    VE_DUMMY
};
