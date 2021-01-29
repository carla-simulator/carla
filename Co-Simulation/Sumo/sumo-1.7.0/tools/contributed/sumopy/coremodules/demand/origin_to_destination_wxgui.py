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

# @file    origin_to_destination_wxgui.py
# @author  Joerg Schweizer
# @date

import wx


import agilepy.lib_base.classman as cm
import agilepy.lib_base.arrayman as am
from agilepy.lib_base.misc import get_inversemap
from agilepy.lib_wx.ogleditor import *
from agilepy.lib_wx.objpanel import ObjPanel
from coremodules.network.network import SumoIdsConf, MODES


class OdCommonMixin:
    def add_odoptions_common(self, modes=None, activitytypes=None):
        # print 'add_odoptions_common',modes
        self.add(am.AttrConf('t_start', 0,
                             groupnames=['options'],
                             perm='rw',
                             name='Start time',
                             unit='s',
                             info='Start time of interval',
                             ))

        self.add(am.AttrConf('t_end', 3600,
                             groupnames=['options'],
                             perm='rw',
                             name='End time',
                             unit='s',
                             info='End time of interval',
                             ))

        # here we ged classes not vehicle type
        # specific vehicle type within a class will be generated later
        if modes is None:
            modechoices = {'': -1}
        else:
            modechoices = modes.names.get_indexmap()
        # print '  modechoices',modechoices
        self.add(am.AttrConf('id_mode',   -1,
                             groupnames=['options'],
                             choices=modechoices,
                             name='Mode',
                             info='Transport mode.',
                             ))

        self.add(cm.AttrConf('scale', 1.0,
                             groupnames=['options'],
                             perm='rw',
                             name='Scale',
                             info='Scale demand by this factor before adding. Value od 1.0 means no scaling.'
                             ))

        if activitytypes is None:
            activitytypechoices = {'': -1}
            id_act_orig = -1
            id_act_dest = -1
        else:
            activitytypechoices = activitytypes.names.get_indexmap()
            id_act_orig = activitytypes.names.get_id_from_index('home')
            id_act_dest = activitytypes.names.get_id_from_index('work')

        self.add(cm.AttrConf('id_activitytype_orig', id_act_orig,
                             groupnames=['options'],
                             choices=activitytypechoices,
                             perm='rw',
                             name='Activity at orig.',
                             info='Activity type at origin.',
                             ))

        self.add(cm.AttrConf('id_activitytype_dest', id_act_dest,
                             groupnames=['options'],
                             choices=activitytypechoices,
                             perm='rw',
                             name='Activity at dest.',
                             info='Activity type at destination.',
                             ))


