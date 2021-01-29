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
/// @file    Requests.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.lisa;


/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
class Requests {
    public final static Requests INSTANCE = new Requests();
    private final static String SCHLOT = "\"http://www.schlothauer.de/omtc/services\"";



    /**
     * This does not work
     *
     * @param id
     * @return
     */
    String getGetTaskRequest(int id) {
        StringBuilder sb = new StringBuilder();
        sb.append("<GetTaskRequest xmlns=" + SCHLOT + ">");
        sb.append("<ID>");
        sb.append(id);
        sb.append("</ID>");
        sb.append("</GetTaskRequest>");

        return sb.toString();
    }

    String getRemoveTaskListRequest(int id) {
        StringBuilder sb = new StringBuilder();
        sb.append("<RemoveTaskRequest xmlns=" + SCHLOT + ">");
        sb.append("<ID>");
        sb.append(id);
        sb.append("</ID>");
        sb.append("</RemoveTaskRequest>");

        return sb.toString();
    }

    String getPutMessageRequest(String message) {
        StringBuilder sb = new StringBuilder();
        sb.append(getHeader("Message"));
        sb.append("<Msg>");
        sb.append(message);
        sb.append("</Msg>");
        sb.append("</Message>");

        return sb.toString();
    }

    /*
    <StgKennung><ZNr>1</ZNr><FNr>1</FNr>
    </StgKennung><Callback><URL>http://localhost:9010</URL></Callback>
    <Cycle><IntervallSec>60</IntervallSec></Cycle>
    <TaskElement><MessageType>MeldungType</MessageType></TaskElement>
    <TaskElement><MessageType>WunschVektorType</MessageType></TaskElement>
    <TaskElement><MessageType>DetFlType</MessageType></TaskElement>
    <TaskElement><MessageType>OevTelegrammType</MessageType></TaskElement>
    <TaskElement><MessageType>APWertZustType</MessageType></TaskElement>
    <TaskElement><MessageType>IstvektorProjType</MessageType></TaskElement></SetTaskRequest>
    */
    String getSetTaskRequest(int zNr, int fNr, int intervall) {
        StringBuilder sb = new StringBuilder();
        sb.append(getHeader("SetTaskRequest"));
        sb.append("<ID>").append(0).append("</ID>");
        sb.append("<StgKennung>");
        sb.append("<ZNr>").append(zNr).append("</ZNr>");
        sb.append("<FNr>").append(fNr).append("</FNr>");
        sb.append("</StgKennung>");
        sb.append("<Callback><URL>http://localhost:9010</URL></Callback>"); //not necessary
        sb.append("<Cycle><IntervallSec>").append(intervall).append("</IntervallSec></Cycle>"); //not clear what for
        sb.append("<TaskElement><MessageType>MeldungType</MessageType></TaskElement>");
        sb.append("<TaskElement><MessageType>WunschVektorType</MessageType></TaskElement>");
        sb.append("<TaskElement><MessageType>DetFlType</MessageType></TaskElement>");
        sb.append("<TaskElement><MessageType>OevTelegrammType</MessageType></TaskElement>");
        sb.append("<TaskElement><MessageType>APWertZustType</MessageType></TaskElement>");
        sb.append("<TaskElement><MessageType>IstvektorProjType</MessageType></TaskElement></SetTaskRequest>");

        sb.append("</SetTaskRequest>");

        return sb.toString();
    }



    /**
     * <ObjectListRequest xmlns="http://www.schlothauer.de/omtc/services">
     * <StgKennung><ZNr>1</ZNr><FNr>1</FNr></StgKennung></ObjectListRequest>
     *
     * @param zNr
     * @param fNr
     * @return
     */
    String getObjectListRequest(int zNr, int fNr) {
        StringBuilder sb = new StringBuilder();
        sb.append(getHeader("ObjectListRequest"));
        sb.append("<StgKennung>");
        sb.append("<ZNr>").append(zNr).append("</ZNr>");
        sb.append("<FNr>").append(fNr).append("</FNr>");
        sb.append("</StgKennung>");
        sb.append("</ObjectListRequest>");

        return sb.toString();
    }

    private String getHeader(String request) {
        return "<" + request + " xmlns=" + SCHLOT + ">";
    }

    /**
     *
     * @param uri
     * @return
     */
    String getSetDataDirRequest(String uri) {
        StringBuilder sb = new StringBuilder();
        sb.append(getHeader("SetDataDirRequest"));
        sb.append("<Value>");
        sb.append(uri);
        sb.append("</Value>");
        sb.append("</SetDataDirRequest>");

        return sb.toString();
    }

    /**
     * Este es verso
     *
     * @return
     */
    String getMessageTypeListRequest() {
        StringBuilder sb = new StringBuilder();
        sb.append("<SetTaskRequest xmlns=" + SCHLOT + ">");
        sb.append("</SetTaskRequest>");

        return sb.toString();
    }


    /**
     *
     * @return
     */
    String getTaskListRequest(int zNr, int fNr) {
        StringBuilder sb = new StringBuilder();
        sb.append("<GetTaskListRequest xmlns=" + SCHLOT + ">");

        sb.append("<Detail>").append("false").append("</Detail>");

        sb.append("<StgKennung>");
        sb.append("<ZNr>").append(zNr).append("</ZNr>");
        sb.append("<FNr>").append(fNr).append("</FNr>");
        sb.append("</StgKennung>");

        sb.append("</GetTaskListRequest>");

        return sb.toString();
    }
}
