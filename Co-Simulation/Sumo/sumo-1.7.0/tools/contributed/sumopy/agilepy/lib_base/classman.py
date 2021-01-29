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

# @file    classman.py
# @author  Joerg Schweizer
# @date


# python classman.py

# TODO:
# - store old values in attrcons and recover with undo


# To be or not to be.  -- Shakespeare
# To do is to be.  -- Nietzsche
# To be is to do.  -- Sartre
# Do be do be do.  -- Sinatra

# save with is saved flag
# xml mixin
# different attrconfig classe (numbers, strings, lists, colors,...)

import types
import os
import pickle
import sys
import string
from collections import OrderedDict
from datetime import datetime
#import numpy as np
import xmlman as xm
from logger import Logger
from misc import get_inversemap

##########


# event triggers
#       plugtype    plugcode
EVTDEL = 0  # delete attribute
EVTSET = 1  # set attribute
EVTGET = 2  # get attribute
EVTADD = 3  # add/create attribute

EVTDELITEM = 20  # delete attribute
EVTSETITEM = 21  # set attribute
EVTGETITEM = 22  # get attribute
EVTADDITEM = 23  # add/create attribute


ATTRS_NOSAVE = ('value', 'plugin', '_obj', '_manager', 'get', 'set', 'add',
                'del', 'delete', 'childs', 'parent', '_attrconfig_id_tab')
ATTRS_SAVE = ('ident', '_name', 'managertype', '_info', 'xmltag', '_attrs_nosave')
ATTRS_SAVE_TABLE = ('_is_localvalue', 'attrname', '_colconfigs', '_ids', '_inds', '_attrconfigs',
                    '_groups', 'plugin', '_is_indexing', '_index_to_id', '_id_to_index')

STRUCTS_COL = ('odict', 'array')
STRUCTS_SCALAR = ('scalar', 'list', 'matrix', 'scalar.func')

NUMERICTYPES = (types.BooleanType, types.FloatType, types.IntType, types.LongType, types.ComplexType)
STRINGTYPES = (types.StringType, types.UnicodeType)


def save_obj(obj, filename, is_not_save_parent=False):
    """
    Saves python object to a file with filename.
    Filename may also include absolute or relative path.
    If operation fails a False is returned and True otherwise.
    """
    # print 'save_obj',is_not_save_parent,filename,obj.parent
    try:
        file = open(filename, 'wb')
    except:
        print 'WARNING in save: could not open', filename
        return False

    if is_not_save_parent:
        parent = obj.parent
        obj.parent = None
    # print '  before',is_not_save_parent,parent,obj.parent
    pickle.dump(obj, file, protocol=2)
    file.close()
    #  set all objects and attrubutes to unsaved again
    # obj.set_unsaved()
    # no, decided to eliminate _is_saved restriction
    # print '  after',is_not_save_parent,parent,obj.parent
    if is_not_save_parent:
        obj.parent = parent
    return True


def load_obj(filename, parent=None, is_throw_error=False):
    """
    Reads python object from a file with filename and returns object.
    Filename may also include absolute or relative path.
    If operation fails a None object is returned.
    """
    print 'load_obj', filename

    if is_throw_error:
        f = open(filename, 'rb')
    else:
        try:
            f = open(filename, 'rb')
        except:
            print 'WARNING in load_obj: could not open', filename
            return None

    # try:
    # print '  pickle.load...'
    obj = pickle.load(f)
    f.close()

    # init_postload_internal is to restore INTERNAL states from INTERNAL states
    # print 'load_obj->init_postload_internal',obj.ident
    obj.init_postload_internal(parent)
    # print '  after init_postload_internal  obj.parent',obj.parent

    # init_postload_external is to restore INTERNAL states from EXTERNAL states
    # such as linking
    #obj.init_postload_external(is_root = True)
    obj.init_postload_external()

    # _init4_ is to do misc stuff when everything is set
    # obj._init4_config()

    return obj

# class ObjXmlMixin:


# class AttrXmlMixin:


class Plugin:

    def __init__(self, obj, is_enabled=True):
        self._obj = obj  # this can be either attrconf or main object
        self._events = {}
        self._has_events = False
        self._is_enabled = is_enabled

    def get_obj(self):
        return self._obj

    def unplug(self):
        del self._events
        self._events = {}
        self._has_events = False

    def add_event(self, trigger, function):
        """
        Standard plug types are automatically set but the system:

        """
        if not self._events.has_key(trigger):
            self._events[trigger] = []
        self._events[trigger].append(function)
        self._has_events = True

    def del_event(self, trigger):
        del self._events[trigger]
        if len(self._events) == 0:
            self._has_events = False

    def enable(self, is_enabled=True):
        self._is_enabled = is_enabled

    def exec_events(self, trigger):
        if self._has_events & self._is_enabled:
            # print '**PuginMixin.exec_events',trigger,(EVTGETITEM,EVTGET)
            # if trigger!=EVTGET:
            #    print '  call set_modified',self._obj
            #    self._obj.set_modified(True)

            for function in self._events.get(trigger, []):
                function(self._obj)

    def exec_events_attr(self, trigger, attrconfig):
        if self._has_events & self._is_enabled:
            # print '**PuginMixin.exec_events',trigger,(EVTGETITEM,EVTGET)
            # if trigger!=EVTGET:
            #    print '  call set_modified',self._obj
            #    self._obj.set_modified(True)

            for function in self._events.get(trigger, []):
                function(self._obj, attrconfig)

    def exec_events_ids(self, trigger, ids):
        """
        Executes all functions assigned for this trigger for multiple ids.
        """
        if self._has_events & self._is_enabled:
            # print '**ArrayConf._execute_events_keys',trigger,ids
            # print '  _events',self._events

            for function in self._events.get(trigger, []):
                function(self._obj, ids)


