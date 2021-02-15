/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
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
/// @file    MathTools.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.tools;

import java.math.BigDecimal;
import java.math.RoundingMode;
import java.util.Random;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class MathTools {

    /**
     *
     * Returns an integer given two bytes as input parameters.
     *
     * @param b0 The first and most significant byte
     * @param b1 The seconds and less significant byte
     * @return An integer
     */
    public static int getInteger(byte b0, byte b1) {
        int a = MathTools.toUnsignedInteger(b0);
        int b = MathTools.toUnsignedInteger(b1);

        a <<= 8;

        return a + b;
    }

    /**
     *
     * @param value
     * @return
     */
    public static final byte[] intToByteArray(int value) {
        return new byte[] {
                   (byte)(value >>> 24),
                   (byte)(value >>> 16),
                   (byte)(value >>> 8),
                   (byte)value
               };
    }

    /**
     *
     * @param value
     * @return
     */
    public static final byte[] longToByteArray(long value) {
        return new byte[] {
                   (byte)(value >>> 56),
                   (byte)(value >>> 48),
                   (byte)(value >>> 40),
                   (byte)(value >>> 32),
                   (byte)(value >>> 24),
                   (byte)(value >>> 16),
                   (byte)(value >>> 8),
                   (byte)value
               };
    }

    /**
     *
     * @param b0
     * @param b1
     * @param b2
     * @param b3
     * @return
     */
    public static int getInteger(byte b0, byte b1, byte b2, byte b3) {
        int in1 = getInteger(b0, b1);
        int in2 = getInteger(b2, b3);

        in1 <<= 16;

        return in1 + in2;
    }

    /**
     *
     * @param b0
     * @param b1
     * @param b2
     * @param b3
     * @param b4
     * @param b5
     * @param b6
     * @param b7
     * @return
     */
    public static long getLong(byte b0, byte b1, byte b2, byte b3, byte b4, byte b5, byte b6, byte b7) {
        long in0 = toUnsignedLong(b0) << 56;
        long in1 = toUnsignedLong(b1) << 48;
        long in2 = toUnsignedLong(b2) << 40;
        long in3 = toUnsignedLong(b3) << 32;
        long in4 = toUnsignedLong(b4) << 24;
        long in5 = toUnsignedLong(b5) << 16;
        long in6 = toUnsignedLong(b6) << 8;
        long in7 = toUnsignedLong(b7);

        return in0 + in1 + in2 + in3 + in4 + in5 + in6 + in7;
    }

    /**
     * Returns a pseudo-random number between min and max, inclusive.
     * The difference between min and max can be at most
     * <code>Integer.MAX_VALUE - 1</code>.
     *
     * @param min Minimum value
     * @param max Maximum value.  Must be greater than min.
     * @return Integer between min and max, inclusive.
     * @see java.util.Random#nextInt(int)
     */
    public static int randInt(int min, int max) {
        // NOTE: Usually this should be a field rather than a method
        // variable so that it is not re-seeded every call.
        Random rand = new Random();

        // nextInt is normally exclusive of the top value,
        // so add 1 to make it inclusive
        int randomNum = rand.nextInt((max - min) + 1) + min;

        return randomNum;
    }

    /**
     *
     * @param value
     * @param places
     * @return
     */
    public static double roundUp(double value, int places) {
        if (places < 0) {
            throw new IllegalArgumentException();
        }

        BigDecimal bd = new BigDecimal(value);
        bd = bd.setScale(places, RoundingMode.HALF_UP);

        return bd.doubleValue();
    }

    /**
     *
     * @param value
     * @param places
     * @return
     */
    public static long roundUp(long value, int places) {
        long aa = (long)(Math.pow(10, places));

        if (value % aa == 0) {
            return value;
        }

        long tmp = value / aa;
        tmp++;

        return tmp * aa;
    }

    /**
     *
     * @param value
     * @param places
     * @return
     */
    public static long roundHalf(long value, int places) {
        if (places < 0) {
            throw new IllegalArgumentException();
        }

        int plac = (int)Math.pow(10, places);

        double tmp = (double)value;
        tmp /= plac;

        double tronco = (long)tmp * plac;
        double resto = value - tronco;

        if (resto >= (plac / 2)) {
            tronco += plac;
        }

        return (long)tronco;
    }

    /**
     *
     * @param value
     * @param places
     * @return
     */
    public static int roundHalf(int value, int places) {
        if (places < 0) {
            throw new IllegalArgumentException();
        }

        int plac = (int)Math.pow(10, places);

        double tmp = (double)value;
        tmp /= plac;

        double tronco = (int)tmp * plac;
        double resto = value - tronco;

        if (resto >= (plac / 2)) {
            tronco += plac;
        }

        return (int)tronco;
    }


    /**
     *
     * @param b
     * @return
     */
    public static int toUnsignedInteger(byte b) {
        return Integer.parseInt(BytesTools.bytesToBinaryString(b), 2);
    }

    /**
     *
     * @param b
     * @return
     */
    public static long toUnsignedLong(byte b) {
        return Long.parseLong(BytesTools.bytesToBinaryString(b), 2);
    }

    public static long toUnsignedLong(int i) {
        return Long.parseLong(Integer.toBinaryString(i), 2);
    }

    /**
    * Reverses an Array of bytes
    * @param bytes
    */
    public static byte[] reverseArray(byte... bytes) {
        byte[] temp = new byte[bytes.length];

        for (int i = 0; i < bytes.length; i++) {
            temp[i] = bytes[bytes.length - 1 - i];
        }

        return temp;
    }
}
