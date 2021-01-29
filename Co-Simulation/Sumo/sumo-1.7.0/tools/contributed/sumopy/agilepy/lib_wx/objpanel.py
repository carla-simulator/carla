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

# @file    objpanel.py
# @author  Joerg Schweizer
# @date

# cd /home/joerg/projects/sumopy/tools/sumopy/wxpython
# python objpanel.py

import sys
import os
import types


import time
import random
import string
import numpy as np
import wx
import wx.lib as wxlib
import wx.grid as gridlib
from wx.lib import scrolledpanel, hyperlink, colourdb, masked
import wx.lib.editor as editor
import wx.lib.colourselect as coloursel
from collections import OrderedDict
havePopupWindow = 1
if wx.Platform == '__WXMAC__':
    havePopupWindow = 0
    wx.PopupWindow = wx.PopupTransientWindow = wx.Window


if __name__ == '__main__':
    APPDIR = os.path.join(os.path.dirname(__file__), "..", "..")
    sys.path.append(APPDIR)

IMAGEDIR = os.path.join(os.path.dirname(__file__), 'images')

from wxmisc import KEYMAP, AgilePopupMenu, AgileToolbarMixin, get_tablecolors

# used for demo:
import wx.py as py  # pyshell
wildcards_all = "All files (*.*)|*.*"

provider = wx.SimpleHelpProvider()
wx.HelpProvider_Set(provider)

import agilepy.lib_base.classman as cm
from agilepy.lib_base.misc import filepathlist_to_filepathstring, filepathstring_to_filepathlist

METATYPES_LINKSTYLE = ('obj', 'id', 'tabid', 'ids')

NUMERICTYPES = cm.NUMERICTYPES  # (types.BooleanType,types.FloatType,types.IntType,types.LongType,types.ComplexType)
STRINGTYPES = cm.STRINGTYPES  # (types.StringType,types.UnicodeType)


def list_to_str(l, lb='', rb=''):
    # print 'list_to_str',l,len(l)
    if len(l) == 0:
        return lb+rb
    else:
        s = lb
        for e in l[:-1]:
            s += unicode(e)+','
        # print '  returns',s+unicode(l[-1])+rb
        return s+unicode(l[-1])+rb


def is_list_flat(l):
    if type(l) in STRINGTYPES:
        return False

    is_flat = True
    for e in l:
        if hasattr(e, '__iter__'):
            is_flat = False
            break
    return is_flat


def str_to_list(s, lb='[', rb=']'):
    """
    Tries to convert a string to a flat list.
    Attention, this conversion is extremely tolerant.
    For example:
        s='bus,bike' and s="['bus','bike']" will return the same result.
        s='5,2.5' and s="['5','2.5']" will also return the same result
          trying to convert numbers in integers or floats
    """
    s = s.strip()
    if len(s) >= 2:
        if (s[0] == lb) & (s[-1] == rb):  # are there braces
            sl = s[1:-1].split(',')
        else:
            sl = s.split(',')  # no braces, but could still be a list
    else:
        sl = s.split(',')

    if (len(sl) == 1) & (sl[0] == ''):
        return []

    l = []
    for e in sl:
        l.append(str_to_obj(e))
    # print 'str_to_list',s,l
    return l


def str_to_tuple(s):
    return str_to_list(s, lb='(', rb=')')


def str_to_obj(s):
    """
    Only for flat objects at the moment
    """
    # print 'str_to_obj',type(s),s
    s = s.strip()
    if len(s) == 0:
        return ''

    if s[0] == s[-1]:
        if s[0] in("'", '"'):
            s = s[1:-1]
    # print 'str_to_obj',s,len(s),type(s),s.isdigit(),s.isalnum()
    if s.isdigit():
        return string.atoi(s)
    else:
        try:
            return string.atof(s)
        except:
            return s


def str_to_obj_nested(s):
    """
    NOT WORKING, NOT IN USE
    Only for flat objects at the moment
    """
    # print 'str_to_obj',s
    x = str_to_list(s)
    # print '  ',x
    if x is None:
        x = str_to_tuple(s)
        if x is None:
            # if type(s) == types.StringType:
            if s.isdigit():
                return string.atoi(s)
            elif s.isalnum():
                return s
            else:
                try:
                    return string.atof(s)
                except:
                    return s
        else:
            return x

    else:
        return x


class AttrBase:
    """
    Mixin class that provides methods to support text representation 
    of attributes
    """

    def num_to_text(self, value, attr, obj):
        """
        Returns string of a numeric value taking account of formating
        info for this defined in obj.
        """
        pass

    def color_to_ffffff(self, color):
        """
        Returns a 3- tuple with values for rgb between 0 and 255.
        Takes an array or list with rgb values between 0.0 and 1.0.
        """
        # print 'color_to_ffffff',color
        return (color[0], color[1], color[2])

    def ffffff_to_color(self, ffffff):
        """
        Returns an array  with rgb values between 0.0 and 1.0.
        Returns a 3- tople with values for rgb between 0 and 255.

        """
        # print 'ffffff_to_color',ffffff,type(ffffff),type(array(ffffff,float)/255.0)
        return np.array(ffffff, float)/255.0
        # return np.array(ffffff,int)


class WidgetContainer:
    """
    Contains one or several widgets representing a scalar attribute.
    Should be overwritten to accomodate various interactive datatypes.
    Default is simple text. 

    """

    def __init__(self, parent, attrconf, mainframe=None, color_bg=None,
                 panelstyle='default', immediate_apply=False,
                 equalchar=':'):
        """
        Parent is the ScalarPanel panel,
        attr is the attribute name in parent.obj
        """
        self.parent = parent
        self.panelstyle = panelstyle
        self.immediate_apply = immediate_apply
        self._attrconf = attrconf
        self.equalchar = equalchar

        self.mainframe = mainframe

        if color_bg is None:
            self.color_bg = wx.NamedColour('grey85')
        else:
            self.color_bg = color_bg

        # self.widgets=[]
        # self.untitwidget=None
        # self.valuewidget=None
        # self.bitmapwidget=None

        self.create_widgets(self.define_widgetset())


# -------------------------------------------------------------------------------
    # the following methods are usually overwritten to create
    # attribute specific widgets

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        To be overwritten.
        """
        return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                ('value',   self.create_valuewidget(), wx.EXPAND), ]

    def get_valuewidget_write(self):
        """
        Returns instance of editable widget
        To be overwritten.
        """
        # if non is returned attribute is not editable
        # even though permission is set to write
        return None

    def get_valuewidget_read(self):
        """
        Returns instance of non-editable widget
        To be overwritten.
        """

        #text = '%s%s'%(self._attrconf.format_value(),self._attrconf.format_unit())
        text = self.format_value_obj(show_unit=True, show_parentesis=False)
        # if not editable, no unitwidgetcontainer will be created so we need unit
        # if len(text)>40:
        #    text=text[:10]+'...'+text[-10:]
        #widget=wx.StaticText(self.parent, -1, text,style=wx.ALIGN_RIGHT)
        # ,style= wx.ALIGN_RIGHT
        #widget=wx.StaticText(self.parent, -1,'TEST!',style=wx.ALIGN_RIGHT)
        # print '   value,widget',string_value,widget

        # no, showed only first character
        #widget = editor.Editor(self.parent, -1, style=wx.SUNKEN_BORDER)
        # widget.SetText(text)
        # print 'get_valuewidget_read',self._attrconf.attrname
        # print '  text RET',text.find('n'),':\n',text
        if text.find('n') > 0:
            widget = wx.TextCtrl(self.parent, -1, str(text), style=wx.ALIGN_LEFT | wx.TE_MULTILINE | wx.TE_READONLY)
        else:
            widget = wx.TextCtrl(self.parent, -1, str(text), style=wx.ALIGN_LEFT | wx.TE_READONLY)
        # self.set_textevents(widget)

        widget.Enable(False)

        return widget

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        return None  # self.widgets['value'][0].GetLabel()

    def set_widgetvalue(self, value):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        text = str(value)
        # if len(text)>40:
        #    text=text[:10]+'...'+text[-10:]
        if hasattr(self.widgets['value'][0], 'SetValue'):
            self.widgets['value'][0].SetValue(text)

        # OR?
        # if self._attrconf.is_writable():
        #    self.widgets['value'][0].SetValue(value)
        # else:

        # OR?
        # if hasattr(self.widgets['value'][0],'SetValue'):
        #    self.widgets['value'][0].SetValue(str(value))
        # elif hasattr(self.widgets['value'][0],'SetText'):
        #    print 'set_widgetvalue mixin',type(value),value
        #    self.widgets['value'][0].SetText(str(value))
        # self.valuewidget.SetLabel(str(value))

# -------------------------------------------------------------------------------
    # these methods are normally not overwritten

    def create_valuewidget(self):
        """
        Returns widget representing the value of attribute.
        Dependent on read or write access the
        get_valuewidget_write or get_valuewidget_read 
        will be called to generate the widget. 
        """
        # print 'create_valuewidget for',self._attrconf.attrname,'is_writable=',self._attrconf.is_writable()

        if self._attrconf.is_writable():
            # in write mode unit widget should be created separately
            widget = self.get_valuewidget_write()
            if widget is None:
                # print '   editable valuewidget not available'
                widget = self.get_valuewidget_read()
        else:
            widget = self.get_valuewidget_read()

        self.set_tooltip(widget)
        self.extend_widgetsize(widget)
        self.valuewidget = widget
        return widget

    def create_namewidget(self):
        # if self._attrconf.get_obj().ident =='vehicles':
        # print 'create_namewidget',self._attrconf.get_obj().ident,self._attrconf.attrname,self._attrconf.get_name(),type(self._attrconf.get_name())
        widget = wx.StaticText(self.parent, -1,
                               self._attrconf.get_name().title()+self.equalchar,
                               style=wx.ALIGN_RIGHT  # |wx.SIMPLE_BORDER   #wx.STATIC_BORDER #
                               )
        widget.SetBackgroundColour(self.color_bg)
        self.extend_widgetsize(widget)
        # widget.SetMinSize((-1,20))

        #

        self.set_tooltip(widget)

        return widget

    def create_unitwidget(self):
        if self._attrconf.has_unit():
            widget = wx.StaticText(self.parent, -1,
                                   self._attrconf.format_unit(),
                                   style=wx.ALIGN_LEFT  # |wx.SIMPLE_BORDER #wx.STATIC_BORDER #|  #
                                   )
            widget.SetBackgroundColour(self.color_bg)
            self.extend_widgetsize(widget)
            # print 'create_unitwidget', self.config_attr['unit']
            return widget

        else:
            return (0, 0)

    def extend_widgetsize(self, widget, xborder=3, yborder=3):
        """
        Extends the widget by incresing its minimum size by border in pixels.
        """
        s = widget.GetSize()
        widget.SetMinSize((s[0]+xborder, s[1]+yborder))

    def create_widgets(self, widgetdata):
        """
        Generates the widgets representing this attribute.
        """
        self.widgets = {}
        self.widgetnames = []
        for name, widget, align in widgetdata:
            self.widgets[name] = (widget, align)
            self.widgetnames.append(name)

    def get_widgetnames(self):
        """
        Returns a list with names of widgets, representing this attribute.
        """
        return self.widgetnames

    def get_widgetsinfo(self, widgetnames=[]):
        """
        Returns a list of widget infos representing this attribute 
        according to the widgetnames.
        One widget info is a tuple with widget object and desired alignment
        within the panel.  
        If widgetnames is empty then all widgets are returned in the 
        order defined in get_widgetnames. 
        """

        if len(widgetnames) == 0:
            widgetnames = self.get_widgetnames()

        widgetsinfo = []
        for name in widgetnames:
            widgetsinfo.append(self.widgets.get(name, ((0, 0), 0)))

        return widgetsinfo

    def apply_obj_to_valuewidget(self):
        """
        Value of obj is read and applied to value widget.
        To be overwritten.
        """
        value = self.get_value_obj()
        # print 'apply_obj_to_valuewidget',self._attrconf.attrname, value
        self.set_widgetvalue(value)

    def get_value_obj(self):
        """
        Reads current value from object.
        """
        if self._attrconf.is_colattr():
            return self._attrconf[self.parent.id]
        else:
            return self._attrconf.get_value()

    def set_value_obj(self, value):
        """
        Sets given value to object.
        """
        # print 'set_value_obj',self._attrconf.attrname,self.parent.id, self._attrconf.is_colattr()
        if self._attrconf.is_colattr():  # parent is holding the row id
            self._attrconf[self.parent.id] = value
        else:
            self._attrconf.set_value(value)

    def format_value_obj(self, show_unit=False, show_parentesis=False):
        """
        Return formatted value of object.
        """

        if self._attrconf.is_colattr():  # parent is holding the row id
            return self._attrconf.format_value(self.parent.id,
                                               show_unit=show_unit,
                                               show_parentesis=show_parentesis)
        else:
            # print 'format_value_obj',self._attrconf.attrname, self._attrconf.format_value( )
            return self._attrconf.format_value(show_unit=show_unit,
                                               show_parentesis=show_parentesis)

    def apply_valuewidget_to_obj(self, value=None):
        """
        Applies current value widget to object attribute.  

        Makes no sense for static text.
        If no value argument is given then the get_widgetvalue method is
        called to recover widget value.
        """
        # pass
        # in general values maust be transferred from widget to
        # attribute of objet in the correct way.

        # self._attrconf.get_value()
        if self._attrconf.is_writable():
            if value is None:  # value not given
                value = self.get_widgetvalue()

            if value is not None:
                # value returned by widget is valid
                # print 'apply_valuewidget_to_obj',value,self.parent.obj.ident,self.attr
                self.set_value_obj(value)

    def set_textevents(self, widget):
        """
        Sets events to text input widhets
        """
        # print 'set_textevents',widget
        # print '  immediate_apply',self.immediate_apply
        if self.immediate_apply:
            widget.Bind(wx.EVT_KILL_FOCUS, self.on_apply_immediate)
            #widget.Bind(wx.EVT_CHAR, self.on_apply_immediate)

    def on_apply_immediate(self, event):
        """
        A key has been pressed in valuewidget and it is requested
        to apply it immediately to the object
        """
        # print 'on_apply_immediate'
        self.apply_valuewidget_to_obj()

    def get_obj(self):
        """
        Returns object to be displayed on panel.
        """
        return self._attrconf.get_obj()

    # def get_pentable(self):
    #    """
    #    Returns pentable instance
    #    """
    #    return self.parent.get_pentable()

    def get_objvalue(self):
        """
        Read current value from object and convert into string.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        if self._attrconf.is_colattr():
            value = self._attrconf[self.parent.id]

        else:
            # print 'get_objvalue',self.attr,self.parent.obj
            value = self._attrconf.get_value()

        # print '  value=',value
        return value

    def set_tooltip(self, widget=None):

        # TODO : check for global tooltip on/off
        if self._attrconf.has_info():
            # print 'set_tooltip',self.attr,self.config_attr['info']
            widget.SetToolTipString(self._attrconf.get_info())
            widget.SetHelpText(self._attrconf.get_info())


class NumericWidgetContainer(AttrBase, WidgetContainer):
    """
    Contains one or several widgets representing a scalar numeric attribute.

    """

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        if self._attrconf.has_unit():
            # print 'define_widgetset num +unit',self._attrconf.attrname
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND),
                    ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL), ]
        else:
            # print 'define_widgetset num ',self._attrconf.attrname
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND)]

    def get_valuewidget_read(self):
        """
        Returns instance of non-editable widget
        To be overwritten.
        """
        widget = self.get_valuewidget_write()
        widget.Enable(False)
        # Here just the plain static text widget is returned

        # printformat=self.get_printformat_fortran(self.attr,self.parent.obj,main=self.mainframe)

        #value = printformat%value

        # add unit if exist
        # print 'get_valuewidget_read with unit:',self.parent.obj.get_unit(self.attr)
        # if self.parent.obj.get_unit(self.attr)!='':
        #    value+=' '+self.parent.obj.get_unit(self.attr)

        # print 'create_valuewidget: '+value,self.attr
        #widget=wx.StaticText(self.parent, wx.ID_ANY, self.format_value_obj(),style=wx.ALIGN_RIGHT)

        return widget

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """
        value = self.get_value_obj()
        # strange way to convert numpy type numbers into native python numbers
        if type(value) not in (types.IntType, types.LongType, types.FloatType, types.ComplexType):
            value = value.tolist()
        # print 'NumericWidgetContainer.get_valuewidget_write ',value,type(value),self._attrconf.digits_fraction

        # if self._attrconf.digits_fraction  is None:
        #    self._attrconf.digits_fraction = 3

        # print '  panelstyle=',self.panelstyle
        # print '  value=',value
        # numpy returns dtype... even for scalars
        # make sure to convert value in a native python scalar
        # if value  is None:
        # value=NaN
        # elif type(value) not in (types.IntType, types.LongType, types.FloatType):
        #    value=value.tolist()

        # if self.config_attr['min'] is None:
        allow_negative = True
        # else:
        #    allow_negative = self.config_attr['min'] < 0.0

        #min = self.config_attr['min']
        #max = self.config_attr['max']

        # if min  is None:
        #    if value<0:
        #        min = -5*value
        #    else:
        #        min = 0
        #
        # if max  is None:
        #    max = 5*abs(value)

        if np.isinf(value):
            widget = wx.StaticText(self.parent, -1,
                                   value.__repr__(),
                                   style=wx.ALIGN_LEFT  # |wx.SIMPLE_BORDER #wx.STATIC_BORDER #|  #
                                   )
            # widget.SetBackgroundColour(self.color_bg)
            self.extend_widgetsize(widget)
            # print 'create_unitwidget', self.config_attr['unit']
            return widget

        # if self.panelstyle == 'fancy': #'instrumental':
        #    print 'NumericWidgetContainer.get_valuewidget_write fancy mode'
        #    print 'TODO: no bindings yet for immediate apply'
        #    slider = wx.Slider(self.parent, wx.ID_ANY,
        #                value,
        #                min, max,
        #                style=wx.SL_HORIZONTAL| wx.SL_AUTOTICKS
        #                #style=wx.SL_HORIZONTAL | wx.SL_AUTOTICKS | wx.SL_LABELS
        #                )
        #
        #    slider.SetTickFreq(5, 1)
        #    return slider

        # else:

        widget = wx.TextCtrl(self.parent, -1, str(value), style=wx.ALIGN_RIGHT)
        # standard numerical widget
        # widget = masked.Ctrl(self.parent, id = -1,
        #                        value = value,
        #                        #integerWidth=None,
        #                        fractionWidth = self._attrconf.digits_fraction,
        #                        autoSize = True,
        #                        controlType=masked.controlTypes.NUMBER,
        #                        style= wx.ALIGN_RIGHT
        #                        )
        # widget = masked.NumCtrl(
        #     self.parent, id = -1,
        #     value = value,
        #     pos = wx.DefaultPosition,
        #     size = wx.DefaultSize,
        #     style = 0,
        #     validator = wx.DefaultValidator,
        #     name = "masked.number",
        #     integerWidth = 10,
        #     fractionWidth = 0,
        #     allowNone = False,
        #     allowNegative = True,
        #     useParensForNegatives = False,
        #     groupDigits = False,
        #     groupChar = ',',
        #     decimalChar = '.',
        #     min = None,
        #     max = None,
        #     limited = False,
        #     #limitOnFieldChange = False,
        #     selectOnEntry = True,
        #     foregroundColour = "Black",
        #     signedForegroundColour = "Red",
        #     emptyBackgroundColour = "White",
        #     validBackgroundColour = "White",
        #     invalidBackgroundColour = "Yellow",
        #     autoSize = False#True
        #     )
        self.set_textevents(widget)
        return widget

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        return float(self.valuewidget.GetValue())

    def set_widgetvalue(self, value):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        if 1:  # self._attrconf.is_writable():
            # set value to label
            # numpy returns dtype... even for scalars
            # make sure to convert value in a native python scalar
            if type(value) not in (types.IntType, types.LongType, types.FloatType):
                value = value.tolist()
            self.valuewidget.SetValue(str(value))


