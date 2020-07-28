/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2002-2020 German Aerospace Center (DLR) and others.
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
/// @file    FareToken.h
/// @author  Ricardo Euler
/// @date    Thu, 17 August 2018
///
// Fare Modul for calculating prices during intermodal routing
/****************************************************************************/

#pragma once
enum class FareToken : int {
    None =  0,
    Free =  1, // walking and other things can be free
    H    =  2,
    L    =  3,
    T1   =  4,
    T2   =  5,
    T3   =  6,
    Z    =  7,
    M    =  8,
    U    =  9,
    KL   =  10,
    KH   =  11,
    K    =  12,
    KHU  =  13,
    KLU  =  14,
    KHZ  =  15,
    KLZ  =  16,
    ZU   =  17,   //Artificial state for after leaving short trip khu or klu since a pedestrian edge does not allow us
    //to distinguish between z or u zones
    START = 18

};

namespace FareUtil {

inline std::string tokenToString(FareToken const& token) {
    switch (token) {
        case FareToken::H:
            return "H";
        case FareToken::L:
            return "L";
        case FareToken::T1:
            return "T1";
        case FareToken::T2:
            return "T2";
        case FareToken::T3:
            return "T3";
        case FareToken::U:
            return "U";
        case FareToken::Z:
            return "Z";
        case FareToken::M:
            return "M";
        case FareToken::K:
            return "K";
        case FareToken::KL:
            return "KL";
        case FareToken::KH:
            return "KH";
        case FareToken::ZU:
            return "ZU";
        case FareToken::None:
            return "None";
        case FareToken::Free:
            return "Free";
        case FareToken::KHU:
            return "KHU";
        case FareToken::KLU:
            return "KLU";
        case FareToken ::KHZ:
            return "KHZ";
        case FareToken ::KLZ:
            return  "KLZ";
        default:
            return "";
    }
}


inline std::string tokenToTicket(FareToken const& token) {
    switch (token) {
        case FareToken::H:
            return "Einzelticket Halle";
        case FareToken::L:
            return "Einzelticket Leipzig";
        case FareToken::T1:
            return "Einzelticket Stadtverkehr 1";
        case FareToken::T2:
            return "Einzelticket Stadtverkehr 2";
        case FareToken::T3:
            return "Einzelticket Stadtverkehr 3";
        case FareToken::U:
            return "Einzelticket";
        case FareToken::Z:
            return "Einzelticket";
        case FareToken::M:
            return "Einzelticket Verbundpreis";
        case FareToken::K:
            return "Kurzstreckenticket";
        case FareToken::KL:
            return "Kurzstreckenticket Leipzig";
        case FareToken::KH:
            return "Kurzstreckenticket Halle";
        case FareToken::ZU:
            return "None";
        case FareToken::None:
            return "None";
        case FareToken::Free:
            return "Free";
        case FareToken::KHU:
            return "Kurzstreckenticket Halle";
        case FareToken::KLU:
            return "Kurzstreckenticket Leipzig";
        case FareToken ::KHZ:
            return "Kurzstreckenticket Halle";
        case FareToken ::KLZ:
            return  "Kurzstreckenticket Leipzig";
        case FareToken ::START:
            return "forbidden START";
    }
    return ""; //surpress compiler warning
}

inline FareToken stringToToken(std::string str) {
    if (str == "H") {
        return FareToken::H;
    }
    if (str == "L") {
        return FareToken::L;
    }
    if (str == "T1") {
        return FareToken::T1;
    }
    if (str == "T2") {
        return FareToken::T2;
    }
    if (str == "T3") {
        return FareToken::T3;
    }
    if (str == "1") {
        return FareToken::T1;
    }
    if (str == "2") {
        return FareToken::T2;
    }
    if (str == "3") {
        return FareToken::T3;
    }
    if (str == "U") {
        return FareToken::U;
    }
    if (str == "Z") {
        return FareToken::Z;
    }
    if (str == "M") {
        return FareToken::M;
    }
    if (str == "K") {
        return FareToken::K;
    }
    if (str == "KL") {
        return FareToken::KL;
    }
    if (str == "KH") {
        return FareToken::KH;
    }
    if (str == "ZU") {
        return FareToken::ZU;
    }
    if (str == "None") {
        return FareToken::None;
    }
    if (str == "Free") {
        return FareToken::Free;
    }
    if (str == "KHU") {
        return FareToken::KHU;
    }
    if (str == "KLU") {
        return FareToken::KLU;
    }
    if (str == "KHZ") {
        return FareToken::KHZ;
    }
    if (str == "KLZ") {
        return FareToken::KLZ;
    }
    if (str == "NOTFOUND") {
        return FareToken::None;
    }
    assert(false);
    return FareToken::None;
}

}
