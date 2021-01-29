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
/// @file    CustomToggleButton.java
/// @author  Maximiliano Bottazzi
/// @date    2014
///
//
/****************************************************************************/
package de.dlr.ts.commons.javafx.buttons;

//import de.dlr.ts.display.Commons;
import javafx.beans.value.ObservableValue;
import javafx.scene.Node;
import javafx.scene.control.ToggleButton;
import javafx.scene.control.Tooltip;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public class CustomToggleButton extends ToggleButton {

    public CustomToggleButton() {
    }

    public CustomToggleButton(String text) {
        super(text);
    }

    public CustomToggleButton(String text, Node graphic) {
        super(text, graphic);
    }

    public CustomToggleButton(Node graphic) {
        super("", graphic);
    }

    /**
     *
     * @param imageName
     * @param toolTipText
     */
    public CustomToggleButton(String imageName, String toolTipText) {
        // super("", IconsAndMarkers.instance.getIconImageView(imageName, 16));
        setTooltip(new Tooltip(toolTipText));
    }


    {
        this.selectedProperty().addListener((ObservableValue<? extends Boolean> observable, Boolean oldValue, Boolean newValue) -> {
            /*
            if(newValue)
                getGraphic().setEffect(Commons.instance.toggleButtonEffect);
            else
                getGraphic().setEffect(null);
            */
        });
    }
}
