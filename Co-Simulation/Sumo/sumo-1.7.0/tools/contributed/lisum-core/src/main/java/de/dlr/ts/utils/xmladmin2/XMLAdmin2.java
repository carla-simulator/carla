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
/// @file    XMLAdmin2.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.utils.xmladmin2;

import de.dlr.ts.commons.logger.DLRLogger;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;

/**
 *
 * @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class XMLAdmin2 {

    public final static String xmlDeclarator = "<?xml version='1.0' encoding='UTF-8'?>";

    private DocumentBuilderFactory dbFactory;
    private DocumentBuilder dBuilder;
    private Document doc;
    private Element root;
    private XMLNode rootNode;
    private boolean includeDeclarator = true;
    //static PrintStream err = System.err;

    /**
     * Sets the name of the root node.
     *
     * @param name
     */
    public void setName(final String name) {
        rootNode.setName(name);
    }

    /**
     *
     * @param index
     * @return
     */
    public XMLNode getNode(int index) {
        return rootNode.getNode(index);
    }

    /**
     *
     * @return
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException @throws
     * @throws de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException
     */
    public int getNodesCount() throws MalformedKeyOrNameException, XMLNodeNotFoundException {
        return rootNode.getNodesCount("*");
    }

    static void printException(Throwable throwable) {
        /*
        StringWriter sw = new StringWriter();
        throwable.printStackTrace(new PrintWriter(sw));
        String exceptionAsString = sw.toString();
        err.print(exceptionAsString);
        */

        DLRLogger.severe("XMLAdmin2", throwable);
    }

    static void printException(String message) {
        DLRLogger.severe("XMLAdmin2", message);
    }

    /**
     *
     *
     * @return
     */
    public boolean isIncludeDeclarator() {
        return includeDeclarator;
    }

    /**
     * Sets if the resulting xml will start with the XML declarator
     *
     * @param includeDeclarator
     */
    public void setIncludeDeclarator(boolean includeDeclarator) {
        this.includeDeclarator = includeDeclarator;
    }

    /**
     * Returns the final xml file stored in this XMLAdmin2.
     *
     * @param formatted Determines if the xml will be formatted
     * @return The resulting xml file.
     */
    public String getXML(boolean formatted) {
        int a = 0;

        if (!formatted) {
            a = -1;
        }

        String indent = "";

        if (formatted) {
            indent = "\n";
        }

        if (includeDeclarator) {
            return xmlDeclarator + indent + rootNode.getXML(a);
        } else {
            return rootNode.getXML(a);
        }
    }

    /**
     * Returns the root node of this XMLAdmin2.
     *
     * @return
     */
    public XMLNode getRootNode() {
        return rootNode;
    }

    /**
     * Returns a node from this XMLAdmin2 following the <i>key</i> convention.
     *
     * @param key The key leading to the desired node.
     * @return
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     * @throws de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException
     */
    public XMLNode getNode(String key) throws MalformedKeyOrNameException,
        XMLNodeNotFoundException {
        return rootNode.getNode(key);
    }

    /**
     *
     * @param key
     * @return
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     */
    public boolean hasNode(String key) throws MalformedKeyOrNameException {
        return rootNode.hasNode(key);
    }

    /**
     *
     * @param key
     * @param occurrence
     * @return
     * @throws XMLNodeNotFoundException
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     */
    public XMLNode getNode(String key, int occurrence) throws XMLNodeNotFoundException, MalformedKeyOrNameException {
        return rootNode.getNode(key, occurrence);
    }

    /**
     *
     *
     * @param key
     * @return
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     * @throws de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException
     */
    public int getNodesCount(String key) throws MalformedKeyOrNameException, XMLNodeNotFoundException {
        return rootNode.getNodesCount(key);
    }

    /**
     *
     */
    public XMLAdmin2() {
        try {
            this.rootNode = new XMLNode("defaultName");
        } catch (MalformedKeyOrNameException ex) {
            printException(ex);
        }
        createBuilders();
    }

    /**
     *
     */
    private void createBuilders() {
        dbFactory = DocumentBuilderFactory.newInstance();

        try {
            dBuilder = dbFactory.newDocumentBuilder();
        } catch (ParserConfigurationException ex) {
            printException(ex);
        }
    }

    /**
     * Loads an xml document from a file located in <i>xmlFileName</i>
     *
     * @param xmlFileName the path to the XML file
     * @return
     * @throws SAXException
     * @throws IOException
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     */
    public XMLAdmin2 load(String xmlFileName) throws SAXException, IOException, MalformedKeyOrNameException {
        load(new File(xmlFileName));

        return this;
    }

    /**
     * Loads an xml document from a <i>File</i> object
     *
     * @param file
     * @return the same XMLAdmin2
     * @throws SAXException
     * @throws IOException
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     */
    public XMLAdmin2 load(File file) throws SAXException, IOException, MalformedKeyOrNameException {
        doc = dBuilder.parse(file);
        root = doc.getDocumentElement();
        Tools.extract(root, rootNode);

        return this;
    }

    /**
     * Loads an xml document from an array of bytes
     *
     * @param bytes
     * @return
     * @throws SAXException
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     */
    public XMLAdmin2 load(byte[] bytes) throws SAXException, MalformedKeyOrNameException {
        try {
            dbFactory.setNamespaceAware(true);

            doc = dBuilder.parse(new ByteArrayInputStream(bytes));
            root = doc.getDocumentElement();
            Tools.extract(root, rootNode);

            return this;
        } catch (IOException ex) {
            printException(ex);
        }

        return null;
    }

    /**
     * Loads an xml document from an <i>InputStream</i>
     *
     * @param is
     * @return
     * @throws SAXException
     * @throws de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException
     */
    public XMLAdmin2 load(InputStream is) throws SAXException, MalformedKeyOrNameException {
        try {
            dbFactory.setNamespaceAware(true);
            doc = dBuilder.parse(new InputSource(is));
            root = doc.getDocumentElement();
            Tools.extract(root, rootNode);

            return this;
        } catch (IOException ex) {
            printException(ex);
        }

        return null;
    }

    /**
     *
     * @param fileName
     * @param indentation
     */
    public void saveToDisc(String fileName, boolean indentation) {
        if (!fileName.endsWith(".xml")) {
            fileName += ".xml";
        }

        FileOutputStream fos = null;
        try {
            String xml = getXML(true);

            fos = new FileOutputStream(fileName);
            fos.write(xml.getBytes(), 0, xml.getBytes().length);
            fos.flush();
            fos.close();
        } catch (FileNotFoundException ex) {

        } catch (IOException ex) {

        } finally {
            try {
                fos.close();
            } catch (IOException ex) {
            }
        }
    }

}