class AttrConf:
    """
    Contains additional information on the object's attribute.
    """

    def __init__(self, attrname, default,
                 groupnames=[], perm='rw',
                 is_save=True,
                 # is_link = False, # define link class
                 is_copy=True,
                 name='', info='',
                 unit='',
                 xmltag=None,
                 xmlsep=' ',
                 is_plugin=False,
                 struct='scalar',
                 metatype='',
                 is_returnval=True,
                 **attrs):
        # if struct == 'None':
        #    if hasattr(default, '__iter__'):
        #        struct = 'scalar'
        #    else:
        #        struct = 'list'

        # these states will be saved and reloaded
        self.attrname = attrname
        self.groupnames = groupnames
        self.metatype = metatype
        self.struct = struct

        self._default = default

        self._is_save = is_save  # will be set properly in set_manager
        self._is_copy = is_copy
        self._is_localvalue = True  # value stored locally, set in set_manager
        self._is_returnval = is_returnval
        self._unit = unit
        self._info = info
        self._name = name
        self._perm = perm

        # states below need to be resored after load
        self._manager = None  # set later by attrsman , necessary?
        self._obj = None  # parent object, set later by attrsman

        self._is_modified = False
        self._is_saved = False

        self.init_plugin(is_plugin)
        # self._init_xml(xmltag)
        self.set_xmltag(xmltag, xmlsep)

        # set rest of attributes passed as keyword args
        # no matter what they are used for
        for attr, value in attrs.iteritems():
            setattr(self, attr, value)

    def is_save(self):
        return self._is_save

    def set_save(self, is_save):
        if is_save:
            self._manager.do_save_attr(self.attrname)
        else:
            self._manager.do_not_save_attr(self.attrname)

    def add_groupnames(self, groupnames):
        self.groupnames = list(set(self.groupnames+groupnames))
        self._manager.insert_groupnames(self)

    def has_group(self, groupname):
        return groupname in self.groupnames

    def enable_plugin(self, is_enabled=True):
        if self.plugin is not None:
            self.plugin.enable(is_enabled)

    def get_metatype(self):
        return self.metatype

    def init_plugin(self, is_plugin):
        if is_plugin:
            self.plugin = Plugin(self)
            self.set = self.set_plugin
            self.get = self.get_plugin
        else:
            self.plugin = None

    # def _init_xml(self,xmltag=None):
    #    if xmltag is not None:
    #        self.xmltag = xmltag
    #    else:
    #        self.xmltag = self.attrname

    def set_xmltag(self, xmltag, xmlsep=' '):
        self.xmltag = xmltag
        self.xmlsep = xmlsep

    def get_value_from_xmlattr(self, xmlattrs):
        """
        Return a value of the correct data type
        from the xml attribute object

        If this configuration is not found in xmlattrs
        then None is returned.
        """
        if (self.xmltag is not None):
            if xmlattrs.has_key(self.xmltag):
                return self.get_value_from_string(xmlattrs[self.xmltag])
            else:
                return None

    def get_value_from_string(self, s, sep=','):
        """
        Returns the attribute value from a string in the correct type. 
        """

        if self.metatype == 'color':
            return xm.parse_color(s, sep)

        # TODO: allow arrays
        # elif hasattr(val, '__iter__'):
        #    if len(val)>0:
        #        if hasattr(val[0], '__iter__'):
        #            # matrix
        #            fd.write(xm.mat(self.xmltag,val))
        #        else:
        #            # list
        #            fd.write(xm.arr(self.xmltag,val,self.xmlsep))
        #    else:
        #        # empty list
        #        #fd.write(xm.arr(self.xmltag,val))
        #        # don't even write empty lists
        #        pass

        elif hasattr(self, 'xmlmap'):
            imap = get_inversemap(self.xmlmap)
            # print 'get_value_from_string',s,imap
            if imap.has_key(s):
                return imap[s]
            else:
                return self.get_numvalue_from_string(s)

        else:
            return self.get_numvalue_from_string(s)

    def get_numvalue_from_string(self, s):
        t = type(self._default)
        if t in (types.UnicodeType, types.StringType):
            return s

        elif t in (types.UnicodeType, types.StringType):
            return s

        elif t in (types.LongType, types.IntType):
            return int(s)

        elif t in (types.FloatType, types.ComplexType):
            return float(s)

        elif t == types.BooleanType:  # use default and hope it is no a numpy bool!!!
            if s in ('1', 'True'):
                return True
            else:
                return False

        else:
            return None  # unsuccessful

    def write_xml(self, fd):
        if self.xmltag is not None:
            self._write_xml_value(self.get_value(), fd)

    def _write_xml_value(self, val, fd):
        # print 'write_xml',self.xmltag,hasattr(val, '__iter__')
        if self.metatype == 'color':
            fd.write(xm.color(self.xmltag, val))

        elif hasattr(val, '__iter__'):
            if len(val) > 0:
                if hasattr(val[0], '__iter__'):
                    # matrix
                    fd.write(xm.mat(self.xmltag, val))
                else:
                    # list
                    fd.write(xm.arr(self.xmltag, val, self.xmlsep))
            else:
                # empty list
                # fd.write(xm.arr(self.xmltag,val))
                # don't even write empty lists
                pass

        elif hasattr(self, 'xmlmap'):
            if self.xmlmap.has_key(val):
                fd.write(xm.num(self.xmltag, self.xmlmap[val]))
            else:
                fd.write(xm.num(self.xmltag, val))

        elif hasattr(self, 'choices'):
            if type(self.choices) == types.ListType:
                fd.write(xm.num(self.xmltag, val))
            else:
                # print '_write_xml_value',self.attrname
                # print '  val,self.choices.values()',val,self.choices.values()
                i = self.choices.values().index(val)
                fd.write(xm.num(self.xmltag, self.choices.keys()[i]))

        elif type(self._default) == types.BooleanType:  # use default and hope it is no a numpy bool!!!
            if val:
                fd.write(xm.num(self.xmltag, 1))
            else:
                fd.write(xm.num(self.xmltag, 0))

        elif type(self._default) in (types.UnicodeType, types.StringType):
            if len(val) > 0:
                fd.write(xm.num(self.xmltag, val))

        else:
            # scalar number or string
            fd.write(xm.num(self.xmltag, val))

    def get_name(self):
        return self._name

    def is_modified(self):
        # print 'is_modified', self.attrname, self._is_modified
        return self._is_modified

    def set_modified(self, is_modified):
        self._is_modified = is_modified

    def set_manager(self, manager):
        """
        Method to set manager to attribute configuration object.
        This is either attribute manager or table manager.
        Used by add method of AttrManager
        """
        self._manager = manager
        self._is_localvalue = manager.is_localvalue()
        self.set_save(self._is_save)

    def get_manager(self):
        """
        Method to get manager to attribute configuration object.
        """
        return self._manager

    def set_obj(self, obj):
        """
        Method to set instance of managed object.
        Used by add method of AttrManager
        """
        self._obj = obj

    def get_obj(self):
        return self._obj

    def get(self):
        #  return attribute, overridden with indexing for array and dict struct
        return self.get_value()

    def set(self, value):
        # set attribute, overridden with indexing for array and dict struct
        if value != self.get_value():
            self.set_value(value)
            self._is_modified = True
        return value

    def get_plugin(self):
        """
        Default get method with plugin for scalar attrs
        """
        #  return attribute, overridden with indexing for array and dict struct
        self.plugin.exec_events(EVTGET)

        return self.get_value()

    def set_plugin(self, value):
        """
        Default set method with plugin for scalar attrs
        """
        # set attribute, overridden with indexing for array and dict struct
        if value != self.get_value():
            self.set_value(value)
            self._is_modified = True
            self.plugin.exec_events(EVTSET)

        return value

    def set_default(self, value):
        self._default = value

    def get_default(self):
        return self._default

    def get_init(self):
        """
        Returns initialization of attribute.
        Usually same as get_default for scalars.
        Overridden by table configuration classes
        """
        value = self.get_default()
        # store locally if required
        if self._is_localvalue:
            self.value = value
        return value

    def reset(self):
        if self._is_localvalue:
            self.value = self.get_default()
        else:
            setattr(self._obj, self.attrname, self.get_default())

    def clear(self):
        self.reset()

    # def is_tableattr(self):
    #    return self.struct in ('dict','array','list')

    def set_perm(self, perm):
        self._perm = perm

    def get_perm(self):
        return self._perm

    def is_returnval(self):
        if hasattr(self, '_is_returnval'):  # for back compatibility
            return self._is_returnval
        else:
            return True

    def is_readonly(self):
        return 'w' not in self._perm

    def is_writable(self):
        return 'w' in self._perm

    def is_editable(self):
        """Can attribute be edited """
        return 'e' in self._perm

    def has_unit(self):
        return self._unit != ''

    def has_info(self):
        return self.get_info() is not None

    def is_colattr(self):
        return hasattr(self, '__getitem__')

    def get_info(self):
        if self._info is None:
            return self.__doc__
        else:
            return self._info

    def format_unit(self, show_parentesis=False):
        if self._unit in ('', None):
            return ''
        if show_parentesis:
            return '[%s]' % self._unit
        else:
            return '%s' % self._unit

    def format_value(self, show_unit=False, show_parentesis=False):
        if show_unit:
            unit = ' '+self.format_unit(show_parentesis)
        else:
            unit = ''
        # return repr(self.get_value())+unit
        return str(self.get_value())+unit

    def format_symbol(self):
        if hasattr(self, 'symbol'):
            symbol = self.symbol
        else:
            symbol = self._name

        return symbol+' '+self.format_unit(show_parentesis=True)

    ####
    def get_value(self):
        # always return attribute, no indexing, no plugin
        if self._is_localvalue:
            return self.value
        else:
            return getattr(self._obj, self.attrname)

    def set_value(self, value):
        # set entire attribute, no indexing, no plugin
        # print 'AttrConf.set_value',self.attrname, self._is_localvalue, value, type(value)
        if self._is_localvalue:
            self.value = value
        else:
            return setattr(self._obj, self.attrname, value)

    def predelete(self):
        """
        Cleanup operations before deleting
        """
        if self._is_localvalue:
            del self.value  # delete value
        else:
            del self._obj.__dict__[self.attrname]  # delete value

    # def init_presave_internal(self, man, obj):
    #    pass
    # not a good idea to delete links, plugins here

    # def save_value(self, state):
    #    """
    #    Save attribute value of managed object to state dict.
    #
    #    move this into __getstate__
    #
    #    restore value in _obj during postllad_external
    #
    #    make _getstate_ for speecific save
    #    """
    #    #print 'save_value',self.attrname,self._is_save, self._is_localvalue,
    #    #
    #    # Attention can be called fron __getstate__ of obj if _is_localvalue=False
    #    # or from __getstate__ of attribute config if _is_localvalue=True

    def _getstate_specific(self, state):
        """
        Called by __getstate__ to add/change specific states,
        before returning states.
        To be overridden.
        """
        pass

    def __getstate__(self):
        # print 'AttrConf.__getstate__',self.get_obj().format_ident_abs(),self.attrname
        # print '  self.__dict__=\n',self.__dict__.keys()
        if self._is_saved:
            # this message indicates a loop!!
            print 'WARNING in __getstate__: Attribute already saved:', self.get_obj().format_ident_abs(), self.attrname
        state = {}
        for attr in self.__dict__.keys():

            if attr == 'plugin':
                plugin = self.__dict__[attr]
                if plugin is not None:
                    state[attr] = True
                else:
                    state[attr] = False

            elif attr not in ATTRS_NOSAVE:
                state[attr] = self.__dict__[attr]

        if self._is_save:
            self._is_modified = False
            state['value'] = self.get_value()

        self._getstate_specific(state)
        # print  '  state',state
        return state

    def __setstate__(self, state):
        # print '__setstate__',self

        # this is always required, but will not be saved
        self.plugins = {}

        for attr in state.keys():
            # print '  state key',attr, state[attr]
            # done in init_postload_internal...
            # if attr=='plugin':
            #    if state[attr]==True:
            #        self.__dict__[attr] = Plugin(self)
            #    else:
            #        self.__dict__[attr]= None
            # else:
            self.__dict__[attr] = state[attr]

    def init_postload_internal(self, man, obj):
        # print 'AttrConf.init_postload_internal',self.attrname,hasattr(self,'value'),self._is_save,self._is_localvalue,'obj:',obj.ident

        self.set_manager(man)
        self.set_obj(obj)
        self.init_plugin(self.plugin)

        # set initial values for unsafed attributes
        if not self._is_save:
            self.set_value(self.get_init())
        else:
            if self._is_localvalue:
                # OK self.value already set in __setstate__
                pass
            else:
                setattr(self._obj, self.attrname, self.value)  # TODO: could be made nicer with method
                del self.value  # no longer needed

        # print '  check',hasattr(self,'value')
        # print '  value=',self.get_value()
        self._is_saved = False

    def init_postload_external(self):
        pass


class NumConf(AttrConf):
    """
    Contains additional information on the object's attribute.
    Here specific number related attributes are defined.
    """

    def __init__(self, attrname, default,
                 digits_integer=None, digits_fraction=None,
                 minval=None, maxval=None,
                 **kwargs):
        self.min = minval
        self.max = maxval
        self.digits_integer = digits_integer
        self.digits_fraction = digits_fraction

        AttrConf.__init__(self,  attrname, default, metatype='number',
                          **kwargs
                          )


class ObjConf(AttrConf):
    """
    Contains additional information on the object's attribute.
    Configures Pointer to another object .
    This other object must have an ident.
    it can be either a child (then it will be saved)
    or a link (then only the id will saved)
    If it is a child the is_child = True (default value) 
    """

    def __init__(self, valueobj, is_child=True, **kwargs):
        attrname = valueobj.get_ident()
        self._is_child = is_child
        AttrConf.__init__(self,  attrname, valueobj,
                          struct='scalar',
                          metatype='obj',
                          perm='r',
                          **kwargs
                          )

    def set_obj(self, obj):
        """
        Method to set instance of managed object.
        Used by add method of AttrManager
        """
        # print 'ObjConf.set_obj',self.attrname,obj.ident
        AttrConf.set_obj(self, obj)

        if self._is_child:
            # tricky: during first initialization
            # child instance is stored in default
            obj.set_child(self)

    def predelete(self):
        AttrConf.predelete(self)
        if self._is_child:
            self.get_obj().del_child(self.attrname)

    def reset(self):
        if self._is_child:
            self.get_value().reset()

    def clear(self):
        if self._is_child:
            self.get_value().clear()

    def is_child(self):
        return self._is_child

    def _getstate_specific(self, state):
        """
        Called by __getstate__ to add/change specific states,
        before returning states.
        To be overridden.
        """
        # print 'ObjConf._getstate_specific',self.attrname,self._is_child,self._is_save
        if self._is_save:
            if self._is_child:
                # OK self.value already set in
                pass
            else:
                # print ' remove object reference from value and create ident'
                state['value'] = None
                state['_ident_value'] = self.get_value().get_ident_abs()
                # print '  ',

    def is_modified(self):
        # if self._is_child
        #is_modified = self.get_value().is_modified()
        # print 'is_modified', self.attrname, is_modified
        return self.get_value().is_modified()

    def set_modified(self, is_modified):
        if self._is_child:
            self.get_value().set_modified(is_modified)

    def write_xml(self, fd):
        """
        Objects are not written here, but in write_xml of the parent obj.
        """
        pass

    def init_postload_internal(self, man, obj):
        # print 'ObjConf.init_postload_internal',self.attrname,hasattr(self,'value'),self._is_save,self._is_localvalue,'parent obj:',obj.ident

        AttrConf.init_postload_internal(self, man, obj)
        if self._is_child:
            # print '  make sure children get initialized'
            # self.get_value().init_postload_internal(obj)
            # print '  call init_postload_internal of',self.get_value().ident,self.get_value(),self.get_value().__class__,self.get_value().init_postload_internal
            self.get_value().init_postload_internal(obj)

    def init_postload_external(self):
        # print 'ObjConf.init_postload_external',self.attrname,self._is_child
        if self._is_child:
            # restore normally
            AttrConf.init_postload_external(self)
            self.get_value().init_postload_external()
        else:
            # Substitute absolute ident with link object.
            # Called from init_postload_external of attrsman during load_obj
            #
            ident_abs = self._ident_value
            # print 'reset_linkobj',self.attrname,ident_abs
            obj = self.get_obj()
            rootobj = obj.get_root()
            # print '  rootobj',rootobj.ident
            linkobj = rootobj.get_obj_from_ident(ident_abs)
            # print '  linkobj',linkobj.ident
            self.set_value(linkobj)

    # def get_valueobj(self):
    #    """
    #    This is called by get_childobj to retrive the child instance.
    #    """
    #    return self.get_value()

    def get_name(self):
        return self.get_value().get_name()

    def get_info(self):
        return self.get_value().__doc__

    def format_value(self, show_unit=False, show_parentesis=False):
        return repr(self.get_value())


