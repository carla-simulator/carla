# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2016-2020 German Aerospace Center (DLR) and others.
# SUMOPy module
# Copyright (C) 2012-2017 University of Bologna - DICAM
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    test_notebook.py
# @author  Joerg Schweizer
# @date


#import images
import wx


class PanelOne(wx.Panel):
    """
    This will be the first notebook tab
    """
    # ----------------------------------------------------------------------

    def __init__(self, parent):
        """"""

        wx.Panel.__init__(self, parent=parent, id=wx.ID_ANY)

        sizer = wx.BoxSizer(wx.VERTICAL)
        txtOne = wx.TextCtrl(self, wx.ID_ANY, "")
        txtTwo = wx.TextCtrl(self, wx.ID_ANY, "")

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(txtOne, 0, wx.ALL, 5)
        sizer.Add(txtTwo, 0, wx.ALL, 5)

        self.SetSizer(sizer)


class NestedPanel(wx.Panel):
    """
    This will be the first notebook tab
    """
    # ----------------------------------------------------------------------

    def __init__(self, parent):
        """"""

        wx.Panel.__init__(self, parent=parent, id=wx.ID_ANY)

        sizer = wx.BoxSizer(wx.VERTICAL)

        # Create some nested tabs on the first tab
        nestedNotebook = wx.Notebook(self, wx.ID_ANY)
        nestedTabOne = PanelOne(nestedNotebook)
        nestedTabTwo = PanelOne(nestedNotebook)
        nestedNotebook.AddPage(nestedTabOne, "NestedTabOne")
        nestedNotebook.AddPage(nestedTabTwo, "NestedTabTwo")

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(nestedNotebook, 1, wx.ALL | wx.EXPAND, 5)

        self.SetSizer(sizer)


########################################################################
class NestedNotebookDemo(wx.Notebook):
    """
    Notebook class
    """

    # ----------------------------------------------------------------------
    def __init__(self, parent):
        wx.Notebook.__init__(self, parent, id=wx.ID_ANY, style=wx.BK_DEFAULT
                             # wx.BK_TOP
                             # wx.BK_BOTTOM
                             # wx.BK_LEFT
                             # wx.BK_RIGHT
                             )

        # Create the first tab and add it to the notebook
        tabOne = NestedPanel(self)
        self.AddPage(tabOne, "TabOne")

        # Show how to put an image on one of the notebook tabs,
        # first make the image list:
        il = wx.ImageList(16, 16)
        idx1 = il.Add(images.Smiles.GetBitmap())
        self.AssignImageList(il)

        # now put an image on the first tab we just created:
        self.SetPageImage(0, idx1)

        # Create and add the second tab
        tabTwo = PanelOne(self)
        self.AddPage(tabTwo, "TabTwo")

        # Create and add the third tab
        self.AddPage(PanelOne(self), "TabThree")

        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED, self.OnPageChanged)
        self.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGING, self.OnPageChanging)

    def OnPageChanged(self, event):
        old = event.GetOldSelection()
        new = event.GetSelection()
        sel = self.GetSelection()
        print 'OnPageChanged,  old:%d, new:%d, sel:%d\n' % (old, new, sel)
        event.Skip()

    def OnPageChanging(self, event):
        old = event.GetOldSelection()
        new = event.GetSelection()
        sel = self.GetSelection()
        print 'OnPageChanging, old:%d, new:%d, sel:%d\n' % (old, new, sel)
        event.Skip()


########################################################################
class DemoFrame(wx.Frame):
    """
    Frame that holds all other widgets
    """

    # ----------------------------------------------------------------------
    def __init__(self):
        """Constructor"""
        wx.Frame.__init__(self, None, wx.ID_ANY,
                          "Notebook Tutorial",
                          size=(600, 400)
                          )
        panel = wx.Panel(self)

        notebook = NestedNotebookDemo(panel)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(notebook, 1, wx.ALL | wx.EXPAND, 5)
        panel.SetSizer(sizer)
        self.Layout()

        self.Show()


# ----------------------------------------------------------------------
if __name__ == "__main__":
    app = wx.PySimpleApp()
    frame = DemoFrame()
    app.MainLoop()