class IntegerWidgetContainer(NumericWidgetContainer):
    """
    Contains one or several widgets representing a scalar numeric attribute.

    """

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """
        # if self.panelstyle == 'instrumental':
        #    # return a spin control in instrumental style
        #    sc = wx.SpinCtrl(self.parent, wx.ID_ANY, "", (30, 50))
        #    sc.SetRange(self.config_attr['min'],self.config_attr['max'])
        #    sc.SetValue(value)
        #    return sc
        # else:
        # use standard numerical masked text otherwise
        return NumericWidgetContainer.get_valuewidget_write(self)

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        return int(self.valuewidget.GetValue())


class BooleanWidgetContainer(AttrBase, WidgetContainer):
    """
    Contains one or several widgets representing a boolean attribute.

    """

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT),
                ('value',   self.create_valuewidget(), wx.EXPAND),
                ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT), ]

    def get_valuewidget_read(self):
        """
        Returns instance of non-editable widget
        To be overwritten.
        """
        widget = self.get_valuewidget_write()
        widget.Enable(False)

        return widget

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """
        # print 'get_numeditwidget',value

        widget = wx.CheckBox(self.parent, -1)
        if self.immediate_apply:
            self.parent.Bind(wx.EVT_CHECKBOX, self.on_apply_immediate, widget)
        value = self.get_value_obj()
        widget.SetValue(value)
        return widget

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        return self.valuewidget.GetValue()

    def set_widgetvalue(self, value):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        # if self._attrconf.is_writable():
        self.valuewidget.SetValue(value)


class ChoiceWidgetContainer(WidgetContainer):
    """
    Contains one or several widgets representing a text attribute.

    """

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        if type(self._attrconf.choices) in (OrderedDict, types.DictionaryType):
            self._choicevalues = self._attrconf.choices.values()
            self._choicenames = self._attrconf.choices.keys()
        else:
            self._choicevalues = list(self._attrconf.choices)
            self._choicenames = list(self._attrconf.choices)
        # if type(self._attrconf.choices) in [types.ListType,types.TupleType,numpy.ndarray]:

        if self._attrconf.has_unit() & self._attrconf.is_writable():
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND),
                    ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL), ]
        else:
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND)]

    def get_valuewidget_read(self):
        """
        Return widget to read only numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """
        value = self.get_value_obj()
        # print 'ChoiceWidgetContainer.get_valuewidget_read',value,type(value)
        # print '  choices',self._attrconf.choices

        if type(self._attrconf.choices) in (OrderedDict, types.DictionaryType):
            #value = self._attrconf.choices[value]
            value = self._choicenames[self._choicevalues.index(value)]
        # print '  value =',value
        widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_RIGHT)
        widget.Enable(False)
        return widget

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """

        value = self.get_value_obj()
        # print 'ChoiceWidgetContainer.get_valuewidget_read',self._attrconf.attrname, value,type(value)
        # print '  choices',self._attrconf.choices
        # print '  self._choicenames',self._choicenames
        # print '  self._choicevalues',self._choicevalues
        widget = wx.Choice(self.parent, -1, (100, 50), choices=self._choicenames)
        if self.immediate_apply:
            self.parent.Bind(wx.EVT_CHOICE, self.on_apply_immediate, widget)
        if value in self._choicevalues:
            ind = self._choicevalues.index(value)
        else:
            ind = 0
        widget.SetSelection(ind)

        return widget

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        val = self.valuewidget.GetSelection()
        # if self._choicevalues.count(val)>0:
        return self._choicevalues[val]
        # else:
        #    return val

    def set_widgetvalue(self, val):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        # print 'set_widgetvalue',self._attrconf.attrname, val,self._choicevalues
        # if self._choicevalues.count(val)>0:
        #ind = self._choicevalues.index(val)
        # self.valuewidget.SetSelection(ind)
        # else:
        # self.valuewidget.SetValue(val)

        try:
            #ind = self._choicenames.index(value)
            ind = self._choicevalues.index(val)
        except:
            print 'WARNING in ChoiceWidgetContainer.set_widgetvalue: %s with value "%s" not in choice list' % (
                self._attrconf.attrname, val)
            return
        # print '  ind',ind,self.valuewidget
        if self._attrconf.is_writable():
            self.valuewidget.SetSelection(ind)
        else:
            self.valuewidget.SetValue(self._choicenames[ind])


class TextWidgetContainer(WidgetContainer):
    """
    Contains one or several widgets representing a text attribute.

    """

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        if self._attrconf.has_unit() & self._attrconf.is_writable():
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND),
                    ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL), ]
        else:
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND)]

    def get_valuewidget_read(self):
        """
        Return widget to read only numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """
        # print 'TextWidgetContainer.get_valuewidget_read'
        # widget=self.get_valuewidget_write()
        value = self.get_value_obj()
        widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_RIGHT | wx.TE_READONLY)
        widget.Enable(False)

        return widget

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """

        value = self.get_value_obj()
        # print 'get_editwidget text',value,type(value)

        widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_RIGHT)
        self.set_textevents(widget)
        return widget
        #self.MultiLine = wx.TextCtrl(parent = panel, id = -1, pos = (38, 70), size = (410, 90), style = wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_AUTO_URL)

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        return self.widgets['value'][0].GetValue()

    def set_widgetvalue(self, value):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        # print 'set_widgetvalue',dir(self.widgets['value'][0])
        self.widgets['value'][0].SetValue(value)


class DatetimeWidgetContainer(IntegerWidgetContainer):
    """
    Contains one or several widgets representing a time and date attributes.

    """

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        if self._attrconf.has_unit() & self._attrconf.is_writable():
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND),
                    ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL), ]
        else:
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND)]

    def get_valuewidget_read(self):
        """
        Return widget to read only numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """
        # print 'TextWidgetContainer.get_valuewidget_read'
        # widget=self.get_valuewidget_write()
        dtime = self.get_value_obj()
        #time.strftime("%a, %d %b %Y %H:%M:%S +0000", time.gmtime())
        value = time.strftime("%a, %d %b %Y %H:%M:%S", time.localtime(dtime))

        widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_RIGHT | wx.TE_READONLY)
        widget.Enable(False)

        return widget


class ListWidgetContainer(WidgetContainer):
    """
    Contains one or several widgets representing a text attribute.

    """

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        if self._attrconf.has_unit() & self._attrconf.is_writable():
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND),
                    ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL), ]
        else:
            return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                    ('value',   self.create_valuewidget(), wx.EXPAND)]

    # def get_valuewidget_read(self):
    #    """
    #    Return widget to read only numeric value of attribute
    #    This is effectively the parametrisation of the masked.NumCtrl widget.
    #    """
    #    value = self.get_value_obj()
    #    #print 'ListWidgetContainer',value
    #
    #    widget =  wx.TextCtrl( self.parent, -1, list_to_str(list(value)),style= wx.ALIGN_RIGHT|wx.TE_MULTILINE|wx.TE_READONLY)
    #    self.set_textevents(widget)
    #    widget.Enable(False)
    #    return widget

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """

        value = list_to_str(self.get_value_obj())
        if is_list_flat(value):
            widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_LEFT | wx.TE_MULTILINE)
        else:  # only flat lists can be edited :(
            widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_RIGHT | wx.TE_MULTILINE)
            widget.Enable(False)

        self.set_textevents(widget)
        return widget

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        # print 'get_widgetvalue',  self.widgets['value'][0].GetValue()
        # print ' returns', str_to_list(self.widgets['value'][0].GetValue())
        return str_to_list(self.widgets['value'][0].GetValue())

    def set_widgetvalue(self, value):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        # print 'set_widgetvalue',value,is_list_flat(value)
        if is_list_flat(value):  # &self._attrconf.is_editable():
            self.widgets['value'][0].SetValue(list_to_str(value))
        else:  # only flat lists can be edited :(
            self.widgets['value'][0].SetValue(repr(value))


class ObjWidgetContainer(AttrBase, WidgetContainer):
    """
    Contains one or several widgets representing an obj attribute.

    """

    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        # print 'define_widgetset',self._attrconf.attrname, self._attrconf.get_value()

        return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                ('value',   self.create_valuewidget(), wx.EXPAND),
                ]

    def get_valuewidget_read(self):
        """
        Returns instance of non-editable widget
        To be overwritten.
        """
        # print 'get_valuewidget_read',self._attrconf.attrname
        # print 'get_numeditwidget',value
        # Here just the plain static text widget is returned
        #obj = self._attrconf.get_value()
        #METATYPES_LINKSTYLE = ('obj','id','tabid','ids')
        mt = self._attrconf.get_metatype()
        obj = value = self.get_value_obj()
        # print '  value',value,type(obj),str(obj),mt
        if mt == 'obj':
            text = str(obj.format_ident())

        elif mt == 'id':
            if value == -1:
                # id not yet assigned
                widget = wx.TextCtrl(self.parent, -1, '-', style=wx.ALIGN_RIGHT | wx.TE_READONLY)
                widget.Enable(False)
                return widget
            else:
                text = self._attrconf.get_linktab().format_id(value)

        elif mt == 'ids':
            if value is None:
                # id not yet assigned
                widget = wx.TextCtrl(self.parent, -1, '', style=wx.ALIGN_RIGHT | wx.TE_READONLY)
                widget.Enable(False)
                return widget
            else:
                text = self._attrconf.get_linktab().format_ids(value)
            # print '  text',text,type(text)
        #
        # elif mt == 'tabid':
        #    text = str(obj)
        else:
            text = str(obj)

        # print 'create_valuewidget: '+value,self.attr
        #widget=wx.StaticText(self.parent, -1,text,style=wx.ALIGN_RIGHT)
        widget = hyperlink.HyperLinkCtrl(self.parent, wx.ID_ANY, text, URL=text)
        widget.AutoBrowse(False)

        widget.Bind(hyperlink.EVT_HYPERLINK_LEFT, self.on_objlink)

        return widget

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        """

        # print 'get_valuewidget_write',self._attrconf.attrname
        # Here just the plain static text widget is returned
        #obj = self._attrconf.get_value()
        #METATYPES_LINKSTYLE = ('obj','id','tabid','ids')
        mt = self._attrconf.get_metatype()
        obj = value = self.get_value_obj()
        linktab = self._attrconf.get_linktab()
        # print '  value',value,type(value),str(obj),mt
        if mt == 'id':
            #text = self._attrconf.get_linktab().format_id(value)
            choices = linktab.format_id(linktab.get_ids()).split(',')
            if len(choices) > 0:
                widget = wx.Choice(self.parent, -1, (100, 50), choices=choices)
                ind = choices.index(linktab.format_id(value))
                if value == -1:
                    # no id specified, use first one
                    widget.SetSelection(0)
                else:
                    widget.SetSelection(ind)
            else:
                # there are no ids to point to
                widget = wx.TextCtrl(self.parent, -1, '-', style=wx.ALIGN_RIGHT | wx.TE_READONLY)
                widget.Enable(False)
            return widget

        if mt == 'obj':
            text = str(obj.format_ident())

        elif mt == 'ids':
            text = self._attrconf.get_linktab().format_ids(value)
        #
        # elif mt == 'tabid':
        #    text = str(obj)
        else:
            text = str(obj)

        # print 'create_valuewidget: '+value,self.attr
        #widget=wx.StaticText(self.parent, -1,text,style=wx.ALIGN_RIGHT)
        widget = hyperlink.HyperLinkCtrl(self.parent, wx.ID_ANY, text, URL=text)
        widget.AutoBrowse(False)

        widget.Bind(hyperlink.EVT_HYPERLINK_LEFT, self.on_objlink)

        return widget

    def on_objlink(self, event):
        # print 'on_objlink:',self.get_objvalue().ident

        value = self.get_objvalue()
        mt = self._attrconf.get_metatype()

        if self.parent.func_change_obj:
            if mt == 'obj':
                if type(value) == types.InstanceType:
                    # print '  ident?',hasattr(value,'ident')
                    if hasattr(value, 'ident'):
                        navitimer = wx.FutureCall(1, self.parent.func_change_obj, value)

            elif mt == 'id':
                linktab = self._attrconf.get_linktab()
                navitimer = wx.FutureCall(1, self.parent.func_change_obj, linktab, value)  # here value is id

            elif mt == 'ids':
                linktab = self._attrconf.get_linktab()
                navitimer = wx.FutureCall(1, self.parent.func_change_obj, linktab,
                                          None, value)  # here value is list with ids

            elif mt == 'tabid':
                linktab, id = value
                navitimer = wx.FutureCall(1, self.parent.func_change_obj, linktab, id)

            # if self._attrconf.is_colattr(): # parent is holding the row id
            #self._attrconf[self.parent.id] = value
        else:
            #navitimer = wx.FutureCall(1, self.parent.func_change_obj,self.get_objvalue())
            pass

        event.Skip()

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        mt = self._attrconf.get_metatype()
        val = self.valuewidget.GetStringSelection()  # GetString
        # print 'get_widgetvalue', val,mt
        # print '  ',dir(self.valuewidget)
        # if self._choicevalues.count(val)>0:
        if mt == 'id':
            if val == '-':
                return -1
            else:
                return self._attrconf.get_linktab().get_id_from_formatted(val)
        else:
            return self.valuewidget.GetValue()

    def set_widgetvalue(self, val):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        # print 'set_widgetvalue',self._attrconf.attrname, val, self._attrconf.is_writable()
        # if self._choicevalues.count(val)>0:
        #ind = self._choicevalues.index(val)
        # self.valuewidget.SetSelection(ind)
        # else:
        # self.valuewidget.SetValue(val)

        # try:
        #    #ind = self._choicenames.index(value)
        #    ind = self._choicevalues.index(val)
        # except:
        #    print 'WARNING in ChoiceWidgetContainer.set_widgetvalue: %s with value "%s" not in choice list'%(self._attrconf.attrname,val)
        #    return
        # print '  ind',ind,self.valuewidget
        if self._attrconf.is_writable():
            self.valuewidget.SetStringSelection(self._attrconf.get_linktab().format_id(val))
            # self.valuewidget.SetSelection(ind)
        else:
            pass
            # self.valuewidget.SetValue(val)


