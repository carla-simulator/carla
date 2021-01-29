/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2014-2020 German Aerospace Center (DLR) and others.
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
/// @file    XMLNodeBase.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.utils.xmladmin2;

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
class XMLNodeBase extends Value {

    protected List<XMLNode> children = new ArrayList<>();
    protected Attributes attributes = new Attributes();
    protected String name = null;

    /**
     *
     * @param key
     * @return
     */
    public XMLNode addNode(String key) throws MalformedKeyOrNameException {
        XMLNode child = Tools.createNodes(key);
        children.add(child);
        return child;
    }

    public void setName(String name) {
        this.name = name;
    }

    /**
     *
     * @return
     */
    public String getName() {
        return name;
    }

    /**
     *
     * @param child
     * @return The added child
     */
    public XMLNode addNode(XMLNode child) {
        children.add(child);
        return child;
    }

    /**
     *
     * @param key
     * @param value
     * @throws MalformedKeyOrNameException
     */
    public void addNode(String key, String value) throws MalformedKeyOrNameException {
        XMLNode child = Tools.createNodes(key, value);
        children.add(child);
    }

    /**
     *
     * @return
     */
    public Attributes getAttributes() {
        return attributes;
    }

    /**
     *
     */
    public XMLNodeBase() {
    }

    /**
     *
     * @param name
     */
    public XMLNodeBase(String name) throws MalformedKeyOrNameException {
        if (!Tools.validateNodeName(name)) {
            throw new MalformedKeyOrNameException(name);
        }

        this.name = name;
    }

    /**
     *
     * @param name
     * @param value
     */
    public XMLNodeBase(String name, String value) throws MalformedKeyOrNameException {
        this(name);
        this.value = value;
    }

    /**
     *
     * @param tab
     * @return
     */
    public String getXML(int tab) {
        StringBuilder ret = new StringBuilder();

        ret.append(getTabs(tab));
        ret.append("<");
        ret.append(name);

        if (!attributes.isEmpty()) {
            ret.append(attributes.getXML());
        }

        ret.append(">");

        if (children.size() > 0) {
            for (XMLNode node : children) {
                if (tab >= 0) {
                    ret.append(System.lineSeparator());
                    ret.append(node.getXML(tab + 1));
                } else {
                    ret.append(node.getXML(tab));
                }
            }

            if (tab >= 0) {
                ret.append(System.lineSeparator());
            }

            ret.append(getTabs(tab));
            ret.append("</").append(name).append(">");
        } else {
            if (value != null) {
                ret.append(value);
                ret.append("</").append(name).append(">");
            } else {
                ret.deleteCharAt(ret.length() - 1);  //deletes the last ">"
                ret.append("/>");
            }
        }

        return ret.toString();
    }

    /**
     *
     * @param count
     * @return
     */
    private String getTabs(int count) {
        if (count < 0) { //wanna no tabs
            return "";
        }

        String ret = "";
        for (int i = 0; i < count; i++) {
            ret += "\t";
        }

        return ret;
    }

    /**
     *
     * @return
     */
    public XMLNode getNode(int index) {
        return children.get(index);
    }

    /**
     *
     * @return
     */
    public int getNodesCount() throws MalformedKeyOrNameException, XMLNodeNotFoundException {
        return children.size();
    }
}
