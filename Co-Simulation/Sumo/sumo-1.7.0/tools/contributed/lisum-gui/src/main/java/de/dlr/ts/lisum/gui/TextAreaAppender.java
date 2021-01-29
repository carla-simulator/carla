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
/// @file    TextAreaAppender.java
/// @author  Maximiliano Bottazzi
/// @date    2016
///
//
/****************************************************************************/
package de.dlr.ts.lisum.gui;

import javafx.application.Platform;
import javafx.scene.control.TextArea;

import org.apache.logging.log4j.core.Filter;
import org.apache.logging.log4j.core.Layout;
import org.apache.logging.log4j.core.LogEvent;
import org.apache.logging.log4j.core.appender.AbstractAppender;
import org.apache.logging.log4j.core.config.plugins.PluginAttribute;
import org.apache.logging.log4j.core.config.plugins.PluginElement;
import org.apache.logging.log4j.core.config.plugins.PluginFactory;
import org.apache.logging.log4j.core.layout.PatternLayout;

import java.io.Serializable;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;


/**
 * TextAreaAppender for Log4j 2
 */
public final class TextAreaAppender extends AbstractAppender {

    private static TextArea textArea;

    private final ReadWriteLock rwLock = new ReentrantReadWriteLock();
    private final Lock readLock = rwLock.readLock();

    protected TextAreaAppender(String name, Filter filter,
                               Layout<? extends Serializable> layout,
                               final boolean ignoreExceptions) {
        super(name, filter, layout, ignoreExceptions);
    }

    /**
     * This method is where the appender does the work.
     *
     * @param event Log event with log data
     */
    @Override
    public void append(LogEvent event) {
        readLock.lock();

        final String message = new String(getLayout().toByteArray(event));

        // append log text to TextArea
        try {
            Platform.runLater(() -> {
                try {
                    if (textArea != null) {
                        if (textArea.getText().length() == 0) {
                            textArea.setText(message);
                        } else {
                            textArea.selectEnd();
                            textArea.insertText(textArea.getText().length(),
                                                message);
                        }
                    }
                } catch (final Throwable t) {
                    System.out.println("Error while append to TextArea: "
                                       + t.getMessage());
                }
            });
        } catch (final IllegalStateException ex) {
            ex.printStackTrace(System.out);
        } finally {
            readLock.unlock();
        }
    }

    /**
     * Factory method. Log4j will parse the configuration and call this factory method to construct the appender with the configured attributes.
     *
     * @param name Name of appender
     * @param layout Log layout of appender
     * @param filter Filter for appender
     * @return The TextAreaAppender
     */
    @PluginFactory
    public static TextAreaAppender createAppender(
        @PluginAttribute("name") String name,
        @PluginElement("Layout") Layout<? extends Serializable> layout,
        @PluginElement("Filter") final Filter filter) {
        if (name == null) {
            LOGGER.error("No name provided for TextAreaAppender");
            return null;
        }
        if (layout == null) {
            layout = PatternLayout.createDefaultLayout();
        }
        return new TextAreaAppender(name, filter, layout, true);
    }

    /**
     * Set TextArea to append
     *
     * @param textArea TextArea to append
     */
    public static void setTextArea(TextArea textArea) {
        TextAreaAppender.textArea = textArea;
    }
}