class ColorWidgetContainer(AttrBase, TextWidgetContainer):
    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                ('value',   self.create_valuewidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL),  # wx.EXPAND
                #('unit',   self.create_unitwidget(), wx.ALIGN_LEFT|wx.ALIGN_CENTER_VERTICAL),
                ]

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        """

        value = self.get_value_obj()
        # print 'ColorWidgetContainer.get_valuewidget_write',self._attrconf.attrname,value,type(value)

        #widget =  wx.TextCtrl( self.parent, -1, str(value),style= wx.ALIGN_RIGHT)
        cint = np.array(np.array(value)*255, np.int32)
        # wx.Colour(cint[0],cint[1],cint[2],cint[3])
        widget = coloursel.ColourSelect(
            self.parent, -1, "", wx.Colour(cint[0], cint[1], cint[2], cint[3]), size=wx.DefaultSize)
        # self.set_textevents(widget)
        # widget.Enable(True)
        if self.immediate_apply:
            widget.Bind(coloursel.EVT_COLOURSELECT, self.on_colorselect)
        return widget

    def on_colorselect(self, event):
        self.apply_valuewidget_to_obj()

    def get_valuewidget_read(self):
        """
        Return widget to edit numeric value of attribute
        """

        value = self.get_value_obj()
        # print 'get_editwidget text',value,type(value)

        #widget =  wx.TextCtrl( self.parent, -1, str(value),style= wx.ALIGN_RIGHT)
        cint = np.array(np.array(value)*255, np.int32)
        widget = coloursel.ColourSelect(
            self.parent, -1, "", wx.Colour(cint[0], cint[1], cint[2], cint[3]), size=wx.DefaultSize)
        # self.set_textevents(widget)
        widget.Enable(False)
        return widget

    def get_widgetvalue(self):
        """
        Returnes current value from valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """

        c = self.widgets['value'][0].GetColour()
        # print 'ColorWidgetContainer.get_widgetvalue',self._attrconf.attrname, c, type(c),type(c) == types.InstanceType#types.ClassType
        #color = np.array([c.Red(),c.Green(),c.Blue(),c.Alpha()],np.float32)/255.0
        return np.array([c.Red(), c.Green(), c.Blue(), c.Alpha()], np.float32)/255.0

    def set_widgetvalue(self, value):
        """
        Sets value for valuewidget.
        Depends on attribute type and hence widgettype.
        To be overwritten.
        """
        cint = np.array(np.array(value)*255, np.int32)
        self.widgets['value'][0].SetValue(wx.Colour(cint[0], cint[1], cint[2], cint[3]))


class FilepathWidgetContainer(AttrBase, TextWidgetContainer):
    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                ('value',   self.create_valuewidget(), wx.EXPAND),
                ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL), ]

    def get_valuewidget_read(self):
        """
        Return widget to read only numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """
        # print 'TextWidgetContainer.get_valuewidget_read'
        # widget=self.get_valuewidget_write()
        value = self.get_value_obj()
        widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_LEFT | wx.TE_READONLY)
        widget.Enable(False)

        return widget

    def get_valuewidget_write(self):
        """
        Return widget to edit numeric value of attribute
        This is effectively the parametrisation of the masked.NumCtrl widget.
        """

        value = self.get_value_obj()
        # print 'get_editwidget text',value,type(value)

        widget = wx.TextCtrl(self.parent, -1, value, style=wx.ALIGN_LEFT)
        self.set_textevents(widget)
        return widget
        #self.MultiLine = wx.TextCtrl(parent = panel, id = -1, pos = (38, 70), size = (410, 90), style = wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_AUTO_URL)

    def create_unitwidget(self):
        #widget = wx.Button(self.parent, wx.ID_OPEN)
        bitmap = wx.ArtProvider.GetBitmap(wx.ART_FILE_OPEN, wx.ART_TOOLBAR)
        widget = wx.BitmapButton(self.parent, -1, bitmap, (12, 12),
                                 (bitmap.GetWidth()+6, bitmap.GetHeight()+6))
        widget.Bind(wx.EVT_BUTTON, self.on_fileopen)
        # print 'create_unitwidget',self._attrconf.attrname,widget
        return widget

    def on_fileopen(self, event):
        # print 'on_fileopen',self._attrconf.attrname
        # if type(self._attrconf.get_default())==types.StringType:
        #    defaultname = self._attrconf.get_default()
        # else:
        #    defaultname = os.getcwd()
        wildcards_all = "All files (*.*)|*.*"
        if hasattr(self._attrconf, 'wildcards'):
            wildcards = self._attrconf.wildcards+"|"+wildcards_all
        else:
            wildcards = wildcards_all

        # guess default value
        filepath = self.get_objvalue()
        if type(filepath) not in STRINGTYPES:
            filepath = ""
            dirpath = os.getcwd()
        else:
            dirpath = os.path.dirname(filepath)

        dlg = wx.FileDialog(self.parent, message="Open file",
                            defaultDir=dirpath,
                            defaultFile=filepath,
                            wildcard=wildcards,
                            style=wx.OPEN | wx.CHANGE_DIR
                            )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()

            # print 'on_fileopen set filepath',filepath
            self.set_widgetvalue(filepath)
            if self.immediate_apply:
                self.apply_valuewidget_to_obj()
            # print '    self.get_widgetvalue()',self.get_widgetvalue()
        dlg.Destroy()


class FilepathsWidgetContainer(FilepathWidgetContainer):

    def on_fileopen(self, event):
        # print 'FilepathsWidgetContainer.on_fileopen',self._attrconf.attrname
        # if type(self._attrconf.get_default())==types.StringType:
        #    defaultname = self._attrconf.get_default()
        # else:
        #    defaultname = os.getcwd()

        wildcards_all = "All files (*.*)|*.*"
        if hasattr(self._attrconf, 'wildcards'):
            wildcards = self._attrconf.wildcards+"|"+wildcards_all
        else:
            wildcards = wildcards_all

        # guess default value
        filepath = self.get_objvalue()
        # print '  filepath',filepath,type(filepath)
        if type(filepath) not in STRINGTYPES:
            filepath = ""
            dirpath = os.getcwd()
        else:
            dirpath = os.path.dirname(filepath.split(',')[0])

        # print '  dirpath',dirpath
        # print '  filepath',filepath
        dlg = wx.FileDialog(self.parent, message="Open file",
                            defaultDir=dirpath,
                            defaultFile=filepath,
                            wildcard=wildcards,
                            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
                            )

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            filepaths = dlg.GetPaths()
            # print 'You selected %d files:' % len(paths)

            # print 'on_fileopen set filepath',filepath
            # self.set_widgetvalue(filepathlist_to_filepathstring(filepaths))

            # save path as comma separated string, but not with special primes
            self.set_widgetvalue(','.join(filepaths))
            if self.immediate_apply:
                self.apply_valuewidget_to_obj()
            # print '    self.get_widgetvalue()',self.get_widgetvalue()
        dlg.Destroy()


class DirpathWidgetContainer(AttrBase, TextWidgetContainer):
    def define_widgetset(self):
        """
        Generates the widgets representing this attribute.
        """
        return [('name',    self.create_namewidget(),  wx.ALIGN_RIGHT | wx.ALIGN_CENTER_VERTICAL),
                ('value',   self.create_valuewidget(), wx.EXPAND),
                ('unit',   self.create_unitwidget(), wx.ALIGN_LEFT | wx.ALIGN_CENTER_VERTICAL), ]

    def create_unitwidget(self):
        if self._attrconf.is_writable():
            #widget = wx.Button(self.parent, wx.ID_OPEN)
            bitmap = wx.ArtProvider.GetBitmap(wx.ART_FOLDER_OPEN, wx.ART_TOOLBAR)
            widget = wx.BitmapButton(self.parent, -1, bitmap, (12, 12),
                                     (bitmap.GetWidth()+6, bitmap.GetHeight()+6))
            widget.Bind(wx.EVT_BUTTON, self.on_diropen)
            # print 'create_unitwidget',self._attrconf.attrname,widget
            return widget
        else:
            return (0, 0)

    def on_diropen(self, event):
        dlg = wx.DirDialog(self.parent, message="Open directory",
                           #style=wx.DD_DEFAULT_STYLE| wx.DD_DIR_MUST_EXIST| wx.DD_CHANGE_DIR
                           style=wx.DEFAULT_DIALOG_STYLE | wx.RESIZE_BORDER | wx.DD_CHANGE_DIR
                           )

        if dlg.ShowModal() == wx.ID_OK:
            dirpath = dlg.GetPath()
            self.set_widgetvalue(dirpath)
            if self.immediate_apply:
                self.apply_valuewidget_to_obj()
            # print '    self.get_widgetvalue()',self.get_widgetvalue()
        dlg.Destroy()


class ScalarPanel(wx.Panel):
    """
    Interactively displays scalar attributes of object on a parent panel.
    """

    def __init__(self, parent,  attrconfigs=[], id=None,
                 func_change_obj=None,
                 mainframe=None, panelstyle='default', immediate_apply=None):
        # wx.Panel.__init__(self,parent,-1,style=wx.WANTS_CHARS)
        wx.Panel.__init__(self, parent, -1, size=(-1, 300))
        #wx.scrolledpanel.ScrolledPanel.__init__(self, parent,wx.ID_ANY)
        #self.maxWidth  = 1000
        #self.maxHeight = 1000
        #self.SetVirtualSize((self.maxWidth, self.maxHeight))
        # self.SetScrollRate(10,10)

        # parent must be panel, scrolled panel or similar
        self.parent = parent

        self.id = id

        # target obj
        #self.obj = obj

        self.func_change_obj = func_change_obj
        self.mainframe = mainframe
        #self.panelstyle =  panelstyle
        #self.immediate_apply = immediate_apply

        # background colors for lines shading:
        self.tablecolors = get_tablecolors()

        # print 'ScalarPanel.__init__ id=',self.id#,parent,self.obj
        # print '  immediate_apply=',immediate_apply

        self.widgetcontainers = []

        self.create_attrs(attrconfigs, immediate_apply=immediate_apply, panelstyle=panelstyle)
        self.SetAutoLayout(True)
        self.Refresh()

    # def get_pentable(self):
    #    """
    #    Returns pentable used for this panel
    #    """
    #    return self.parent.get_pentable()

    def create_attrs(self, attrconfigs, immediate_apply=False, panelstyle='default'):
        """
        Go through list with attribute names and create widgets and
        put them in a grid sizer.
        """
        # print '\n\ncreate_attrs',len(attrconfigs)
        # print '  mt=',self.obj.get_config(attrs[0])['metatype']

        # self.add_id_named(name,id,colors=color)
        # get widgets
        # id=1
        i = 0
        for attrconfig in attrconfigs:

            # generates alternating colors (currently not in use)
            color_bg = self.tablecolors[divmod(i, len(self.tablecolors))[1]]
            container = self.get_widgetcontainer(attrconfig, color_bg=color_bg,
                                                 immediate_apply=immediate_apply,
                                                 panelstyle=panelstyle)
            self.widgetcontainers.append(container)
            # self.add_id_named(attr,widgetcontainers=container)
            # self.create_ids(id,widgetcontainers=container)
            i += 1

        # now ask for widget names and try to align them
        widgetnames = []
        for container in self.widgetcontainers:
            names = container.get_widgetnames()

            if len(widgetnames) == 0:
                widgetnames = names
            else:
                widgetnames = self._align(widgetnames, names)

            # print ' result of alignment:',attr,names,widgetnames
            #    widgetnames.index...

        # now dimensions of grid are known, so configure sizer
        # print 'grid size=',len(attrs), len(widgetnames)
        sizer = wx.FlexGridSizer(len(attrconfigs), len(widgetnames), vgap=2, hgap=0)
        sizer.AddGrowableCol(1)
        # print 'widgetnames',widgetnames
        # throw widgets into sizer
        for container in self.widgetcontainers:
            # print 'build grid for attr',container._attrconf.attrname
            # container=self.get_value_named('widgetcontainers',attr)
            # print 'attr',attr,widgetnames
            widgetsinfo = container.get_widgetsinfo(widgetnames)
            # print '  widgetsinfo=',widgetsinfo
            for widget, align in widgetsinfo:
                # print '    add widget align=',widget,align
                sizer.Add(widget, 0, align)

        # works on panel...
        #parent.SetHelpText('The Agile Panel')
        self.SetAutoLayout(True)
        self.SetSizer(sizer)
        sizer.Fit(self)

    def get_widgetcontainer(self, attrconf, **args):
        """
        Figures out what widget container, containing a set of widgets,
        would be the most aprobriate to represent this attribute.
        """
        # print '\n\nScalarPanel.get_widgetcontainer',attrconf.attrname

        # config=self.obj.get_config(attr)
        # get type list (full depth)
        # t=attrconfig.get_default()

        # take top level type
        # tt=t[0]
        tt = type(attrconf.get_default())

        # get metatype
        mt = attrconf.get_metatype()

        # print ' default,mt,tt',attrconf.get_default(),type(attrconf.get_default()),mt,tt,mt in METATYPES_LINKSTYLE

        # if config.has_key('choices'):
        #    # pop up with a choices list
        #    return ChoiceWidgetContainer(self,attr,**args)

        # check if this is an id within a referenced object
        # if mt=='id':
        #    return IdWidgetContainer(self,attr,**args)
        #
        # check  if widgets for metatypes are availlable
        # elif mt=='color':
        #    #print '  ->WidgetContainer'
        #    return ColorWidgetContainer(self,attr,**args)

        # elif mt=='pentype':
        #    return PentypeContainer(self,attr,**args)

        if mt in METATYPES_LINKSTYLE:
            return ObjWidgetContainer(self, attrconf, **args)

        elif mt == 'color':
            return ColorWidgetContainer(self, attrconf, **args)

        elif hasattr(attrconf, 'choices'):
            return ChoiceWidgetContainer(self, attrconf, **args)

        elif mt == 'filepath':
            return FilepathWidgetContainer(self, attrconf, **args)

        elif mt == 'filepaths':
            return FilepathsWidgetContainer(self, attrconf, **args)

        elif mt == 'dirpath':
            return DirpathWidgetContainer(self, attrconf, **args)
        elif mt == 'datetime':
            return DatetimeWidgetContainer(self, attrconf, **args)

        # elif mt == 'number':
        if tt in (types.IntType, types.LongType):
            return IntegerWidgetContainer(self, attrconf, **args)
        elif tt in (types.FloatType, types.ComplexType):
            return NumericWidgetContainer(self, attrconf, **args)

        # check now native types

        elif tt in (types.BooleanType,):
            return BooleanWidgetContainer(self, attrconf, **args)

        elif tt in STRINGTYPES:
            return TextWidgetContainer(self, attrconf, **args)

        # elif tt in (types.InstanceType,types.ClassType):
        #    return ObjWidgetContainer(self,attrconf,**args)

        elif tt in (types.ListType, types.TupleType):
            return ListWidgetContainer(self, attrconf, **args)

        else:
            # else means use a the base class that can display anything
            return WidgetContainer(self, attrconf, **args)
        # else:
        #    # else means use a the base class that can display anything
        #    return WidgetContainer(self,attr,self,**args)

    def apply(self):
        """
        Widget values are copied to object
        """
        for widgetcontainer in self.widgetcontainers:
            widgetcontainer.apply_valuewidget_to_obj()
        # this is to update widgets of function values
        # self.restore()

    def restore(self):
        """
        Object values are copied into widgets.
        """
        for widgetcontainer in self.widgetcontainers:
            widgetcontainer.apply_obj_to_valuewidget()

    def _align(self, widgetnames, names):
        """
        tries to align widgetnames with a new list of names
        """
        # print '_align'
        names_prepend = []
        i = 0

        while (len(names_prepend) == 0) & (i < len(names)):
            if widgetnames.count(names[i]) > 0:
                names_prepend = names[:i]
            i += 1

        # print '  names_prepend',names_prepend
        newnames = widgetnames
        for name in names_prepend:
            self._prepend_name(newnames, name)

        for name in names:
            self._append_name(newnames, name)

        return newnames

    def _append_name(self, names, name):
        """
        Appends name if not in list
        """
        if names.count(name) == 0:
            names.append(name)

    def _prepend_name(self, names, name):
        """
        Predends name if not in list
        """
        if names.count(name) == 0:
            names.insert(name, 0)


class ScalarPanelScrolled(wxlib.scrolledpanel.ScrolledPanel, ScalarPanel):
    """
    Interactively displays scalar attributes of object on a parent panel.
    """

    def __init__(self, parent,  attrconfigs=[], id=None,
                 func_change_obj=None,
                 mainframe=None, is_modal=False,
                 panelstyle='default', immediate_apply=None):
        # wx.Panel.__init__(self,parent,-1,style=wx.WANTS_CHARS)
        # wx.Panel.__init__(self,parent,-1)
        if is_modal:
            wxlib.scrolledpanel.ScrolledPanel.__init__(self, parent, wx.ID_ANY, size=(600, 400))
        else:
            wxlib.scrolledpanel.ScrolledPanel.__init__(self, parent, wx.ID_ANY, size=wx.DefaultSize)
        #self.maxWidth  = 2000
        #self.maxHeight = 300
        #self.SetVirtualSize((self.maxWidth, self.maxHeight))
        self.SetScrollRate(10, 10)

        # parent must be panel, scrolled panel or similar
        self.parent = parent

        self.id = id

        # target obj
        #self.obj = obj

        self.func_change_obj = func_change_obj
        self.mainframe = mainframe
        #self.panelstyle =  panelstyle
        #self.immediate_apply = immediate_apply

        # background colors for lines shading:
        self.tablecolors = get_tablecolors()

        # print 'ScalarPanel.__init__'#,parent,self.obj
        # print '  immediate_apply=',immediate_apply

        self.widgetcontainers = []

        self.create_attrs(attrconfigs, immediate_apply=immediate_apply, panelstyle=panelstyle)
        # self.SetAutoLayout(True)
        # self.AutoLayout()
        # self.Refresh()
        # self.SetAutoLayout(1)
        # self.SetupScrolling()

        self.Refresh()
        # self.Layout()

        # self.SetupScrolling()

        # self.Refresh()
        # self.Layout()


