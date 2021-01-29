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
/// @file    BytesTools.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.commons.tools;

import java.util.Arrays;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class BytesTools {

    /**
     *
     * @param number
     * @return
     */
    public static byte[] getBytes(long number) {
        byte[] b = new byte[Long.SIZE / Byte.SIZE];
        for (int i = 0; i < Long.SIZE / Byte.SIZE; i++) {
            b[i] = getByte(number, i);
        }

        return b;
    }

    public static boolean getBit(byte b, int index) {
        b >>= index;

        return (b & 0x01) == 1;
    }

    public static String bytesToHex(byte[] array) {
        final StringBuilder builder = new StringBuilder();
        for (byte b : array) {
            builder.append(String.format("%02x", b));
        }

        return "0x" + builder.toString();
    }

    /**
     *
     * @param number
     * @return
     */
    public static byte[] getBytes(int number) {
        int size = Integer.SIZE / Byte.SIZE;

        byte[] b = new byte[size];

        for (int i = 0; i < size; i++) {
            b[i] = getByte(number, i);
        }

        return b;
    }

    public static byte[] getBytes(short value) {
        byte[] b = new byte[2];
        b[0] = getByte(value, 0);
        b[1] = getByte(value, 1);
        return b;
    }

    /**
     *
     * @param number
     * @param index
     * @return
     */
    public static byte getByte(long number, int index) {
        if (index > 7) {
            throw new IllegalArgumentException();
        }

        number = number >>> (index * Byte.SIZE);
        return (byte)(number & 0xff);
    }

    /**
     *
     * @param number
     * @param index
     * @return
     */
    public static byte getByte(short number, int index) {
        if (index > 1) {
            throw new IllegalArgumentException();
        }

        if (index == 0) {
            return (byte)(number & 0xff);
        }

        int tmp = number;
        tmp >>>= 8;
        return (byte)(tmp & 0xff);
    }

    /**
     *
     * @param binaryString
     * @return
     */
    public static byte[] hexStringToBytes(String binaryString) {
        if (binaryString.startsWith("0x")) {
            binaryString = binaryString.replaceFirst("0x", "");
        }

        int len = binaryString.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte)((Character.digit(binaryString.charAt(i), 16) << 4)
                                 + Character.digit(binaryString.charAt(i + 1), 16));
        }
        return data;
    }

    /**
     *
     * @param length
     * @param offset
     * @return
     */
    public static long getMaskOfZeros(int length, int offset) {
        long temp = getMaskOfOnes(length, 0);
        long mask = -1; //all ones

        temp = temp ^ mask;

        for (int i = 0; i < offset; i++) {
            temp <<= 1;
            temp++;
        }

        return temp;
    }

    /**
     *
     * @param length
     * @param offset
     * @return
     */
    public static long getMaskOfOnes(int length, int offset) {
        long l = 0;
        long mask = 0x1;

        for (int i = 0; i < length; i++) {
            l += mask;
            mask <<= 1;
        }

        return l << offset;
    }

    /**
     *
     * @param bytes
     * @return
     */
    public static long bytesToLong(byte... bytes) {
        long ret = 0L;
        long temp;

        for (int i = 0; i < bytes.length; i++) {
            temp = bytes[i] & 0xFF;

            if (i > 0) {
                temp <<= Byte.SIZE * i;
            }

            ret += temp;
        }

        return ret;
    }

    public static byte[] invert(byte... bytes) {
        byte[] a = new byte[bytes.length];
        for (int i = 0; i < bytes.length; i++) {
            a[i] = bytes[bytes.length - 1 - i];
        }

        return a;
    }

    public static long swapEndianness(long value) {
        return bytesToLong(invert(getBytes(value)));
    }

    public static int swapEndianness(int value) {
        return bytesToInt(invert(getBytes(value)));
    }

    public static short swapEndianness(short value) {
        return bytesToShort(invert(getBytes(value)));
    }

    public static short bytesToShort(byte... bytes) {
        int end = 0;
        int help;

        for (int i = 0; i < bytes.length; i++) {
            help = bytes[i] & 0xFF;
            help <<= Byte.SIZE * i;
            end += help;
        }

        return (short) end;
    }

    /**
     *
     * @param bytes
     * @return
     */
    public static int bytesToInt(byte... bytes) {
        int end = 0;
        int help;

        for (int i = 0; i < bytes.length; i++) {
            help = bytes[i] & 0xFF;
            help <<= Byte.SIZE * i;
            end += help;
        }

        return end;
    }

    private static int bytesToInt2(byte... bytes) {
        int end = 0;
        int help;

        for (int i = 0; i < bytes.length; i++) {
            help = bytes[i] & 0xFF;

            if (i > 0) {
                help <<= Byte.SIZE * i;
            }

            end += help;
        }

        return end;
    }

    public static void main2(String[] args) {
        byte[] aa = new byte[3];
        aa[0] = 1;
        aa[1] = 0;
        aa[2] = 0;

        byte[] invert = invert(aa);

        for (int i = 0; i < invert.length; i++) {
            System.out.println("# " + invert[i]);
        }
    }

    /**
     * This function transforms a byte value in a binary string.
     *
     * @param data
     * @return
     */
    public static String byteToBinaryString(byte data) {
        StringBuilder sb = new StringBuilder();
        int un = data & 0xff;

        for (int i = Byte.SIZE - 1; i >= 0; i--) {
            int aa = ((un >> i) & 0x1);

            if (aa == 0) {
                sb.append("0");
            } else {
                sb.append("1");
            }
        }

        return sb.toString();
    }

    /**
     *
     * @param data
     * @return
     */
    public static String longToBinaryString(long data) {
        StringBuilder sb = new StringBuilder();

        for (int i = Long.SIZE - 1; i >= 0; i--) {
            long aa = ((data >>> i) & 0x1);

            if (aa == 0) {
                sb.append("0");
            } else {
                sb.append("1");
            }
        }

        return sb.toString();
    }

    /**
     *
     * @param b
     * @return
     */
    public static String bytesToBinaryString(byte... b) {
        String ret = "";
        for (int i = 0; i < b.length; i++) {
            ret += byteToBinaryString(b[i]);
            if (i != b.length - 1) {
                ret += "-";
            }
        }

        return ret;
    }

}