class FuncConf(AttrConf):
    """
    Configures a function.
    The function with name funcname must be a method of the object. 
    Default value is used to specify the type of output.  
    """

    def __init__(self,  attrname, funcname, exampleoutput, struct='scalar.func', perm='r', **kwargs):
        self.funcname = funcname

        AttrConf.__init__(self,  attrname, exampleoutput,
                          struct=struct,
                          perm=perm,
                          is_save=False,
                          **kwargs
                          )

    def set_obj(self, obj):
        AttrConf.set_obj(self, obj)
        if self._info == '':
            self._info = getattr(self._obj, self.funcname).__doc__

    def get_value(self):
        # print 'get_value',self.attrname
        # always return attribute, no indexing, no plugin
        return getattr(self._obj, self.funcname)()
        # if self._is_localvalue:
        #    return self.value
        # else:
        #    return getattr(self._obj, self.attrname)

    def get_function(self):
        # print 'get_function',self.attrname
        return getattr(self._obj, self.funcname)

    def set_value(self, value):
        # set entire attribute, no indexing, no plugin
        # print 'AttrConf.set_value',self.attrname, self._is_localvalue, value, type(value)
        # this will run the function and return a value
        return self.get_function()

    def is_modified(self):
        return False

    def reset(self):
        pass

    def clear(self):
        pass


class Indexing:
    """
    Mixing to allow any column attribute to be used as index. 
    """

    def _init_indexing(self):
        """
        Init Indexing management attributes.
        """

        self._index_to_id = {}  # OrderedDict()

        # this updates index if values already exist
        if hasattr(self, 'value'):
            ids = self.get_obj().get_ids()
            self.add_indices(ids, self[ids])

    def reset_index(self):
        self._init_indexing()

    def get_indexmap(self):
        return self._index_to_id

    def get_id_from_index(self, index):
        return self._index_to_id[index]

    def has_index(self, index):
        return self._index_to_id.has_key(index)

    def get_ids_from_indices(self, indices):
        ids = len(indices)*[0]
        for i in range(len(indices)):
            # if not self._index_to_id.has_key(indices[i]):
            #    print 'WARNING from get_ids_from_indices: no index',indices[i]
            #    print self._index_to_id
            ids[i] = self._index_to_id[indices[i]]
        return ids

    def get_ids_from_indices_save(self, indices):
        ids = len(indices)*[0]
        for i in range(len(indices)):
            if not self._index_to_id.has_key(indices[i]):
                ids[i] = -1
            else:
                ids[i] = self._index_to_id[indices[i]]
        return ids

    # use set instead of add
    def add_indices(self, ids, indices):
        for _id, index in zip(ids, indices):
            self.add_index(_id, index)

    def add_index(self, _id, index):
        self._index_to_id[index] = _id

    def rebuild_indices(self):
        for idx in self._index_to_id.keys():
            del self._index_to_id[idx]
        ids = self.get_obj().get_ids()
        self.add_indices(ids, self[ids])

    def del_indices(self, ids):

        for _id in ids:
            self.del_index(_id)

    def set_index(self, _id, index):
        # print 'set_index',self.attrname,_id, index
        # print '  B_index_to_id',self._index_to_id
        self.del_index(_id)
        self.add_index(_id, index)
        # print '  A_index_to_id',self._index_to_id

    def set_indices(self, ids, indices):
        self.del_indices(ids)
        self.add_indices(ids, indices)

    def del_index(self, _id):
        index = self[_id]
        # when index is added (with set) no previous index value exists
        if self._index_to_id.has_key(index):
            del self._index_to_id[index]

    def get_ids_sorted(self):
        # print 'get_ids_sorted',self.value
        # print '  _index_to_id',self._index_to_id
        # print '  sorted',sorted(self._index_to_id.iteritems())
        return OrderedDict(sorted(self._index_to_id.iteritems())).values()

    # def indexset(self, indices, values):
    # no! set made with respective attribute
    #    print 'indexset',indices
    #    print '  ids=',self.get_ids_from_indices(indices)
    #    print '  values=',values
    #    self[self.get_ids_from_indices(indices)] = values


class ColConf(Indexing, AttrConf):
    """
    Basic column configuration.
    Here an ordered dictionary is used to represent the data.
    #>>> from collections import OrderedDict
    #>>> spam = OrderedDict([('s',(1,2)),('p',(3,4)),('a',(5,6)),('m',(7,8))])
    >>> spam.values()

    """
    # def __init__(self, **attrs):
    #    print 'ColConf',attrs

    def __init__(self, attrname, default, is_index=False, **attrs):
        # print 'ColConf',attrs
        self._is_index = is_index
        AttrConf.__init__(self,  attrname, default,
                          struct='odict',
                          **attrs)

        if is_index:
            self._init_indexing()

    def is_index(self):
        return self._is_index

    def get_defaults(self, ids):
        # create a list, should work for all types and dimensions
        # default can be scalar or an array of any dimension
        # print '\n\nget_defaults',self.attrname,ids,self.get_default()
        values = []
        for _id in ids:
            values.append(self.get_default())
        # len(ids)*self.get_default() # makes links, not copies
        return values

    def get_init(self):
        """
        Returns initialization of attribute.
        Usually same as get_default for scalars.
        Overridden by table configuration classes
        """
        ids = self._manager.get_ids()

        # print '\n\nget_init',self.attrname,ids
        values = self.get_defaults(ids)
        i = 0
        odict = OrderedDict()
        for _id in ids:
            odict[_id] = values[i]
            i += 1
        # store locally if required
        if self._is_localvalue:
            self.value = odict
        # pass on to calling instance
        # in this cas the data is stored under self._obj
        return odict

    def reset(self):
        # this reset works also for np arrays!
        odict = self.get_init()
        if not self._is_localvalue:
            setattr(self._obj, self.attrname, odict)
        if self._is_index:
            self.reset_index()

    def init_plugin(self, is_plugin):
        if is_plugin:
            self.plugin = Plugin(self)
            self.set = self.set_plugin
            self.get = self.get_plugin
            self.add = self.add_plugin
            self.delete = self.delete_plugin
        else:
            self.plugin = None

    def write_xml(self, fd, _id):
        if self.xmltag is not None:
            self._write_xml_value(self[_id], fd)

    def __delitem__(self, ids):
        # print '        before=\n',self.__dict__[attr]
        #attr = self.attrconf.get_attr()
        if hasattr(ids, '__iter__'):
            if self._is_index:
                self.del_indices(ids)

            array = self.get_value()
            for _id in ids:
                del array[_id]

        else:
            if self._is_index:
                self.del_index(ids)

            del self.get_value()[ids]

    def delete_item(self, _id):
        # print '        before=\n',self.__dict__[attr]
        #attr = self.attrconf.get_attr()
        del self.get_value()[_id]

    def __getitem__(self, ids):
        # print '__getitem__',key
        if hasattr(ids, '__iter__'):
            items = len(ids)*[None]
            i = 0
            array = self.get_value()
            for _id in ids:
                items[i] = array[_id]
                i += 1
            return items
        else:
            return self.get_value()[ids]

    def __setitem__(self, ids, values):
        # print '__setitem__',ids,values,type(self.get_value())
        if hasattr(ids, '__iter__'):
            if self._is_index:
                self.set_indices(ids, values)  # must be set before setting new value
            i = 0
            array = self.get_value()
            for _id in ids:
                array[_id] = values[i]
                i += 1
            # if self._is_index:
            #    self.add_indices(ids, values)
        else:

            if self._is_index:
                self.set_index(ids, values)  # must be set before setting new value
            self.get_value()[ids] = values

            # if self._is_index:
            #    self.add_index(ids, values)

    def add(self, ids, values=None):
        if not hasattr(ids, '__iter__'):
            _ids = [ids]
            if values is not None:
                _values = [values]
        else:
            _ids = ids
            _values = values

        if values is None:
            _values = self.get_defaults(_ids)

        # print 'add ids, _values',ids, _values
        # trick to prevent updating index before value is added
        if self._is_index:
            is_index_backup = True
            self._is_index = False
        else:
            is_index_backup = False

        self[_ids] = _values

        if is_index_backup:
            self._is_index = True
            self.add_indices(_ids, _values)

        self._is_modified = True

    def add_plugin(self, ids, values=None):
        if not hasattr(ids, '__iter__'):
            _ids = [ids]
            if values is not None:
                _values = [values]
        else:
            _ids = ids
            _values = values

        # print 'add ids, _values',ids, _values
        if values is None:
            _values = self.get_defaults(_ids)
        # trick to prevent updating index before value is added
        if self._is_index:
            is_index_backup = True
            self._is_index = False
        else:
            is_index_backup = False

        self[_ids] = _values

        if is_index_backup:
            self._is_index = True
            self.add_indices(_ids, _values)

        self._is_modified = True

        if self.plugin:
            self.plugin.exec_events_ids(EVTADDITEM, _ids)

    def get(self, ids):
        """
        Central function to get the attribute value associated with ids.
        should be overridden by specific array configuration classes
        """
        return self[ids]

    def get_plugin(self, ids):
        """
        Central function to get the attribute value associated with ids.
        should be overridden by specific array configuration classes
        """
        if self._plugin:
            if not hasattr(ids, '__iter__'):
                self.plugin.exec_events_ids(EVTGETITEM, [ids])
            else:
                self.plugin.exec_events_ids(EVTGETITEM, ids)
        return self[ids]

    def set(self, ids, values):
        """
        Returns value of array element for all ids.
        """

        self[ids] = values
        self._is_modified = True
        # print 'set',self.attrname
        if self._is_index:
            self.set_indices(ids, values)

    def set_plugin(self, ids, values):
        """
        Returns value of array element for all ids.
        """
        self[ids] = values
        self._is_modified = True
        # print 'set',self.attrname

        if self.plugin:
            if not hasattr(ids, '__iter__'):
                self.plugin.exec_events_ids(EVTSETITEM, [ids])
            else:
                self.plugin.exec_events_ids(EVTSETITEM, ids)
        if self._is_index:
            self.set_indices(ids, values)

    def delete(self, ids):
        """
        removes key from array structure
        To be overridden
        """
        del self[ids]
        self._is_modified = True

    def delete_plugin(self, ids):
        """
        removes key from array structure
        To be overridden
        """
        if self.plugin:
            if not hasattr(ids, '__iter__'):
                self.plugin.exec_events_ids(EVTGETITEM, [ids])
            else:
                self.plugin.exec_events_ids(EVTGETITEM, ids)

        del self[ids]
        self._is_modified = True

    def format_value(self, _id, show_unit=False, show_parentesis=False):
        if show_unit:
            unit = ' '+self.format_unit(show_parentesis)
        else:
            unit = ''
        # return repr(self[_id])+unit

        #self.min = minval
        #self.max = maxval
        #self.digits_integer = digits_integer
        #self.digits_fraction = digits_fraction
        val = self[_id]
        tt = type(val)

        if tt in (types.LongType, types.IntType):
            return str(val)+unit

        elif tt in (types.FloatType, types.ComplexType):
            if hasattr(attrconf, 'digits_fraction'):
                digits_fraction = self.digits_fraction
            else:
                digits_fraction = 3
            return "%."+str(digits_fraction)+"f" % (val)+unit

        else:
            return str(val)+unit

        # return str(self[_id])+unit

    def format(self, ids=None):
        # TODO: incredibly slow when calling format_value for each value
        text = ''

        if ids is None:
            ids = self._manager.get_ids()
        if not hasattr(ids, '__iter__'):
            ids = [ids]

        #unit =  self.format_unit()
        attrname = self.attrname
        for id in ids:
            text += '%s[%d] = %s\n' % (attrname, id, self.format_value(id, show_unit=True))

        return text[:-1]  # remove last newline