class TableGrid(AttrBase, gridlib.PyGridTableBase):

    """
    This is all it takes to make a custom data table to plug into a
    wxGrid.  There are many more methods that can be overridden, but
    the ones shown below are the required ones.  This table simply
    provides strings containing the row and column values.
    """

    def __init__(self, parent, tab, attrconfigs, ids, show_ids=False, **args):
        gridlib.PyGridTableBase.__init__(self)

        self.odd = gridlib.GridCellAttr()
        self.odd.SetBackgroundColour("sky blue")
        self.even = gridlib.GridCellAttr()
        self.even.SetBackgroundColour("sea green")

        # target obj
        self.tab = tab
        self.parent = parent

        # print 'gridlib.PyGridTableBase\n'#,dir(gridlib.PyGridTableBase)
        self.ids = ids
        self.show_ids = show_ids
        self.attrconfigs = attrconfigs

        # print 'TableGrid: attrs,ids=',len(self.attrconfigs),'X',len(self.ids)

        col = 0
        self.colnames = []
        self.celltypes = []
        # self.cellattrs=[]
        # print '  make header'
        for attrconf in attrconfigs:
            if hasattr(attrconf, 'symbol'):
                symbol = attrconf.symbol
            else:
                symbol = attrconf.get_name()
            unit = attrconf.format_unit(show_parentesis=True)
            if len(unit) > 0:
                if len(symbol) > 7:
                    symbol += '\n'+unit
                else:
                    symbol += ' '+unit
            # print '    symbol',symbol
            self.colnames.append(symbol)
            self.celltypes.append(self.get_celltype(attrconf))
            col += 1
        # print '  done: header'

    def get_celltype(self, attrconf):
        """
        Returns type code string  of a wx cell.
        http://permalink.gmane.org/gmane.comp.python.wxpython/102660
        """
        # config=self.obj.get_config(attr)
        # tt=config['type'][0]
        # check fist if widgets for metatypes are availlable
        # if mt=='color':
        #    #print '  ->WidgetContainer'
        #    return 'color'
        #    #return WidgetContainer(self,attr,parent,**args)

        # if config.has_key('choices'):
        #    choicestring=wxGRID_VALUE_CHOICE+':'
        #    for c in config['choices']:
        #        choicestring+=c+','
        #    choicestring=choicestring[:-1]
        #    return choicestring

        # check now native types
        # if len(config['type'])==1:
        mt = attrconf.metatype
        tt = type(attrconf.get_default())
        # print 'get_celltype',attrconf.attrname,mt,tt,hasattr(attrconf,'choices'),attrconf.is_writable()

        if mt == 'objs':
            return gridlib.GRID_VALUE_STRING

        elif mt == 'id':
            obj = attrconf.get_linktab()
            if attrconf.is_writable():
                if hasattr(attrconf, 'choices'):
                    if len(attrconf.choices) < 200:
                        # print '  choices=',attrconf.choices
                        # (types.ListType, types.TupleType):
                        if type(attrconf.choices) in (OrderedDict, types.DictionaryType):
                            # print '  GridCellChoiceEditor',attrconf.choices,':'+','.join(attrconf.choices.keys())
                            return gridlib.GRID_VALUE_CHOICE+':'+','.join(attrconf.choices.keys())
                        else:
                            # print '  GridCellChoiceEditor',attrconf.choices,':'+','.join(attrconf.choices)
                            return gridlib.GRID_VALUE_CHOICE+':'+','.join(attrconf.choices)

                            #gridlib.GridCellChoiceEditor(['what','ever'], allowOthers=True)
                    else:
                        return gridlib.GRID_VALUE_STRING  # still display a string with choice

                elif len(obj) < 200:
                    # print '  GridCellChoiceEditor',attrconf.attrname,':'+obj.format_ids(obj.get_ids())
                    return gridlib.GRID_VALUE_CHOICE+':'+obj.format_ids(obj.get_ids())
                else:
                    return gridlib.GRID_VALUE_STRING

            return gridlib.GRID_VALUE_STRING

        elif mt == 'tabid':
            return gridlib.GRID_VALUE_STRING

        elif mt == 'ids':
            return gridlib.GRID_VALUE_STRING

        elif hasattr(attrconf, 'choices'):
            if attrconf.is_writable():
                if len(attrconf.choices) < 200:
                    # print '  dir(gridlib)',dir(gridlib)
                    # (types.ListType, types.TupleType):
                    if type(attrconf.choices) in (OrderedDict, types.DictionaryType):
                        # print '  GridCellChoiceEditor',attrconf.choices,':'+','.join(attrconf.choices.keys())
                        return gridlib.GRID_VALUE_CHOICE+':'+','.join(attrconf.choices.keys())
                    else:
                        # print '  GridCellChoiceEditor',attrconf.choices,':'+','.join(attrconf.choices)
                        return gridlib.GRID_VALUE_CHOICE+':'+','.join(attrconf.choices)

                        #gridlib.GridCellChoiceEditor(['what','ever'], allowOthers=True)
                else:
                    return gridlib.GRID_VALUE_STRING
            else:
                return gridlib.GRID_VALUE_STRING

        elif tt in (types.LongType, types.IntType):
            if (hasattr(attrconf, 'min') & hasattr(attrconf, 'max')):
                return gridlib.GRID_VALUE_NUMBER+':'\
                    + str(attrconf.min)+','+str(attrconf.max)
            else:
                return gridlib.GRID_VALUE_NUMBER

        elif tt in (types.FloatType, types.ComplexType):
            if (hasattr(attrconf, 'digits_integer') & hasattr(attrconf, 'digits_fraction')):
                return gridlib.GRID_VALUE_FLOAT+':'\
                    + str(attrconf.digits_integer)+','\
                    + str(attrconf.digits_fraction)
            else:
                return gridlib.GRID_VALUE_FLOAT

        elif tt in STRINGTYPES:
            return gridlib.GRID_VALUE_STRING

        elif tt in (types.BooleanType,):
            return gridlib.GRID_VALUE_BOOL

        else:
            # else means use a the base class that can display anything
            return gridlib.GRID_VALUE_STRING

    def GetNumberRows(self):
        return len(self.ids)

    def GetNumberCols(self):
        return len(self.attrconfigs)

    def IsEmptyCell(self, row, col):
        return (row > len(self.ids)) | (col > len(self.attrconfigs))

    def GetValue(self, row, col):
        """
        Returns object value to be displayed in the cell.
        """
        # TODO: this should be all handled by format_value of the attrconf !!!!
        # try:
        if 1:
            rowid, attrconf = self.get_id_attrconf(row, col)
            if (rowid is None) | (attrconf is None):
                return 'Err'
            # attrconf=self.attrconfigs[col]
            val = self.get_objvalue(row, col)
            mt = attrconf.metatype

            # print 'GetValue',self.ids[row],attrconf.attrname,val,mt,hasattr(attrconf,'choices'),attrconf.is_writable()

            if mt == 'objs':
                return val.format_ident()

            elif mt == 'id':
                # print '  id=',val,idtext
                # print '  linktab',attrconf.get_linktab(),idtext
                if val == -1:
                    return '-'

                if hasattr(attrconf, 'choices'):
                    # this is for backwards compatibility
                    # choices should no longer osed to index id
                    # instead, the format_ids method should be used
                    # (types.ListType, types.TupleType):
                    if type(attrconf.choices) in (OrderedDict, types.DictionaryType):
                        if attrconf.choices.values().count(val) > 0:
                            ind = attrconf.choices.values().index(val)
                            # print '  return',attrconf.choices.keys()[ind]
                            return attrconf.choices.keys()[ind]
                        else:
                            return attrconf.get_linktab().format_ids([val])
                    else:
                        # print '  GridCellChoiceEditor',attrconf.choices,':'+','.join(attrconf.choices)
                        return attrconf.get_linktab().format_ids([val])

                else:
                    # print ' return ',attrconf.get_linktab().format_ids([val])
                    return attrconf.get_linktab().format_ids([val])

            elif mt == 'ids':
                # print '  ids=',val,attrconf.get_linktab().format_ids(val)
                if val is None:
                    return ''
                else:
                    return attrconf.get_linktab().format_ids(val)

            elif mt == 'tabid':
                obj, _id = val
                return obj.format_ident_row(_id)

            elif mt == 'tabidlist':
                return attrconf.format_value(rowid)  # <<<<<<<< should work for all types !!!

            elif hasattr(attrconf, 'choices'):
                # print '   attrconf.choices',attrconf.choices
                if type(attrconf.choices) in (OrderedDict, types.DictionaryType):  # (types.ListType, types.TupleType):
                    if attrconf.choices.values().count(val) > 0:
                        ind = attrconf.choices.values().index(val)
                        # print '  return',attrconf.choices.keys()[ind]
                        return attrconf.choices.keys()[ind]
                    else:
                        return val
                else:
                    if attrconf.choices.count(val) > 0:
                        ind = attrconf.choices.index(val)
                        # print '  return',attrconf.choices[ind]
                        return attrconf.choices[ind]
                    else:
                        return val

            elif mt == 'color':  # special config for colors
                # done by cell backgroundcolor
                return ''

            # elif config.has_key('do_init_arrayobj'):
            #    if config['do_init_arrayobj']:
            #        return val.ident
            #    else:
            #        return val

            else:
                return val

        else:
            return 'Err'
        # except IndexError:
        #    return 'Err'

    def get_objvalue(self, row, col):
        """
        Returns value of referred object according to row and col of grid
        """
        return self.attrconfigs[col][self.ids[row]]

    def get_valueconfigs(self):
        return self.attrconfigs

    def get_id_attrconf(self, row, col):
        if (col >= 0) & (col < len(self.attrconfigs)):
            if (row >= 0) & (row < len(self.ids)):
                return self.ids[row], self.attrconfigs[col]
            else:
                return None, self.attrconfigs[col]
        else:
            if (row >= 0) & (row < len(self.ids)):
                return self.ids[row], None
            else:
                return None, None

    # def get_perm(self, attrconf,id):
    #    """
    #    Return read write permission
    #    """
    #    # TODO: must be enhanced with grid specific rw
    #    #print 'get_perm',id, attr
    #    return attrconf.get_perm()

    def SetValue(self, row, col, value):
        """
        Transfer of cell value to object.
        """
        id, attrconf = self.get_id_attrconf(row, col)
        if attrconf is not None:

            if attrconf.is_writable():
                mt = attrconf.metatype
                # print 'SetValue(%d, %d, "%s").\n' % (row, col, value)

                # print '    attrconf',id,attrconf.attrname, attrconf.is_writable(),hasattr(attrconf,'choices')
                # attr=self.attrs[col]
                tt = type(attrconf.get_default())
                if hasattr(attrconf, 'choices'):
                    # print '  type(attrconf.choices)',type(attrconf.choices),type(attrconf.choices) in (OrderedDict, types.DictionaryType)
                    # (types.ListType, types.TupleType):
                    if type(attrconf.choices) in (OrderedDict, types.DictionaryType):
                        # print '  set choices[value]',attrconf.choices[value]
                        if attrconf.choices.has_key(value):
                            attrconf[id] = attrconf.choices[value]
                        else:
                            attrconf[id] = value

                    else:
                        # print '  set value',value
                        attrconf[id] = value
                elif mt == 'id':
                    attrconf[id] = attrconf.get_linktab().get_id_from_formatted(value)

                elif (tt in NUMERICTYPES):
                    # set only values of types that are allowed for the grid
                    # TODO: this grid types data must be organized more central
                    attrconf[id] = value
                else:
                    # TODO: for other types, like color or arrays, this must be done beforehand
                    print 'WARNING in SetValue: cannot write to this type:', tt
            else:
                # readonly
                pass
                # print 'SetValue(%d, %d, "%s") read only.\n' % (row, col, value)

# --------------------------------------------------
    # Some optional methods

    # Called when the grid needs to display labels
    def GetColLabelValue(self, col):
        if (col > -1) & (col < len(self.colnames)):
            return self.colnames[col]
        else:
            return '-'

    def GetRowLabelValue(self, col):
        return self.FormatRowLabel(self.tab, self.ids[col])

    def FormatRowLabel(self, tab, id):
        # if tab.is_keyindex():
        #    label = str(tab.get_key_from_id(id))
        # else:
        label = str(id)
        return label

    #    if size is None:
    #        return label
    #    else:
    #        if size>=len(label):
    #            return (size*' '+label)[-size:]
    #        else:
    #            return label

    # def ComputeRowLabelSize(self):
    #    size = 0
    #    tab = self.tab
    #    for id in self.ids:
    #        l = len(self.FormatLabel( tab, id))
    #        if l>size:
    #            size=l
    #    print 'ComputeRowLabelSize',size*16
    #    return size*16# TODO here should be the font size as multiplier

    def ComputeColLabelSize(self):
        """
        Compute the maximum number of lines required for column labelling
        """
        # print 'ComputeColLabelSize',self.colnames
        lines = 0
        for name in self.colnames:
            l = name.count('\n')
            if l > lines:
                lines = l
        size = (lines+1)*32
        # print '  size=',size
        return size  # TODO here should be the font size as multiplier

    def CanHaveAttributes(self):
        # TODO: if the story with returning the attribute
        # for each cell is getting too slow we might want to restrict this
        # to cases when we actually need to modify the attribute
        # as for example with colors
        return True

    def GetAttr(self, row, col, flag=0):
        # print 'GetAttr row,col,flag',row,col,flag
        attrconf = self.attrconfigs[col]
        # print '  self.ids',self.ids
        id = self.ids[row]
        # config=self.obj.get_config(attr)
        tt = type(attrconf.get_default())
        mt = attrconf.metatype
        val = attrconf[id]
        # print 'GetAttr',attrconf.attrname,val#,config['type'][-1]
        # define cell attribute for this column
        cellattr = gridlib.GridCellAttr()

        if attrconf.is_readonly():
            cellattr.SetReadOnly(True)

        if mt == 'color':  # special config for colors

            cint = np.array(np.array(val)*255, np.int32)

            # print '  cint=',cint
            # cellattr.SetBackgroundColour(wx.Colour(cint[0],cint[1],cint[2],cint[3]))
            cellattr.SetBackgroundColour(wx.Colour(*cint))

        # elif config['metatype']=='penstyle':
        #    cellattr.SetRenderer(PenstyleRenderer(self.obj.get_pentable()))

        # check whether to make a hyperlink in blue to an instance
        # if len(config['type'])==1:
        # if (tt in (types.InstanceType,types.ClassType)):
        #    if hasattr(val,'ident'):
            # print '  is configurator object',val.ident
        if mt in METATYPES_LINKSTYLE:
            cellattr.SetTextColour('blue')
            # no!!cellattr.SetFlags (wx.TEXT_ATTR_FONT_UNDERLINE)
            cellattr.SetReadOnly(False)

        return cellattr

    # Called to determine the kind of editor/renderer to use by
    # default, doesn't necessarily have to be the same type used
    # natively by the editor/renderer if they know how to convert.
    def GetTypeName(self, row, col):
        if (col > -1) & (col < len(self.celltypes)):
            return self.celltypes[col]
        else:
            return wx._mainframe  # this can do anything

    # Called to determine how the data can be fetched and stored by the
    # editor and renderer.  This allows you to enforce some type-safety
    # in the grid.

    def CanGetValueAs(self, row, col, typeName):

        colType = self.celltypes[col].split(':')[0]
        if typeName == colType:
            return True
        else:
            return False

    def CanSetValueAs(self, row, col, typeName):
        #id, attr = self.get_id_attrconf( row, col )
        # print 'CanSetValueAs',id, attr
        return self.CanGetValueAs(row, col, typeName)


# ---------------------------------------------------------------------------

# class TablePanel(wx.Panel):
#    def __init__(self, parent, tab, **args):
#        wx.Panel.__init__(self, parent, -1)
#        grid = TableFrame(self,tab, **args)
#        self.Sizer = wx.BoxSizer()
#        self.Sizer.Add(grid, 1, wx.EXPAND)
#
#        #grid.SetReadOnly(5,5, True)
#    def restore(self):
#        pass
#        #self.grid.restore()