class OdFlowsWxGuiMixin:
    """Contains OdFlow spacific functions that communicate between the widgets of the main wx gui
    and the functions of the plugin.
    """

    def refresh_odflow(self, is_refresh):
        if is_refresh:
            neteditor = self.get_neteditor()
            #neteditor.add_tool(AddODflowTool(self, odintervals))
            neteditor.add_toolclass(AddODflowTool)

    def add_menu_odflows(self, menubar):
        menubar.append_menu('demand/Zone-to-zone demand',
                            bitmap=self.get_icon("fig_od_24px.png"),
                            )
        menubar.append_item('demand/Zone-to-zone demand/add zone-to-zone flows...',
                            self.on_add_odtrips,
                            info='Add or import trips between origin and destination zones, with a certain mode during a certain time interval.',
                            bitmap=self.get_agileicon("Document_Import_24px.png"),
                            )

        menubar.append_item('demand/Zone-to-zone demand/generate trips from flows',
                            self.on_generate_odtrips,
                            # info=self.on_generate_odtrips.__doc__.strip(),
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )

        menubar.append_item('demand/Zone-to-zone demand/generate routes from flows, if connected',
                            self.on_generate_odroutes,
                            # info=self.on_generate_odtrips.__doc__.strip(),
                            #bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_SAVE_AS,wx.ART_MENU),
                            )

        menubar.append_item('demand/Zone-to-zone demand/clear zone-to-zone flows',
                            self.on_clear_odtrips,
                            info='Clear all zone to zone trips.',
                            bitmap=wx.ArtProvider.GetBitmap(wx.ART_DELETE, wx.ART_MENU),
                            )

    def on_add_odtrips(self, event=None):
        """
        Opend odm wizzard
        """
        dlg = AddOdDialog(self._mainframe, self._demand.odintervals)
        dlg.Show()
        dlg.MakeModal(True)
        self._mainframe.browse_obj(self._demand.odintervals)
        # self.scenariopanel.refresh_panel(self.scenario)

    def on_generate_odtrips(self, event=None):
        """
        Generates trips from origin to destination zone from current OD matrices.
        """
        self._demand.odintervals.generate_trips(n_trials_connect=-1,
                                                is_make_route=False,)
        self._mainframe.browse_obj(self._demand.trips)

    def on_generate_odroutes(self, event=None):
        """
        Generates routes from origin to destination zone from current OD matrices, if connected.
        """
        self._demand.odintervals.generate_trips(n_trials_connect=5,
                                                is_make_route=True,)
        self._mainframe.browse_obj(self._demand.trips)

    def on_clear_odtrips(self, event=None):
        """
        Generates trips from origin to destination zone from current OD matrices.
        """
        self._demand.odintervals.clear_od_trips()
        self._mainframe.browse_obj(self._demand.odintervals)


