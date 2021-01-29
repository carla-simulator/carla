#!/usr/bin/env python
# -*- coding: utf8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2010-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    launcher.py
# @author  Jakob Erdmann
# @date    2015-01-18

from __future__ import absolute_import
import os
import re
import subprocess
from Tkinter import Canvas, Menu, StringVar, Tk, LEFT
from ttk import Button, Frame, Menubutton, Label, Scrollbar, Entry
import tkFileDialog
from sumolib.options import Option, readOptions

THISDIR = os.path.dirname(__file__)
BINDIR = os.path.join(THISDIR, '..', 'bin')

APPLICATIONS = ['netconvert', 'netgenerate', 'polyconvert', 'od2trips', 'duarouter', 'jtrrouter',
                'dfrouter', 'marouter', 'sumo', 'sumo-gui', 'activitygen']


class ResizingCanvas(Canvas):

    """ a subclass of Canvas for dealing with resizing of windows
    http://stackoverflow.com/questions/22835289/how-to-get-tkinter-canvas-to-dynamically-resize-to-window-width
    """

    def __init__(self, parent, **kwargs):
        Canvas.__init__(self, parent, **kwargs)
        self.bind("<Configure>", self.on_resize)
        self.height = self.winfo_reqheight()
        self.width = self.winfo_reqwidth()

    def on_resize(self, event):
        # determine the ratio of old width/height to new width/height
        wscale = float(event.width) / self.width
        hscale = float(event.height) / self.height
        self.width = event.width - 2
        self.height = event.height - 2
        # print "on_resize %s %s %s %s" % (self.width, self.height,
        #        self.winfo_reqwidth(), self.winfo_reqheight())
        # resize the canvas
        self.config(width=self.width, height=self.height)
        # rescale all the objects tagged with the "all" tag
        self.scale("all", 0, 0, wscale, hscale)


class ScrollableFrame(Frame):

    def __init__(self, root):
        Frame.__init__(self, root)
        self.canvas = ResizingCanvas(self, borderwidth=0)
        self.frame = Frame(self.canvas)
        self.vsb = Scrollbar(
            self, orient="vertical", command=self.canvas.yview)
        self.canvas.configure(yscrollcommand=self.vsb.set)
        self.vsb.pack(side="right", fill="y")
        self.canvas.pack(side="left", fill="both", expand=True)
        self.canvas.create_window(
            (4, 4), window=self.frame, anchor="nw", tags="self.frame")
        self.frame.bind("<Configure>", self.OnFrameConfigure)

    def OnFrameConfigure(self, event):
        '''Reset the scroll region to encompass the inner frame'''
        # print "OnFrameConfigure"
        self.canvas.configure(scrollregion=self.canvas.bbox("all"))


def buildValueWidget(frame, optType):
    if optType == "FOO":
        pass
    else:
        var = StringVar()
        widget = Entry(frame, textvariable=var)
        return widget, var


class Launcher:

    def __init__(self, root, app, appOptions):
        self.title_prefix = "SUMO Application launcher"
        self.root = root
        self.appVar = StringVar()
        self.appVar.set(app)
        self.appOptions = appOptions
        self.optionValues = {}

        self.root.title(self.title_prefix)
        self.root.minsize(700, 200)
        numButtons = self.mainButtons()
        for i in range(numButtons):
            root.columnconfigure(i, weight=1)
        root.rowconfigure(0, weight=10)
        root.rowconfigure(1, weight=1)

        sFrame = ScrollableFrame(root)
        sFrame.grid(row=1, column="0", columnspan=numButtons, sticky="NSEW")
        self.optFrame = sFrame.frame

        self.buildAppOptions(appOptions)

        # define options for opening or saving a file
        self.file_opt = options = {}
        self.filedir = os.getcwd()
        options['defaultextension'] = 'cfg'
        options['filetypes'] = [('all files', '.*')]
        options['initialdir'] = self.filedir
        options['parent'] = root

    def buildAppOptions(self, appOptions):
        NAME, VALUE, HELP = range(3)
        row = 0
        for o in appOptions:
            row += 1
            Label(self.optFrame, text=o.name).grid(
                row=row, column=NAME, sticky="NW")
            widget, var = buildValueWidget(self.optFrame, o.type)
            self.optionValues[o.name] = var
            widget.grid(row=row, column=VALUE, sticky="NW")
            Label(self.optFrame, text=o.help, justify=LEFT).grid(
                row=row, column=HELP, sticky="NW")

    def mainButtons(self):
        row = 0
        col = 0
        self.buttons = []

        mb = Menubutton(self.root, text="Select Application")
        mb.menu = Menu(mb, tearoff=0)
        mb["menu"] = mb.menu
        for app in APPLICATIONS:
            mb.menu.add_radiobutton(label=app, variable=self.appVar,
                                    command=self.onSelectApp)
        mb.grid(row=row, column=col, sticky="NEW")
        col += 1

        self.buttons.append(mb)
        otherButtons = (
            ("Run %12s" % self.appVar.get(), self.runApp),
            ("load Config", self.loadCfg),
            ("Save Config", self.saveCfg),
            ("Save Config as", self.saveCfgAs),
            ("Quit", self.root.quit),
        )

        for text, command in otherButtons:
            self.buttons.append(Button(self.root, text=text, command=command))
            self.buttons[-1].grid(row=row, column=col, sticky="NEW")
            col += 1
        return len(self.buttons)

    def onSelectApp(self):
        self.buttons[1].configure(text="Run %12s" % self.appVar.get())

    def runApp(self):
        subprocess.call(os.path.join(BINDIR, self.appVar.get()))

    def loadCfg(self):
        self.file_opt['title'] = 'Load configuration file'
        filename = tkFileDialog.askopenfilename(**self.file_opt)
        self.root.title(self.title_prefix + " " + filename)
        self.loadedOptions = readOptions(filename)
        for o in self.loadedOptions:
            self.optionValues[o.name].set(o.value)

    def saveCfg(self):
        pass

    def saveCfgAs(self):
        pass


def parse_help(app):
    binary = os.path.join(BINDIR, app)
    reOpt = re.compile(r"--([^ ]*) (\w*) (.*$)")
    helpstring = subprocess.check_output([binary, '--help'])
    options = []
    optName = None
    optHelp = ""
    optType = ""
    for line in helpstring.split(os.linesep):
        if '--' in line:
            if optName is not None:
                options.append(Option(optName, None, optType, optHelp))
            match = reOpt.search(line)
            if match is not None:
                optName = match.group(1)
                optType = match.group(2)
                optHelp = match.group(3).strip()
        elif "                                     " in line:
            optHelp += "\n" + line.strip()
    if optName is not None:
        if optType == '':
            optType = 'BOOL'
        options.append(Option(optName, None, optType, optHelp))

    return options


def main():
    app = "netconvert"
    appOptions = parse_help(app)
    # appOptions = []
    root = Tk()
    app = Launcher(root, app, appOptions)
    root.mainloop()


if __name__ == "__main__":
    main()
