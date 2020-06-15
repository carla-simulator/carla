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
/// @file    FareZones.h
/// @author  Ricardo Euler
/// @date    Thu, 17 August 2018
///
// Fare Modul for calculating prices during intermodal routing
/****************************************************************************/

#pragma once
#include <config.h>

#include <unordered_map>
#include <cstdint>

static std::unordered_map<long long int, int> repToFareZone = std::unordered_map<long long int, int> {
    {1, 110},
    {2, 121},
    {4, 122},
    {8, 123},
    {16, 124},
    {32, 125},
    {64, 126},
    {128, 127},
    {256, 128},
    {512, 129},
    {1024, 131},
    {2048, 132},
    {4096, 133},
    {8192, 134},
    {16384, 141},
    {32768, 142},
    {65536, 143},
    {131072, 144},
    {262144, 145},
    {524288, 146},
    {1048576, 147},
    {2097152, 151},
    {4194304, 152},
    {8388608, 153},
    {16777216, 154},
    {33554432, 155},
    {67108864, 156},
    {134217728, 162},
    {268435456, 163},
    {536870912, 164},
    {1073741824, 165},
    {2147483648, 166},
    {4294967296, 167},
    {8589934592, 168},
    {17179869184, 210},
    {34359738368, 221},
    {68719476736, 222},
    {137438953472, 223},
    {274877906944, 224},
    {549755813888, 225},
    {1099511627776, 231},
    {2199023255552, 232},
    {4398046511104, 233},
    {8796093022208, 234},
    {17592186044416, 241},
    {35184372088832, 242},
    {70368744177664, 243},
    {140737488355328, 251},
    {281474976710656, 252},
    {562949953421312, 253},
    {1125899906842624, 254},
    {2251799813685248, 255},
    {4503599627370496, 256},
    {9007199254740992, 257},
    {18014398509481984, 258},
    {36028797018963968, 259},
    {72057594037927936, 261},
    {144115188075855872, 299},
    {288230376151711744, 321},
    {576460752303423488, 322},
    {1152921504606846976, 323},
    {2305843009213693952, 324}
};

static std::unordered_map<int, long long int> fareZoneToRep  = std::unordered_map<int, long long int > {
    {110, 1 },
    {121, 2 },
    {122, 4 },
    {123, 8 },
    {124, 16 },
    {125, 32 },
    {126, 64 },
    {127, 128 },
    {128, 256 },
    {129, 512 },
    {131, 1024 },
    {132, 2048 },
    {133, 4096 },
    {134, 8192 },
    {141, 16384 },
    {142, 32768 },
    {143, 65536 },
    {144, 131072 },
    {145, 262144 },
    {146, 524288 },
    {147, 1048576 },
    {151, 2097152 },
    {152, 4194304 },
    {153, 8388608 },
    {154, 16777216 },
    {155, 33554432 },
    {156, 67108864 },
    {162, 134217728 },
    {163, 268435456 },
    {164, 536870912 },
    {165, 1073741824 },
    {166, 2147483648 },
    {167, 4294967296 },
    {168, 8589934592 },
    {210, 17179869184 },
    {221, 34359738368 },
    {222, 68719476736 },
    {223, 137438953472 },
    {224, 274877906944 },
    {225, 549755813888 },
    {231, 1099511627776 },
    {232, 2199023255552 },
    {233, 4398046511104 },
    {234, 8796093022208 },
    {241, 17592186044416 },
    {242, 35184372088832 },
    {243, 70368744177664 },
    {251, 140737488355328 },
    {252, 281474976710656 },
    {253, 562949953421312 },
    {254, 1125899906842624 },
    {255, 2251799813685248 },
    {256, 4503599627370496 },
    {257, 9007199254740992 },
    {258, 18014398509481984 },
    {259, 36028797018963968 },
    {261, 72057594037927936 },
    {299, 144115188075855872 },
    {321, 288230376151711744 },
    {322, 576460752303423488 },
    {323, 1152921504606846976 },
    {324, 2305843009213693952 }
};

/**
 * Returns the zone the specified lower rank zones is a part of
 * @return
 */

inline int getOverlayZone(int zoneNumber) {
    if (zoneNumber < 400) {
        return zoneNumber;    //real "zone" numbers, no city zones
    }

    switch (zoneNumber) {
        case 511:
            return 165;
        case 512:
            return 166;
        case 513:
            return 167;
        case 514:
            return 142;
        case 515:
            return 123;
        case 516:
            return 127;
        case 518:
            return 145;
        case 519:
            return 144;
        case 521:
            return 153;
        case 551:
            return 231;
        case 552:
            return 232;
        case 553:
            return 233;
        case 554:
            return 259;
        case 555:
            return 241;
        case 556:
            return 255;
        case 571:
            return 322;
        case 572:
            return 324;
        default:
            return zoneNumber;
    }
}

