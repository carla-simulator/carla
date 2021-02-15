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
/// @file    HelpersHarmonoise.cpp
/// @author  Daniel Krajzewicz
/// @author  Jakob Erdmann
/// @author  Michael Behrisch
/// @date    Mon, 10.05.2004
///
// Noise data collector for edges/lanes
/****************************************************************************/
#include <config.h>

#include "PollutantsInterface.h"
#include "HelpersHarmonoise.h"
#include <limits>
#include <cmath>


// ===========================================================================
// static definitions
// ===========================================================================
// ---------------------------------------------------------------------------
// vehicle class noise emission coefficients
// ---------------------------------------------------------------------------
// rolling component, light vehicles, alpha
double
HelpersHarmonoise::myR_A_C1_Parameter[27] = { 69.9, 69.9, 69.9, 74.9, 74.9, 74.9, 77.3, 77.5, 78.1, 78.3, 78.9, 77.8, 78.5, 81.9, 84.1, 86.5, 88.6, 88.2, 87.6, 85.8, 82.8, 80.2, 77.6, 75.0, 72.8, 70.4, 67.9 };

// rolling component, light vehicles, beta
double
HelpersHarmonoise::myR_B_C1_Parameter[27] = { 33.0, 33.0, 33.0, 15.2, 15.2, 15.2, 41.0, 41.2, 42.3, 41.8, 38.6, 35.5, 31.7, 21.5, 21.2, 23.5, 29.1, 33.5, 34.1, 35.1, 36.4, 37.4, 38.9, 39.7, 39.7, 39.7, 39.7 };


// rolling component, heavy vehicles, alpha
double
HelpersHarmonoise::myR_A_C3_Parameter[27] = { 80.5, 80.5, 80.5, 82.5, 83.5, 83.5, 86.5, 88.3, 88.7, 88.3, 91.4, 92.2, 96.0, 98.1, 97.8, 98.4, 97.2, 94.6, 95.9, 90.5, 87.1, 85.1, 83.2, 81.3, 81.3, 81.3, 81.3 };

// rolling component, heavy vehicles, beta
double
HelpersHarmonoise::myR_B_C3_Parameter[27] = { 33.0, 33.0, 33.0, 30.0, 30.0, 30.0, 41.0, 41.2, 42.3, 41.8, 38.6, 35.5, 31.7, 21.5, 21.2, 23.5, 29.1, 33.5, 34.1, 35.1, 36.4, 37.4, 38.9, 39.7, 39.7, 39.7, 39.7 };



// traction component, light vehicles, alpha
double
HelpersHarmonoise::myT_A_C1_Parameter[27] = { 90.0, 92.0, 89.0, 91.0, 92.4, 94.8, 90.8, 86.8, 86.2, 84.5, 84.5, 84.8, 83.5, 81.8, 81.4, 79.0, 79.2, 81.4, 85.5, 85.8, 85.2, 82.9, 81.0, 78.2, 77.2, 75.2, 74.2 };

// traction component, light vehicles, beta
double
HelpersHarmonoise::myT_B_C1_Parameter[27] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4, 9.4 };


// traction component, heavy vehicles, alpha
double
HelpersHarmonoise::myT_A_C3_Parameter[27] = { 97.7, 97.3, 98.2, 103.3, 109.5, 104.3, 99.8, 100.2, 98.9, 99.5, 100.7, 101.2, 100.6, 100.2, 97.4, 97.1, 97.8, 97.3, 95.8, 94.9, 92.7, 90.6, 89.9, 87.9, 85.9, 83.8, 82.2 };

// traction component, heavy vehicles, beta
double
HelpersHarmonoise::myT_B_C3_Parameter[27] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7, 11.7 };


// ---------------------------------------------------------------------------
// A-weighted correction for octave bands
// ---------------------------------------------------------------------------
double
HelpersHarmonoise::myAOctaveBandCorrection[27] = { -44.7, -39.4, -34.6, -30.2, -26.2, -22.5, -19.1, -16.1, -13.4,
                                                   -10.9, -8.6, -6.6, -4.8, -3.2, -1.9, -0.8, 0.0, +0.6,
                                                   +1.0, +1.2, +1.3, +1.2, +1.0, +0.5, -0.1, -1.1, -2.5
                                                   };



double
mySurfaceCorrection[27] = { 0.7, 0.2, 3.6, -1.0, -1.8, -0.1, -0.9, -0.7, -1.1, -0.5, -1.5,
                            -2.4, -3.0, -4.6, -5.8, -6.5, -7.9, -7.8, -7.2, -6.3, -5.6,
                            -5.5, -4.8, -4.3
                            };


// ===========================================================================
// method definitions
// ===========================================================================
double
HelpersHarmonoise::computeNoise(SUMOEmissionClass c, double v, double a) {
    double* alphaT, *betaT, *alphaR, *betaR;
    double ac = 0;
    if (PollutantsInterface::isHeavy(c)) {
        alphaT = myT_A_C3_Parameter;
        betaT = myT_B_C3_Parameter;
        alphaR = myR_A_C3_Parameter;
        betaR = myR_B_C3_Parameter;
        ac = 5.6;
    } else if (!PollutantsInterface::isSilent(c)) {
        alphaT = myT_A_C1_Parameter;
        betaT = myT_B_C1_Parameter;
        alphaR = myR_A_C1_Parameter;
        betaR = myR_B_C1_Parameter;
        ac = 4.4;
    } else {
        return 0;
    }
    //
    double L_low = 0;
    double L_high = 0;
    v = v * 3.6;
    double s = -30.;//
    for (int i = 0; i < 27; ++i) {
        double crc_low = alphaR[i] + betaR[i] * log10(v / 70.) + 10.*log10(.8); // + mySurfaceCorrection[i];
        double ctc_low = alphaT[i] + betaT[i] * ((v - 70.) / 70.) + a * ac + 10.*log10(.2);
        double Li_low = 10. * log10(pow(10., (crc_low / 10.)) + pow(10., (ctc_low / 10.)));
        Li_low += s;
        double crc_high = alphaR[i] + betaR[i] * log10(v / 70.) + 10.*log10(.2); // + mySurfaceCorrection[i];
        double ctc_high = alphaT[i] + betaT[i] * ((v - 70.) / 70.) + a * ac + 10.*log10(.8);
        double Li_high = 10. * log10(pow(10., (crc_high / 10.)) + pow(10., (ctc_high / 10.)));
        Li_high += s;
        L_low += pow(10., (Li_low + myAOctaveBandCorrection[i]) / 10.);
        L_high += pow(10., (Li_high + myAOctaveBandCorrection[i]) / 10.);
    }
    L_low = (10. * log10(L_low));
    L_high = (10. * log10(L_high));
    double v1 = (double)(10. * log10(pow(10., L_low / 10.) + pow(10., L_high / 10.)));
    return v1;
}


/****************************************************************************/
