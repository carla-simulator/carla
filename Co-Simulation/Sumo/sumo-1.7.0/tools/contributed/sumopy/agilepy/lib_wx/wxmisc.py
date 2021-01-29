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

# @file    wxmisc.py
# @author  Joerg Schweizer
# @date


import sys
import os
import types
#AGILEDIR = os.path.join(os.path.dirname(__file__),"..")
# sys.path.append(os.path.join(AGILEDIR,"lib_base"))
#IMAGEDIR =  os.path.join(os.path.dirname(__file__),"images")


import wx

import time
import string

# def OnOpen(self,e):
##        """ Open a file"""
##        self.dirname = ''
##        dlg = wx.FileDialog(self, "Choose a file", self.dirname, "", "*.*", wx.OPEN)
# if dlg.ShowModal() == wx.ID_OK:
##            self.filename = dlg.GetFilename()
##            self.dirname = dlg.GetDirectory()
##            f = open(os.path.join(self.dirname, self.filename), 'r')
# self.control.SetValue(f.read())
# f.close()
# dlg.Destroy()

# alternating colors in tables "light blue" 'WHEAT'
#TABLECOLORS=[wx.NamedColour('grey85'),wx.NamedColour("light blue")]


def get_tablecolors():
    cs = [wx.NamedColour('grey85'), wx.NamedColour("light blue")]
    # cs = [wx.SystemSettings.GetColour(wx.SYS_COLOUR_WINDOW ) ,\
    #        wx.SystemSettings.GetColour(wx.SYS_COLOUR_BTNHIGHLIGHT)]
    # print 'resources.get_tablecolors',cs
    return cs


class AgileToolbarMixin:
    """
    Easy to use toolbar where strings can be used to identify tools. 
    Mixin for frames.
    """

    def init_toolbar(self, orientation='horizontal', size=(16, 16), separation=5):
        """
        This initialization works for panels and windows. 
        For frames use the AgileToolbar frames mixin class
        """
        if orientation == 'horizontal':
            self.toolbar = wx.ToolBar(self, -1, style=wx.TB_HORIZONTAL | wx.TB_NODIVIDER
                                      | wx.NO_BORDER
                                      | wx.TB_FLAT
                                      | wx.TB_TEXT
                                      )
        else:
            self.toolbar = wx.ToolBar(self, -1, style=wx.TB_VERTICAL | wx.TB_NODIVIDER
                                      | wx.NO_BORDER
                                      | wx.TB_FLAT
                                      | wx.TB_TEXT
                                      )

        self.toolbar.SetToolBitmapSize(size)
        self.toolbar.SetToolSeparation(separation)

        self._tools = {}
        self.toolbar.Realize()
        S = self.toolbar.GetSize()
        self.toolbar.SetSizeHints(S[0], S[1])

    def get_size_toolbaricons(self):
        """
        Returns tuple with width and height of toolbar icons.
        This is required to create them.
        """
        return self.toolbar.GetToolBitmapSize()

    def disable_tools(self):
        pass
        # GetToolEnabled

    def restore_tools(self):
        pass
        # GetToolEnabled

    def add_tool(self, key, func=None, bitmap=None, info='', widget=None, **args):
        """
        Add a tool to the tool bar
        """
        # print 'add_tool',self,key,func

        id = wx.NewId()
        if not args.has_key('name'):
            name = string.capitalize(key)
        else:
            name = args['name']

        if widget is not None:
            # tool is a widget
            self.toolbar.AddControl(widget)
        else:
            # tools is a simple button
            if not bitmap:
                bitmap = wx.NullBitmap

            # print 'bitmap=',name,bitmap
            self.toolbar.AddSimpleTool(id, bitmap, name, info)
            #self.toolbar.AddTool(id,name, bitmap, shortHelpString=info)

            wx.EVT_TOOL(self, id, func)
            # if args.has_key('func_rclick'):
            #    wx.EVT_TOOL_RCLICKED(self.parent, id, args['func_rclick'])

        self._tools[key] = id
        # Final thing to do for a toolbar is call the Realize() method. This
        # causes it to render (more or less, that is).
        self.toolbar.Realize()
        S = self.toolbar.GetSize()
        self.toolbar.SetSizeHints(S[0], S[1])
        return id

    def enable_tool(self, key, enable=True):
        if self._tools.has_key(key):
            # self._tools[key].Show(False)
            self.toolbar.EnableTool(self._tools[key], enable)

        else:
            print 'enable_tool: no tool named:', key

    def del_tool(self, key):
        if self._tools.has_key(key):
            # self._tools[key].Show(False)
            self.toolbar.RemoveTool(self._tools[key])
            del self._tools[key]

            # Final thing to do for a toolbar is call the Realize() method. This
            # causes it to render (more or less, that is).
            self.toolbar.Realize()
        else:
            print 'del_tool: no tool named:', key