class AddODflowTool(OdCommonMixin, SelectTool):
    """
    OD flow toolfor OGL canvas.
    """

    def __init__(self, parent, mainframe=None):
        """
        To be overridden by specific tool.
        """
        self.init_common('odflow', parent, 'Add OD flow',
                         info="""Tool to add a flow between a zone of origin and a zone of destination:
                            <LEFT MOUSE> on the respective borderlines of zone of origin, and zone of destination.
                            Enter time interval and transport mode.
                            Enter number of trips and press "add flows".
                            <LEFT MOUSE> on border of a new zone of destination to add flows to other zones of destination.
                            Or press button "Reset zones" to start with a new assignment.
                            """,
                         is_textbutton=False,
                         )

        self._init_select(is_show_selected=False)

        #self.drawobj_zone_orig = None
        #self.drawobj_zone_dest = None
        self.add_odoptions_common()
        # make options
        self.add(cm.AttrConf('id_orig', -1,
                             groupnames=['options'],
                             choices={'': -1},
                             perm='r',
                             name='Orig zone',
                             info='Name of traffic assignment zone of origin of trip. Click on zone border to specify.',
                             ))

        self.add(cm.AttrConf('id_dest', -1,
                             dtype='object',
                             groupnames=['options'],
                             choices={'': -1},
                             perm='r',
                             name='Dest zone',
                             info='Name of traffic assignment zone of destination of trip.Click on zone border to specify.',
                             ))

        self.add(cm.AttrConf('tripnumber', 0,
                             groupnames=['options'],
                             perm='rw',
                             name='Trips',
                             info='Number of trips from zone of origin to zone of destination.',
                             ))

    def set_button_info(self, bsize=(32, 32)):
        # print 'set_button_info select tool'  self.get_icon("icon_sumo_24px.png")
        iconpath = os.path.join(os.path.dirname(__file__), 'images')
        self._bitmap = wx.Bitmap(os.path.join(iconpath, 'fig_od_32px.png'), wx.BITMAP_TYPE_PNG)
        self._bitmap_sel = self._bitmap

    def set_cursor(self):
        # http://www.wxpython.org/docs/api/wx.Cursor-class.html
        if self._canvas is not None:
            self._canvas.SetCursor(wx.StockCursor(wx.CURSOR_RIGHT_ARROW))

    def activate(self, canvas=None):
        """
        This call by metacanvas??TooldsPallet signals that the tool has been
        activated and can now interact with metacanvas.
        """
        # print 'activate'
        SelectTool.activate(self, canvas)

        zones = self.get_zones()
        zonenames = zones.ids_sumo.get_indexmap()
        zonenames[''] = -1
        self.id_orig.choices = zonenames
        self.id_dest.choices = zonenames

        modechoices = self.get_scenario().demand.vtypes.get_modechoices()
        self.id_mode.set_value(modechoices['passenger'])
        self.id_mode.choices = modechoices

        activitytypes = self.get_scenario().demand.activitytypes
        self.id_activitytype_orig.choices = activitytypes.names.get_indexmap()
        self.id_activitytype_dest.choices = activitytypes.names.get_indexmap()
        self.id_activitytype_orig.set_value(activitytypes.names.get_id_from_index('home'))
        self.id_activitytype_dest.set_value(activitytypes.names.get_id_from_index('work'))

        self.highlight_zones()
        canvas.draw()

    def deactivate(self):
        """
        This call by metacanvas signals that the tool has been
        deactivated and can now interact with metacanvas.
        """

        self._is_active = False
        self.unhighlight_zones()
        self._canvas.draw()
        self.deactivate_select()

    def on_execute_selection(self, event):
        """
        Definively execute operation on currently selected drawobjects.
        """
        # print 'AddODflowTool.on_execute_selection',self.get_netelement_current()
        # self.set_objbrowser()
        # self.highlight_current()
        self.unhighlight_current()
        self.unhighlight_zones()
        netelement_current = self.get_netelement_current()
        if netelement_current is not None:
            (zones, id_zone) = netelement_current
            if zones.get_ident() == 'zones':
                # print '  check',self.name_orig.get_value(),'*',self.name_orig.get_value() is ''
                if self.id_orig.get_value() < 0:
                    # print '    set name_orig',zones.ids_sumo[id_zone]
                    self.id_orig.set_value(id_zone)

                else:
                    # print '    set name_dest',zones.ids_sumo[id_zone]
                    self.id_dest.set_value(id_zone)

                self.unselect_all()  # includes unhighlight
                self.highlight_zones()
                self.parent.refresh_optionspanel(self)
        return True

    def highlight_zones(self):
       # print 'highlight_zones',self.id_orig.value,self.id_dest.value
        zones = self.get_zones()
        drawing = self.get_drawing()
        if self.id_orig.value >= 0:
            drawing.highlight_element(zones, [self.id_orig.value], is_update=True)
        if self.id_dest.value >= 0:
            drawing.highlight_element(zones, [self.id_dest.value], is_update=True)

    def unhighlight_zones(self):
        zones = self.get_zones()
        drawing = self.get_drawing()
        if self.id_orig.value >= 0:
            drawing.unhighlight_element(zones, self.id_orig.value, is_update=True)
        if self.id_dest.value >= 0:
            drawing.unhighlight_element(zones, self.id_dest.value, is_update=True)

    def on_change_selection(self, event):
        """
        Called after selection has been changed with SHIFT-click
        Do operation on currently selected drawobjects.
        """
        # self.set_objbrowser()
        return False

    def get_netelement_current(self):
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            drawobj, _id = self.get_current_selection()
            if drawobj is not None:
                obj = drawobj.get_netelement()
                return obj, _id
            else:
                return None
        else:
            return None

    def get_scenario(self):
        # get net and scenario via netdrawing
        return self.get_drawing().get_net().parent

    def get_odintervals(self):
        return self.get_scenario().demand.odintervals

    def get_zones(self):
        return self.get_scenario().landuse.zones

    def on_add_new(self, event=None):
        self._optionspanel.apply()
        zonenames = self.get_zones().ids_sumo
        # print 'on_add_new',
        # print '  odintervals',self.get_odintervals(),dir(self.get_odintervals())
        odtrips = self.get_odintervals().add_od_flow(self.t_start.value, self.t_end.value,
                                                     self.id_mode.value,
                                                     self.id_activitytype_orig.value,
                                                     self.id_activitytype_dest.value,
                                                     self.scale.value,
                                                     zonenames[self.id_orig.value],
                                                     zonenames[self.id_dest.value],
                                                     self.tripnumber.value,
                                                     )
        self.unselect_all()
        self.unhighlight_zones()
        mainframe = self.parent.get_mainframe()
        if mainframe is not None:
            mainframe.browse_obj(odtrips)

        self.id_dest.set_value(-1)  # set empty zone
        self.highlight_zones()
        self.parent.refresh_optionspanel(self)
        self._canvas.draw()

    def on_clear_zones(self, event=None):
        self.unhighlight_zones()
        self.id_orig.set_value(-1)  # set empty zone
        self.id_dest.set_value(-1)  # set empty zone
        self.unselect_all()

        self.parent.refresh_optionspanel(self)
        self._canvas.draw()

    def get_optionspanel(self, parent, size=wx.DefaultSize):
        """
        Return tool option widgets on given parent
        """
        size = (200, -1)
        buttons = [('Add flow', self.on_add_new, 'Add a new OD flow to demand.'),
                   ('Rest Zones', self.on_clear_zones, 'Clear the fields with zones of origin and destination.'),
                   #('Save flows', self.on_add, 'Save OD flows to current demand.'),
                   #('Cancel', self.on_close, 'Close wizzard without adding flows.'),
                   ]
        defaultbuttontext = 'Add flow'
        self._optionspanel = ObjPanel(parent, obj=self,
                                      attrconfigs=None,
                                      groupnames=['options'],
                                      func_change_obj=None,
                                      show_groupnames=False, show_title=True, is_modal=False,
                                      mainframe=self.parent.get_mainframe(),
                                      pos=wx.DefaultPosition, size=size, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                                      immediate_apply=True, panelstyle='default',  # 'instrumental'
                                      buttons=buttons, defaultbutton=defaultbuttontext,
                                      standartbuttons=[],  # standartbuttons=['restore']
                                      )

        return self._optionspanel


