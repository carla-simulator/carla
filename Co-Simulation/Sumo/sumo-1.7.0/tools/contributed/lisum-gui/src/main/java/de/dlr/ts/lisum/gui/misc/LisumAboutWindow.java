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
/// @file    LisumAboutWindow.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui.misc;

import de.dlr.ts.commons.javafx.windows.AboutWindow;
import de.dlr.ts.lisum.gui.SystemProperties;
import javafx.scene.image.ImageView;
import javafx.stage.Window;

/**
 *
 * @author @author <a href="mailto:maximiliano.bottazzi@dlr.de">Maximiliano Bottazzi</a>
 */
public final class LisumAboutWindow {
    private final AboutWindow win;

    /**
     *
     * @param owner
     */
    public LisumAboutWindow(Window owner) {
        ImageView iv = new ImageView(SystemProperties.getInstance().getMainIcon());
        iv.setScaleX(.8);
        iv.setScaleY(.8);

        win = new AboutWindow(owner);
        win.setIcon(iv);
        win.setSystemNameString(SystemProperties.getInstance().getSystemName());
        win.setCopyright("\u00A92017 DLR Institute of Transportation Systems");

        StringBuilder sb = new StringBuilder();
        sb.append("Product Version").append("\n");
        sb.append(SystemProperties.getInstance().getSystemName()).append("\n").append("\n");

        sb.append("Contact: Maximiliano Bottazzi (maximiliano.bottazi@dlr.de)").append("\n");

        win.setLicense(sb.toString());
    }

    /**
     *
     */
    public void show() {
        win.show();
    }
}