class AgileToolbarFrameMixin(AgileToolbarMixin):
    """
    Easy to use toolbar where strings can be used to identify tools. 
    Mixin for frames only.
    """

    def init_toolbar(self, orientation='horizontal', size=(16, 16), separation=5):
        """
        Only this initialization is specific to frames.
        """
        # Use the wxFrame internals to create the toolbar and associate it all
        # in one tidy method call.
        if orientation == 'horizontal':
            self.toolbar = self.CreateToolBar(wx.TB_HORIZONTAL
                                              | wx.NO_BORDER
                                              | wx.TB_FLAT
                                              | wx.TB_TEXT
                                              )
        else:
            self.toolbar = self.CreateToolBar(wx.TB_VERTICAL
                                              | wx.NO_BORDER
                                              | wx.TB_FLAT
                                              | wx.TB_TEXT
                                              )

        self.toolbar.SetToolBitmapSize(size)
        self.toolbar.SetToolSeparation(separation)

        self._tools = {}


class AgileMenuMixin:
    """
    Easy to use menue mixin for main menu and popup
    """

    def init_menu(self, parent):
        """
        Initialize internal variables.
        """
        self.parent = parent
        self.__tree = {}

    def get_menu(self, path):
        items = self.__tree
        val = (None, None, -1)
        # print 'get_menu',path
        for key in path:
            # print '  ',key,items
            if items.has_key(key):
                val = items[key]
                # print val
                if len(val) == 3:
                    items = val[1]
                else:
                    return None, None, -1
            else:
                print 'WARNING in get_menu: invalid menu key', key, 'in path'
                return None, None, -1

        return val[0], val[1], val[2]

    def get_menuitem(self, path):
        menupath = string.split(path, '/')[:-1]
        key = string.split(path, '/')[-1]

        # print '\nget_menuitem',self.__tree
        if len(menupath) == 0:
            # print  "  *create toplevel menu",key
            menuitem, id = self.__tree[key]

        else:
            # print "  *create submenu",key
            menu, tree, id = self.get_menu(menupath)
            menuitem, id = tree[key]

        # print '  menuitem,id',menuitem,id
        return menuitem

    def append_menu(self, path, **args):
        """
        """
        # print 'append_menu',path,args
        if path == '.':
            # print "  use '.' as root for popups"
            # create an entry for database but do not append a menue

            key = '.'
            # create special menu as direct root
            id = wx.NewId()
            self.__tree[key] = (self, {}, id)
            # print ' ROOTMENU ATTRS: self,dir(self)',self,dir(self)

        else:
            # print "  normal menu"
            menupath = string.split(path, '/')[:-1]
            key = string.split(path, '/')[-1]

            if len(menupath) == 0:
                # print  "  *create toplevel menu",key
                menu, id = self._create_menu(self, key, **args)
                self.__tree[key] = (menu, {}, id)

            else:
                # print "  *create submenu",key
                parentmenu, parentmenu_dict, parentid = self.get_menu(menupath)
                # print ' PARENTMENU ATTRS: self,dir(parentmenu)',parentmenu,dir(parentmenu)
                # print '  parentmenu,key,parentmenu_dict,parentid',parentmenu,key,parentmenu_dict,parentid
                menu, id = self._create_menu(parentmenu, key, **args)
                parentmenu_dict[key] = (menu, {}, id)

    def _create_menu(self, parentmenu, key, menu=None, **args):
        # print '\n\n_create_menu key',key
        # default menu/submenu item
        data = {'alt': True}
        # print ' PARENTMENU ATTRS: self,dir(parentmenu)',parentmenu,dir(parentmenu)
        # overwrite with args
        data.update(args)

        if not data.has_key('name'):
            data['name'] = string.capitalize(key)

        if data['alt']:
            alt = '&'
        else:
            alt = ''

        id = wx.NewId()
        if menu is None:
            menu = wx.Menu()

        if parentmenu == self:
            id = -1
            if data.get('popup', False):

                # !! this is the way to append a menue to toplevel
                # popup menu which is a menu itself
                self.AppendMenu(id, data['name'], menu)

            else:
                # !! this is the way to append a menue to toplevel
                # which is a menubar !!
                self.Append(menu, alt+data['name'])

        else:
            # attach menu to a submenu
            # print '  make submenu'

            item, id = self._create_item(key, parentmenu, function=None, **args)
            item.SetSubMenu(menu)
            parentmenu.AppendItem(item)
            # parentmenu.AppendMenu(id,alt+data['name'],menu)

        return menu, id

    def append_item(self, path, function, **args):

        # print 'append_item',path,function
        menupath = string.split(path, '/')[:-1]
        key = string.split(path, '/')[-1]

        menu, menu_dict, menuid = self.get_menu(menupath)

        # print '  key=',key

        if menu:
            # create item and append to menu instance
            item, id = self._create_item(key, menu, function, **args)

            # append item
            # print '  menu',menu,type(menu),dir(menu)
            menu.AppendItem(item)
            if args.has_key('check'):
                menu.Check(item.GetId(), args['check'])
            # if (args.has_key('check'))&(args.has_key('checked')):
            #    menu.Check(item.GetId(), args['checked'])

            menu_dict[key] = (item, id)
            return item, id
        else:
            print 'WARNING: in append_item: invalid menu path', menupath
            return None, None

    def _create_item(self, key, menu, function=None, **args):
        # default item
        data = {'alt': True,
                'shortkey': '',
                # 'info':'',
                }

        # overwrite with args
        data.update(args)

        if not data.has_key('name'):
            data['name'] = string.capitalize(key)

        if not data.has_key('info'):
            if function.__doc__ is not None:
                data['info'] = function.__doc__.replace('\n', ' ').strip()
            else:
                data['info'] = data['name']

        # print '_create_item',data

        # create item
        id = wx.NewId()
        if data['alt']:
            alt = '&'
        else:
            alt = ''
        itemtext = alt+data['name']
        if data['shortkey'] != '':
            itemtext += '\t'+data['shortkey']

        if data.has_key('radio'):

            item = wx.MenuItem(menu, id, itemtext, data['info'], wx.ITEM_RADIO)
            # print ' radio item'

        elif data.has_key('check'):
            item = wx.MenuItem(menu, id, itemtext, data['info'], wx.ITEM_CHECK)
            # check boxes AFTER append

        else:
            item = wx.MenuItem(menu, id, itemtext, data['info'], wx.ITEM_NORMAL)
            # print ' normal item'

        if data.has_key('bitmap'):
            # print '_create_item  bitmap',data['bitmap']
            # TODO: allow more image formats in menuitem
            # item.SetBitmap(images.getSmilesBitmap())
            item.SetBitmap(data['bitmap'])

        # define callback function
        if function is not None:  # None for submenueitem
            # wx.EVT_MENU(self.parent,id,function)
            self.parent.Bind(wx.EVT_MENU, function, id=id)

        return item, id

    def del_item(self, path):
        menupath = string.split(path, '/')[:-1]
        key = string.split(path, '/')[-1]

        menu, menu_dict, menuid = self.get_menu(menupath)
        if menu:
            if menu_dict.has_key(key):
                menu.RemoveItem(menu_dict[key][0])
                # menu_dict[key][0].Remove()
                del menu_dict[key]

    def del_menu(self, path):
        menupath = string.split(path, '/')[:-1]
        key = string.split(path, '/')[-1]
        allpath = string.split(path, '/')
        parentmenu, parentmenu_dict, parentmenuid = self.get_menu(menupath)
        menu, menu_dict, menuid = self.get_menu(allpath)

        if menu:
            if menuid > -1:
                parentmenu.Remove(menuid)               # works
                del parentmenu_dict[key]
            else:
                self.Remove(menu)
                del parentmenu_dict[key]

    def __setitem__(self, menupath, **data):
        # print 'set menue',menupath,'to',data

        if type(name) != types.TupleType:
            # create main menue entry, if necessary
            if not self.menus.has_key(name):
                newmenue = wx.Menu()
                self.Append(newmenue, '&'+name)
                self.menus[name] = (-1, {})

        elif len(name) == 2:
            # create submenu entry, if necessary
            name1, name2 = name
            if not self.menus.has_key(name1):
                newmenue = wx.Menu()
                self.Append(newmenue, '&'+name1)
                self.menus[name] = (-1, {})

            menuid, submenus = self.menus[name1]

            if not submenus.has_key(name2):
                id = wx.NewId()
                get_menu_item()
                newmenue = wx.Menu()
                self.Append(newmenue, '&'+name1)
                self.menus[name] = {}

            submenu = self.menus
        parentmenu = None
        for m in menu:
            if not submenu.has_key(m):
                newmenue = wx.Menu()

    def get_menu_item(self, id):
        item = self.FindItemById(id)
        menu = item.GetMenu()
        return menu, item

    def get_menuitem_from_id(self, id):
        menu, item = self.get_menu_item(id)
        return item


