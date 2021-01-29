/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2017-2020 German Aerospace Center (DLR) and others.
// TraaS module
// Copyright (C) 2016-2017 Dresden University of Technology
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
/// @file    StringList.java
/// @author  Mario Krumnow
/// @author  Evamarie Wiessner
/// @date    2016
///
//
/****************************************************************************/
package de.tudresden.sumo.util;

import it.polito.appeal.traci.TraCIException;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

import de.tudresden.sumo.config.Constants;
import de.uniluebeck.itm.tcpip.Storage;

class StringList implements List<String> {

    private final List<String> list;

    public StringList() {
        list = new ArrayList<String>();
    }

    public StringList(List<String> list) {
        this.list = list;
    }

    public StringList(Storage storage, boolean verifyType) throws TraCIException {
        if (verifyType) {
            if (storage.readByte() != Constants.TYPE_STRINGLIST) {
                throw new TraCIException("string list expected");
            }
        }

        int len = storage.readInt();
        list = new ArrayList<String>(len);
        for (int i = 0; i < len; i++) {
            list.add(storage.readStringASCII());
        }
    }

    public void writeTo(Storage out, boolean writeTypeID) {
        if (writeTypeID) {
            out.writeByte(Constants.TYPE_STRINGLIST);
        }
        out.writeInt(list.size());
        for (String str : list) {
            out.writeStringASCII(str);
        }
    }

    @Override
    public String toString() {
        return list.toString();
    }

    /**
     * @param index
     * @param element
     * @see java.util.List#add(int, java.lang.Object)
     */
    public void add(int index, String element) {
        list.add(index, element);
    }

    /**
     * @param e

     * @see java.util.List#add(java.lang.Object)
     */
    public boolean add(String e) {
        return list.add(e);
    }

    /**
     * @param c

     * @see java.util.List#addAll(java.util.Collection)
     */
    public boolean addAll(Collection<? extends String> c) {
        return list.addAll(c);
    }

    /**
     * @param index
     * @param c

     * @see java.util.List#addAll(int, java.util.Collection)
     */
    public boolean addAll(int index, Collection<? extends String> c) {
        return list.addAll(index, c);
    }

    /**
     *
     * @see java.util.List#clear()
     */
    public void clear() {
        list.clear();
    }

    /**
     * @param o

     * @see java.util.List#contains(java.lang.Object)
     */
    public boolean contains(Object o) {
        return list.contains(o);
    }

    /**
     * @param c

     * @see java.util.List#containsAll(java.util.Collection)
     */
    public boolean containsAll(Collection<?> c) {
        return list.containsAll(c);
    }

    /**
     * @param o

     * @see java.util.List#equals(java.lang.Object)
     */
    public boolean equals(Object o) {
        return list.equals(o);
    }

    /**
     * @param index

     * @see java.util.List#get(int)
     */
    public String get(int index) {
        return list.get(index);
    }

    /**

     * @see java.util.List#hashCode()
     */
    public int hashCode() {
        return list.hashCode();
    }

    /**
     * @param o

     * @see java.util.List#indexOf(java.lang.Object)
     */
    public int indexOf(Object o) {
        return list.indexOf(o);
    }

    /**

     * @see java.util.List#isEmpty()
     */
    public boolean isEmpty() {
        return list.isEmpty();
    }

    /**

     * @see java.util.List#iterator()
     */
    public Iterator<String> iterator() {
        return list.iterator();
    }

    /**
     * @param o

     * @see java.util.List#lastIndexOf(java.lang.Object)
     */
    public int lastIndexOf(Object o) {
        return list.lastIndexOf(o);
    }

    /**

     * @see java.util.List#listIterator()
     */
    public ListIterator<String> listIterator() {
        return list.listIterator();
    }

    /**
     * @param index

     * @see java.util.List#listIterator(int)
     */
    public ListIterator<String> listIterator(int index) {
        return list.listIterator(index);
    }

    /**
     * @param index

     * @see java.util.List#remove(int)
     */
    public String remove(int index) {
        return list.remove(index);
    }

    /**
     * @param o

     * @see java.util.List#remove(java.lang.Object)
     */
    public boolean remove(Object o) {
        return list.remove(o);
    }

    /**
     * @param c

     * @see java.util.List#removeAll(java.util.Collection)
     */
    public boolean removeAll(Collection<?> c) {
        return list.removeAll(c);
    }

    /**
     * @param c

     * @see java.util.List#retainAll(java.util.Collection)
     */
    public boolean retainAll(Collection<?> c) {
        return list.retainAll(c);
    }

    /**
     * @param index
     * @param element

     * @see java.util.List#set(int, java.lang.Object)
     */
    public String set(int index, String element) {
        return list.set(index, element);
    }

    /**

     * @see java.util.List#size()
     */
    public int size() {
        return list.size();
    }

    /**
     * @param fromIndex
     * @param toIndex

     * @see java.util.List#subList(int, int)
     */
    public List<String> subList(int fromIndex, int toIndex) {
        return list.subList(fromIndex, toIndex);
    }

    /**

     * @see java.util.List#toArray()
     */
    public Object[] toArray() {
        return list.toArray();
    }

    /**
     * @param <T>
     * @param a
     * @see java.util.List#toArray(Object[])
     */
    public <T> T[] toArray(T[] a) {
        return list.toArray(a);
    }


}
