/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
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
/// @file    Storage.java
/// @author  Axel Wegener
/// @author  Enrico Gueli
/// @author  Mario Krumnow
/// @date    2016
///
//
/****************************************************************************/
package de.uniluebeck.itm.tcpip;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.Serializable;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.ListIterator;

/*
 * Modifications by Enrico Gueli:
 * - replaced LinkedList to ArrayList
 * - used the basic List interface
 * - toString() implementation
 * - standard comments converted to Javadoc
 * - getStorageList() made public (but read-only)
 */


public class Storage implements Serializable {

    private static final long serialVersionUID = 1L;
    private List<Byte> storageList;
    private int position;
    private ListIterator<Byte> listIt;

    public Storage() {
        storageList = new ArrayList<Byte>();
        init();
    }

    public Storage(byte[] packet) {
        this(packet, 0, packet.length);
    }

    public Storage(byte[] packet, int offset, int length) {
        if (packet == null) {
            throw new NullPointerException("packet can't be null");
        }
        if (length > packet.length) {
            throw new IndexOutOfBoundsException("length exceeds packet length");
        }
        if (offset + length > packet.length) {
            throw new IndexOutOfBoundsException("content is outside the array");
        }

        storageList = new ArrayList<Byte>();

        for (int i = offset; i < length; i++) {
            writeByte(packet[i]);
        }

        init();
    }

    public Storage(short[] packet) {
        this(packet, 0, packet.length);
    }


    public Storage(short[] packet, int offset, int length) {
        if (packet == null) {
            throw new NullPointerException("packet can't be null");
        }
        if (length > packet.length) {
            throw new IndexOutOfBoundsException("length exceeds packet length");
        }
        if (offset + length > packet.length) {
            throw new IndexOutOfBoundsException("content is outside the array");
        }

        storageList = new ArrayList<Byte>();

        for (int i = offset; i < length; i++) {
            writeByte(packet[i]);
        }

        init();
    }

    public boolean validPos() {
        return (position < storageList.size()
                && position >= 0
                && storageList.size() != 0);
    }

    public int position() {
        return position;
    }

    /**
     * Write a byte value to the List
     * a signed value will be converted to its unsigned equivalent first
     * @param value the byte to be written
     */
    public void writeByte(short value) throws IllegalArgumentException {
        writeByte((int)value);
    }

    public void writeByte(int value) throws IllegalArgumentException {
        if (value < -128 || value > 127) {
            throw new IllegalArgumentException("Error writing byte: byte value may only range from -128 to 127.");
        }

        storageList.add(Byte.valueOf((byte)(value)));
    }

    /**
     * Read a byte value from the List
     * @return the read byte as an Integer value (unsigned)
     */
    public short readByte() throws IllegalStateException {
        if (!validPos()) {
            throw new IllegalStateException("Error reading byte, invalid list position specified for reading: " + position);
        }

        position++;
        return (short) listIt.next();

    }

    public void writeUnsignedByte(short value) throws IllegalArgumentException {
        writeUnsignedByte((int) value);
    }

    public void writeUnsignedByte(int value) throws IllegalArgumentException {
        if (value < 0 || value > 255) {
            throw new IllegalArgumentException("Error writing unsigned byte: byte value may only range from 0 to 255.");
        }

        // 0 -> 0
        // 127 -> 127
        // 128 -> -128
        // 255 -> -1

        if (value > 127) {
            storageList.add(Byte.valueOf((byte)(value - 256)));
        } else {
            storageList.add(Byte.valueOf((byte)(value)));
        }
    }

    public short readUnsignedByte() throws IllegalStateException {
        if (!validPos()) {
            throw new IllegalStateException("Error reading unsigned byte, invalid list position specified for reading: " + position);
        }

        // -128 -> 128
        // -1 -> 255
        // 0 -> 0
        // 127 -> 127

        position++;
        return (short)((listIt.next() + 256) % 256);

    }