class AgilePopupMenu(wx.Menu, AgileMenuMixin):
    """
    Easy to use popup
    """

    def __init__(self, parent):
        wx.Menu.__init__(self)
        self.init_menu(parent)

        # append special root menu
        AgileMenuMixin.append_menu(self, '.')

    def append_menu(self, path, **args):
        AgileMenuMixin.append_menu(self, './'+path, popup=True, **args)

    def append_item(self, path, function, **args):
        return AgileMenuMixin.append_item(self, './'+path, function, **args)

    def get_menuitem(self, path):
        return AgileMenuMixin.get_menuitem(self, './'+path)

# -------------------------------------------------------------------------------
    # the folling methods are used with contect<sebitive menus
    # on a grid based widget.
    # When initializing the menu the row and column of its location can
    # be stored for later retrival during an event:
    # event.GetEventObject().get_row() event.GetEventObject().get_row()

    def set_row(self, row):
        self.row = row

    def set_col(self, col):
        self.col = col

    def get_row(self):
        return self.row

    def get_col(self):
        return self.col


KEYMAP = {
    wx.WXK_BACK: "BACK",
    wx.WXK_TAB: "TAB",
    wx.WXK_RETURN: "RETURN",
    wx.WXK_ESCAPE: "ESCAPE",
    wx.WXK_SPACE: "SPACE",
    wx.WXK_DELETE: "DELETE",
    wx.WXK_START: "START",
    wx.WXK_LBUTTON: "LBUTTON",
    wx.WXK_RBUTTON: "RBUTTON",
    wx.WXK_CANCEL: "CANCEL",
    wx.WXK_MBUTTON: "MBUTTON",
    wx.WXK_CLEAR: "CLEAR",
    wx.WXK_SHIFT: "SHIFT",
    wx.WXK_ALT: "ALT",
    wx.WXK_CONTROL: "CONTROL",
    wx.WXK_MENU: "MENU",
    wx.WXK_PAUSE: "PAUSE",
    wx.WXK_CAPITAL: "CAPITAL",
    wx.WXK_PRIOR: "PRIOR",
    wx.WXK_NEXT: "NEXT",
    wx.WXK_END: "END",
    wx.WXK_HOME: "HOME",
    wx.WXK_LEFT: "LEFT",
    wx.WXK_UP: "UP",
    wx.WXK_RIGHT: "RIGHT",
    wx.WXK_DOWN: "DOWN",
    wx.WXK_SELECT: "SELECT",
    wx.WXK_PRINT: "PRINT",
    wx.WXK_EXECUTE: "EXECUTE",
    wx.WXK_SNAPSHOT: "SNAPSHOT",
    wx.WXK_INSERT: "INSERT",
    wx.WXK_HELP: "HELP",
    wx.WXK_NUMPAD0: "NUMPAD0",
    wx.WXK_NUMPAD1: "NUMPAD1",
    wx.WXK_NUMPAD2: "NUMPAD2",
    wx.WXK_NUMPAD3: "NUMPAD3",
    wx.WXK_NUMPAD4: "NUMPAD4",
    wx.WXK_NUMPAD5: "NUMPAD5",
    wx.WXK_NUMPAD6: "NUMPAD6",
    wx.WXK_NUMPAD7: "NUMPAD7",
    wx.WXK_NUMPAD8: "NUMPAD8",
    wx.WXK_NUMPAD9: "NUMPAD9",
    wx.WXK_MULTIPLY: "MULTIPLY",
    wx.WXK_ADD: "ADD",
    wx.WXK_SEPARATOR: "SEPARATOR",
    wx.WXK_SUBTRACT: "SUBTRACT",
    wx.WXK_DECIMAL: "DECIMAL",
    wx.WXK_DIVIDE: "DIVIDE",
    wx.WXK_F1: "F1",
    wx.WXK_F2: "F2",
    wx.WXK_F3: "F3",
    wx.WXK_F4: "F4",
    wx.WXK_F5: "F5",
    wx.WXK_F6: "F6",
    wx.WXK_F7: "F7",
    wx.WXK_F8: "F8",
    wx.WXK_F9: "F9",
    wx.WXK_F10: "F10",
    wx.WXK_F11: "F11",
    wx.WXK_F12: "F12",
    wx.WXK_F13: "F13",
    wx.WXK_F14: "F14",
    wx.WXK_F15: "F15",
    wx.WXK_F16: "F16",
    wx.WXK_F17: "F17",
    wx.WXK_F18: "F18",
    wx.WXK_F19: "F19",
    wx.WXK_F20: "F20",
    wx.WXK_F21: "F21",
    wx.WXK_F22: "F22",
    wx.WXK_F23: "F23",
    wx.WXK_F24: "F24",
    wx.WXK_NUMLOCK: "NUMLOCK",
    wx.WXK_SCROLL: "SCROLL",
    wx.WXK_PAGEUP: "PAGEUP",
    wx.WXK_PAGEDOWN: "PAGEDOWN",
    wx.WXK_NUMPAD_SPACE: "NUMPAD_SPACE",
    wx.WXK_NUMPAD_TAB: "NUMPAD_TAB",
    wx.WXK_NUMPAD_ENTER: "NUMPAD_ENTER",
    wx.WXK_NUMPAD_F1: "NUMPAD_F1",
    wx.WXK_NUMPAD_F2: "NUMPAD_F2",
    wx.WXK_NUMPAD_F3: "NUMPAD_F3",
    wx.WXK_NUMPAD_F4: "NUMPAD_F4",
    wx.WXK_NUMPAD_HOME: "NUMPAD_HOME",
    wx.WXK_NUMPAD_LEFT: "NUMPAD_LEFT",
    wx.WXK_NUMPAD_UP: "NUMPAD_UP",
    wx.WXK_NUMPAD_RIGHT: "NUMPAD_RIGHT",
    wx.WXK_NUMPAD_DOWN: "NUMPAD_DOWN",
    wx.WXK_NUMPAD_PRIOR: "NUMPAD_PRIOR",
    wx.WXK_NUMPAD_PAGEUP: "NUMPAD_PAGEUP",
    wx.WXK_NUMPAD_NEXT: "NUMPAD_NEXT",
    wx.WXK_NUMPAD_PAGEDOWN: "NUMPAD_PAGEDOWN",
    wx.WXK_NUMPAD_END: "NUMPAD_END",
    wx.WXK_NUMPAD_BEGIN: "NUMPAD_BEGIN",
    wx.WXK_NUMPAD_INSERT: "NUMPAD_INSERT",
    wx.WXK_NUMPAD_DELETE: "NUMPAD_DELETE",
    wx.WXK_NUMPAD_EQUAL: "NUMPAD_EQUAL",
    wx.WXK_NUMPAD_MULTIPLY: "NUMPAD_MULTIPLY",
    wx.WXK_NUMPAD_ADD: "NUMPAD_ADD",
    wx.WXK_NUMPAD_SEPARATOR: "NUMPAD_SEPARATOR",
    wx.WXK_NUMPAD_SUBTRACT: "NUMPAD_SUBTRACT",
    wx.WXK_NUMPAD_DECIMAL: "NUMPAD_DECIMAL",
    wx.WXK_NUMPAD_DIVIDE: "NUMPAD_DIVIDE",
}