class TabPanel(AttrBase, gridlib.Grid):
    def __init__(self, parent, tab, objpanel=None, attrconfigs=None, ids=None, show_ids=True,
                 func_change_obj=None,
                 func_choose_id=None,
                 func_choose_attr=None,
                 func_apply=None,
                 mainframe=None, **args):
        """
        parent: parent widget, containing the grid
        tab:    table containing the actual data of an object
        """
        # TODO: Attention: parent is always the main object panel

        # print '  gridlib.Grid',dir(gridlib.Grid)
        gridlib.Grid.__init__(self, parent, -1)  # , style =wx.EXPAND| wx.ALL)#wx.EAST | wx.WEST) )

        # parent must be panel, scrolled panel or similar
        self.parent = parent
        self.tab = tab
        self._objpanel = objpanel
        self.func_change_obj = func_change_obj
        self.func_choose_id = func_choose_id
        self.func_choose_attr = func_choose_attr
        self.func_apply = func_apply
        self.mainframe = mainframe

        if ids is None:
            # show all ids
            ids = tab.get_ids()  # ordered=True

        if attrconfigs is None:
            # show all array type attributes
            attrconfigs = tab.get_colconfigs()

        # print 'init TablePanel',tab.get_ident(),ids
        # print '  start init TableGrid',len(ids),len(attrconfigs)
        # this table contains the gui functionalily of the grid
        table = TableGrid(self, tab, attrconfigs=attrconfigs, ids=ids,
                          show_ids=show_ids, **args)
        # print '  end init TableGrid'
        # The second parameter means that the grid is to take ownership of the
        # table and will destroy it when done.  Otherwise you would need to keep
        # a reference to it and call it's Destroy method later.
        self.SetTable(table, True)
        # print '  done SetTable'
        # self.SetRowLabelSize(16)
        # self.SetColLabelSize(16)
        # self.SetRowLabelSize(table.ComputeRowLabelSize())
        self.SetColLabelSize(table.ComputeColLabelSize())
        # print '  done SetColLabelSize'
        #self.AutoSize ()

        # for col in xrange(len(attrconfigs)):
        #    self.AutoSizeColLabelSize (col)

        # these operations are nice but slow for huge tables!!
        if len(ids) < 100:
            self.AutoSizeColumns(True)
        # print '  done AutoSizeColumns'
        if len(ids) < 100:
            self.AutoSizeRows(True)
        # print '  done AutoSizeRows'

        # self.Bind(gridlib.EVT_GRID_CELL_CHANGE, self.on_cell_change) #works, but not used

        # EVT_GRID_CELL_RIGHT_CLICK(self, self.OnRightDown)  #added
        gridlib.EVT_GRID_CELL_RIGHT_CLICK(self, self.on_contextmenu)  # added
        self.Bind(gridlib.EVT_GRID_LABEL_RIGHT_CLICK, self.on_contextmenu_label)

        # does not seem to work
        gridlib.EVT_GRID_CELL_LEFT_DCLICK(self, self.on_edit_cell)
        gridlib.EVT_GRID_CELL_LEFT_CLICK(self, self.on_click_cell)

        gridlib.EVT_GRID_LABEL_LEFT_DCLICK(self, self.OnLabelLeftDoubleClick)
        # ??gridlib.EVT_GRID_LABEL_LEFT_CLICK(self, self.OnLabelLeftClick)
        #EVT_CONTEXT_MENU(self, self.on_contextmenu)
        #self.Bind(wx.EVT_CONTEXT_MENU, self.on_contextmenu)
        # print '  done init TabPanel'
    # def on_contextmenu(self, event):
    #    print '\non_contextmenu!!!'

    def Reset(self):
        """reset the view based on the data in the table.  Call
        this when rows are added or destroyed"""
        # self._table.ResetView(self)
        self.AdjustScrollbars()
        self.ForceRefresh()

    def logw(self, s):
        """
        Write to logger if existent print otherwise
        """
        pass

    def on_cell_change(self, event):
        Row = event.GetRow()
        Col = event.GetCol()

        # All cells have a value, regardless of the editor.
        # print 'Changed cell: (%u, %u)' % (Row, Col)
        # print 'value: %s' % self.grid1.GetCellValue(Row, Col)

    def get_obj(self):
        """
        Returns object wich is represented by this table.
        """
        return self.tab.get_obj()

    def on_contextmenu(self, event):
        # print "on_contextmenu (%d,%d) %s\n" %\
        #               (event.GetRow(), event.GetCol(), event.GetPosition())
        menu = AgilePopupMenu(self.parent)

        # tell popup menue in which cell it has been called
        # in this way a later event can retrieve cell coordinates
        menu.set_row(event.GetRow())
        menu.set_col(event.GetCol())
        menu.append_item('edit...', self.on_edit_cell_from_popupmenu,
                         info='Edit data of this cell')

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.
        self.parent.PopupMenu(menu)
        menu.Destroy()

    def on_edit_cell_from_popupmenu(self, event):
        """Start cell editor"""
        popupmenu = event.GetEventObject()

        table = self.GetTable()
        id, attrconf = table.get_id_attrconf(popupmenu.get_row(), popupmenu.get_col())
        # print 'on_edit_cell EventObject=',id, attrconf.attrname#popupmenu
        if (id >= 0) & (attrconf is not None):

            # dlg = ObjPanelDialog(self,self.tab.get_obj(),table = self.tab,id,attrconfigs=[attrconf,], size=(350, 200),
            #             #style = wxCAPTION | wxSYSTEM_MENU | wxTHICK_FRAME
            #             style = wx.DEFAULT_DIALOG_STYLE
            #             )

            dlg = ObjPanelDialog(self,
                                 self.tab.get_obj(),
                                 attrconfigs=[attrconf, ],
                                 #tables = None,
                                 table=self.tab, id=id, ids=None,
                                 #groupnames = None,
                                 # title = '', size = wx.DefaultSize, pos = wx.DefaultPosition,\
                                 style=wx.DEFAULT_DIALOG_STYLE,
                                 # choose_id=False, choose_attr=False,
                                 # func_choose_id=None,func_change_obj=None,
                                 func_apply=self.func_apply,
                                 #panelstyle = 'default',
                                 #immediate_apply = False
                                 )

            dlg.CenterOnScreen()

            # this does not return until the dialog is closed.
            val = dlg.ShowModal()

            if val == wx.ID_OK:
                # apply current widget values
                dlg.apply()
            else:
                # print ">>>>>>>>>You pressed Cancel\n"
                pass

            dlg.Destroy()

            event.Skip()
            self.ForceRefresh()
        #sz = miniframe.GetBestSize()
        #miniframe.SetSize( (sz.width+20, sz.height+20) )

    def on_contextmenu_label(self, event):
        # print "on_contextmenu_label (%d,%d) %s\n" %\
        #               (event.GetRow(), event.GetCol(), event.GetPosition())
        menu = AgilePopupMenu(self.parent)

        # tell popup menue in which cell it has been called
        # in this way a later event can retrieve cell coordinates
        col = event.GetCol()
        row = event.GetRow()
        menu.set_row(row)
        menu.set_col(col)
        if col < 0:

            # popup row options menu
            self._contextfunctions = {}
            # print '  configs',self.tab.get_configs(is_all=True)
            for config in self.tab.get_configs(is_all=True):
                # print '  ',config.attrname,config.groupnames,'rowfunctions' in config.groupnames
                if 'rowfunctions' in config.groupnames:
                    item, id = menu.append_item(config.get_name(), self.on_select_rowfunction, info=config.get_info())
                    #item,id = menu.append_item( config.name,config.get_function(), info=config.info)
                    self._contextfunctions[id] = config.get_function()
                    # print '  append ',config.name,id
                    #menu.append_item( config.name,self.on_select_contextmenu, info=config.info)

        # default
        menu.append_item('Export to CSV...', self.on_export_csv,
                         info='Export entire table in CSV format.')

        # Menu on_highlight events
        #self.parent.Bind(wx.EVT_MENU_HIGHLIGHT_ALL, self.on_)

        # Popup the menu.  If an item is selected then its handler
        # will be called before PopupMenu returns.

        self.parent.PopupMenu(menu)
        menu.Destroy()

    def on_dummy(self, event):
        # event.Skip()
        pass

    def on_select_rowfunction(self, event):
        popupmenu = event.GetEventObject()
        # Show how to get menu item info from this event handler
        table = self.GetTable()
        row = popupmenu.get_row()
        col = popupmenu.get_col()
        id, attrconf = table.get_id_attrconf(row, col)
        id_menu = event.GetId()
        # print 'on_select_contextmenu id_menu,id, attrconf',id_menu,id, attrconf
        # print '  GetSelection',event.GetSelection()
        # print '  GetString',event.GetString()

        # print '  GetId',id_menu,event.Id
        # print '  GetClientObject',event.GetClientObject()

        # call selected row function with row id
        self._contextfunctions[id_menu](id)

        # item = popupmenu.get_menuitem_from_id(event.GetId())# OK but not neede
        # if self._rowfunctions.has_keid_menu:
        #    funcname = item.GetText()
        #    #help = item.GetHelp()
        #    #??? menu id->name->function??????????
        #    print 'on_select_contextmenu: found function:',funcname#,getattr(table,funcname)
        #    self._rowfunctions[id]
        # else:
        #    print 'on_select_contextmenu: No function found'

        event.Skip()
        # self.ForceRefresh()
        # self.grid.AutoSizeColumn(col)
        self.Reset()
        self.Refresh()
        self.ForceRefresh()

        #del table
        newtable = TableGrid(self, self.tab, attrconfigs=self.tab.get_colconfigs(),
                             ids=self.tab.get_ids(), show_ids=table.show_ids)
        del table
        # The second parameter means that the grid is to take ownership of the
        # table and will destroy it when done.  Otherwise you would need to keep
        # a reference to it and call it's Destroy method later.
        self.SetTable(newtable, True)

        return None

    def on_export_csv(self, event):
        wildcards_all = "CSV file (*.csv,*.txt)|*.csv;*.CSV;*.txt;*.TXT|All files (*.*)|*.*"
        # if hasattr(attrconf,'wildcards'):
        #    wildcards = attrconf.wildcards+"|"+wildcards_all
        # else:
        #    wildcards = wildcards_all
        dlg = wx.FileDialog(self.parent, message="Export table %s to CSV" % self.tab.get_name(),
                            # defaultDir=os.getcwd(),
                            defaultFile="",
                            wildcard=wildcards_all,
                            style=wx.SAVE | wx.CHANGE_DIR
                            )

        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            path = dlg.GetPath()
            # print 'You selected %d files:' % len(paths)
            if type(path) in STRINGTYPES:
                self.tab.export_csv(path, sep=',', name_id='ID',
                                    file=None, attrconfigs=self.GetTable().get_valueconfigs(),
                                    groupname=None, is_header=True)

    def on_click_cell(self, event):
        """
        Action for first click on cell.
        """
        # check if there is something to navigate
        if self.CanEnableCellControl():
            table = self.GetTable()
            id, attrconf = table.get_id_attrconf(event.GetRow(), event.GetCol())
            mt = attrconf.metatype
            # config=table.obj.get_config(attr)
            # print 'on_click_cell ,id,attrconf',id, attrconf.get_name(),mt
            # print '  func_change_obj=',self.func_change_obj
            # if hasattr(attrconf,'attrname'):
            #    print '  attrconf.attrname=',attrconf.attrname

            # navigate!
            value = attrconf[id]
            # print '  value ' ,  value, type(value)
            if mt == 'obj':
                if type(value) == types.InstanceType:
                    # print '  ident?',hasattr(value,'ident')
                    if hasattr(value, 'ident'):
                        navitimer = wx.FutureCall(1, self.func_change_obj, value)

                    else:
                        event.Skip()
                else:
                    event.Skip()

            elif mt == 'id':
                if attrconf.is_readonly():
                    linktab = attrconf.get_linktab()
                    # print '  FutureCall',linktab, value
                    if value != -1:
                        navitimer = wx.FutureCall(1, self.func_change_obj, linktab, value)  # here value is id
                    else:
                        event.Skip()
                else:
                    event.Skip()

            elif mt == 'tabid':
                if value is not None:
                    linktab, id = value
                    navitimer = wx.FutureCall(1, self.func_change_obj, linktab, id)  # here value is id
                else:
                    event.Skip()

            elif mt == 'ids':
                if value is not None:
                    linktab = attrconf.get_linktab()
                    navitimer = wx.FutureCall(1, self.func_change_obj, linktab, None,  value)  # here value is ids list
                else:
                    event.Skip()
            else:
                event.Skip()
            # elif config.has_key('do_init_arrayobj'):
            #    if config['do_init_arrayobj']:
            #        navitimer = wx.FutureCall(1, self.func_change_obj,table.obj[attr,id])
            #    else:
            #        event.Skip()

        else:
            event.Skip()

    def on_edit_cell(self, event):
        """
        Decide how to to edit the cell. Possibilities are:
        - popup with special editing windows
        - in-cell editing
        """
        table = self.GetTable()
        id, attrconf = table.get_id_attrconf(event.GetRow(), event.GetCol())

        # check first if there are callback functions
        if self.func_choose_id is not None:
            # call self.func_choose_id
            #
            wx.FutureCall(1, self.func_choose_id, id, attrconf)
            # event.Skip()
            return

        elif self.func_choose_attr is not None:
            # call self.func_choose_id
            wx.FutureCall(1, self.func_choose_attr, attrconf)
            # event.Skip()
            return

        if self.CanEnableCellControl():

            #self.logw(' func_choose_id id=%d,attr=%s,ident=%s='%(id, attr,table.obj.ident))
            # print 'on_edit_cell',attrconf.attrname
            # print  '  obj=',table.obj

            # print '  config=',config

            metatype = attrconf.metatype

            if metatype == 'color':  # special config for colors
                dlg = wx.ColourDialog(self.parent)

                # Ensure the full colour dialog is displayed,
                # not the abbreviated version.
                dlg.GetColourData().SetChooseFull(True)

                if dlg.ShowModal() == wx.ID_OK:
                    # If the user selected OK, then the dialog's wx.ColourData will
                    # contain valid information. Fetch the data ...
                    c = dlg.GetColourData().GetColour()

                    # print 'on_edit_cell:'
                    # print '  wxc=',[wxc.Red(),wxc.Green(),wxc.Blue()]
                    # print '  table.obj[attr, id]=',table.obj[attr, id],type(table.obj[attr, id])
                    # print '  ffffff_to_color=',self.ffffff_to_color( [wxc.Red(),wxc.Green(),wxc.Blue()] ),type(self.ffffff_to_color( [wxc.Red(),wxc.Green(),wxc.Blue()] ))

                    # self.ffffff_to_color([wxc.Red(),wxc.Green(),wxc.Blue()] )
                    attrconf[id] = np.array([c.Red(), c.Green(), c.Blue(), c.Alpha()], np.float32)/255.0
                    self.SetCellBackgroundColour(event.GetRow(), event.GetCol(), c)
                    self.ForceRefresh()
                    if self.func_apply is not None:
                        wx.FutureCall(1, self.func_apply, table, id, None)

                # Once the dialog is destroyed, Mr. wx.ColourData is no longer your
                # friend. Don't use it again!
                dlg.Destroy()
                # event.Skip()
                # cellattr.SetBackgroundColour(obj[])

            elif metatype == 'filepath':
                filepath = self.on_fileopen(attrconf)
                if filepath is not None:
                    attrconf[id] = filepath
                    self.ForceRefresh()
                    if self.func_apply is not None:
                        wx.FutureCall(1, self.func_apply, table, id, None)

            elif metatype == 'dirpath':
                dirpath = self.on_diropen(attrconf)
                if dirpath is not None:
                    attrconf[id] = dirpath
                    self.ForceRefresh()
                    if self.func_apply is not None:
                        wx.FutureCall(1, self.func_apply, table, id, None)
            # elif metatype == 'penstyle':
            #    #Penstyles(table.obj.get_pentable())
            #    #penstyles=Penstyles(table.obj.get_pentable())
            #
            #    dlg = PenstyleDialog(self.parent,
            #                        table.obj.get_pentable(),
            #                        size = (300,400),
            #                        #choose_id=True,
            #                        #func_change_obj=self.on_edit_linestyle
            #                        )
            #
            #    if dlg.ShowModal() == wx.ID_OK:
            #        attr_current,idc_current=dlg.get_current_attrconf_id()
            #        table.obj[attr, id]=idc_current
            #
            #    self.ForceRefresh()
            #    dlg.Destroy()
            #    event.Skip()

            else:
                self.EnableCellEditControl()
                # try this ...
                # if self.func_apply is not None:
                #        wx.FutureCall(1,self.func_apply, table, id, None)
                # ...put  this in apply method

    def on_fileopen(self, attrconf):
        # print 'on_fileopen',self._attrconf.attrname
        # if type(self._attrconf.get_default())==types.StringType:
        #    defaultname = self._attrconf.get_default()
        # else:
        #    defaultname = os.getcwd()
        wildcards_all = "All files (*.*)|*.*"
        if hasattr(attrconf, 'wildcards'):
            wildcards = attrconf.wildcards+"|"+wildcards_all
        else:
            wildcards = wildcards_all
        dlg = wx.FileDialog(self.parent, message="Open file",
                            # defaultDir=os.getcwd(),
                            defaultFile="",
                            wildcard=wildcards,
                            style=wx.OPEN | wx.MULTIPLE | wx.CHANGE_DIR
                            )

        if dlg.ShowModal() == wx.ID_OK:
            # This returns a Python list of files that were selected.
            paths = dlg.GetPaths()
            # print 'You selected %d files:' % len(paths)
            if len(paths) == 1:
                return paths[0]
            elif len(paths) > 1:
                return paths
            else:
                return None
        else:
            return None

    def on_diropen(self, attrconf):
        dlg = wx.DirDialog(self.parent, message="Open directory",
                           style=wx.DD_DEFAULT_STYLE | wx.DD_DIR_MUST_EXIST | wx.DD_CHANGE_DIR
                           )

        if dlg.ShowModal() == wx.ID_OK:
            return dlg.GetPath()
        else:
            return None

    def on_edit_cell_miniframe(self, event):
        """Start miniframe cell editor"""
        EventObject = event.GetEventObject()

        table = self.GetTable()
        id, attrconf = table.get_id_attrconf(event.GetRow(), event.GetCol())
        #miniframe=DataMiniFrame( self, table.obj, id =id, attrs = [attr])
        print 'on_edit_cell_miniframe EventObject=', id, attrconf
        dlg = ObjPanelDialog(self, table.obj, id, attrconfigs=[attrconf], size=(350, 200),
                             #style = wxCAPTION | wxSYSTEM_MENU | wxTHICK_FRAME
                             func_apply=self.func_apply,
                             style=wx.DEFAULT_DIALOG_STYLE
                             )
        dlg.CenterOnScreen()

        # this does not return until the dialog is closed.
        val = dlg.ShowModal()

        if val == wx.ID_OK:
            # apply current widget values
            dlg.apply()
        else:
            # print ">>>>>>>>>You pressed Cancel\n"
            pass

        dlg.Destroy()

        evt.Skip()

        self.ForceRefresh()
        #sz = miniframe.GetBestSize()
        #miniframe.SetSize( (sz.width+20, sz.height+20) )
        # self.Refresh()#no

    def OnLabelLeftDoubleClick(self, evt):
        # print "OnLabelLeftDoubleClick: (%d,%d) %s\n" %\
        #               (evt.GetRow(), evt.GetCol(), evt.GetPosition())
        if evt.GetRow() >= 0:
            table = self.GetTable()
            # click has been on row labels = ids
            id = table.ids[evt.GetRow()]

            if self.func_choose_id is None:

                # print '  call ScalarPanel editor for ID=',id

                dlg = ObjPanelDialog(self, self.tab.get_obj(), id=id,  # table = self.tab,
                                     #title = '',
                                     #size=(350, 200),
                                     #style = wx.DEFAULT_DIALOG_STYLE
                                     func_apply=self.func_apply,
                                     )

                dlg.CenterOnScreen()

                # this does not return until the dialog is closed.
                val = dlg.ShowModal()

                if val == wx.ID_OK:
                    # apply current widget values
                    dlg.apply()
                    if self._objpanel:
                        self._objpanel.restore()  # thi will also update scalars on objpanel
                    self.ForceRefresh()
                else:
                    # print ">>>>>>>>>You pressed Cancel\n"
                    pass

                dlg.Destroy()

            else:
                # call self.func_choose_id
                wx.FutureCall(1, self.func_choose_id, id, '')

        evt.Skip()

    def OnRightDown(self, event):
        print "hello", self.GetSelectedRows()

    def apply(self):
        """
        Widget values are copied to object
        """

        # grid always updated
        # but there may be callback functions
        # which expects a return with current selection
        pass
        # self.ForceRefresh()

        col = self.GetGridCursorCol()
        row = self.GetGridCursorRow()

        table = self.GetTable()
        id, attrconf = table.get_id_attrconf(row, col)

        if self.func_apply is not None:
            wx.FutureCall(1, self.func_apply, table, id, None)

        #self.logw('grid.apply row%d col %d'%(row,col))
        if self.func_choose_id is not None:
            # call self.func_choose_id
            wx.FutureCall(1, self.func_choose_id, id, attrconf)
            return

        elif self.func_choose_attr is not None:
            # call self.func_choose_id
            wx.FutureCall(1, self.func_choose_attr, attrconf)
            return

    def get_current_attrconf_id(self):
        """
        Return current attribute and id
        """
        col = self.GetGridCursorCol()
        row = self.GetGridCursorRow()

        table = self.GetTable()
        id, attrconf = table.get_id_attrconf(row, col)

        return attrconf, id

    def restore(self):
        """
        Object values are copied into widgets.
        """
        # grid always updated
        pass
        # self.ForceRefresh()