    /**
     * Write a signed short value to the list
     * @param value the short value to be written
     */
    public void writeShort(int value) throws IllegalArgumentException {
        ByteArrayOutputStream byteOut = new ByteArrayOutputStream(2);
        DataOutputStream dataOut = new DataOutputStream(byteOut);
        byte bytes[] = new byte[2];

        if (value < -32768 || value > 32768) {
            throw new IllegalArgumentException("Error writing short: short value may only range from -32768 to 32768.");
        }

        try {
            dataOut.writeShort(value);
            dataOut.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        bytes = byteOut.toByteArray();
        for (int i = 0; i < 2; i++) {
            writeByte(bytes[i]);
        }
    }

    /**
     * Read a short value from the list
     * @return the read short value as an Integer
     */
    public int readShort() throws IllegalStateException {
        ByteArrayInputStream byteIn;
        DataInputStream dataIn;
        byte content[] = new byte[2];
        int result = 0;

        for (int i = 0; i < 2; i++) {
            content[i] = (byte) readByte();
        }
        byteIn =  new ByteArrayInputStream(content);
        dataIn = new DataInputStream(byteIn);
        try {
            result = dataIn.readShort();
            dataIn.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }

    /**
     * Write an signed Integer to the list
     * the int value is being split up into 4 bytes in msb first order
     * @param value the int value to be written
     */
    public void writeInt(int value) throws IllegalArgumentException {

        ByteArrayOutputStream byteOut = new ByteArrayOutputStream(4);
        DataOutputStream dataOut = new DataOutputStream(byteOut);
        byte bytes[] = new byte[4];

        try {
            dataOut.writeInt(value);
            dataOut.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        bytes = byteOut.toByteArray();
        for (int i = 0; i < 4; i++) {
            writeByte(bytes[i]);
        }

    }

    /**
     * Read a signed integer value, that was split up into 4 bytes
     * @return the read int value
     */
    public int readInt() throws IllegalStateException {
        ByteArrayInputStream byteIn;
        DataInputStream dataIn;
        byte content[] = new byte[4];
        int result = 0;

        for (int i = 0; i < 4; i++) {
            content[i] = (byte) readByte();
        }
        byteIn =  new ByteArrayInputStream(content);
        dataIn = new DataInputStream(byteIn);
        try {
            result = dataIn.readInt();
            dataIn.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }

    /**
     * Write a Float to the list
     * the float value is beeing split up into 4 bytes in msb first order
     * @param value the float value to be written
     */
    public void writeFloat(float value) throws IllegalArgumentException {
        ByteArrayOutputStream byteOut = new ByteArrayOutputStream(4);
        DataOutputStream dataOut = new DataOutputStream(byteOut);
        byte bytes[] = new byte[4];

        try {
            dataOut.writeFloat(value);
            dataOut.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        bytes = byteOut.toByteArray();
        for (int i = 0; i < 4; i++) {
            writeByte(bytes[i]);
        }

    }

    /**
     * Read a Float value, that was split up into 4 bytes
     * @return the read float value
     */
    public float readFloat() throws IllegalStateException {
        ByteArrayInputStream byteIn;
        DataInputStream dataIn;
        byte content[] = new byte[4];
        float result = 0;

        for (int i = 0; i < 4; i++) {
            content[i] = (byte) readByte();
        }
        byteIn =  new ByteArrayInputStream(content);
        dataIn = new DataInputStream(byteIn);
        try {
            result = dataIn.readFloat();
            dataIn.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }

    /**
     * Write a Double to the list
     * the value is beeing split up into 8 bytes in msb first order
     * @param value the double value to be written
     */
    public void writeDouble(double value) throws IllegalArgumentException {
        ByteArrayOutputStream byteOut = new ByteArrayOutputStream(8);
        DataOutputStream dataOut = new DataOutputStream(byteOut);
        byte bytes[] = new byte[8];

        try {
            dataOut.writeDouble(value);
            dataOut.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        bytes = byteOut.toByteArray();
        for (int i = 0; i < 8; i++) {
            writeByte(bytes[i]);
        }

    }

    /**
     * Read a Double value, that was split up into 8 bytes
     * @return the read double value
     */
    public double readDouble() throws IllegalStateException {
        ByteArrayInputStream byteIn;
        DataInputStream dataIn;
        byte content[] = new byte[8];
        double result = 0;

        for (int i = 0; i < 8; i++) {
            content[i] = (byte) readByte();
        }
        byteIn =  new ByteArrayInputStream(content);
        dataIn = new DataInputStream(byteIn);
        try {
            result = dataIn.readDouble();
            dataIn.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return result;
    }

    /**
     * Write a string to the list by encoding the characters in UTF-8
     * @param value the String to be written
     */
    public void writeStringUTF8(String value) throws IllegalArgumentException {
        writeString(value, "UTF-8");
    }

    /**
     * Write a string to the list by encoding the characters in ASCII
     * @param value the String to be written
     */
    public void writeStringASCII(String value) throws IllegalArgumentException {
        writeString(value, "US-ASCII");
    }

    /**
     * Write a string to the list by encoding the characters in ISO-LATIN1
     * @param value the String to be written
     */
    public void writeStringISOLATIN1(String value) throws IllegalArgumentException {
        writeString(value, "ISO-8859-1");
    }

    /**
     * Write a string to the list by encoding the characters in UTF-16 Big Endian
     * @param value the String to be written
     */
    public void writeStringUTF16BE(String value) throws IllegalArgumentException {
        writeString(value, "UTF-16BE");
    }

    /**
     * Write a string to the list by encoding the characters in UTF-16 Little Endian
     * @param value the String to be written
     */
    public void writeStringUTF16LE(String value) throws IllegalArgumentException {
        writeString(value, "UTF-16LE");
    }

    private void writeString(String value, String charset) throws IllegalArgumentException {
        byte bytes[];

        try {
            bytes = value.getBytes(charset);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
            return;
        }

        writeInt(value.length());

        for (int i = 0; i < bytes.length; i++) {
            writeByte(bytes[i]);
        }
    }

    /**
     * Read a String from the list, that was encoded using UTF-8
     * @return the read String
     */
    public String readStringUTF8() throws IllegalArgumentException {
        return readString("UTF-8");
    }

    /**
     * Read a String from the list, that was encoded using ASCII
     * @return the read String
     */
    public String readStringASCII() throws IllegalArgumentException {
        return readString("US-ASCII");
    }

    /**
     * Read a String from the list, that was encoded using ISO-LATIN1
     * @return the read String
     */
    public String readStringISOLATIN1() throws IllegalArgumentException {
        return readString("ISO-8859-1");
    }

    /**
     * Read a String from the list, that was encoded using UTF-16 Big Endian
     * @return the read String
     */
    public String readStringUTF16BE() throws IllegalArgumentException {
        return readString("UTF-16BE");
    }

    /**
     * Read a String from the list, that was encoded using UTF-16 Little Endian
     * @return the read String
     */
    public String readStringUTF16LE() throws IllegalArgumentException {
        return readString("UTF-16LE");
    }

    private String readString(String charset) throws IllegalStateException {
        byte content[];
        String result = new String("");
        int length;

        length = readInt();
        content = new byte[length];
        for (int i = 0; i < length; i++) {
            content[i] = (byte) readByte();
        }

        try {
            result = new String(content, charset);

        } catch (UnsupportedEncodingException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        return result;
    }

    public void reset() {
        storageList.clear();
        init();
    }

    /**
     * Retrieve the Size of the internal storage list
     * @return size  of the storage list
     */
    public int size() {
        return storageList.size();
    }

    /**
     * Retrieve the internal list that is used to store the data
     * @return the internal storage list
     */
    public List<Byte> getStorageList() {
        return Collections.unmodifiableList(storageList);
    }

    private void init() {
        position = 0;
        listIt = storageList.listIterator();
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < storageList.size(); i++) {
            if (i == position) {
                sb.append("[");
            } else {
                sb.append(" ");
            }

            sb.append(String.format("%02X", storageList.get(i)));
        }

        return sb.toString();
    }

    public String debug() {
        return ("storageSize=" + storageList.size() + " pos=" + position + " storage=" + storageList);
    }
}