class KeyHandler:
    """
    Mixin for handling key events in wx
    """

    def init_keys(self, parent=None):
        """
        Sets events and variables for parent.
        If no parent is defined then self is assumed to be parent.
        """
        if parent is None:
            parent = self

        self.key_pressed = ''  # string code of currently pressed key
        wx.EVT_ENTER_WINDOW(self, self.on_enter_window)
        wx.EVT_KEY_DOWN(self, self.on_key_down)
        wx.EVT_KEY_UP(self, self.on_key_up)

    def on_enter_window(self, evt):
        # print 'on_enter_window'
        self.SetFocus()
        #self.haveFocus = False
        # self.Refresh()

    def on_key_down(self, event):
        """
        A key has been pressed down...
        """
        if self.GUIMode is None:
            # try custom tool of metacanvas
            input = self.set_keypress(event)
            # print 'on_key_down',input
            self.metacanvas.make_transition(input, self)

    def on_key_up(self, event):
        """
        A key has been pressed down...
        """
        # print 'on_key_up'
        self.del_keypress()
        print '  key_pressed', self.key_pressed

    def del_keypress(self):
        """
        Empties self.key_pressed string
        """
        self.key_pressed = ''

    def set_keypress(self, evt, ignore_specials=False):
        """
        This method analyses the event and sets the self.key_pressed string. 
        Method returns a string with the key.


        """
        keycode = evt.GetKeyCode()
        keyname = KEYMAP.get(keycode, None)
        if keyname is None:
            # regular character key
            if keycode < 256:
                if keycode == 0:
                    keyname = "NUL"
                elif (keycode < 27) & (not ignore_specials):
                    # keycode is a special control key
                    keyname = "Ctrl-%s" % chr(ord('A') + keycode-1)
                elif (keycode < 27):
                    keyname = ''
                else:
                    keyname = "\"%s\"" % chr(keycode)
            else:
                keyname = ''  # ignore

        elif ignore_specials:  # keycode is a special key
            keyname = ''  # ignore

        self.key_pressed = keyname

        return keyname

    def get_code_special_keys(self, event, sep='+'):
        """
        This method analyses the special keypress and returns a string
        that codes SHIFT, ALT, CONTROL
        """
        code = ''

        if event.ControlDown():
            code += 'CONTROL'+sep

        if event.ShiftDown():
            code += 'SHIFT'+sep

        if event.AltDown():
            code += 'ALT'+sep

        return code