class ObjPanelMixin:
    """
    Common methods for panels
    """

    def init_panel(self, parent, mainframe=None):
        """
        Initialization of general panel, independent by which type of
        widow the panel is actually represented

        parent = parent widget

        """

        self.parent = parent
        self.mainframe = mainframe
        self.widgets = None

        # list with widgets containing data
        # these widgets must have an apply and restore method
        # which is calle when the respective buttons are pressed.
        self.datawidgets = []

        # self.pentable=pentable

    # def get_pentable(self):
    #    """
    #    Returns pentable used for this panel
    #    """
    #    if self.pentable is None:
    #        return self.obj.get_pentable()
    #    else:
    #        return self.pentable

    def get_obj(self):
        return self.obj

    def get_id(self):
        return self.id

    def clear(self):
        for widget in self.datawidgets:
            widget.Destroy()

    def recreate_panel(self, obj, id=None, ids=None, groupnames=None, **kwargs):
        # print '\nrecreate_panel:',obj.format_ident(), id , ids, groupnames

        # print '  args =',kwargs
        self.obj = obj
        self.id = id
        self.ids = ids

        # print '  n_colconfigs =',len(obj.get_colconfigs(filtergroupnames = groupnames))
        # print '  n_configs =',len(obj.get_configs(filtergroupnames = groupnames))
        if (id is None) & (ids is None):

            if obj.managertype == 'table':
                # print '  managertype',obj.managertype,obj.get_attrsman()
                if len(obj.get_colconfigs(filtergroupnames=groupnames)) > 0:
                    self.recreate_panel_table(groupnames=groupnames, **kwargs)
                else:
                    self.recreate_panel_scalar(groupnames=groupnames, **kwargs)
            else:
                self.recreate_panel_scalar(groupnames=groupnames, **kwargs)
        else:
            if ids is not None:
                # table with specific ids
                self.recreate_panel_table(groupnames=groupnames, **kwargs)
            else:
                # scalar panel for single id
                self.recreate_panel_scalar(groupnames=groupnames, **kwargs)

    def recreate_panel_table(self, attrconfigs=None,
                             groupnames=None, show_groupnames=False,
                             show_title=True,
                             is_modal=False,
                             immediate_apply=False,
                             panelstyle='default',
                             func_change_obj=None,
                             func_choose_id=None,
                             func_choose_attr=None,
                             func_apply=None,
                             is_scrolled=True,
                             **buttonargs):
        """
        Recreates panel and destroys previous contents:
            attr = a list ith attribute names to be shown
            groups = list with group names to be shown
            show_groupnames = the group names are shown 
                              together with the respective attributes
            is_modal = if true the panel will be optimized for dialog window
            immediate_apply = changes to widgets will be immediately applied to 
                                obj attribute without pressing apply
            buttonargs = a dictionary with arguments for botton creation
                    obj = the object to be displayed
            func_change_obj = function to be called if user clicks on on object
                                with obj as argument
            func_choose_id = function to be called when user clicks on id(table row)
                                with id as argument
            func_choose_attr = function to be called when user clicks on attribute 
                                with attr as argument. In this case the attribute
                                names of scalars turn into buttons. Array attributes
                                are selected by clicking on the column name. 
            func_apply = is executed when apply button is pressed, and after widget values
                                have been applied. Arguments are: obj, id, ids
        """
        #
        # print 'recreate_panel_table:',self.obj.ident,immediate_apply, func_apply,groupnames

        self.func_change_obj = func_change_obj
        self.func_choose_id = func_choose_id
        self.func_choose_attr = func_choose_attr
        self.func_apply = func_apply
        self._show_title = show_title

        # if is_scrolled  is None:
        #    is_scrolled = not is_modal #for dialog windows use non crolled panels by default

        attrsman = self.obj.get_attrsman()
        attrconfigs_scalar = attrsman.get_configs(structs=cm.STRUCTS_SCALAR, filtergroupnames=groupnames)
        # print '  attrconfigs_scalar',attrconfigs_scalar
        # if obj.managertype=='table': # obj is a single table
        # print '  is_scalar_panel & is_singletab:'
        table = self.obj

        #attrconfigs_scalar = attrsman.get_configs( structs = cm.STRUCTS_SCALAR)

        if len(attrconfigs_scalar) > 0:
            # print '  create a panel with scalars and a table below'
            self.make_scalartablepanel(self, attrconfigs_scalar, table, objpanel=self,
                                       attrconfigs=None,
                                       immediate_apply=immediate_apply, panelstyle=panelstyle,
                                       is_modal=is_modal, show_title=show_title,
                                       is_scrolled=is_scrolled, **buttonargs)

        else:
            # print '  create only a table, without scalars'#,attrsman.get_colconfigs(filtergroupnames = groupnames)
            self.make_tablepanel(self, table, objpanel=self, attrconfigs=None, groupnames=groupnames,
                                 immediate_apply=immediate_apply, panelstyle=panelstyle,
                                 is_modal=is_modal, show_title=show_title,
                                 is_scrolled=is_scrolled, **buttonargs)

        self.restore()
        return True

    def recreate_panel_scalar(self, attrconfigs=None,
                              groupnames=None, show_groupnames=False,
                              show_title=True,
                              is_modal=False,
                              immediate_apply=False,
                              panelstyle='default',
                              func_change_obj=None,
                              func_choose_id=None,
                              func_choose_attr=None,
                              func_apply=None,
                              is_scrolled=True,
                              **buttonargs):
        """
        Recreates panel and destroys previous contents:
            attr = a list ith attribute names to be shown
            groups = list with group names to be shown
            show_groupnames = the group names are shown 
                              together with the respective attributes
            is_modal = if true the panel will be optimized for dialog window
            immediate_apply = changes to widgets will be immediately applied to 
                                obj attribute without pressing apply
            buttonargs = a dictionary with arguments for botton creation
                    obj = the object to be displayed
            func_change_obj = function to be called if user clicks on on object
                                with obj as argument
            func_choose_id = function to be called when user clicks on id(table row)
                                with id as argument
            func_choose_attr = function to be called when user clicks on attribute 
                                with attr as argument. In this case the attribute
                                names of scalars turn into buttons. Array attributes
                                are selected by clicking on the column name. 
            func_apply = is executed when apply button is pressed, and after widget values
                                have been applied. Arguments are: obj, id, ids
        """
        #
        # print 'recreate_panel_scalar:',self.obj.ident, func_apply,groupnames

        self.func_change_obj = func_change_obj
        self.func_choose_id = func_choose_id
        self.func_choose_attr = func_choose_attr
        self.func_apply = func_apply
        self._show_title = show_title

        # print '  ',self.obj.managertype,self.obj.get_attrsman()

        # if is_scrolled  is None:
        #    is_scrolled = not is_modal #for dialog windows use non crolled panels by default

        attrsman = self.obj.get_attrsman()
        attrconfigs_scalar = attrsman.get_configs(structs=cm.STRUCTS_SCALAR, filtergroupnames=groupnames)
        if self.id is not None:
            attrconfigs_scalar += attrsman.get_colconfigs()
        # print '  just one scalar obj panel',buttonargs.keys()
        # print '  groupnames', groupnames
        # create vertical main sizer
        # print '   attrconfigs_scalar',attrconfigs_scalar
        # print '   cm.STRUCTS_SCALAR',cm.STRUCTS_SCALAR
        self.make_scalarpanel(self, attrconfigs_scalar, id=self.id,  # groupnames=groupnames,
                              immediate_apply=immediate_apply, panelstyle=panelstyle,
                              is_modal=is_modal, show_title=show_title,
                              is_scrolled=is_scrolled, **buttonargs)

        # self.restore()
        return True

    def add_scalarpage(self, attrconfigs=None, groupnames=None,
                       id=None, immediate_apply=False, panelstyle='default',
                       is_modal=False, show_title=False,
                       show_buttons=True, is_scrolled=True, **buttonargs):
        """
        Add a new page to the notebook.
        """
        # print 'add_scalarpage',groupnames,attrconfigs
        # print '  args',args
        pan = wx.Panel(self.nb, -1)
        #pan = wx.Window(self, -1)
        widget = self.make_scalarpanel(pan, attrconfigs,  # groupnames=groupnames,
                                       id=id, immediate_apply=immediate_apply, panelstyle=panelstyle,
                                       is_modal=is_modal, show_title=show_title,
                                       show_buttons=show_buttons, is_scrolled=is_scrolled, **buttonargs)

        # Add network tab with editor
        p = self.nb.AddPage(pan, 'General')
        self.pages[self.obj.get_ident()] = pan
        # self.nb.SetSelection(p)

        return pan

    def add_tablepage(self, table, groupnames=None):
        """
        Add a new page to the notebook.
        """
        # print 'context.add_view',ViewClass
        # print '  args',args
        widget = self.add_table(self.nb, table, objpanel=self, groupnames=groupnames, show_title=False)
        self.tablewidgets.append(widget)  # need that later for refresh
        # Add network tab with editor
        # print '\nadd_tablepage  name',type(table.get_name()),table.get_name(),table
        # try:
        #    print   '  try:',table.name.attrname
        # except:
        #    print '  ok'

        p = self.nb.AddPage(widget, table.get_name())
        self.pages[table.format_ident()] = widget
        # self.nb.SetSelection(p)

        return widget

    def make_scalartablepanel(self, parent, attrconfigs_scalar, table, objpanel=None, attrconfigs=None, groupnames=None,
                              id=None, immediate_apply=False, panelstyle='default',
                              is_modal=False, show_title=False,
                              show_buttons=True, is_scrolled=True, **buttonargs):
        # print 'make_scalarpanellll',attrconfigs_scalar
        sizer = wx.BoxSizer(wx.VERTICAL)
        if show_title:
            self.add_title(sizer, id=id)

        splitter = wx.SplitterWindow(parent, -1, style=wx.SP_LIVE_UPDATE |
                                     wx.SP_BORDER | wx.SP_3DBORDER, size=wx.DefaultSize)
        splitter.SetMinimumPaneSize(10)

        scalarpanel = self.add_scalars(splitter, attrconfigs_scalar,  # groupnames = groupnames,
                                       id=id, immediate_apply=immediate_apply,
                                       panelstyle=panelstyle, is_modal=False, is_scrolled=is_scrolled)

        # if  datawidget is not None:
        #sizer.Add(datawidget, 1, wx.EXPAND|wx.TOP|wx.LEFT,5)
        #sizer.Add(datawidget, 1,0)
        # datawidget.Layout()

        tablepanel = self.add_table(splitter, table, objpanel=objpanel, ids=self.ids,
                                    attrconfigs=attrconfigs, groupnames=groupnames)
        # if  datawidget is not None:
        #sizer.Add(datawidget, 1, wx.EXPAND|wx.ALL, 5)
        #sizer.Add(datawidget, 0, wx.EXPAND)
        #sizer.Add(datawidget, 2,0,0)
        # datawidget.Layout()

        splitter.SplitHorizontally(scalarpanel, tablepanel, -100)
        splitter.SetSashPosition(100, True)
        sizer.Add(splitter, 1, wx.GROW, 5)

        if show_buttons:
            self.add_buttons(parent, sizer, is_modal=is_modal, **buttonargs)
        # sizer.SetSizeHints(self)
        parent.SetSizer(sizer)
        sizer.Fit(parent)

    def make_tablepanel(self, parent, tab, objpanel=None, attrconfigs=None, groupnames=None,
                        id=None, immediate_apply=False, panelstyle='default',
                        is_modal=False, show_title=False,
                        show_buttons=True, is_scrolled=True, **buttonargs):
        # print 'make_tablepanel',groupnames
        sizer = wx.BoxSizer(wx.VERTICAL)
        if show_title:
            self.add_title(sizer, id=id)

        datawidget = self.add_table(parent, tab, ids=self.ids, objpanel=objpanel,
                                    attrconfigs=attrconfigs, groupnames=groupnames)

        if datawidget is not None:
            sizer.Add(datawidget, 1, wx.EXPAND | wx.ALL, 5)

        if show_buttons:
            self.add_buttons(parent, sizer, is_modal=is_modal, **buttonargs)
        parent.SetSizer(sizer)
        sizer.Fit(parent)

    def make_scalarpanel(self, parent, attrconfigs=None,
                         id=None, immediate_apply=False, panelstyle='default',
                         is_modal=False, show_title=False,
                         show_buttons=True, is_scrolled=True, **buttonargs):

        # TODO: Attention: parent is always self
        # print 'make_scalarpanellll', attrconfigs
        sizer = wx.BoxSizer(wx.VERTICAL)
        if show_title:
            self.add_title(sizer, id=id)

        datawidget = self.add_scalars(parent, attrconfigs,
                                      id=id, immediate_apply=immediate_apply,
                                      panelstyle=panelstyle, is_modal=is_modal, is_scrolled=is_scrolled)

        if datawidget is not None:
            sizer.Add(datawidget, 1, wx.EXPAND | wx.ALL, 5)

        if show_buttons:
            self.add_buttons(parent, sizer, is_modal=is_modal, **buttonargs)
        parent.SetSizer(sizer)
        sizer.Fit(parent)

        # self.SetAutoLayout(True)
        # self.AutoLayout()
        # self.Refresh()
        self.SetAutoLayout(1)

        self.Refresh()
        self.Layout()

    def add_scalars(self, parent, attrconfigs,
                    id=None, immediate_apply=False, panelstyle='default',
                    is_modal=False, is_scrolled=True):
        """
        Add all scalar attributes to panel.
        """

        # print 'add_scalars for attrconfigs',attrconfigs,is_scrolled

        if attrconfigs is not None:
            if (not is_scrolled):
                datawidget = ScalarPanel(parent, attrconfigs, id=id,
                                         func_change_obj=self.func_change_obj,
                                         immediate_apply=immediate_apply,
                                         panelstyle=panelstyle,
                                         mainframe=self.mainframe)
            else:
                datawidget = ScalarPanelScrolled(parent, attrconfigs, id=id,
                                                 func_change_obj=self.func_change_obj,
                                                 immediate_apply=immediate_apply,
                                                 panelstyle=panelstyle,
                                                 is_modal=is_modal,
                                                 mainframe=self.mainframe)
            self.datawidgets.append(datawidget)  # used for apply and resore

            return datawidget

    def add_tables(self, sizer, tables, groupnames, show_title=False):

        for table in tables:
            self.add_hline(sizer)
            self.add_table(sizer, table, groupnames=groupnames, show_title=show_title)

    def add_table(self, parent, tab, ids=None, objpanel=None, attrconfigs=None, groupnames=None):
        """
        Add all array-type attributes to panel.
        """
        # TODO: group selection
        # TODO: Attention: parent is always self

        if attrconfigs is None:
            # This could be done way earlier!!!
            attrconfigs = tab.get_colconfigs(filtergroupnames=groupnames)

        # print 'add_table=',tab.get_name(),len(tab.get_colconfigs())

        # check if there are attributes
        if (attrconfigs is not None) & (attrconfigs != []):
            #panel=wx.Panel(self.parent, -1,wx.DefaultPosition,wx.DefaultSize,wx.SUNKEN_BORDER|wx.WANTS_CHARS)
            # panel.SetAutoLayout(True)
            datawidget = TabPanel(parent, tab, attrconfigs=attrconfigs, ids=ids, objpanel=objpanel,
                                  func_change_obj=self.func_change_obj,
                                  func_choose_id=self.func_choose_id,
                                  func_choose_attr=self.func_choose_attr,
                                  func_apply=self.func_apply,
                                  mainframe=self.mainframe)
            # panel.Refresh()
            # datawidget=ScalarPanel(self,attrs)
            # self.datawidgets.append(datawidget)

            self.datawidgets.append(datawidget)  # used for apply and resore
            return datawidget

    def add_buttons(self, parent, sizer,
                    is_modal=False,
                    standartbuttons=['apply', 'restore', 'cancel'],
                    buttons=[],
                    defaultbutton='apply',
                    ):
        """
        Add a button row to sizer
        """

        # print '\nadd_buttons is_modal',is_modal
        # print '  standartbuttons',standartbuttons
        # print '  buttons',buttons
        if parent is None:
            parent = self
        # print 'add_buttons'
        # print '  buttons=',buttons
        self.data_standartbuttons = {
            'apply':    ('Apply',   self.on_apply,      'Apply current values'),
            'restore':  ('Restore', self.on_restore,    'Restore previous values'),
            'ok':  ('OK', self.on_ok,    'Apply and close window'),
            'cancel': ('Cancel', self.on_cancel,    'Cancel and close window')
        }

        # compose button row on bottom of panel
        allbuttons = []

        # add custom buttons
        allbuttons += buttons
        # print '\n  allbuttons',allbuttons
        # print '  buttons',buttons,len(buttons),is_modal&(len(buttons)==0)
        if is_modal & (len(buttons) == 0):
            # if in dialog mode use only OK and cancel by default
            standartbuttons = []

        # add standart buttons
        # print '  standartbuttons=',standartbuttons
        for key in standartbuttons:
            # print '  append:',key
            allbuttons.append(self.data_standartbuttons[key])

        if (len(allbuttons) > 0) | is_modal:
            self.add_hline(sizer)

            # Init the context help button.
            # And even include help text about the help button :-)

            if is_modal & (len(buttons) == 0):
                # standartbuttons=[]
                # print 'add_buttons modal buttons',buttons
                btnsizer = wx.StdDialogButtonSizer()
                # standartbuttons=[]
                #helpbutton = wx.ContextHelpButton(self.parent)
                #helpbutton.SetHelpText(' Click to put application\n into context sensitive help mode.')
                #btnsizer.AddButton(helpbutton )

            else:
                if defaultbutton == '':
                    defaultbutton = 'Apply'
                # print 'add_buttons ',allbuttons
                btnsizer = wx.BoxSizer(wx.HORIZONTAL)

                # add help button

            # print '  allbuttons',allbuttons
            # create button widgets
            for (name, function, info) in allbuttons:

                b = wx.Button(parent, -1, name, (2, 2))
                self.Bind(wx.EVT_BUTTON, function, b)
                b.SetHelpText(info)
                if defaultbutton == name:
                    b.SetDefault()
                if is_modal & (len(buttons) == 0):
                    # print '  create button modal',name,is_modal
                    btnsizer.AddButton(b)
                else:
                    # print '  create button',name,is_modal
                    btnsizer.Add(b)

            if is_modal & (len(buttons) == 0):
                # print '  Add OK and CANCEL'
                # add OK and CANCEL if panel appears in separate window
                b = wx.Button(parent, wx.ID_OK)
                b.SetHelpText('Apply values and close window')
                #self.Bind(wx.EVT_BUTTON, self.on_ok_modal, b)
                if defaultbutton == '':
                    # set apply button default if there is no other default
                    b.SetDefault()
                btnsizer.AddButton(b)
                # print 'Add OK',b

                # add cancel
                b = wx.Button(parent, wx.ID_CANCEL)
                b.SetHelpText('Ignore modifications and close window')
                btnsizer.AddButton(b)
                # print 'Add cancel',b

                btnsizer.Realize()
            # else:
            #    btnsizer.Realize()
            # if wx.Platform != "__WXMSW__":
            #    btn = wx.ContextHelpButton(self)
            #    btnsizer.AddButton(btn)

            # btnsizer.Realize()

            #b = csel.ColourSelect(self.parent, -1, 'test', (255,0,0), size = wx.DefaultSize)
            # sizer.Add(b)
            # add buttomrow to main
            #sizer.Add(btnsizer, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5)

            #widget=wx.StaticText(self.parent, -1,'That is it!!')
            # sizer.Add(widget)

            # TEST
            #btnsizer = wx.StdDialogButtonSizer()

            # if wx.Platform != "__WXMSW__":
            #    btn = wx.ContextHelpButton(self)
            #    btnsizer.AddButton(btn)