class NumcolConf(ColConf):
    def __init__(self, attrname, default,
                 digits_integer=None, digits_fraction=None,
                 minval=None, maxval=None,
                 **attrs):
        self.min = minval
        self.max = maxval
        self.digits_integer = digits_integer
        self.digits_fraction = digits_fraction

        ColConf.__init__(self, attrname, default, **attrs)


class IdsConf(ColConf):
    """
    Column, where each entry is the id of a single Table. 
    """

    def __init__(self, attrname, tab,  id_default=-1, is_index=False, perm='r', **kwargs):
        self._is_index = is_index
        self._tab = tab

        AttrConf.__init__(self,  attrname,
                          id_default,  # default id
                          struct='odict',
                          metatype='id',
                          perm=perm,
                          **kwargs
                          )
        self.init_xml()
        # print 'IdsConf.__init__',attrname
        # print '  ',self._tab.xmltag,self._attrconfig_id_tab

    def set_linktab(self, tab):
        self._tab = tab

    def get_linktab(self):
        return self._tab

    def init_xml(self):
        # print 'init_xml',self.attrname
        if self._tab.xmltag is not None:

            # necessary?? see ObjMan.write_xml
            xmltag_tab, xmltag_item_tab, attrname_id_tab = self._tab.xmltag
            if (attrname_id_tab is None) | (attrname_id_tab is ''):
                self._attrconfig_id_tab = None
            else:
                self._attrconfig_id_tab = getattr(self._tab, attrname_id_tab)  # tab = tabman !

            if not hasattr(self, 'is_xml_include_tab'):
                # this means that entire table rows will be included
                self.is_xml_include_tab = False
            # print '  xmltag_tab, xmltag_item_tab, attrname_id_tab',xmltag_tab, xmltag_item_tab, attrname_id_tab,self.is_xml_include_tab

        else:
            self._attrconfig_id_tab = None
            self.is_xml_include_tab = False

    def write_xml(self, fd, _id, indent=0):
        if (self.xmltag is not None) & (self[_id] >= 0):
            if self._attrconfig_id_tab is None:
                self._write_xml_value(self[_id], fd)
            elif self.is_xml_include_tab:
                # this means that entire table rows will be included
                self._tab.write_xml(fd, indent, ids=self[_id], is_print_begin_end=False)
            else:
                self._write_xml_value(self._attrconfig_id_tab[self[_id]], fd)

    def _write_xml_value(self, val, fd):
        # print 'write_xml',self.xmltag,hasattr(val, '__iter__')
        if hasattr(val, '__iter__'):
            if len(val) > 0:
                if hasattr(val[0], '__iter__'):
                    # matrix
                    fd.write(xm.mat(self.xmltag, val))
                else:
                    # list
                    fd.write(xm.arr(self.xmltag, val, self.xmlsep))
            else:
                # empty list
                # fd.write(xm.arr(self.xmltag,val))
                # don't even write empty lists
                pass

        elif type(self._default) in (types.UnicodeType, types.StringType):
            if len(val) > 0:
                fd.write(xm.num(self.xmltag, val))

        else:
            # scalar number or string
            fd.write(xm.num(self.xmltag, val))

    def get_defaults(self, ids):
        # create a list, should work for all types and dimensions
        # default can be scalar or an array of any dimension
        # print '\n\nget_defaults',self.attrname,ids,self.get_default()
        return len(ids)*[self.get_default()]

    def _getstate_specific(self, state):
        """
        Called by __getstate__ to add/change specific states,
        before returning states.
        To be overridden.
        """
        if self._is_save:
            # if self._is_child:
            #    # OK self.value already set in
            #    pass
            # else:
            #    # remove table reference and create ident
            # print '_getstate_specific',self._tab.get_ident_abs()
            state['_tab'] = None
            state['_ident_tab'] = self._tab.get_ident_abs()

    def init_postload_internal(self, man, obj):
        # print 'IdsConf.init_postload_internal',self.attrname,hasattr(self,'value'),self._is_save,self._is_localvalue,'obj:',obj.ident

        AttrConf.init_postload_internal(self, man, obj)
        # if self._is_child:
        #    print '  make sure children get initialized'
        #    print '  call init_postload_internal of',self._tab.ident
        #    self._tab.init_postload_internal(obj)

    def init_postload_external(self):
        # if self._is_child:
        #    # restore normally
        #    AttrConf.init_postload_external(self)
        #    self._tab.init_postload_external()
        # else:

        # Substitute absolute ident with link object.
        # Called from init_postload_external of attrsman during load_obj
        #
        ident_abs = self._ident_tab
        # print 'reset_linkobj',self.attrname,ident_abs
        obj = self.get_obj()
        rootobj = obj.get_root()
        # print '  rootobj',rootobj.ident
        linkobj = rootobj.get_obj_from_ident(ident_abs)
        # print '  linkobj',linkobj.ident
        self._tab = linkobj
        self.init_xml()

    def is_modified(self):
        return False


class TabIdsConf(ColConf):
    """
    Column, where each entry contains a tuple with table object and id. 
    """

    def __init__(self, attrname,  is_index=False, **kwargs):
        self._is_index = is_index
        AttrConf.__init__(self,  attrname,
                          -1,  # default id
                          struct='odict',
                          metatype='tabids',
                          **kwargs
                          )

    def get_defaults(self, ids):
        # create a list, should work for all types and dimensions
        # default can be scalar or an array of any dimension
        # print '\n\nget_defaults',self.attrname,ids,self.get_default()
        return len(ids)*[(None, -1)]

    def reset(self):
        # TODO: this will reset all the tables
        # instead should reset only the specified ids
        if self._is_child:
            for tab, ids in self.get_value():
                tab.reset()

    def clear(self):
        self.reset()
        # necessary? because tbles have been cleared from manager
        # if self._is_child:
        #    for tab, ids in self.get_value():
        #        tab.clear()

    def _getstate_specific(self, state):
        """
        Called by __getstate__ to add/change specific states,
        before returning states.
        To be overridden.
        """
        if self._is_save:
            n = len(state['value'])
            state['value'] = None
            _tabids_save = n*[None]
            i = 0
            for tab, ids in self.get_value():
                _tabids_save[i] = [tab.get_ident_abs(), ids]
                i += 1
            state['_tabids_save'] = _tabids_save

    def init_postload_internal(self, man, obj):
        # print 'IdsConf.init_postload_internal',self.attrname,hasattr(self,'value'),self._is_save,self._is_localvalue,'obj:',obj.ident

        AttrConf.init_postload_internal(self, man, obj)
        # if self._is_child:
        #    print '  make sure children get initialized'
        #    print '  call init_postload_internal of',self._tab.ident
        #    self._tab.init_postload_internal(obj)

    def init_postload_external(self):
        # if self._is_child:
        #    # restore normally
        #    AttrConf.init_postload_external(self)
        #    self._tab.init_postload_external()
        # else:

        # Substitute absolute ident with link object.
        # Called from init_postload_external of attrsman during load_obj
        #
        #ident_abs = self._ident_tab
        # print 'reset_linkobj',self.attrname,ident_abs
        #obj = self.get_obj()
        #rootobj = obj.get_root()
        # print '  rootobj',rootobj.ident
        #linkobj = rootobj.get_obj_from_ident(ident_abs)
        # print '  linkobj',linkobj.ident
        #self._tab = linkobj

        # Substitute absolute ident with link object.
        # Called from init_postload_external of attrsman during load_obj
        #
        _tabids_save = self._tabids_save
        #ident_abs = self._ident_value
        # print 'init_postload_external',self.attrname,_tabids_save
        obj = self.get_obj()
        rootobj = obj.get_root()
        # print '  rootobj',rootobj.ident
        tabids = len(self._tabids_save)*[None]
        i = 0
        for tabident, ids in self._tabids_save:
            tab = rootobj.get_obj_from_ident(tabident)
            # print '  ',tab.get_ident_abs(), ids
            tabids[i] = [tab, ids]
            i += 1

        self.set_value(tabids)

    def is_modified(self):
        return False