# class ToolMixin(AgileToolbarMixin):


class OptionsFrame(wx.Frame):
    """
    Simple wx frame with some special features.
    """

    def __init__(self, parent,  mainframe=None, id=-1, title='testframe', pos=wx.DefaultPosition,
                 size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE,
                 name='frame'):

        if mainframe is None:
            self._mainframe = parent
        else:
            self._mainframe = mainframe

        # Forcing a specific style on the window.
        #   Should this include styles passed?
        style = wx.DEFAULT_FRAME_STYLE | wx.NO_FULL_REPAINT_ON_RESIZE
        wx.Frame.__init__(self, parent, id, title, pos, size, style, name)
        #super(GLFrame, self).__init__(parent, id, title, pos, size, style, name)

    def get_metacanvas(self):
        return metacanvas


class OptionsPanel(AgileToolbarMixin, wx.Panel):
    """
    UNUSED
    Simple wx panele with some special features.
    """

    def __init__(self, parent, mainframe=None):

        wx.Panel.__init__(self, parent, -1, wx.DefaultPosition,
                          wx.DefaultSize, wx.SUNKEN_BORDER | wx.WANTS_CHARS)
        if mainframe is None:
            self._mainframe = parent
        else:
            self._mainframe = mainframe

    def get_metacanvas(self):
        return metacanvas


