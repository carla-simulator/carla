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
/// @file    XMLNode.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.utils.xmladmin2;

//import de.dlr.ts.commons.logger.DLRLogger;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
public class XMLNode extends XMLNodeBase {

    /**
     *
     */
    public XMLNode() {
    }

    /**
     *
     * @param name
     */
    public XMLNode(String name) throws MalformedKeyOrNameException {
        super(name);
    }

    /**
     *
     * @param name
     * @param value
     */
    public XMLNode(String name, String value) throws MalformedKeyOrNameException {
        super(name, value);
    }

    /**
     *
     */
    public void print() {
        try {
            System.out.println(this);

            for (int i = 0; i < this.getNodesCount(); i++) {
                getNode(i).print();
            }
        } catch (MalformedKeyOrNameException | XMLNodeNotFoundException ex) {
            XMLAdmin2.printException(ex);
        }
    }

    /**
     *
     * @param keys
     * @return
     */
    private XMLNode getNode(List<KeyNode> keys) throws XMLNodeNotFoundException {
        if (keys != null) {
            KeyNode tmp = keys.remove(0);

            List<XMLNode> filtered = filterNodes(children, tmp);

            if (filtered.isEmpty()) {
                throw new XMLNodeNotFoundException("[XMLReader] XMLNode '" + tmp.getName() + "' not found");
            }

            if (tmp.getOccurrence() > filtered.size() - 1) {
                throw new XMLNodeNotFoundException("[XMLReader] Occurrence " + tmp.getOccurrence()
                                                   + " for " + tmp.getName() + " out of bounds");
            } else {
                XMLNode son = filtered.get(tmp.getOccurrence());

                if (keys.isEmpty()) {
                    return son;
                } else {
                    return son.getNode(keys);
                }
            }

        }

        return null;
    }

    /**
     *
     * @param key
     * @return
     */
    public XMLNode getNode(String key) throws XMLNodeNotFoundException, MalformedKeyOrNameException {
        return getNode(key, 0);
    }

    /**
     *
     * @param key
     * @param occurrence
     * @return
     */
    public XMLNode getNode(String key, int occurrence) throws XMLNodeNotFoundException, MalformedKeyOrNameException {
        List<KeyNode> parsedKey = Tools.parseKey(key, occurrence);

        XMLNode node = getNode(parsedKey);

        if (node == null) {
            throw new XMLNodeNotFoundException("[XMLReader] " + key + " node could not be found");
        }

        return node;
    }

    /**
     *
     * @param key
     * @return
     */
    public int getNodesCount(String key) throws MalformedKeyOrNameException,
        XMLNodeNotFoundException {
        List<KeyNode> parseKey = Tools.parseKey(key, 0);

        if (parseKey.size() == 1) {
            return XMLNode.filterNodes(children, parseKey.get(0)).size();
        }

        if (parseKey.size() > 1) {
            KeyNode removed = parseKey.remove(parseKey.size() - 1);
            XMLNode node = getNode(parseKey);

            return XMLNode.filterNodes(node.children, removed).size();
        }

        return 0;
    }

    /**
     *
     * @param key
     * @return
     */
    public boolean hasNode(String key) throws MalformedKeyOrNameException {
        try {
            getNode(key);
            return true;
        } catch (XMLNodeNotFoundException ex) {
            return false;
        }
    }

    /**
     *
     * @param list
     * @param keyNode
     * @return
     */
    private static List<XMLNode> filterNodes(List<XMLNode> list, KeyNode keyNode) {
        List<XMLNode> temp = new ArrayList<>();

        for (XMLNode ch : list) {
            if (ch.equals(keyNode)) {
                temp.add(ch);
            }
        }

        return temp;
    }

    /**
     *
     * @param keyNode
     * @return
     */
    boolean equals(KeyNode keyNode) {
        if (!keyNode.getName().isEmpty()) {
            if (!keyNode.getName().trim().equals("*")) {
                if (!keyNode.getName().equals(this.getName())) {
                    return false;
                }

                if (keyNode.getValue() != null && !keyNode.getValue().isEmpty()) {
                    if (!keyNode.getValue().equals(this.getValue())) {
                        return false;
                    }
                }
            }

        }

        if (keyNode.getAttributes().hasAttributes()) {
            for (int i = 0; i < keyNode.getAttributes().size(); i++) {
                if (keyNode.getAttributes().get(i).getValue() != null && !keyNode.getAttributes().get(i).getValue().isEmpty()) {
                    if (!attributes.hasAttribute(keyNode.getAttributes().get(i).getName(), keyNode.getAttributes().get(i).getValue())) {
                        return false;
                    }
                } else if (attributes.hasAttribute(keyNode.getAttributes().get(i).getName())) {
                    return false;
                }
            }
        }

        return true;
    }

    /**
     *
     * @return
     */
    @Override
    public String toString() {
        try {
            StringBuilder sb = new StringBuilder(this.getName() + " - " + this.getValue() + " (" + getNodesCount() + ")");

            for (int i = 0; i < attributes.size(); i++) {
                sb.append(System.lineSeparator());
                sb.append("\t");
                Attribute att = attributes.get(i);
                sb.append(att.getName());
                sb.append(" = ");
                sb.append(att.getValue());
            }

            return sb.toString();
        } catch (MalformedKeyOrNameException | XMLNodeNotFoundException ex) {
            XMLAdmin2.printException(ex);
        }

        return "";
    }

}