class AddOdWizzard(OdCommonMixin, am.ArrayObjman):
    """Contains information and methods to add an od matrix for 
    a certain mode and for a certain time interval to the scenario.
    """

    def __init__(self, odintervals):
        # print 'AddOdWizzard',odintervals#,odintervals.times_start
        # print ' dir(odintervals)',dir(odintervals)
        zones = odintervals.get_zones()

        self._init_objman('odm_adder', parent=odintervals,
                          name='ODM Wizzard',
                          info='Wizzard to add origin zone to destination zone demand informations.',
                          )

        self.add_odoptions_common(odintervals.parent.get_scenario().net.modes, odintervals.parent.activitytypes)

        self.add_col(am.ArrayConf('names_orig', '',
                                  dtype='object',
                                  groupnames=['state'],
                                  choices=list(zones.ids_sumo.get_value()),
                                  name='Orig zone',
                                  info='Name of traffic assignment zone of origin of trip.',
                                  ))

        self.add_col(am.ArrayConf('names_dest', '',
                                  dtype='object',
                                  groupnames=['state'],
                                  choices=list(zones.ids_sumo.get_value()),
                                  name='Dest zone',
                                  info='Name of traffic assignment zone of destination of trip.',
                                  ))

        self.add_col(am.ArrayConf('tripnumbers', 0,
                                  groupnames=['state'],
                                  perm='rw',
                                  name='Trips',
                                  info='Number of trips from zone of origin to zone of destination.',
                                  xmltag='tripnumber',
                                  ))

        self.add(cm.FuncConf('func_make_row', 'on_add_row', None,
                             groupnames=['rowfunctions', '_private'],
                             name='New OD flow.',
                             info='Add a new OD flow.',
                             ))

        self.add(cm.FuncConf('func_delete_row', 'on_del_row', None,
                             groupnames=['rowfunctions', '_private'],
                             name='Del OD flow',
                             info='Delete OD flow.',
                             ))

        # self.attrs.print_attrs()

    def on_del_row(self, id_row):
        # print 'on_del_row', id_row
        if id_row is not None:
            self.del_row(id_row)

    def on_add_row(self, id=None):
        if len(self) > 0:
            # copy previous
            od_last = self.get_row(self.get_ids()[-1])
            #id_orig = self.odtab.ids_orig.get(id_last)
            #id_dest = self.odtab.ids_dest.get(id_last)
            #id = self.suggest_id()
            self.add_row(**od_last)
        else:
            # create empty
            self.add_row()

    def add_demand(self):
        """
        Add demand to scenario.
        """
        # print 'AddOdm.add_demand'
        odintervals = self.parent
        #demand = self._scenario.demand
        # odm={} # create a temporary dict with (o,d) as key and trips as value
        ids = self.get_ids()
        odintervals.add_od_flows(self.t_start.value, self.t_end.value,
                                 self.id_mode.value,
                                 self.id_activitytype_orig.value, self.id_activitytype_dest.value,
                                 self.scale.value,
                                 self.names_orig[ids], self.names_dest[ids],
                                 self.tripnumbers[ids]
                                 )

        #t_start, t_end, id_mode,
        #                   id_activitytype_orig, id_activitytype_dest,
        #                  scale, names_orig,names_dest,tripnumbers):

    def import_csv(self, filepath, sep=",", n_firstline=1):
        names_zone = self.parent.get_zones().ids_sumo
        # make shure that index table is updated
        names_zone.rebuild_indices()
        f = open(filepath, 'r')
        # print '  open',filepath
        i_line = n_firstline
        for line in f.readlines():
            # print '    ',line,
            cols = line.split(sep)
            if len(cols) == 3:
                name_orig, name_dest, tripnumbers_str = cols

                name_orig = name_orig.strip()
                name_dest = name_dest.strip()
                if names_zone.has_index(name_orig) & names_zone.has_index(name_dest):
                    id_new = self.suggest_id()
                    self.add_row(id_new,
                                 names_orig=name_orig,
                                 names_dest=name_dest,
                                 tripnumbers=int(tripnumbers_str)
                                 )
                else:
                    print 'WARNING: unknown zonename in line %d of file %s' % (i_line, filepath)

            else:
                if len(cols) != 0:
                    print 'WARNING: inconsistent o,d,trips info in line %d of file %s' % (i_line, filepath)
            i_line += 1
        # self.odtab.print_rows()
        f.close()