class ObjsConf(ColConf):
    """
    Column, where each entry is an object of class objclass with 
    ident= (attrname, id). 
    """
    # TODO:
    # there is a problems with objects that are stored here
    # in particular if objects are Table. What is their correct ident
    # or absolute ident. Currently it is not correct.
    # This leads to incorrect referencing when linked from elsewhere
    # for example within TabIdListArrayConf
    # .get_ident_abs() needs to to be correct, such that
    # get_obj_from_ident can locate them
    # maybe it is not an issue of ObjsConf  itself,
    # but the Objects stored must have a special getident method

    def __init__(self, attrname,  is_index=False, **kwargs):
        self._is_index = is_index
        self._is_child = True  # at the moment no links possible
        AttrConf.__init__(self,  attrname,
                          None,  # BaseObjman('empty'), # default id
                          struct='odict',
                          metatype='obj',
                          perm='r',
                          **kwargs
                          )

    def set_obj(self, obj):
        """
        Method to set instance of managed object.
        Used by add method of AttrManager
        """
        # print 'set_obj',self.attrname,obj.ident
        AttrConf.set_obj(self, obj)

        # if self._is_child:
        obj.set_child(self)

    # def get_valueobj(self, id = None):
    #    """
    #    This is called by get_childobj to retrive the child instance.
    #    Here this is just the table.
    #    """
    #    return self._tab

    def predelete(self):
        AttrConf.predelete(self)
        # if self._is_child:
        self.get_obj().del_child(self.attrname)

    # def _getstate_specific(self, state):
    #    """
    #    Called by __getstate__ to add/change specific states,
    #    before returning states.
    #    To be overridden.
    #    """
    #    if self._is_save:
    #        if self._is_child:
    #            # OK self.value already set in
    #            pass
    #        else:
    #            # remove column reference and create column with idents
    #            state['value']= None
    #            idents_obj = OrderedDict()
    #            linkobjs = self.get_value()
    #            for _id in self.get_ids():
    #                idents_obj[_id] = linkobjs[_id].get_ident_abs()
    #            state['_idents_obj'] = idents_obj

    def init_postload_internal(self, man, obj):
        # print 'ObjsConf.init_postload_internal',self.attrname,hasattr(self,'value'),self._is_save,self._is_localvalue,'obj:',obj.ident

        AttrConf.init_postload_internal(self, man, obj)
        # if self._is_child:

        # make sure all children in column get initialized
        # print '  make sure childrenS get initialized'
        childobjs = self.get_value()

        obj = self.get_obj()
        # print 'init_postload_internal',self.attrname,obj,obj.ident
        for _id in obj.get_ids():
            # print '  call init_postload_internal of',childobjs[_id].ident
            childobjs[_id].init_postload_internal(obj)  # attention obj is the parent object!

    def reset(self):
        # print 'ObjsConf.reset',self.get_value(),len(self.get_obj().get_ids())
        #obj = self.get_obj()
        # print 'init_postload_internal',self.attrname,obj,obj.ident
        childobjs = self.get_value()
        for _id in self.get_obj().get_ids():
            # print '  call reset of',childobjs[_id].ident,_id
            childobjs[_id].reset()

    def clear(self):
        odict = self.get_init()
        if not self._is_localvalue:
            setattr(self._obj, self.attrname, odict)
        if self._is_index:
            self.reset_index()

    def is_modified(self):
        # if self._is_child
        #is_modified = self.get_value().is_modified()
        # print 'is_modified', self.attrname, is_modified

        childobjs = self.get_value()

        #obj = self.get_obj()
        # print 'init_postload_internal',self.attrname,obj,obj.ident
        for _id in self.get_obj().get_ids():
            # print '  call init_postload_internal of',childobjs[_id].ident
            if childobjs[_id].is_modified():
                return True

    def set_modified(self, is_modified):
        childobjs = self.get_value()

        obj = self.get_obj()
        # print 'init_postload_internal',self.attrname,obj,obj.ident
        for _id in self.get_obj().get_ids():
            # print '  call init_postload_internal of',childobjs[_id].ident
            childobjs[_id].set_modified(is_modified)

    def init_postload_external(self):
        # if self._is_child:
        # restore normally
        AttrConf.init_postload_external(self)
        childobjs = self.get_value()

        for _id in self.get_obj().get_ids():
            childobjs[_id].init_postload_external()

    # def get_name(self):
    #    return self.'Table ID for '+self._tab.get_name()
    #
    # def get_info(self):
    #    return 'ID for Table:\n'+self._tab.get_info()


class Attrsman:
    """
    Manages all attributes of an object

    if argument obj is specified with an instance
    then  attributes are stored under this instance.
    The values of attrname is then directly accessible with

    obj.attrname

    If nothing is specified, then column attribute will be stored under
    the respective config instance of this  attrsman (self).
    The values of attrname is then directly accessible with

    self.attrname.value
    """

    def __init__(self, obj, attrname='attrsman', is_plugin=False):

        if obj is None:
            # this means that column data will be stored
            # in value attribute of attrconfigs
            obj = self
            self._is_localvalue = True
        else:
            # this means that column data will be stored under obj
            self._is_localvalue = False

        self._obj = obj  # managed object
        self._attrconfigs = []  # managed attribute config instances
        self.attrname = attrname  # the manager's attribute name in the obj instance

        self._attrs_nosave = set(ATTRS_NOSAVE)

        # groupes of attributes
        # key=groupname, value = list of attribute config instances
        self._groups = {}

        self.init_plugin(is_plugin)

    def init_plugin(self, is_plugin):
        if is_plugin:
            self.plugin = Plugin(self)
        else:
            self.plugin = None

    def enable_plugin(self, is_enabled=True):
        if self.plugin is not None:
            self.plugin.enable(is_enabled)

    def is_localvalue(self):
        return self._is_localvalue

    def has_attrname(self, attrname):
        # attention this is a trick, exploiting the fact that the
        # attribute object with all the attr info is an attribute
        # of the attr manager (=self)
        return hasattr(self, attrname)

    def is_modified(self):
        for attrconf in self._attrconfigs:
            # TODO: not very clean
            if hasattr(attrconf, 'is_child'):
                if attrconf.is_child():
                    if attrconf.is_modified():
                        return True
            else:
                if attrconf.is_modified():
                    return True

        return False

    def set_modified(self, is_modified=True):
        for attrconf in self._attrconfigs:
            attrconf.set_modified(is_modified)

    def get_modified(self):
        # returns a list of modified attributes
        modified = []
        for attrconf in self._attrconfigs:
            if attrconf.is_modified():
                modified.append(attrconf)
        return modified

    def get_config(self, attrname):
        return getattr(self, attrname)  # a bit risky

    def get_configs(self, is_all=False, structs=None, filtergroupnames=None):
        # print 'get_configs',self,self._obj.ident,structs,filtergroupnames,len(self._attrconfigs)
        if is_all:
            return self._attrconfigs
        else:
            attrconfigs = []
            for attrconf in self._attrconfigs:
                # print '  found',attrconf.attrname,attrconf.struct
                is_check = True
                if (structs is not None):
                    if (attrconf.struct not in structs):
                        is_check = False

                if is_check:
                    # print '  **is_check',is_check
                    if len(attrconf.groupnames) > 0:
                        if '_private' not in attrconf.groupnames:
                            # print '    not private'
                            if filtergroupnames is not None:
                                # print '     apply filtergroupnames',filtergroupnames,attrconf.groupnames
                                if not set(filtergroupnames).isdisjoint(attrconf.groupnames):
                                    # print '       append',attrconf.attrname
                                    attrconfigs.append(attrconf)
                            else:
                                # print '     no filtergroupnames'
                                attrconfigs.append(attrconf)
                    else:
                        if filtergroupnames is None:
                            attrconfigs.append(attrconf)

            return attrconfigs

    # def get_colconfigs(self, is_all = False):
    #    return []

    def get_obj(self):
        return self._obj

    def add(self, attrconf, is_overwrite=False):
        """
        Add a one or several new attributes to be managed.
        kwargs has attribute name as key and Attribute configuration object
        as value.
        """

        attrname = attrconf.attrname
        # print '\n\nAttrsman.add',self.get_obj().ident,'add',attrname,self.has_attrname(attrname)
        dir(self._obj)
        if (not self.has_attrname(attrname)) | is_overwrite:
            attrconf.set_obj(self._obj)
            attrconf.set_manager(self)

            # set configuration object as attribute of AttrManager
            setattr(self, attrname, attrconf)

            # append also to the list of managed objects
            self._attrconfigs.append(attrconf)

            # insert in groups
            self.insert_groupnames(attrconf)

            if self.plugin:
                self.plugin.exec_events_attr(EVTADD, attrconf)

            # return default value as attribute of managed object
            if (attrconf.struct in STRUCTS_SCALAR) & (attrconf.is_returnval()):  # == 'scalar':
                return attrconf.get_init()
            else:
                return None  # table configs do their own init

        else:
            # print '  attribute with this name already exists',attrname,type(attrconf)
            # TODO: here we could do some intelligent updating
            del attrconf
            attrconf = getattr(self, attrname)
            # print '  existing',attrconf,type(attrconf)
            if (attrconf.struct in STRUCTS_SCALAR) & (attrconf.is_returnval()):  # == 'scalar':
                return attrconf.get_value()
            else:
                return None  # table configs do their own init

    def do_not_save_attr(self, attrname):
        self._attrs_nosave.add(attrname)

    def do_save_attr(self, attrname):
        if attrname in self._attrs_nosave:
            self._attrs_nosave.remove(attrname)

    def do_not_save_attrs(self, attrnames):
        self._attrs_nosave.update(attrnames)

    def insert_groupnames(self, attrconf):
        if len(attrconf.groupnames) > 0:
            for groupname in attrconf.groupnames:

                if not self._groups.has_key(groupname):
                    self._groups[groupname] = []

                if attrconf not in self._groups[groupname]:
                    self._groups[groupname].append(attrconf)

    def get_groups(self):
        return self._groups

    def get_groupnames(self):
        return self._groups.keys()

    def has_group(self, groupname):
        return self._groups.has_key(groupname)

    def get_group(self, name):
        """
        Returns a list with attributes that belong to that group name.
        """
        # print 'get_group self._groups=\n',self._groups.keys()
        return self._groups.get(name, [])

    def get_group_attrs(self, name):
        """
        Returns a dictionary with all attributes of a group. 
        Key is attribute name and value is attribute value. 
        """
        # print 'get_group_attrs', self._groups
        attrs = OrderedDict()
        if not self._groups.has_key(name):
            return attrs
        for attrconf in self._groups[name]:
            # print '  attrconf.attrname',attrconf.attrname
            attrs[attrconf.attrname] = getattr(self._obj, attrconf.attrname)
        # print '  attrs',attrs
        return attrs

    def print_attrs(self, show_unit=True, show_parentesis=False, attrconfigs=None):
        print 'Attributes of', self._obj._name, 'ident_abs=', self._obj.get_ident_abs()
        if attrconfigs is None:
            attrconfigs = self.get_configs()

        for attrconf in attrconfigs:
            print '  %s =\t %s' % (attrconf.attrname, attrconf.format_value(show_unit=True))

    def save_values(self, state):
        """
        Called by the managed object during save to save the
        attribute values. 
        """
        for attrconfig in self.get_configs():
            attrconfig.save_value(state)

    def delete(self, attrname):
        """
        Delete attibite with respective name
        """
        # print '.__delitem__','attrname=',attrname

        # if hasattr(self,attrname):
        attrconf = getattr(self, attrname)

        if attrconf in self._attrconfigs:
            if self.plugin:
                self.plugin.exec_events_attr(EVTDEL, attrconf)

            for groupname in attrconf.groupnames:
                self._groups[groupname].remove(attrconf)

            self._attrconfigs.remove(attrconf)
            attrconf.predelete()  # this will remove also the value attribute

            #attrname = attrconf.attrname
            del self.__dict__[attrname]    # delete config
            return True

        return False  # attribute not managed
        # return False # attribute not existant

    def __getstate__(self):
        # if hasattr(self,'attrname'):
        #    print 'Attrsman.__getstate__',self.attrname,'  of  obj=',self._obj.ident
        # else:
        #    print 'WARNING in Attrsman.__getstate__','attrname missing'

        if not hasattr(self, '_obj'):
            print 'WARNING: unknown obj in attrman', self, type(self)
            # print '  dir',dir(self)
            # if hasattr(self,'attrname'):
            #    print '    No attrman but attribute',self.attrname
            # for attrconf in self.get_configs(is_all=True):
            #    print '  attrname=',attrconf.attrname
            return {}

        # if not hasattr(self,'_attrs_nosave'):
        #    print 'WARNING: in __getstate__ of',self.attrname#,'obj',self._obj,'has no attr _attrs_nosave'
        #    #self.print_attrs()
        #    print 'dict=\n',self.__dict__

        # print '  self.__dict__=\n',self.__dict__.keys()

        state = {}
        for attr in self.__dict__.keys():
                # print '  attr',attr,self.__dict__[attr]
                # TODO: optimize and put this at the end
            if attr == 'plugin':
                plugin = self.__dict__[attr]
                if plugin is not None:
                    state[attr] = True
                else:
                    state[attr] = False

            elif attr == '_attrconfigs':
                attrconfigs_save = []
                for attrconfig in self._attrconfigs:
                    if attrconfig.is_save():
                        attrconfigs_save.append(attrconfig)
                state[attr] = attrconfigs_save

            elif attr not in self._attrs_nosave:
                state[attr] = self.__dict__[attr]

        # print  '  _attrs_nosave=', self._attrs_nosave
        # print  '  state=', state
        return state

    def __setstate__(self, state):
        # print '__setstate__',self

        # this is always required, but will not be saved
        # self.plugins={}

        for attr in state.keys():
            # print '  set state',attr
            # plugin set in init_postload_internal
            # if attr=='plugin':
            #    if state[attr]==True:
            #        self.__dict__[attr] = Plugin(self)
            #    else:
            #        self.__dict__[attr]= None
            # else:
            self.__dict__[attr] = state[attr]

    def init_postload_internal(self, obj):
        """
        Called after set state.
        Link internal states.
        """
        # print 'Attrsman.init_postload_internal of obj:',obj.ident

        if not hasattr(self, '_attrs_nosave'):
            self._attrs_nosave = set(ATTRS_NOSAVE)

        # if not hasattr(self,'_attrs_nosave'):
        #    print 'WARNING: in init_postload_internal of',self.attrname,'obj',obj,'has no attr _attrs_nosave'

        self._obj = obj
        self.init_plugin(self.plugin)
        for attrconfig in self.get_configs(is_all=True):
            # print '  call init_postload_internal of',attrconfig.attrname
            attrconfig.init_postload_internal(self, obj)

    def init_postload_external(self):
        """
        Called after set state.
        Link external states.
        """
        # print 'init_postload_external',self._obj.get_ident()

        for attrconfig in self.get_configs(is_all=True):
            # print '  ***',attrconfig.attrname,attrconfig.metatype
            attrconfig.init_postload_external()