##            btn = wx.Button(self, wx.ID_OK)
##            btn.SetHelpText("The OK button completes the dialog")
# btn.SetDefault()
# btnsizer.AddButton(btn)
##
##            btn = wx.Button(self, wx.ID_CANCEL)
##            btn.SetHelpText("The Cancel button cnacels the dialog. (Cool, huh?)")
# btnsizer.AddButton(btn)
# btnsizer.Realize()
            ###

            sizer.Add(btnsizer, 0, wx.ALIGN_CENTER_VERTICAL | wx.ALL, 5)

    def add_hline(self, sizer, hspace=5):
        """
        Add a horizontal line to sizer
        """
        line = wx.StaticLine(self, -1, size=(20, -1), style=wx.LI_HORIZONTAL)
        sizer.Add(line, 0, wx.GROW | wx.ALIGN_CENTER_VERTICAL | wx.RIGHT | wx.CENTER, hspace)

    def add_title(self, sizer, title=None, id=None, fontsize=12):
        """
        Add a title to the option panel.
        If no title is given, the titel will be derived from the object name
        and id if given.
        """
        # print 'add_title',self.obj.get_name()
        if title is None:
            fontsize = 14
            if id is None:
                title = self.obj.get_name().title()
            else:
                title = self.obj.get_name().title()+' ['+str(id)+']'

        #p=wx.Panel( self, wx.NewId(), wx.DefaultPosition,style=wx.SUNKEN_BORDER)
        titlewidget = wx.StaticText(self, wx.NewId(), title)
        font = wx.Font(fontsize, wx.SWISS, wx.NORMAL, wx.NORMAL)
        titlewidget.SetFont(font)
        # sizer.Add((5,5))
        # sizer.Add((5,5))
        # self.add_hline(sizer)
        # sizer.Add((20,20))
        sizer.Add(titlewidget, 0, wx.ALIGN_CENTER | wx.ALIGN_CENTER_VERTICAL, 10)
        # sizer.Add((10,10))
        self.add_hline(sizer, 15)

    def add_group(self, sizer, groupname):
        """
        Add a group to the option panel.

        This method does diaplay the entire group in a group
        widget.
        """
        pass

    def on_apply(self, event):
        """
        Apply widget contents to object
        """
        self.apply()

    def on_restore(self, event):
        """
        Copy object values to widget contents
        """
        self.restore()

    # def on_ok_modal(self,event):
    #    """ NO! valueas are rtead out from calling function
    #    Apply values, and continues withy modal windoe event handling
    #    """
    #    self.apply()
    #    event.Skip()

    def on_ok(self, event):
        """
        Apply values, destroy itself and parent
        """
        # print 'on_ok...'

        # check if we must execute some callbacks if ids or attributes
        # have been selected
        # for widget in self.datawidgets:
        #    if hasattr(widget,'on_ok')
        # else:
        self.apply()

        self.Close()
        # self.parent.Destroy()
        # self.Destroy()

    def on_cancel(self, event):
        """
        Apply values, destroy itself and parent
        """
        self.Close()

    def apply(self):
        """
        Widget values are copied to object
        """
        # print 'apply',self.obj.ident,self.datawidgets,self.func_apply
        for widget in self.datawidgets:
            # print '  widget',widget
            widget.apply()
            widget.restore()

        if self.func_apply is not None:
            # print '  call self.func_choose_id',self.obj, self.id, self.ids
            #
            #wx.FutureCall(1,self.func_apply, self.obj, self.id, self.ids)
            wx.CallAfter(self.func_apply, self.obj, self.id, self.ids)
            # event.Skip()
            return

    def restore(self):
        """
        Object values are copied into widgets.
        """
        for widget in self.datawidgets:
            widget.restore()

    # def OnSetFocus(self, evt):
    #    print "OnSetFocus"
    #    evt.Skip()
    # def OnKillFocus(self, evt):
    #    print "OnKillFocus"
    #    evt.Skip()
    # def OnWindowDestroy(self, evt):
    #    print "OnWindowDestroy"
    #    evt.Skip()


class ObjPanelDialog(ObjPanelMixin, wx.Dialog):
    def __init__(self, parent,
                 obj, id=None, ids=None,
                 attrconfigs=None,
                 #tables = None,
                 table=None,
                 groupnames=None, show_groupnames=False,
                 title='', size=wx.DefaultSize, pos=wx.DefaultPosition,\
                 style=wx.DEFAULT_DIALOG_STYLE | wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                 choose_id=False, choose_attr=False,
                 func_choose_id=None, func_change_obj=None, func_apply=None,
                 panelstyle='default',
                 immediate_apply=False,
                 **buttonargs):

        # print 'ObjPanelDialog.__init__',choose_id,choose_attr,table

        # print '  buttonargs,groupnames',buttonargs,groupnames
        # print '  size=',size
        # Instead of calling wx.Dialog.__init__ we precreate the dialog
        # so we can set an extra style that must be set before
        # creation, and then we create the GUI dialog using the Create
        # method.
        pre = wx.PreDialog()
        pre.SetExtraStyle(wx.DIALOG_EX_CONTEXTHELP)

        if title == '':
            title = 'Dialog for: '+obj.get_name()
        pre.Create(parent, -1, title, pos, size=size, style=style)

        # This next step is the most important, it turns this Python
        # object into the real wrapper of the dialog (instead of pre)
        # as far as the wxPython extension is concerned.
        self.PostCreate(pre)
        self.init_panel(parent)

        if choose_id:
            self.recreate_panel(obj,
                                id=id, ids=ids,
                                attrconfigs=attrconfigs,
                                #tables = tables,
                                #table =table,
                                groupnames=groupnames, show_groupnames=show_groupnames,
                                show_title=False, is_modal=True,
                                immediate_apply=immediate_apply,
                                panelstyle=panelstyle,
                                func_choose_id=self.on_choose_id, func_apply=func_apply,
                                func_change_obj=func_change_obj,
                                **buttonargs
                                )

        elif choose_attr:
            self.recreate_panel(obj, id=id, ids=ids,
                                attrconfigs=attrconfigs,
                                #tables = tables,
                                #table =table,
                                groupnames=groupnames, show_groupnames=show_groupnames,
                                show_title=False, is_modal=True,
                                immediate_apply=immediate_apply,
                                panelstyle=panelstyle,
                                func_choose_attr=self.on_choose_attr, func_apply=func_apply,
                                func_change_obj=func_change_obj,
                                **buttonargs
                                )

        else:
            # print ' normal mode without special callbacks',self.recreate_panel
            self.recreate_panel(obj, id=id, ids=ids,
                                attrconfigs=attrconfigs,
                                #tables = tables,
                                #table =table,
                                groupnames=groupnames, show_groupnames=show_groupnames,
                                show_title=False, is_modal=True,
                                immediate_apply=immediate_apply,
                                panelstyle=panelstyle,
                                func_choose_id=func_choose_id,
                                func_change_obj=func_change_obj, func_apply=func_apply,
                                **buttonargs
                                )

        self.id_chosen = -1
        self.attrconf_chosen = None

        # self.recreate_panel(attrconfigs=attrconfigs,groups=groups,is_modal=True,
        #                    immediate_apply=immediate_apply, panelstyle=panelstyle)

        # self.SetSize(self.GetSize())
        # self.SetSize(wx.DefaultSize)
        # self.SetSize(size)
        self.SetAutoLayout(1)
        self.Refresh()

    def on_choose_id(self, id, attrconf=None):
        self.id_chosen = id
        self.attrconf_chosen = attrconf
        self.EndModal(wx.ID_OK)

    def get_current_attrconf_id(self):
        """
        Returns selected or current attribute and id
        """
        # print 'get_current_attrconf_id',self.attrconf_chosen,self.id_chosen
        if (self.id_chosen == -1) & (self.attrconf_chosen == ''):
            # on_choose_id has not yet determined id
            # do though datawidgets and
            for widget in self.datawidgets:
                # print '  widget',widget
                if hasattr(widget, 'get_current_attrconf_id'):
                    return widget.get_current_attrconf_id()

            return self.attrconf_chosen, self.id_chosen

        else:
            return self.attrconf_chosen, self.id_chosen

    # def get_valueconf_chosen(self):
    #    return self.attrconf_chosen

# class ObjPanel(ObjPanelMixin,wxlib.scrolledpanel.ScrolledPanel):


class ObjPanel(ObjPanelMixin, wx.Panel):
    """
    Interactively displays attributes of object on a panel.
    """

    def __init__(self, parent, obj=None,
                 id=None, ids=None,
                 attrconfigs=None,
                 #tables = None,
                 #table = None,
                 groupnames=None,
                 func_change_obj=None, func_apply=None,
                 show_groupnames=False, show_title=True, is_modal=False,
                 mainframe=None,
                 pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.MAXIMIZE_BOX | wx.RESIZE_BORDER,
                 immediate_apply=False, panelstyle='default', **buttonargs):

        wx.Panel.__init__(self, parent, -1, pos, size, wx.SUNKEN_BORDER | wx.WANTS_CHARS)
        if obj is None:
            obj = cm.BaseObjman('empty')
        #wxlib.scrolledpanel.ScrolledPanel.__init__(self, parent,wx.ID_ANY,size=size)
        #self.maxWidth  = 100
        #self.maxHeight = 100
        # self.SetVirtualSize((300,-1))
        # self.SetScrollRate(20,20)

        self.init_panel(parent, mainframe=mainframe)
        # (re)create panel widgets
        # print 'ObjPanel.__init__',obj.ident,id,func_apply,groupnames
        self.recreate_panel(obj, id=id, ids=ids,
                            attrconfigs=attrconfigs,
                            groupnames=groupnames, show_groupnames=show_groupnames,
                            show_title=show_title, is_modal=is_modal,
                            immediate_apply=immediate_apply,
                            panelstyle=panelstyle,
                            func_change_obj=func_change_obj,
                            func_apply=func_apply,
                            **buttonargs)

        # optionpanel.GetSize()

        # self.SetAutoLayout(1)


class TablePanel(ObjPanelMixin, wx.Panel):
    """
    Common methods for panels
    """

    def __init__(self, parent, table,
                 attrconfigs=None,
                 id=None, ids=None,
                 groupnames=None,
                 func_change_obj=None, func_apply=None,
                 show_groupnames=False, show_title=True, is_modal=False,
                 mainframe=None, pos=wx.DefaultPosition, size=wx.DefaultSize,
                 immediate_apply=False, panelstyle='default', **buttonargs):

        wx.Panel.__init__(self, parent, -1, pos, size, wx.SUNKEN_BORDER)

        self.init_panel(parent, mainframe=mainframe)
        # (re)create panel widgets
        # print 'TablePanel. __init__',table.get_name()
        self.recreate_panel(table,
                            attrconfigs=attrconfigs,
                            # ids=ids,
                            groupnames=groupnames, show_groupnames=show_groupnames,
                            show_title=show_title, is_modal=is_modal,
                            immediate_apply=immediate_apply, func_apply=func_apply,
                            panelstyle=panelstyle,
                            func_change_obj=func_change_obj,
                            **buttonargs)

        # optionpanel.GetSize()

        self.SetAutoLayout(1)

    def recreate_panel(self, obj,  # obj is table
                       attrconfigs=None,
                       # ids=None,
                       groupnames=None, show_groupnames=False,
                       show_title=True,
                       is_modal=False,
                       immediate_apply=False,
                       panelstyle='default',
                       func_change_obj=None,
                       func_choose_id=None,
                       func_choose_attr=None,
                       func_apply=None,
                       **buttonargs):
        """
        Recreates panel and destroys previous contents:
            attr = a list ith attribute names to be shown
            groups = list with group names to be shown
            show_groupnames = the group names are shown 
                              together with the respective attributes
            is_modal = if true the panel will be optimized for dialog window
            immediate_apply = changes to widgets will be immediately applied to 
                                obj attribute without pressing apply
            buttonargs = a dictionary with arguments for botton creation
                    obj = the object to be displayed
            id = used if there is only one single id to be displayed
            ids = used if a selection of ids need to be displayed
            func_change_obj = function to be called if user clicks on on object
                                with obj as argument
            func_choose_id = function to be called when user clicks on id(table row)
                                with id as argument
            func_choose_attr = function to be called when user clicks on attribute 
                                with attr as argument. In this case the attribute
                                names of scalars turn into buttons. Array attributes
                                are selected by clicking on the column name. 
        """
        #self.id = id
        #self.ids = ids
        self.obj = obj
        self.func_change_obj = func_change_obj
        self.func_choose_id = func_choose_id
        self.func_choose_attr = func_choose_attr
        self.func_apply = func_apply

        # print '\n\nrecreate_panel name for id,ids=',obj.get_name(),ids
        # print '  attrs,groups=',attrs,groups
        # print '  immediate_apply',immediate_apply
        # this will be looked up by widgets to decide how to react to an input
        #self.immediate_apply = immediate_apply

        # remove previous contents
        # ids_widgetcontainers=self.get_ids()
        # self.parent.DestroyChildren()
        #del self[ids_widgetcontainers]

        # create vertical main sizer
        sizer = wx.BoxSizer(wx.VERTICAL)
        if show_title:
            self.add_title(sizer)

        # show table with one or several ids
        self.add_table(sizer, obj, attrconfigs=attrconfigs,
                       ids=ids, groupnames=groupnames, show_title=False)

        self.SetSizer(sizer)
        sizer.Fit(self)

        # some widgets like color need this to expand into their maximum space
        self.restore()


