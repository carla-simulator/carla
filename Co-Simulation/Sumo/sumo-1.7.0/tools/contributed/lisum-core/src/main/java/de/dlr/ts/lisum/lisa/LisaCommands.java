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
/// @file    LisaCommands.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;

import de.dlr.ts.commons.logger.DLRLogger;
import de.dlr.ts.commons.logger.ToString;
import de.dlr.ts.lisum.exceptions.LisaRESTfulServerNotFoundException;
import de.dlr.ts.utils.xmladmin2.XMLAdmin2;
import de.dlr.ts.utils.xmladmin2.MalformedKeyOrNameException;
import de.dlr.ts.utils.xmladmin2.XMLNodeNotFoundException;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import org.apache.http.HttpResponse;
import org.apache.http.ProtocolVersion;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.HttpClients;
import org.xml.sax.SAXException;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano
 * Bottazzi</a>
 */
final class LisaCommands {

    private static int lisaServerPort;
    private static String lisaServerAddress;

    public static final String GET_TASK_URI = "/services/PDService/getTask";
    private static final String SET_DATA_DIR_URI = "/services/DDService/setDataDir";
    private static final String SET_TASK_LIST_URI = "/services/PDService/setTaskList";
    private static final String GET_TASK_LIST_URI = "/services/PDService/getTaskList";
    private static final String GET_OBJECT_LIST_URI = "/services/PDService/getObjectList";
    private static final String REMOVE_TASK_LIST_URI = "/services/PDService/removeTaskList";
    private static final String PUT_MESSAGE_CALLBACK_URI = "/services/PDCallback/putMessage";

    private static final HttpClient HTTP_CLIENT = HttpClients.createDefault();

    /**
     *
     */
    LisaCommands(String lsa, int lsp) {
        lisaServerPort = lsp;
        lisaServerAddress = lsa;
    }

    /**
     *
     * @return
     */
    @Override
    public String toString() {
        ToString ts = new ToString("LisaCommands");

        return ts.add("lisaServerAddress", lisaServerAddress).add("lisaServerPort", lisaServerPort).toString();
    }

    /**
     *
     * @return
     */
    private static String getURI() {
        return "http://" + lisaServerAddress + ":" + lisaServerPort;
    }

    /**
     *
     * @return
     */
    public static boolean TestRESTfulServer() {
        try {
            /**
             * Sending anything to the server in order to test it.
             */
            HttpPost post = new HttpPost(getURI() + GET_OBJECT_LIST_URI);
            String request = Requests.INSTANCE.getObjectListRequest(1, 1);
            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            execute(post);
//        } catch (UnsupportedEncodingException ex) {
//            DLRLogger.severe("", ex);
        } catch (LisaRESTfulServerNotFoundException ex) {
            return false;
        }

        return true;
    }

    /**
     * This does not work
     *
     * @param id
     */
    public void getTask(int id) throws LisaRESTfulServerNotFoundException {
//        try {
            //HttpPost post = new HttpPost(lisaServerAddress + PUT_MESSAGE_SERVICE_URI);
            HttpPost post = new HttpPost(getURI() + PUT_MESSAGE_CALLBACK_URI);
            String request = Requests.INSTANCE.getGetTaskRequest(id);
            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            DLRLogger.finer(this, request);

            Response response = execute(post);
            DLRLogger.finer(this, response.statusCode + " | " + response.body);
//        } catch (UnsupportedEncodingException ex) {
//            DLRLogger.severe(this, ex);
//        }
    }

    /**
     *
     * @param message
     * @return
     */
    public PutMessageResponse putMessage(String message) throws LisaRESTfulServerNotFoundException {
        PutMessageResponse vektor = null;

//        try {
            //HttpPost post = new HttpPost(lisaServerAddress + PUT_MESSAGE_SERVICE_URI);
            HttpPost post = new HttpPost(getURI() + PUT_MESSAGE_CALLBACK_URI);
            String request = Requests.INSTANCE.getPutMessageRequest(message);
            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            DLRLogger.finer(this, "Sending requesto to Lisa: " + request);

            Response response = execute(post);
            DLRLogger.finest(this, "Lisa responds: " + response.statusCode + " | " + response.body);

            if (response.statusCode == 200) {
                vektor = new PutMessageResponse(response.body);
            }

//        } catch (UnsupportedEncodingException ex) {
//            DLRLogger.severe(this, ex);
//        }

        return vektor;
    }

