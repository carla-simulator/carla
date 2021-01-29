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
/// @file    Tools.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.utils.xmladmin2;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Source;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerException;
import javax.xml.transform.sax.SAXSource;
import javax.xml.transform.sax.SAXTransformerFactory;
import javax.xml.transform.stream.StreamResult;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.InputSource;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
class Tools {

    /**
     *
     * @param xml
     * @return
     */
    public static String formatXML(String xml) {
        try {
            Transformer serializer = SAXTransformerFactory.newInstance().newTransformer();
            serializer.setOutputProperty(OutputKeys.INDENT, "yes");
            serializer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "2");

            Source xmlSource = new SAXSource(new InputSource(new ByteArrayInputStream(xml.getBytes())));
            StreamResult res =  new StreamResult(new ByteArrayOutputStream());
            serializer.transform(xmlSource, res);

            return new String(((ByteArrayOutputStream)res.getOutputStream()).toByteArray());
        } catch (IllegalArgumentException | TransformerException e) {
            return xml;
        }
    }

    /**
     *
     * @param name
     * @return
     */
    public static boolean validateNodeName(final String name) {
        if (!name.matches("^[^\\d].*")) { //if it has a number at the beginning
            return false;
        }

        //if(name.indexOf('.') != -1)
        //  return false;

        if (name.contains("=") || name.contains("[") || name.contains("]") || name.contains(" ")) {
            return false;
        }

        return !name.toLowerCase().startsWith("xml");
    }

    /**
     *
     * @param orig
     * @param new_
     * @throws de.dlr.ts.utils.xmladmin2.exceptions.MalformedKeyOrNameException
     */
    static void extract(Node orig, XMLNode new_) throws MalformedKeyOrNameException {
        new_.setName(orig.getNodeName());

        if (orig.getFirstChild() != null && orig.getFirstChild().getNodeValue() != null) {
            new_.setValue(orig.getFirstChild().getNodeValue().trim());
        }

        /**
         * Extracting attributes
         */
        NamedNodeMap atts = orig.getAttributes();

        for (int i = 0; atts != null && i < atts.getLength(); i++) {
            Node item = atts.item(i);
            Attribute aa = new Attribute();
            aa.setName(item.getNodeName());
            aa.setValue(item.getNodeValue());

            new_.getAttributes().add(aa);
        }

        /**
         * Extracting Children
         */
        NodeList list = orig.getChildNodes();

        for (int i = 0; i < list.getLength(); i++) {
            Node item = list.item(i);

            if (item.getNodeType() != Node.ELEMENT_NODE) {
                continue;
            }

            XMLNode node_ = new XMLNode(item.getNodeName(), item.getNodeValue());
            new_.addNode(node_);

            extract(item, node_);
        }
    }

    /**
     *
     * @param key
     * @param value
     * @return
     * @throws MalformedKeyOrNameException
     */
    static XMLNode createNodes(String key, String value) throws MalformedKeyOrNameException {
        List<KeyNode> pk = parseKey(key);

        XMLNode first = new XMLNode();

        if (!pk.isEmpty()) {
            XMLNode node = first;

            for (int i = 0; i < pk.size(); i++) {
                KeyNode get = pk.get(i);

                if (i == 0) {
                    node.setName(get.getName());
                } else {
                    XMLNode tmp = new XMLNode(get.getName());
                    node.addNode(tmp);
                    node = tmp;
                }

                if (i == pk.size() - 1 && value != null) {
                    node.setValue(value);
                }

                if (!get.getAttributes().isEmpty()) {
                    for (int j = 0; j < get.getAttributes().size(); j++) {
                        node.getAttributes().add(get.getAttributes().get(j));
                    }
                }
            }
        }

        return first;
    }

    /**
     *
     * @param key
     * @return
     * @throws de.dlr.ts.utils.xmladmin2.exceptions.MalformedKeyOrNameException
     */
    static XMLNode createNodes(String key) throws MalformedKeyOrNameException {
        return createNodes(key, null);
    }



    /**
     *
     * @param key
     * @return
     * @throws MalformedKeyOrNameException
     */
    static List<KeyNode> parseKey(String key) throws MalformedKeyOrNameException {
        return parseKey(key, 0);
    }


    /**
     *
     * @param key
     * @param occurrence
     * @return
     * @throws de.dlr.ts.utils.xmladmin2.exceptions.MalformedKeyOrNameException
     */
    static List<KeyNode> parseKey(String key, int occurrence) throws MalformedKeyOrNameException {
        if (key.isEmpty()) {
            throw new MalformedKeyOrNameException("Key cannot be empty");
        }

        if (key.endsWith(".")) {
            throw new MalformedKeyOrNameException("Key cannot end with '.'");
        }

        if (key.startsWith(".")) {
            throw new MalformedKeyOrNameException("Key cannot start with '.'");
        }

        String[] keys = split(key);

        List<KeyNode> nono = new ArrayList<>();

        for (String k : keys) {
            nono.add(new KeyNode(k));
        }

        nono.get(nono.size() - 1).setOccurrence(occurrence);

        return nono;
    }

    /**
     *
     * @param s
     * @return
     * @throws MalformedKeyOrNameException
     */
    private static String[] split(String s) throws MalformedKeyOrNameException {
        ParsingResult cleaned = cleanAttributes(s);
        cleaned.keys = cleaned.string.split("\\.");

        String[] keys = restoreAttributes(cleaned);

        return keys;
    }

    /**
     *
     * @param keys
     * @return
     */
    private static String[] restoreAttributes(ParsingResult res) {
        String[] tmp = new String[res.keys.length];

        for (int i = 0; i < res.keys.length; i++) {
            if (res.keys[i].contains(ParsingResult.ATTS_TOKEN)) {
                while (res.keys[i].contains(ParsingResult.ATTS_TOKEN)) {
                    String get = res.list.remove(0);
                    tmp[i] = res.keys[i].replaceFirst(ParsingResult.ATTS_TOKEN, get);
                    res.keys[i] = tmp[i];
                }
            } else {
                tmp[i] = res.keys[i];
            }
        }

        return tmp;
    }

    /**
     *
     * @param s
     * @return
     */
    static ParsingResult cleanAttributes(String s) throws MalformedKeyOrNameException {
        ParsingResult pr = new ParsingResult();

        int first;
        while ((first = s.indexOf("[")) != -1) {
            int end = s.indexOf("]");
            if (end == -1) {
                throw new MalformedKeyOrNameException("']' expected");
            }

            String subs = s.substring(first, end + 1);
            s = s.replace(subs, ParsingResult.ATTS_TOKEN);
            pr.list.add(subs);
        }

        pr.string = s;

        return pr;
    }

    /**
     *
     * @param s
     * @return
     * @throws MalformedKeyOrNameException
     */
    static Attribute parseKeyAttribute(String s) throws MalformedKeyOrNameException {
        s = s.replace("[", "").trim();
        s = s.substring(0, s.indexOf("]"));

        if (!s.contains("=")) {
            throw new MalformedKeyOrNameException("[XMLReader] No '=' in attribute declaration");
        }

        String name = s.substring(0, s.indexOf("="));
        String value = s.replace(name + "=", "");

        return new Attribute(name, value);
    }


    /**
     * Auxiliary class
     */
    private static class ParsingResult {
        final static String ATTS_TOKEN = "2saddakk2kk441uu467ru73446efd64df9skj###";
        List<String> list = new ArrayList<>();
        String string;

        String[] keys;
    }
}