class NaviPanelMixin:
    def add_hist(self, obj, id=None, ids=None, kwargs={}):
        """
        Add a new obj to history list.
        The forward history will be eliminated.
        """
        # print 'add_hist',self.hist,self.ind_hist

        n_hist = len(self.hist)
        if n_hist == 0:
            # first entry
            self.ind_hist = 0

        else:
            self.hist = self.hist[:self.ind_hist+1]
            # print '  hist[:self.ind_hist+1]',self.hist[:self.ind_hist+1]

        self.hist.append((obj, id, ids, kwargs))
        self.ind_hist = len(self.hist)-1
        # print '  hist,self.ind_hist',self.hist,self.ind_hist

    def get_hist(self, ind_delta=-1):
        """
        Getting hist with changing pointer
        """
        ind = self.ind_hist + ind_delta
        # print 'get_hist ind_hist,ind_delta,ind,len(self.hist),ok',self.ind_hist,ind_delta,ind,len(self.hist),(ind < len(self.hist)) & (ind >= 0)

        if (ind < len(self.hist)) & (ind >= 0):
            self.ind_hist = ind
            # print '  OK self.ind_hist',self.ind_hist,self.hist[ind]
            return self.hist[ind]
        else:
            # print '  Fail self.ind_hist',self.ind_hist
            return None, None, None, None

    def get_history(self, ind_delta=-1):
        """
        Getting hist without changing pointer
        """
        ind = self.ind_hist + ind_delta
        # print 'get_history',self.ind_hist,ind_delta,(ind < len(self.hist)) & (ind >= 0)

        if (ind < len(self.hist)) & (ind >= 0):
            # self.ind_hist=ind
            return self.hist[ind]
        else:
            return None, None, None, None

    def on_change_obj(self, event):
        # print 'Navicanvas.on_change_obj'
        obj = event.GetObj()
        self.refresh_panel(obj)
        event.Skip()

    def change_obj(self, obj, id=None, ids=None, **kwargs):

        if len(kwargs) == 0:
            # this is to recover kw arguments i.e. callbacks
            obj_last, id_last, ids_last, kwargs_last = self.get_history(0)
            if kwargs_last is not None:
                kwargs = kwargs_last

        if self.func_apply is not None:
            kwargs['func_apply'] = self.func_apply

        # print 'Navipanel.change_obj',obj.format_ident(),id,ids,kwargs.keys()

        if hasattr(obj, 'get_ident'):
            self.refresh_panel(obj, id=id, ids=ids, **kwargs)
            self.add_hist(obj, id, ids, kwargs)

    def get_obj(self):
        """
        Returns currently displayed object
        """
        return self.objpanel.obj

    def refresh_panel(self, obj=None, id=None, ids=None, **kwargs):
        """
        Deletes previous conents 
        Builds panel for obj
        Updates path window and history
        """

        if obj is None:
            if self.get_obj() == self._defaultobj:
                return None  # no changes
            else:
                obj = self._defaultobj

        # print 'Navicanvas.refresh_panel with',obj.ident,kwargs

        if id is not None:
            self.path.SetValue(obj.format_ident_row_abs(id))

        elif ids is not None:
            # no id provided, just show identification string
            self.path.SetValue(obj.format_ident_abs())

        else:  # object and id provided: compose string with id
            self.path.SetValue(obj.format_ident_abs())

        # self.path.SetSize((len(self.path.GetValue())*10,-1))
        # self.path.SetSize((-1,-1))
        # remove previous obj panel
        sizer = self.GetSizer()
        sizer.Remove(1)
        self.objpanel.Destroy()
        #del self.objpanel

        # build new obj panel
        # self.objpanel=ObjPanel(self,obj,id=id,attrs=self.attrs,groups=self.groups,
        #                        func_change_obj=self.change_obj,
        #                        is_modal=self.is_modal,
        #                        mainframe=self.mainframe,
        #                        immediate_apply=self.immediate_apply,
        #                        **self.buttonargs)
        attrconfigs = obj.get_attrsman().get_configs()
        # for attrconfig in obj.get_configs()#attrconfigs:
        #    if '_private' not in attrconfig.groups:
        #        attrconfigs.append(attrconfig)

        args = {'attrconfigs': attrconfigs,
                # 'tables' : None,
                # 'table' : None,
                'id': id, 'ids': ids,
                'groupnames': self.groupnames,
                'func_change_obj': self.change_obj,
                'show_groupnames': False, 'show_title': True, 'is_modal': self.is_modal,
                'mainframe': self.mainframe, 'pos': wx.DefaultPosition, 'size': wx.DefaultSize,
                'immediate_apply': self.immediate_apply, 'panelstyle': 'default'
                }

        args.update(self.buttonargs)
        args.update(kwargs)

        # print '  make ObjPanel\n args =',args

        self.objpanel = ObjPanel(self, obj=obj, **args)
        # if id is not None:
        #    self.objpanel=ObjPanel(self,obj,id=id,func_change_obj=self.change_obj)
        # else:
        #    self.objpanel=ObjPanel(self,obj,func_change_obj=self.change_obj)
        sizer.Add(self.objpanel, 1, wx.GROW)

        self.Refresh()
        # sizer.Fit(self)
        sizer.Layout()
        # add to history
        return None

    def on_change_path(self, event):
        """
        Key stroke on path field. Change object when return is pressed.
        """
        keycode = event.GetKeyCode()
        if keycode == wx.WXK_RETURN:
            # print 'change path to',self.path.GetValue()
            # self.refresh_panel(obj)
            pass
        else:
            event.Skip()

    # def recreate_panel(self,obj):
    #    """
    #    Removes previous contents and displays attributes on panel.
    #    """
    #    pass

    def on_go_back(self, event):
        # print 'on_go_back'
        # print '  hist=',self.hist
        obj, id, ids, kwargs = self.get_hist(-1)
        # if obj is not None:
        #    print '  done: obj, hist, ind_hist',obj.ident,self.hist,self.ind_hist
        # else:
        #    print ' failed: hist, ind_hist',self.hist,self.ind_hist

        if obj is not None:
            # print '  refresh_panel with',obj.ident
            self.refresh_panel(obj, id, ids, **kwargs)
        event.Skip()

    def on_go_forward(self, event):
        obj, id, ids, kwargs = self.get_hist(+1)
        # print 'on_go_forward to obj',obj

        # if obj is not None:
        #    print '  Done:obj.ident,self.hist,self.ind_hist',obj.ident,self.hist,self.ind_hist
        # else:
        #    print '  Failed:self.hist,self.ind_hist',self.hist,self.ind_hist

        if obj is not None:
            # print '  refresh_panel with',obj.ident
            self.refresh_panel(obj, id, ids, **kwargs)
        event.Skip()

    def on_go_up(self, event):

        obj = self.objpanel.get_obj()
        _id = self.objpanel.get_id()

        # this is to recover kw arguments i.e. callbacks
        obj_last, id_last, ids_last, kwargs_last = self.get_history(0)
        if kwargs_last is not None:
            kwargs = kwargs_last
        else:
            kwargs = {}

        parent = obj.get_parent()
        # print 'on_go_up',obj,id,parent
        if _id is None:
            # show parent object
            if type(parent) == types.InstanceType:
                if hasattr(parent, 'ident'):
                    self.refresh_panel(parent, None, None, **kwargs)
                    self.add_hist(parent, None, None, kwargs)
        else:
            # show table object if table row id has been previously
            self.refresh_panel(obj, None, None, **kwargs)
            self.add_hist(obj, None, None, kwargs)

        event.Skip()

    def on_go_home(self, event):
        obj = self.objpanel.get_obj()
        root = obj.get_root()
        if type(root) == types.InstanceType:
            if hasattr(root, 'ident'):
                self.refresh_panel(root)
        event.Skip()
        # print 'on_go_home'

    def on_enter_window(self, evt):
        # print 'on_enter_window'
        self.SetFocus()

    def init_navbar(self, obj, _id=None):
        """
        Initialize toolbar which consist of navigation buttons
        """
        bottonsize = (32, 32)
        bottonborder = 10
        toolbarborder = 1

        # toolbar
        self.toolbar = wx.BoxSizer(wx.HORIZONTAL)

        # self.init_toolbar()
        #bottons = []
        bitmap = wx.ArtProvider_GetBitmap(wx.ART_GO_BACK, wx.ART_TOOLBAR)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize,
                            (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        b.SetToolTipString("Go back in browser history.")
        self.Bind(wx.EVT_BUTTON, self.on_go_back, b)
        self.toolbar.Add(b, flag=wx.ALL, border=toolbarborder)
        # bottons.append(b)
        #self.toolbar.Add(b, flag=wx.ALL, border=toolbarborder)

        bitmap = wx.ArtProvider_GetBitmap(wx.ART_GO_FORWARD, wx.ART_TOOLBAR)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize,
                            (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        b.SetToolTipString("Go forward in browser history.")
        self.Bind(wx.EVT_BUTTON, self.on_go_forward, b)
        self.toolbar.Add(b, 0, wx.EXPAND, border=toolbarborder)
        # bottons.append(b)

        bitmap = wx.ArtProvider_GetBitmap(wx.ART_GO_UP, wx.ART_TOOLBAR)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize,
                            (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        b.SetToolTipString("Go up to parent object.")
        self.Bind(wx.EVT_BUTTON, self.on_go_up, b)
        self.toolbar.Add(b, 0, wx.EXPAND, border=toolbarborder)

        # self.add_tool(
        #    'up',self.on_go_up,
        #    wx.ArtProvider.GetBitmap(wx.ART_GO_UP, wx.ART_TOOLBAR, tsize),
        #    'show panel of parent instance')

        bitmap = wx.ArtProvider_GetBitmap(wx.ART_GO_HOME, wx.ART_TOOLBAR)
        b = wx.BitmapButton(self, -1, bitmap, bottonsize,
                            (bitmap.GetWidth()+bottonborder, bitmap.GetHeight()+bottonborder))
        b.SetToolTipString("Go to main object, the mother of all objects.")
        self.Bind(wx.EVT_BUTTON, self.on_go_home, b)
        self.toolbar.Add(b, 0, wx.EXPAND, border=toolbarborder)

        # self.add_tool(
        #    'home',self.on_go_home,
        #    wx.ArtProvider.GetBitmap(wx.ART_GO_HOME, wx.ART_TOOLBAR, tsize),
        #    'show panel of root instance')

        # size=(-1, -1))#,size=(300, -1))
        self.path = wx.TextCtrl(self, -1, self.get_ident_abs(obj, _id), style=wx.TE_RIGHT)
        #self.path=wx.TextCtrl(self.toolbar, -1, obj.format_ident_abs(), style=wx.TE_RIGHT)
        # wx.EVT_ENTER_WINDOW(self.path,self.on_enter_window)
        # self.add_tool('path',widget=self.path)
        self.path.Bind(wx.EVT_CHAR, self.on_change_path)
        #self.toolbar.Add(self.path, flag=wx.ALL, border=toolbarborder)
        self.toolbar.Add(self.path, 1, wx.EXPAND, border=toolbarborder)
        #self.toolbar.AddControl(wx.TextCtrl(self.toolbar, -1, "Toolbar controls!!", size=(150, -1)))

        # self.toolbar.AddSeparator()

        # panel.SetAutoLayout(True)
        # panel.SetSizer(buttons)

        self.toolbar.Fit(self)

        # only for frames
        # self.SetToolBar(self.toolbar)

    def get_ident_abs(self, obj, _id):
        if _id is not None:
            return obj.format_ident_row_abs(_id)

        else:
            return obj.format_ident_abs()

    def init_objpanel(self, obj, id=None, ids=None, attrconfigs=None, groupnames=None,
                      is_modal=True, mainframe=None, func_apply=None,
                      immediate_apply=False, panelstyle='default', **buttonargs):
        """
        Initializes object panel, bu also stores parameters such as 
        mode, groups etc 
        """
        self.attrconfigs = attrconfigs
        self.groupnames = groupnames
        self.is_modal = is_modal
        self.mainframe = mainframe
        self.immediate_apply = immediate_apply
        self.panelstyle = panelstyle
        self.buttonargs = buttonargs
        # self.objpanel=ObjPanel(self,obj,id=id,ids=ids,attrs=attrs,groups=groups,
        #                        func_change_obj=self.change_obj,
        #                        is_modal=is_modal,
        #                        mainframe=mainframe,
        #                        immediate_apply=immediate_apply, panelstyle=panelstyle,
        #                        **buttonargs)
        self.objpanel = ObjPanel(self, obj=obj,
                                 attrconfigs=self.attrconfigs,
                                 #tables = None,
                                 #table = None,
                                 id=id, ids=ids,
                                 groupnames=groupnames,
                                 func_change_obj=self.change_obj,
                                 show_groupnames=False, show_title=True, is_modal=self.is_modal,
                                 mainframe=self.mainframe, pos=wx.DefaultPosition, size=wx.DefaultSize,
                                 func_apply=func_apply,
                                 immediate_apply=self.immediate_apply, panelstyle='default',
                                 **self.buttonargs)

    def init_hist(self):
        """
        Initialize history variables
        """
        self.hist = []
        self.ind_hist = -1


class NaviPanel(NaviPanelMixin, wx.Panel):
    """

    Interactively navigates through objects and displays attributes 
    on a panel.
    """

    def __init__(self, parent, obj, id=None, mainframe=None, func_apply=None, **args):
        if obj is None:
            obj = cm.BaseObjman('empty')
        wx.Panel.__init__(self, parent, -1, wx.DefaultPosition, wx.DefaultSize)
        # print 'NaviPanel',obj.ident,id,func_apply,args
        sizer = wx.BoxSizer(wx.VERTICAL)
        # define a default object which is shown when
        # refresh is called without arguments
        self._defaultobj = obj
        self.func_apply = func_apply
        # initialize history
        self.init_hist()

        # make navigation toolbar
        self.init_navbar(obj, id)

        # create initial panel
        self.init_objpanel(obj, id=id, mainframe=mainframe, is_modal=False, func_apply=func_apply, **args)

        sizer.Add(self.toolbar, 0, wx.ALL | wx.ALIGN_LEFT | wx.GROW, 4)  # from NaviPanelTest
        sizer.Add(self.objpanel, 1, wx.GROW)  # from NaviPanelTest

        # finish panel setup
        self.SetSizer(sizer)
        sizer.Fit(self)
        # add to history
        # self.add_hist(obj,id,None,args)
        # self.SetSize(parent.GetSize())


class ObjBrowserMainframe(wx.Frame):

    """
    A frame used for the ObjBrowser Demo

    """

    def __init__(self, parent, wxid, title, position, size, obj=None, _id=None, table=None):
        wx.Frame.__init__(self, parent, wxid, title, position, size)
        # print 'ObjBrowserMainframe',obj.ident,_id
        # Set up the MenuBar
        MenuBar = wx.MenuBar()

        file_menu = wx.Menu()
        item = file_menu.Append(-1, "&Open...", "Open an object from file.")
        self.Bind(wx.EVT_MENU, self.OnLoad, item)
        item = file_menu.Append(-1, "&Save as", "Save an object under file.")
        self.Bind(wx.EVT_MENU, self.OnSave, item)
        item = file_menu.Append(-1, "&Close", "Close this frame")
        self.Bind(wx.EVT_MENU, self.OnQuit, item)

        MenuBar.Append(file_menu, "&File")

        help_menu = wx.Menu()
        item = help_menu.Append(-1, "&About",
                                "More information About this program")
        self.Bind(wx.EVT_MENU, self.OnAbout, item)
        MenuBar.Append(help_menu, "&Help")

        self.SetMenuBar(MenuBar)

        self.CreateStatusBar()

        # Create Browser widget here
        #browser = wx.Panel(self, -1, wx.DefaultPosition, size = wx.DefaultSize)
        #browser = ObjPanel(self,obj, id=10)
        if obj is not None:
            # print '  init ObjPanel'
            #self.browser = ObjPanel(self, obj)
            self.browser = NaviPanel(self, obj, _id)
        else:
            # print '  init TablePanel'
            self.browser = TablePanel(self, table)
        #browser = TablePanel(self,obj)
        # self.MsgWindow = wx.TextCtrl(self, wx.ID_ANY,
        #                             "Look Here for output from events\n",
        #                             style = (wx.TE_MULTILINE |
        #                                      wx.TE_READONLY |
        #                                      wx.SUNKEN_BORDER)
        #                             )

        # TODO:INTERESTING:
        # wxTextCtrl *control = new wxTextCtrl(...);
        # wxStreamToTextRedirector redirect(control);
        # // all output to cout goes into the text control until the exit from
        #   // current scope

        #self.MsgWindow  = py.crust.Crust(self, intro='Check it out!')
        self.MsgWindow = py.shell.Shell(self)
        # Create a sizer to manage the Canvas and message window
        MainSizer = wx.BoxSizer(wx.VERTICAL)
        MainSizer.Add(self.browser, 4, wx.EXPAND)
        MainSizer.Add(self.MsgWindow, 1, wx.EXPAND | wx.ALL, 5)

        self.SetSizer(MainSizer)
        self.Bind(wx.EVT_CLOSE, self.OnCloseWindow)

        self.EventsAreBound = False

        # getting all the colors for random objects
        wxlib.colourdb.updateColourDB()
        self.colors = wxlib.colourdb.getColourList()

        return None

    def OnAbout(self, event):
        dlg = wx.MessageDialog(self,
                               "This is a small program to demonstrate\n"
                               "the use of the Objbrowser\n",
                               "About Me",
                               wx.OK | wx.ICON_INFORMATION)
        dlg.ShowModal()
        dlg.Destroy()

    def OnLoad(self, event):

        dlg = wx.FileDialog(self, message="Open object",
                            defaultDir=os.getcwd(),
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
                obj = cm.load_obj(filepath)

                MainSizer = self.GetSizer()
                MainSizer.Remove(0)
                MainSizer.Remove(1)

                self.browser.Destroy()
                self.MsgWindow.Destroy()

                # build new obj panel
                self.browser = ObjPanel(self, obj)
                # sizer.Add(self.objpanel,0,wx.GROW)

                #self.browser = wx.Panel(self, -1)
                #button = wx.Button(self.browser, 1003, "Close Me")
                #button.SetPosition((15, 15))

                # sizer.Add(panel,1,wx.GROW)

                self.MsgWindow = py.shell.Shell(self)
                # Create a sizer to manage the Canvas and message window
                MainSizer.Add(self.browser, 4, wx.EXPAND)
                MainSizer.Add(self.MsgWindow, 1, wx.EXPAND | wx.ALL, 5)

                self.Refresh()
                MainSizer.Layout()

    def OnSave(self, event):
        dlg = wx.FileDialog(
            self, message="Save file as ...", defaultDir=os.getcwd(),
            defaultFile="", wildcard=wildcards_all, style=wx.SAVE
        )

        # This sets the default filter that the user will initially see. Otherwise,
        # the first filter in the list will be used by default.
        dlg.SetFilterIndex(0)

        # Show the dialog and retrieve the user response. If it is the OK response,
        # process the data.
        if dlg.ShowModal() == wx.ID_OK:
            filepath = dlg.GetPath()
            cm.save_obj(self.browser.get_obj(), filepath)

    def OnQuit(self, event):
        self.Close(True)

    def OnCloseWindow(self, event):
        self.Destroy()


class ObjBrowserApp(wx.App):
    """

    """

    def __init__(self, obj, _id=None, output=False, **kwargs):
        self._obj = obj
        self._id = _id
        # print 'ObjBrowserApp.__init__',obj.ident, _id

        wx.App.__init__(self, output, **kwargs)

    def OnInit(self):
        wx.InitAllImageHandlers()
        #DrawFrame = BuildDrawFrame()
        frame = ObjBrowserMainframe(None, -1, "Object browser", wx.DefaultPosition,
                                    (700, 700), obj=self._obj, _id=self._id)

        self.SetTopWindow(frame)
        frame.Show()

        return True


class TableBrowserApp(wx.App):
    """

    """

    def __init__(self, tab,  **kwargs):
        self._tab = tab
        # print 'ViewerApp.__init__',self._net

        wx.App.__init__(self,  **kwargs)

    def OnInit(self):
        wx.InitAllImageHandlers()
        #DrawFrame = BuildDrawFrame()
        frame = ObjBrowserMainframe(None, -1, "Table browser", wx.DefaultPosition, (700, 700), table=self._tab)

        self.SetTopWindow(frame)
        frame.Show()

        return True


def objbrowser(obj, _id=None):
    app = ObjBrowserApp(obj, _id=_id, output=False)  # put in True if you want output to go to it's own window.
    # print 'call MainLoop'
    app.MainLoop()


###############################################################################
if __name__ == '__main__':

    from agilepy.lib_base.test_classman_classes import *
    # no from agilepy.lib_base.classman import *
    from agilepy.lib_base.classman import load_obj, save_obj
    ############
    # make up a test class
    n_test = 0  # <<<<<<<<<<<<<<<<<<
    ############
    _id = None

    if n_test == 0:
        # simple scalar parameters, no table
        obj = TestClass()
        print 'obj.ident', obj.ident

    elif n_test == 1:
        obj = TestTableObjMan()

    elif n_test == -1:
        obj = TestTableObjManNocols()

    elif n_test == 2:
        obj = demand

    elif n_test == 3:
        obj = lines

    elif n_test == 4:
        _id = 1
        obj = TestTableObjMan()
    elif n_test == 5:
        obj = drawing
    elif n_test == -5:
        save_obj(drawing, 'test_drawing.obj')
        print '\nreload'+60*'.'
        obj = load_obj('test_drawing.obj')
        obj.get_attrsman().print_attrs()

    elif n_test == 6:
        obj = polylines
    objbrowser(obj, _id=_id)