    /**
     *
     * @param zNr
     * @param fNr
     * @param interval
     * @return
     */
    public int setTask(int zNr, int fNr, int interval) throws LisaRESTfulServerNotFoundException {
        int newTaskId = 0;
        try {
            HttpPost post = new HttpPost(getURI() + SET_TASK_LIST_URI);
            String request = Requests.INSTANCE.getSetTaskRequest(zNr, fNr, interval);
            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            Response response = execute(post);
            DLRLogger.finer(this, response.statusCode + " | " + response.body);

            XMLAdmin2 x = new XMLAdmin2().load(response.body.getBytes());
            newTaskId = x.getNode("ns2:ID").getValue(0);
        } catch (/* UnsupportedEncodingException |*/ SAXException | MalformedKeyOrNameException | XMLNodeNotFoundException ex) {
            DLRLogger.severe(this, ex);
        }

        return newTaskId;
    }

    /**
     *
     * @return
     */
    public String getObjectList() throws LisaRESTfulServerNotFoundException {
//        try {
            HttpPost post = new HttpPost(getURI() + GET_OBJECT_LIST_URI);
            String request = Requests.INSTANCE.getObjectListRequest(1, 1);
            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            Response response = execute(post);
            DLRLogger.finer(this, response.statusCode + " | " + response.body);

            return response.body;
//        } catch (UnsupportedEncodingException ex) {
//            DLRLogger.severe(this, ex);
//        }

//        return "";
    }

    /**
     *
     * @param id
     */
    public void removeTaskList(int id) throws LisaRESTfulServerNotFoundException {
//        try {
            HttpPost post = new HttpPost(getURI() + REMOVE_TASK_LIST_URI);

            String request = Requests.INSTANCE.getRemoveTaskListRequest(id);
            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            Response execute = execute(post);
            DLRLogger.finer(this, execute.statusCode + " | " + execute.body);
//        } catch (UnsupportedEncodingException ex) {
//            DLRLogger.severe(this, ex);
//        }

    }

    /**
     *
     * @param zNr
     * @param fNr
     * @return The id of the task list or -1 if not found.
     */
    public int[] getTaskList(int zNr, int fNr) throws LisaRESTfulServerNotFoundException {
        int[] ids = null;
        try {
            String uri = GET_TASK_LIST_URI;
            HttpPost post = new HttpPost(getURI() + uri);

            String request = Requests.INSTANCE.getTaskListRequest(zNr, fNr);
            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            Response execute = execute(post);
            DLRLogger.finer(this, execute.statusCode + " | " + execute.body);

            XMLAdmin2 x = new XMLAdmin2().load(execute.body.getBytes());

            //System.out.println(x.getXML(true));
            int nodesCount = x.getNodesCount("ns2:TaskInfo");
            ids = new int[nodesCount];

            for (int i = 0; i < nodesCount; i++) {
                int id = x.getNode("ns2:TaskInfo", i).getNode("ns2:ID").getValue(-1);
                ids[i] = id;
            }
        } catch (/* UnsupportedEncodingException |*/ SAXException | MalformedKeyOrNameException | XMLNodeNotFoundException ex) {
            DLRLogger.severe(this, ex);
        }

        return ids;
    }

    /**
     *
     * @param dataDir
     * @return
     */
    public void setDataDir(final File dataDir) throws LisaRESTfulServerNotFoundException {
//        try {
            HttpPost post = new HttpPost(getURI() + SET_DATA_DIR_URI);
            String request = Requests.INSTANCE.getSetDataDirRequest(dataDir.getAbsolutePath());

            StringEntity entity = new StringEntity(request, "UTF-8");
            post.setEntity(entity);

            Response execute = execute(post);

            if (execute.statusCode == 200) {
                DLRLogger.config(this, "Lisa DataDir set successfully.");
            }
//        } catch (UnsupportedEncodingException ex) {
            //DLRLogger.severe(this, ex);
//            throw new LisaRESTfulServerNotFoundException();
//        }
    }

    /**
     *
     * @param post
     * @return
     */
    static Response execute(HttpPost post) throws LisaRESTfulServerNotFoundException {
        try {
            byte[] responseBody;
            int statusCode;
            String reasonPhrase;
            ProtocolVersion protocolVersion;

            HttpResponse execute = HTTP_CLIENT.execute(post);
            reasonPhrase = execute.getStatusLine().getReasonPhrase();
            statusCode = execute.getStatusLine().getStatusCode();
            protocolVersion = execute.getStatusLine().getProtocolVersion();

            ByteArrayOutputStream outstream = new ByteArrayOutputStream();
            execute.getEntity().writeTo(outstream);
            responseBody = outstream.toByteArray();

            Response response = new Response();
            response.statusCode = statusCode;
            response.body = new String(responseBody);

            return response;
        } catch (IOException ex) {
            throw new LisaRESTfulServerNotFoundException("");
        }
    }

    /**
     *
     */
    private static class Response {
        int statusCode;
        String body;
    }
}