class Tabman(Attrsman):
    """
    Manages all table attributes of an object.

    if argument obj is specified with an instance
    then column attributes are stored under this instance.
    The values of attrname is then directly accessible with

    obj.attrname

    If nothing is specified, then column attribute will be stored under
    the respective config instance of this tab man (self).
    The values of attrname is then directly accessible with

    self.attrname.value

    """

    def __init__(self, obj=None,  **kwargs):
        Attrsman.__init__(self, obj, **kwargs)
        self._colconfigs = []
        self._ids = []

    def add_col(self, attrconf):
        # print 'add_col',attrconf.attrname,attrconf.is_index()
        attrname = attrconf.attrname
        if not self.has_attrname(attrname):
            Attrsman.add(self, attrconf)  # insert in common attrs database
            self._colconfigs.append(attrconf)
            # returns initial array and also create local array if self._is_localvalue == True
            return attrconf.get_init()
        else:
            return getattr(self, attrname).get_value()

    def delete(self, attrname):
        """
        Delete attribute with respective name
        """
        # print '.__delitem__','attrname=',attrname

        if hasattr(self, attrname):
            attrconf = getattr(self, attrname)
            if self.plugin:
                self.plugin.exec_events_attr(EVTDEL, attrconf)
            if Attrsman.delete(self, attrname):
                if attrconf in self._colconfigs:
                    self._colconfigs.remove(attrconf)

    def get_colconfigs(self, is_all=False, filtergroupnames=None):
        if is_all:
            return self._colconfigs
        else:
            colconfigs = []
            for colconfig in self._colconfigs:
                if len(colconfig.groupnames) > 0:
                    if colconfig.groupnames[0] != '_private':
                        if filtergroupnames is not None:
                            if not set(filtergroupnames).isdisjoint(colconfig.groupnames):
                                colconfigs.append(colconfig)
                        else:
                            colconfigs.append(colconfig)

                else:
                    if filtergroupnames is None:
                        colconfigs.append(colconfig)

            return colconfigs

    def get_ids(self):
        return self._ids

    def __len__(self):
        """
        Determine current array length (same for all arrays)
        """

        return len(self._ids)

    def __contains__(self, _id):
        return _id in self._ids

    def select_ids(self, mask):

        ids_mask = []
        i = 0
        for _id in self.get_ids():
            if mask[i]:
                ids_mask.append(_id)
            i += 1

        return ids_mask

    def suggest_id(self, is_zeroid=False):
        """
        Returns a an availlable id.

        Options:
            is_zeroid=True allows id to be zero.

        """
        if is_zeroid:
            id0 = 0
        else:
            id0 = 1

        id_set = set(self.get_ids())
        if len(id_set) == 0:
            id_max = 0
        else:
            id_max = max(id_set)
        # print  'suggest_id',id0,
        return list(id_set.symmetric_difference(xrange(id0, id_max+id0+1)))[0]

    def suggest_ids(self, n, is_zeroid=False):
        """
        Returns a list of n availlable ids.
        It returns even a list for n=1. 

        Options:
            is_zeroid=True allows id to be zero.
        """
        if is_zeroid:
            id0 = 0
        else:
            id0 = 1
        id_set = set(self.get_ids())
        if len(id_set) == 0:
            id_max = 0
        else:
            id_max = max(id_set)

        return list(id_set.symmetric_difference(xrange(id0, id_max+id0+n)))[:n]

    def add_rows(self, n=None, ids=[], **attrs):
        if n is not None:
            ids = self.suggest_ids(n)
        elif len(ids) == 0:
            # get number of rows from any valye vector provided
            ids = self.suggest_ids(len(attrs.values()[0]))
        else:
            # ids already given , no ids to create
            pass

        self._ids += ids
        # print 'add_rows ids', ids
        for colconfig in self._colconfigs:
            colconfig.add(ids, values=attrs.get(colconfig.attrname, None))
        if self.plugin:
            self.plugin.exec_events_ids(EVTADDITEM, ids)
        return ids

    def add_row(self, _id=None, **attrs):
        if _id is None:
            _id = self.suggest_id()
        self._ids += [_id, ]
        for colconfig in self._colconfigs:
            colconfig.add(_id, values=attrs.get(colconfig.attrname, None))
        if self.plugin:
            self.plugin.exec_events_ids(EVTADDITEM, [_id])
        return _id

    def set_row(self, _id, **attrs):
        for colconfig in self._colconfigs:
            colconfig.set(_id, values=attrs.get(colconfig.attrname, None))
        if self.plugin:
            self.plugin.exec_events_ids(EVTSETITEM, [_id])

    def set_rows(self, ids, **attrs):

        # print 'add_rows ids', ids
        for colconfig in self._colconfigs:
            colconfig.set(ids, values=attrs.get(colconfig.attrname, None))
        if self.plugin:
            self.plugin.exec_events_ids(SETSETITEM, ids)

    def get_row(self, _id):
        attrvalues = {}
        if self.plugin:
            self.plugin.exec_events_ids(EVTGETITEM, [_id])
        for attrconfig in self._colconfigs:
            attrvalues[attrconfig.attrname] = attrconfig[_id]

        return attrvalues

    def del_rows(self, ids):
        if self.plugin:
            self.plugin.exec_events_ids(EVTDELITEM, ids)
        for colconfig in self._colconfigs:
            del colconfig[ids]

        for _id in ids:
            self._ids.remove(_id)

    def del_row(self, _id):
        if self.plugin:
            self.plugin.exec_events_ids(EVTDELITEM, [_id])
        for colconfig in self._colconfigs:
            del colconfig[_id]
        self._ids.remove(_id)

    def __delitem__(self, ids):
        """
        remove rows correspondent to the given ids from all array and dict
        attributes
        """
        if hasattr(ids, '__iter__'):
            self.del_rows(ids)
        else:
            self.del_row(ids)

    def print_attrs(self, **kwargs):
        # print 'Attributes of',self._obj._name,'(ident=%s)'%self._obj.ident
        Attrsman.print_attrs(self, attrconfigs=self.get_configs(structs=['scalar']), **kwargs)
        # print '   ids=',self._ids
        for _id in self.get_ids():
            for attrconf in self.get_configs(structs=STRUCTS_COL):
                print '  %s[%d] =\t %s' % (attrconf.attrname, _id, attrconf.format_value(_id, show_unit=True))