class AgileStatusbar(wx.StatusBar):
    def __init__(self, parent, fields=[('action', -4),
                                       ('message', -4),
                                       # ('coords',-1),
                                       # ('zoom',-1),
                                       ('progress', -1),
                                       ('status', -1),
                                       # ('coords',-1),
                                       ]):

        wx.StatusBar.__init__(self, parent, -1, wx.ST_SIZEGRIP)

        self.set_fields(fields)

        self.sizeChanged = False
        self.Bind(wx.EVT_SIZE, self.OnSize)
        #self.Bind(wx.EVT_IDLE, self.OnIdle)

        # Field 0 ... just text
        # self['message']=""

        # create a progrss bar
        self.progressbar = wx.Gauge(self, -1, 100)
        self.progressbar.SetBezelFace(4)
        self.progressbar.SetShadowWidth(4)
        # self.progressbar.SetValue(50)

        # put into position all status bar fields and elements
        self.Reposition()

    def write_action(self, text):
        self['action'] = text

    def write_message(self, text):
        self['message'] = text

    # def write_coords(self,text):
    #    self['coords'] = text

    def set_progress(self, percent):
        """
        Set bar of progressbar in percent
        """
        # print 'set_progress',percent
        self.progressbar.SetValue(int(percent))
        self.Update()

    def set_fields(self, fields):
        """
        Sets basic data of fields in status bar.
        Argument field is a list with the following format:
            [(name1,width1),(name2,width2),...]
        """
        self._ind_fields = {}
        widths = []
        ind = 0
        for name, width in fields:
            widths.append(width)
            self._ind_fields[name] = ind
            ind += 1
        self.SetFieldsCount(ind)
        self.SetStatusWidths(widths)

    def __setitem__(self, key, message):

        ind_field = self._ind_fields[key]
        # print 'AgileStatusbar.__setitem__',key,ind_field
        self.SetStatusText(message, ind_field)
        self.Update()

    def has_key(self, key):
        return self._ind_fields.has_key(key)

    def OnSize(self, evt):
        self.Reposition()  # for normal size events

        # Set a flag so the idle time handler will also do the repositioning.
        # It is done this way to get around a buglet where GetFieldRect is not
        # accurate during the EVT_SIZE resulting from a frame maximize.
        self.sizeChanged = True

    def OnIdle(self, evt):
        if self.sizeChanged:
            self.Reposition()

    # reposition the checkbox
    def Reposition(self):
        # print 1,rect
        rect = self.GetFieldRect(self._ind_fields['progress'])
        self.progressbar.SetPosition(rect.GetPosition())
        self.progressbar.SetSize(rect.GetSize())

        self.sizeChanged = False