class AddOdDialog(wx.Frame):

    """
    A frame used for the ObjBrowser Demo

    """

    def __init__(self, parent, odintervals):
        wx.Frame.__init__(self, parent, -1, title='Add OD flow Wizzard', pos=wx.DefaultPosition, size=wx.DefaultSize)
        self.wizzard = AddOdWizzard(odintervals)
        self.parent = parent
        # Set up the MenuBar
        MenuBar = wx.MenuBar()

        file_menu = wx.Menu()
        item = file_menu.Append(-1, "&Import CSV...",
                                "Import OD data from a CSV text file with format <zonename orig>, <zonename dest>,<number of trips>")
        self.Bind(wx.EVT_MENU, self.on_import_csv, item)
        #item = file_menu.Append(-1, "&Import Exel...","Import OD data from an Exel file.")
        #self.Bind(wx.EVT_MENU, self.on_import_exel, item)

        item = file_menu.Append(-1, "&Save flows and close", "Add OD flows to scenario and close wizzard")
        self.Bind(wx.EVT_MENU, self.on_add, item)

        item = file_menu.Append(-1, "&Close", "Close wizzard withot saving")
        self.Bind(wx.EVT_MENU, self.on_close, item)

        MenuBar.Append(file_menu, "&File")

        edit_menu = wx.Menu()
        item = edit_menu.Append(-1, "&Add OD flow to table",
                                "Add a new flow by defining zones of origin, destination and number of trips in table")
        self.Bind(wx.EVT_MENU, self.on_add_new, item)
        MenuBar.Append(edit_menu, "&Edit")

        if odintervals.get_net().parent is not None:
            self.dirpath = odintervals.get_net().parent.get_workdirpath()
        else:
            self.dirpath = os.getcwd()

        #help_menu = wx.Menu()
        # item = help_menu.Append(-1, "&About",
        #                        "More information About this program")
        #self.Bind(wx.EVT_MENU, self.on_menu, item)
        #MenuBar.Append(help_menu, "&Help")

        self.SetMenuBar(MenuBar)

        self.CreateStatusBar()

        self.browser = self.make_browser()

        # Create a sizer to manage the Canvas and message window
        MainSizer = wx.BoxSizer(wx.VERTICAL)
        MainSizer.Add(self.browser, 4, wx.EXPAND)

        self.SetSizer(MainSizer)
        self.Bind(wx.EVT_CLOSE, self.on_close)

        self.EventsAreBound = False

        # getting all the colors for random objects
        # wxlib.colourdb.updateColourDB()
        #self.colors = wxlib.colourdb.getColourList()

        return None

    def make_browser(self):
        # Create Browser widget here
        buttons = [  # ('Add new OD flow', self.on_add_new, 'Add a new flow by defining zones of origin, destination and number of trips in table.'),
            ('Save flows', self.on_add, 'Save OD flows to current demand.'),
            ('Cancel', self.on_close, 'Close wizzard without adding flows.'),
        ]
        defaultbuttontext = 'Save flows'
        # standartbuttons=['cancel','apply','ok']# apply does not show
        standartbuttons = ['apply']

        browser = ObjPanel(self, self.wizzard,
                           attrconfigs=None,
                           id=None, ids=None,
                           groupnames=None,
                           func_change_obj=None,
                           show_groupnames=False, show_title=False,
                           is_modal=True,
                           mainframe=None,
                           pos=wx.DefaultPosition,
                           size=wx.DefaultSize,
                           style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                           immediate_apply=False,  # True,
                           panelstyle='default',
                           buttons=buttons,
                           standartbuttons=standartbuttons,
                           defaultbutton=defaultbuttontext,
                           )
        return browser

    def on_import_csv(self, event=None):
        # print 'on_import_csv'
        self.browser.apply()
        wizzard = self.browser.obj

        wildcards_all = "CSV files (*.csv)|*.csv|CSV files (*.txt)|*.txt|All files (*.*)|*.*"  # +"|"+otherwildcards
        dlg = wx.FileDialog(self.parent, message="Import CSV file",
                            defaultDir=self.dirpath,
                            defaultFile="",
                            wildcard=wildcards_all,
                            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
                            )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            paths = dlg.GetPaths()
            # print 'You selected %d files:' % len(paths)
            if len(paths) > 0:
                filepath = paths[0]
            else:
                filepath = ''
            dlg.Destroy()
        else:
            return
        ###
        wizzard.import_csv(filepath)
        ##

        self.refresh_browser()
        #
        self.Raise()
        # self.SetFocus()
        # self.MakeModal(False)
        # self.MakeModal(True)
        # self.browser.restore()

    def on_add_new(self, event=None):
        # print 'on_add,AddOdm',self.browser.obj
        self.browser.apply()  # important to transfer widget values to obj!
        wizzard = self.browser.obj
        wizzard.on_add_row()
        self.refresh_browser()

    def on_add(self, event=None):
        # print 'on_add,AddOdm',self.browser.obj
        self.browser.apply()  # important to transfer widget values to obj!
        wizzard = self.browser.obj
        wizzard.add_demand()

        self.on_close(event)

    def on_close(self, event=None):
        self.MakeModal(False)
        self.Destroy()
        pass

    def on_add_close(self, event=None):
        self.on_add()
        self.on_close()

    def refresh_browser(self):
        """
        Deletes previous conents 
        Builds panel for obj
        Updates path window and history
        """
        # print 'Wizzard.refresh_panel with',obj.ident
        # remove previous obj panel
        sizer = self.GetSizer()
        sizer.Remove(0)
        self.browser.Destroy()
        #del self.browser
        self.browser = self.make_browser()

        sizer.Add(self.browser, 1, wx.GROW)

        self.Refresh()
        # sizer.Fit(self)
        sizer.Layout()
        # add to history