class BaseObjman:
    """
    Object management base methods to be inherited by all object managers.
    """

    def __init__(self, ident, is_plugin=False,  **kwargs):
        # print 'BaseObjman.__init__',ident#,kwargs
        self._init_objman(ident, **kwargs)
        self.set_attrsman(Attrsman(self, is_plugin=is_plugin))
        # print 'BaseObjman.__init__',self.format_ident(),'parent=',self.parent
        self._init_attributes()
        self._init_constants()

    def set_attrsman(self, attrsman):
        self._attrsman = attrsman
        return attrsman

    def _init_objman(self, ident='no_ident', parent=None, name=None,
                     managertype='basic', info=None, logger=None,
                     xmltag=None, version=0.0):
        # print 'BaseObjman._init_objman',ident,logger,parent
        self.managertype = managertype
        self.ident = ident
        self.set_version(version)
        self.set_logger(logger)

        #self._is_root = False
        self.parent = parent
        self.childs = {}  # dict with attrname as key and child instance as value

        self._info = info

        self._is_saved = False

        if name is None:
            self._name = self.format_ident()
        else:
            self._name = name

        self.set_xmltag(xmltag)

        # print '  self.parent',self.parent
        # must be called explicitely during  __init__
        # self._init_attributes()
        # self._init_constants()

    def _init_attributes(self):
        """
        This is the place to add all attributes.
        This method will be called to initialize 
        and after loading a saved object.
        Use this method also to update a version.
        """
        pass

    def _init_constants(self):
        """
        This is the place to init any costants that are outside the management.
        Constants are not saved.
        This method will be called to initialize and after loading a saved object.
        """
        pass

    def set_version(self, version):
        self._version = version

    def get_version(self):
        return self._version

    # def _upgrade_version(self):
    #    pass

    # def _init_xml(self,xmltag=None):
    #    if xmltag is not None:
    #        self.xmltag = xmltag
    #    else:
    #        self.xmltag = self.get_ident()

    def reset(self):
        """
        Resets all attributes to default values
        """
        # print 'reset'
        for attrconfig in self.get_attrsman().get_configs(is_all=True):
            # print '  reset',attrconfig.attrname
            attrconfig.reset()

    def clear(self):
        """
        Clear tables and reset scalars.
        """
        for attrconfig in self.get_attrsman().get_configs(is_all=True):
            attrconfig.clear()

        self._init_constants()

    def unplug(self):
        if self.plugin:
            self.plugin.unplug()

    def set_xmltag(self, xmltag, xmlsep=' '):
        self.xmltag = xmltag
        self.xmlsep = xmlsep

    def write_xml(self, fd, ident):
        if self.xmltag is not None:
            # figure out scalar attributes and child objects
            attrconfigs = []
            objconfigs = []
            for attrconfig in self.get_attrsman().get_configs(structs=STRUCTS_SCALAR):
                if (attrconfig.metatype == 'obj'):  # better use self.childs
                    if (attrconfig.get_value().xmltag is not None) & attrconfig.is_child():
                        objconfigs.append(attrconfig)
                elif attrconfig.xmltag is not None:
                    attrconfigs.append(attrconfig)

            # start writing
            if len(attrconfigs) > 0:
                # there are scalar attributes
                fd.write(xm.start(self.xmltag, ident))
                for attrconfig in attrconfigs:
                    attrconfig.write_xml(fd)

                # are there child objects to write
                if len(objconfigs) > 0:
                    fd.write(xm.stop())
                    for attrconfig in objconfigs:
                        attrconfig.get_value().write_xml(fd, ident+2)
                    fd.write(xm.end(self.xmltag, ident))
                else:
                    fd.write(xm.stopit())
            else:
                # no scalars
                fd.write(xm.begin(self.xmltag, ident))
                if len(objconfigs) > 0:
                    for attrconfig in objconfigs:
                        attrconfig.get_value().write_xml(fd, ident+2)
                fd.write(xm.end(self.xmltag, ident))

    def get_logger(self):
        # print 'get_logger',self.ident,self._logger,self.parent
        if self._logger is not None:
            return self._logger
        else:
            return self.parent.get_logger()

    def set_logger(self, logger):
        # print 'set_logger',self.ident,logger
        self._logger = logger

    def __repr__(self):
        # return '|'+self._name+'|'
        return self.format_ident()

    def is_modified(self):
        return self._attrsman.is_modified()

    def set_modified(self, is_modified=True):
        self._attrsman.set_modified(is_modified)

    def get_name(self):
        return self._name

    def get_info(self):
        if self._info is None:
            return self.__doc__
        else:
            return self._info

    def get_ident(self):
        return self.ident

    def _format_ident(self, ident):
        if hasattr(ident, '__iter__'):
            return str(ident[0])+'#'+str(ident[1])
        else:
            return str(ident)

    def format_ident(self):
        return self._format_ident(self.ident)

    def format_ident_abs(self):
        s = ''
        # print 'format_ident_abs',self.get_ident_abs()
        for ident in self.get_ident_abs():
            s += self._format_ident(ident)+'.'
        return s[:-1]

    def get_root(self):
        # if hasattr(self,'_is_root'):
        #    print 'get_root',self.ident,'is_root',self._is_root
        #    if self._is_root:
        #        return self

        if self.parent is not None:
            return self.parent.get_root()
        else:
            return self

    def get_ident_abs(self):
        """
        Returns absolute identity.
        This is the ident of this object in the global tree of objects.
        If there is a parent objecty it must also be managed by the 
        object manager.
        """
        # print 'obj.get_ident_abs',self.ident,self.parent, type(self.parent)
        # if hasattr(self,'_is_root'):
        #    print 'get_ident_abs',self.ident,'is_root',self._is_root
        #    if self._is_root:
        #        return (self.get_ident(),)# always return tuple

        if self.parent is not None:
            return self.parent.get_ident_abs()+(self.ident,)
        else:
            return (self.get_ident(),)  # always return tuple

    def get_obj_from_ident(self, ident_abs):
        # print 'get_obj_from_ident',self.ident,ident_abs
        if len(ident_abs) == 1:
            # arrived at the last element
            # check if it corresponds to the present object
            if ident_abs[0] == self.ident:
                return self
            else:
                return None  # could throw an error
        else:
            return self.get_childobj(ident_abs[1]).get_obj_from_ident(ident_abs[1:])

    # this is an attemt to restore objects from
    # root objects without childs
    # def search_ident_abs(self, childobj):
    #    """
    #    Returns root and absolute ident for the found root.
    #    """
    #    #if hasattr(self,'_is_root'):
    #    #    print 'get_root',self.ident,'is_root',self._is_root
    #    #    if self._is_root:
    #    #        return self
    #
    #    if self.parent is not None:
    #        if self.parent.childs.has_key(childobj.ident)
    #        return self.parent.get_root()
    #    else:
    #        return self

    # def search_obj_from_ident(self, ident_abs, obj_init):
    #
    #    #print 'get_obj_from_ident',self.ident,ident_abs
    #    if len(ident_abs)==1:
    #        # arrived at the last element
    #        # check if it corresponds to the present object
    #        if ident_abs[0] == self.ident:
    #            return self
    #        else:
    #            return None # could throw an error
    #   else:
    #       return self.get_childobj(ident_abs[1]).get_obj_from_ident(ident_abs[1:])

    def get_childobj(self, attrname):
        """
        Return child instance
        """
        if self.childs.has_key(attrname):
            config = self.childs[attrname]
            return config.get_value()
        else:
            return BaseObjman(self)

    def set_child(self, childconfig):
        """
        Set child childconfig
        """
        self.childs[childconfig.attrname] = childconfig

    def del_child(self, attrname):
        """
        Return child instance
        """
        del self.childs[attrname]

    def get_parent(self):
        return self.parent

    # def reset_parent(self, parent):
    #    self.parent=parent

    # def set_attrsman(self, attrsman):
    #    # for quicker acces and because it is only on
    #    # the attribute management is public and also directly accessible
    #    #setattr(self, attrname,Attrsman(self))# attribute management
    #    self._attrsman = attrsman
    #    #return attrsman

    def get_attrsman(self):
        return self._attrsman

    def _getstate_specific(self, state):
        """
        Called by __getstate__ to add/change specific states,
        before returning states.
        To be overridden.
        """
        pass

    def __getstate__(self):
        # print 'BaseObjman.__getstate__',self.ident,self._is_saved
        # print '  self.__dict__=\n',self.__dict__.keys()
        state = {}
        # if not self._is_saved:

        # if self._is_saved:
        #    # this message indicates a loop!!
        #    print 'WARNING in __getstate__: object already saved',self.format_ident_abs()

        # print  '  save standart values'
        for attr in ATTRS_SAVE:
            if hasattr(self, attr):
                state[attr] = getattr(self, attr)

        # print '  save all scalar stuctured attributes'
        # attrsman knows which and how
        # self._attrsman.save_values(state)
        #
        # values of configured attributes are not saved here
        # values are now ALWAYS stored in the value attribute of the
        # attrconfig and reset in main obj

        # print '  save also attrsman'
        state['_attrsman'] = self._attrsman
        self._getstate_specific(state)

        self._is_saved = True

        # else:
        #    print 'WARNING in __getstate__: object %s already saved'%self.ident
        return state

    def __setstate__(self, state):
        # print '__setstate__',self

        # this is always required, but will not be saved
        # self.plugins={}

        for key in state.keys():
            # print '  set state',key
            self.__dict__[key] = state[key]

        self._is_saved = False
        # done in init2_config...
        # set default values for all states tha have not been saved
        # for attr in self._config.keys():
        #    if (not self._config[attr]['save']) & (not hasattr(self,attr)):
        #        print '  config attr',attr
        #        self.config(attr,**self._config[attr])

        # set other states
        # self._setstate(state)

    def init_postload_internal(self, parent):
        """
        Called after set state.
        Link internal states and call constant settings.
        """
        print 'BaseObjman.init_postload_internal', self.ident, 'parent:'
        # if parent is not None:
        #    print parent.ident
        # else:
        #    print 'ROOT'
        self.parent = parent
        self.childs = {}
        self._attrsman.init_postload_internal(self)

    def init_postload_external(self):
        """
        Called after set state.
        Link internal states.
        """

        #self._is_root =  is_root
        print 'init_postload_external', self.ident  # ,self._is_root
        # set default logger
        self.set_logger(Logger(self))
        # for child in self.childs.values():
        #    child.reset_parent(self)
        self._attrsman.init_postload_external()
        self._init_attributes()
        self._init_constants()