class AgileMenubar(AgileMenuMixin, wx.MenuBar):
    """
    Easy to use main menu bar
    """

    def __init__(self, parent):
        wx.MenuBar.__init__(self)
        self.init_menu(parent)


def get_bitmap(name, size=22):
    """Return bitmap of right size from imgImages library"""
    name = name + "_"+str(size)
    try:
        return imgImages.catalog[name].getBitmap()
    except:
        print 'WARNING in get_bitmap: failed to return image', name
        return wx.NullBitmap


# These are some functions for bitmaps of icons.
import cPickle
import zlib


def GetHandData():
    return cPickle.loads(zlib.decompress(
        'x\xda\xd3\xc8)0\xe4\nV72T\x00!\x05Cu\xae\xc4`u=\x85d\x05\xa7\x9c\xc4\xe4l0O\
\x01\xc8S\xb6t\x06A(\x1f\x0b\xa0\xa9\x8c\x9e\x1e6\x19\xa0\xa8\x1e\x88\xd4C\
\x97\xd1\x83\xe8\x80 \x9c2zh\xa6\xc1\x11X\n\xab\x8c\x02\x8a\x0cD!\x92\x12\
\x98\x8c\x1e\x8a\x8b\xd1d\x14\xf4\x90%\x90LC\xf6\xbf\x1e\xba\xab\x91%\xd0\
\xdc\x86C\x06\xd9m\xe8!\xaa\x87S\x86\x1a1\xa7\x07\x00v\x0f[\x17'))