class TableMixin(BaseObjman):

    def format_ident_row(self, _id):
        # print 'format_ident_row',_id
        return self.format_ident()+'['+str(_id)+']'

    def format_ident_row_abs(self, _id):
        return self.format_ident_abs()+'['+str(_id)+']'

    def get_obj_from_ident(self, ident_abs):
        # print 'get_obj_from_ident',self.ident,ident_abs,type(ident_abs)
        if len(ident_abs) == 1:
            # arrived at the last element
            # check if it corresponds to the present object
            ident_check = ident_abs[0]

            #  now 2 things can happen:
            # 1.) the ident is a simple string identical to ident of the object
            # in this case, return the whole object
            # 2.) ident is a tuple with string and id
            # in this case return object and ID
            # if hasattr(ident_check, '__iter__'):
            #    #if (ident_check[0] == self.ident)&(ident_check[1] in self._ids):
            #    if ident_check[1] in self._ids:
            #        return (self, ident_check[1])
            #    else:
            #       return None # could throw an error
            # else:
            if ident_check == self.ident:
                return self
        else:
            childobj = self.get_childobj(ident_abs[1])
            return childobj.get_obj_from_ident(ident_abs[1:])

    def get_childobj(self, ident):
        """
        Return child instance.
        This is any object with ident
        """
        if hasattr(ident, '__iter__'):
            # access of ObjsConf configured child
            # get object from column attrname
            attrname, _id = ident
            config = self.childs[attrname]
            return config[_id]  # config.get_valueobj(_id)
        else:
            # access of ObjConf configured child
            # get object from  attrname
            config = self.childs[ident]
            return config.get_value()

    def __getstate__(self):
        # print '__getstate__',self.ident,self._is_saved
        # print '  self.__dict__=\n',self.__dict__.keys()
        state = {}
        if 1:  # not self._is_saved:

            # print  '  save standart values'
            for attr in ATTRS_SAVE+ATTRS_SAVE_TABLE:
                if attr == 'plugin':
                    plugin = self.__dict__[attr]
                    if plugin is not None:
                        state[attr] = True
                    else:
                        state[attr] = False

                elif hasattr(self, attr):
                    state[attr] = getattr(self, attr)

            # save managed attributes !!!
            for attrconfig in self.get_configs(is_all=True):
                state[attrconfig.attrname] = attrconfig

            # print '  save all scalar stuctured attributes'
            # attrsman knows which and how
            # self.save_values(state)

            # print '  save also attrsman'
            #state['attrsman'] = self._attrsman
            self._is_saved = True

        else:
            print 'WARNING in __getstate__: object %s already saved' % self.ident
        return state

    def __setstate__(self, state):
        # print '__setstate__',self.ident

        # this is always required, but will not be saved
        self.plugins = {}

        for attr in state.keys():
            # print '  set state',key
            if attr == 'plugin':
                if state[attr] == True:
                    self.__dict__[attr] = Plugin(self)
                else:
                    self.__dict__[attr] = None
            else:
                self.__dict__[attr] = state[attr]

        self._is_saved = False
        # done in init2_config...
        # set default values for all states tha have not been saved
        # for attr in self._config.keys():
        #    if (not self._config[attr]['save']) & (not hasattr(self,attr)):
        #        print '  config attr',attr
        #        self.config(attr,**self._config[attr])

        # set other states
        # self._setstate(state)

    def init_postload_internal(self, parent):
        """
        Called after set state.
        Link internal states.
        """
        # print 'TableObjman.init_postload_internal',self.ident,'parent:',
        # if parent is not None:
        #    print parent.ident
        # else:
        #    print 'ROOT'

        if not hasattr(self, '_attrs_nosave'):
            self._attrs_nosave = set(ATTRS_NOSAVE)

        self.parent = parent
        self.childs = {}
        self.set_attrsman(self)
        Attrsman.init_postload_internal(self, self)

        self._is_saved = False

    def init_postload_external(self):
        """
        Called after set state.
        Link internal states.
        """
        Attrsman.init_postload_external(self)
        # no: BaseObjman.init_postload_external(self)
        self._init_attributes()
        self._init_constants()

    def export_csv(self, filepath, sep=',', name_id='ID',
                   file=None, attrconfigs=None, ids=None, groupname=None,
                   is_header=True, is_ident=False, is_timestamp=True):

        # print 'export_csv',filepath,"*"+sep+"*"
        fd = open(filepath, 'w')

        if ids is None:
            ids = self.get_ids()

        if groupname is not None:
            attrconfigs = self.get_group(groupname)
            is_exportall = False

        if attrconfigs is None:
            attrconfigs = self.get_colconfigs(is_all=True)
            is_exportall = False
        else:
            is_exportall = True

        # header
        if is_header:

            row = self._clean_csv(self.get_name(), sep)
            if is_ident:
                row += sep+'(ident=%s)' % self.format_ident_abs()
            fd.write(row+'\n')
            if is_timestamp:
                now = datetime.now()
                fd.write(self._clean_csv(now.isoformat(), sep)+'\n')
            fd.write('\n\n')

        # first table row
        row = name_id
        for attrconf in attrconfigs:
            # print '   write first row',attrconf.attrname
            is_private = attrconf.has_group('_private')
            if ((not is_private) & (attrconf.is_save())) | is_exportall:
                row += sep+self._clean_csv(attrconf.format_symbol(), sep)
        fd.write(row+'\n')

        # rest
        for _id in ids:
            # if self._is_keyindex:
            #    row = str(self.get_key_from_id(id))#.__repr__()
            # else:
            row = str(_id)
            row = self._clean_csv(row, sep)
            for attrconf in attrconfigs:
                is_private = attrconf.has_group('_private')
                if ((not is_private) & (attrconf.is_save())) | is_exportall:
                    row += sep+self._clean_csv('%s' % (attrconf.format_value(_id, show_unit=False)), sep)

            # make sure there is no CR in the row!!
            # print  row
            fd.write(row+'\n')

        if filepath is not None:
            fd.close()

    def _clean_csv(self, row, sep):
        row = row.replace('\n', ' ')
        #row=row.replace('\b',' ')
        row = row.replace('\r', ' ')
        #row=row.replace('\f',' ')
        #row=row.replace('\newline',' ')
        row = row.replace(sep, ' ')
        return row

    def clear_rows(self):
        if self.plugin:
            self.plugin.exec_events_ids(EVTDELITEM, self.get_ids())
        self._ids = []
        for colconfig in self.get_attrsman()._colconfigs:
            # print 'ArrayObjman.clear_rows',colconfig.attrname,len(colconfig.get_value())
            colconfig.clear()
            # print '  done',len(colconfig.get_value())

    def clear(self):
        # print 'ArrayObjman.clear',self.ident
        # clear/reset scalars
        for attrconfig in self.get_attrsman().get_configs(structs=STRUCTS_SCALAR):
            attrconfig.clear()
        self.clear_rows()
        self.set_modified()

    def _write_xml_body(self, fd,  indent, objconfigs, idcolconfig_include_tab, colconfigs,
                        objcolconfigs, xmltag_item, attrconfig_id, xmltag_id, ids, ids_xml):

        # print '_write_xml_body ident,ids',self.ident,ids
        if ids is None:
            ids = self.get_ids()

        if ids_xml is None:
            ids_xml = ids

        for attrconfig in objconfigs:
            attrconfig.get_value().write_xml(fd, indent+2)

        # check if columns contain objects
        #objcolconfigs = []
        scalarcolconfigs = colconfigs
        # for attrconfig in colconfigs:
        #    if attrconfig.metatype == 'obj':
        #        objcolconfigs.append(attrconfig)
        #    else:
        #        scalarcolconfigs.append(attrconfig)

        for _id, id_xml in zip(ids, ids_xml):
            fd.write(xm.start(xmltag_item, indent+2))

            # print '   make tag and id',_id
            if xmltag_id == '':
                # no id tag will be written
                pass
            elif (attrconfig_id is None) & (xmltag_id is not None):
                # use specified id tag and and specified id values
                fd.write(xm.num(xmltag_id, id_xml))

            elif (attrconfig_id is not None):
                # use id tag and values of attrconfig_id
                attrconfig_id.write_xml(fd, _id)

            # print ' write columns',len(scalarcolconfigs)>0,len(idcolconfig_include_tab)>0,len(objcolconfigs)>0
            for attrconfig in scalarcolconfigs:
                # print '    scalarcolconfig',attrconfig.attrname
                attrconfig.write_xml(fd, _id)

            if (len(idcolconfig_include_tab) > 0) | (len(objcolconfigs) > 0):
                fd.write(xm.stop())

                for attrconfig in idcolconfig_include_tab:
                    # print '    include_tab',attrconfig.attrname
                    attrconfig.write_xml(fd, _id, indent+4)

                for attrconfig in objcolconfigs:
                    # print '    objcolconfig',attrconfig.attrname
                    attrconfig[_id].write_xml(fd, indent+4)
                fd.write(xm.end(xmltag_item, indent+2))
            else:
                fd.write(xm.stopit())

        # print '  _write_xml_body: done'

    def write_xml(self, fd, indent, xmltag_id='id', ids=None, ids_xml=None,
                  is_print_begin_end=True, attrconfigs_excluded=[]):
        # print 'write_xml',self.ident#,ids
        if self.xmltag is not None:
            xmltag, xmltag_item, attrname_id = self.xmltag

            if xmltag == '':  # no begin end statements
                is_print_begin_end = False

            if ids is not None:
                if not hasattr(ids, '__iter__'):
                    ids = [ids]

            if attrname_id == '':  # no id info will be written
                attrconfig_id = None
                xmltag_id = ''

            elif attrname_id is not None:  # an attrconf for id has been defined
                attrconfig_id = getattr(self.get_attrsman(), attrname_id)
                xmltag_id = None  # this will define the id tag
            else:
                attrconfig_id = None  # native id will be written using xmltag_id from args

            # print '  attrname_id,attrconfig_id',attrname_id,attrconfig_id
            # if attrconfig_id is not None:
            #    print '  attrconfig_id',attrconfig_id.attrname

            # figure out scalar attributes and child objects
            attrconfigs = []
            objconfigs = []
            colconfigs = []
            objcolconfigs = []
            idcolconfig_include_tab = []
            for attrconfig in self.get_attrsman().get_configs(is_all=True):
                # print '  check',attrconfig.attrname,attrconfig.xmltagis not None,attrconfig.is_colattr(),attrconfig.metatype
                if attrconfig == attrconfig_id:
                    pass
                elif attrconfig in attrconfigs_excluded:
                    pass
                elif attrconfig.is_colattr() & (attrconfig.metatype == 'obj'):
                    objcolconfigs.append(attrconfig)
                elif (attrconfig.is_colattr()) & (attrconfig.metatype in ('ids', 'id')) & (attrconfig.xmltag is not None):
                    if hasattr(attrconfig, "is_xml_include_tab"):
                        if attrconfig.is_xml_include_tab:
                            idcolconfig_include_tab.append(attrconfig)
                        else:
                            colconfigs.append(attrconfig)
                    else:
                        colconfigs.append(attrconfig)
                elif attrconfig.is_colattr() & (attrconfig.xmltag is not None):
                    colconfigs.append(attrconfig)
                elif (attrconfig.metatype == 'obj'):  # better use self.childs
                    if (attrconfig.get_value().xmltag is not None) & attrconfig.is_child():
                        objconfigs.append(attrconfig)
                elif attrconfig.xmltag is not None:
                    attrconfigs.append(attrconfig)

            # print '  attrconfigs',attrconfigs
            # print '  objconfigs',objconfigs
            # print '  idcolconfig_include_tab',idcolconfig_include_tab
            # print '  colconfigs',colconfigs
            # start writing
            if len(attrconfigs) > 0:
                # print '  there are scalar attributes'
                if is_print_begin_end:
                    fd.write(xm.start(xmltag, indent))
                for attrconfig in attrconfigs:
                    attrconfig.write_xml(fd)

                # are there child objects to write
                if (len(objconfigs) > 0) | (len(colconfigs) > 0) | (len(idcolconfig_include_tab) > 0):
                    fd.write(xm.stop())
                    self._write_xml_body(fd, indent, objconfigs, idcolconfig_include_tab,
                                         colconfigs,
                                         objcolconfigs,
                                         xmltag_item, attrconfig_id,
                                         xmltag_id, ids, ids_xml)
                    fd.write(xm.end(xmltag, indent))

                else:
                    fd.write(xm.stopit())
            else:
                # print '  no scalars'
                if is_print_begin_end:
                    fd.write(xm.begin(xmltag, indent))
                self._write_xml_body(fd, indent, objconfigs, idcolconfig_include_tab,
                                     colconfigs,
                                     objcolconfigs,
                                     xmltag_item, attrconfig_id,
                                     xmltag_id, ids, ids_xml)

                if is_print_begin_end:
                    fd.write(xm.end(xmltag, indent))


class TableObjman(Tabman, TableMixin):
    """
    Table Object management manages objects with list and dict based columns. 
    For faster operation use ArrayObjman in arrayman package, which requires numpy.
    """

    def __init__(self, ident, **kwargs):
        self._init_objman(ident, **kwargs)
        self._init_attributes()
        self._init_constants()

    def _init_objman(self, ident, is_plugin=False, **kwargs):
        BaseObjman._init_objman(self, ident, managertype='table', **kwargs)
        Tabman.__init__(self, is_plugin=is_plugin)
        # self.set_attrsman(self)
        self.set_attrsman(self)



###############################################################################
if __name__ == '__main__':
    """
    Test
    """

    pass