def GetHandBitmap():
    return wx.BitmapFromXPMData(GetHandData())

# ----------------------------------------------------------------------


def GetPlusData():
    return cPickle.loads(zlib.decompress(
        'x\xda\xd3\xc8)0\xe4\nV72T\x00!\x05Cu\xae\xc4`u=\x85d\x05\xa7\x9c\xc4\xe4l0O\
\x01\xc8S\xb6t\x06A(\x1f\x0b RF\x0f\x08\xb0\xc9@D\xe1r\x08\x19\xb8j=l2`\r\
\xe82HF\xe9a\xc8\xe8\xe9A\x9c@\x8a\x0c\x0e\xd3p\xbb\x00\x8f\xab\xe1>\xd5\xd3\
\xc3\x15:P)l!\n\x91\xc2\x1a\xd6`)\xec\xb1\x00\x92\xc2\x11?\xb8e\x88\x8fSt\
\x19=\x00\x82\x16[\xf7'))


def GetPlusBitmap():
    return wx.BitmapFromXPMData(GetPlusData())

# ----------------------------------------------------------------------


def GetMinusData():
    return cPickle.loads(zlib.decompress(
        'x\xda\xd3\xc8)0\xe4\nV72T\x00!\x05Cu\xae\xc4`u=\x85d\x05\xa7\x9c\xc4\xe4l0O\
\x01\xc8S\xb6t\x06A(\x1f\x0b RF\x0f\x08\xb0\xc9@D\xe1r\x08\x19\xb8j=\xa2e\
\x10\x16@\x99\xc82zz\x10\'\x90"\x83\xc34r\xdc\x86\xf0\xa9\x9e\x1e\xae\xd0\
\x81Ja\x0bQ\x88\x14\xd6\xb0\x06Ka\x8f\x05\x90\x14\x8e\xf8\xc1-C|\x9c\xa2\xcb\
\xe8\x01\x00\xed\x0f[\x87'))


def GetMinusBitmap():
    return wx.BitmapFromXPMData(GetMinusData())
