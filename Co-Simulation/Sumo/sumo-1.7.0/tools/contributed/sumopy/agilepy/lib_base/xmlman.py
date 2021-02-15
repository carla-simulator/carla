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

# @file    xmlman.py
# @author  Joerg Schweizer
# @date

#import classman as cm
#XMLTAG = 'xmltag'
import numpy as np
np.set_printoptions(suppress=True, precision=4)


def write_obj_to_xml(obj, filepath, attrname_id=None,
                     encoding='UTF-8',  # 'iso-8859-1'
                     ):
    """
    Universal xml writer for objects
    """
    try:
        fd = open(filepath, 'w')
    except:
        print 'WARNING in write_obj_to_xml: could not open', filepath
        return False
    fd.write('<?xml version="1.0" encoding="%s"?>\n' % encoding)
    indent = 0
    obj.write_xml(fd, indent)
    fd.close()


def begin(attr, indent=0):
    return indent*' '+'<%s>\n' % attr


def end(attr, indent=0):
    return indent*' '+'</%s>\n' % attr


def start(attr, indent=0):
    return indent*' '+'<%s ' % attr


def stop():
    return '>\n'


def stopit():
    return '/>\n'


def num(attr, x):
    return ' %s="%s"' % (attr, x)


INTTYPES = (np.int32, np.int32)
FLOATTYPES = (np.int32, np.int32)


def arr(attr, a, sep=' '):
    # TODO: use https://www.decalage.info/en/python/print_list
    s = ' '
    # print 'arr',attr,a
    # if type(a)==np.ndarray:
    #    dt = a.dtype
    #    if dt in
    #    format = '%.3f'
    # else:
    #

    if len(a) > 0:
        format = '%s'
        s += '%s="' % attr
        for i in xrange(len(a)):
            # print '  a[i]',a[i],type(a[i]),str(a[i]),type(str(a[i]))
            #ss = str(a[i])
            # print '  ',type(s),type(ss),type(sep)
            s += format % a[i]+sep
        return s[:-1]+'"'
    else:
        # return s+'%s="<>"'%attr # NO!!
        return s+'%s=""' % attr


def color(attr, val, sep=','):
    return arr(attr, np.array(val[0:3]*255+0.5, np.int32), sep)


def mat(attr, m):
    # TODO: use https://www.decalage.info/en/python/print_list
    s = ' '
    if len(m) > 0:
        s += '%s="' % attr
        for i in xrange(len(m)):
            r = m[i]
            for j in xrange(len(r)-1):
                s += '%s,' % r[j]
            s += '%s ' % r[-1]
        return s[:-1]+'"'
    else:
        # return s+'%s="<>"'%attr# NO!!
        return s+'%s=""' % attr


def parse_color(s, sep=','):
        # print 'parseColor',s
    arr = s.split(sep)
    color = [1.0, 1.0, 1.0, 1.0]
    #np.ones(4,dtype = np.float32)
    i = 0
    for a in arr:
        color[i] = float(a)
        i += 1

    return color


def parse_color_old(s):
    # print 'parseColor',s
    arr = s.split(',')
    ret = []
    for a in arr:
        ret.append(float(a))

    return tuple(ret)

# def process_shape(s, offset = [0.0,0.0], is_flatarray=False):
#    cshape = []
#    #print 'process_shape',type(s),s
#    es = s.rstrip().split(" ")
#    for e in es:
#        p = e.split(",")
#        if is_flatarray:
#            cshape.append(float(p[0])-offset[0])
#            cshape.append(float(p[1]) - offset[1])
#        else:
#            cshape.append((float(p[0])-offset[0], float(p[1]) - offset[1]))
#    return cshape


def process_shape(shapeString, offset=[0.0, 0.0]):
    cshape = []
    es = shapeString.rstrip().split(" ")
    for e in es:
        p = e.split(",")
        if len(p) == 2:
                # 2D coordinates with elevetion = 0
            cshape.append(np.array([float(p[0])-offset[0], float(p[1]) - offset[1], 0.0], np.float32))
        elif len(p) == 3:
            # 3D coordinates
            cshape.append(np.array([float(p[0])-offset[0], float(p[1]) - offset[1], float(p[2])], np.float32))
        else:
            # print 'WARNING: illshaped shape',e
            # cshape.append(np.array([0,0,0],np.float))
            return []  # np.zeros((0,2),np.float32)#None

    # np.array(cshape,np.float32)
    return cshape  # keep list of vertex arrays


def conver_val(s):
    """
    Convert attribute s from a string to a number, if this is possible.
    """
    # if

# helper function for parsing comment line in xml file


def read_keyvalue(line, key):
    data = line.split(' ')
    for element in data:
        if (element.find(key) >= 0):
            k, v = element.split('=')
            value = v.replace('"', '').replace('<', '').replace('/>', '').strip()
            break
    return value

# def getShapeXml(shape):
##    s = " shape=\""
# for i,c in enumerate(shape):
# if i!=0:
##            s+=(" ")
##        s+=(str(c[0]) + "," + str(c[1]))
# s+=("\"")
# return s
##
# def getListXml(l):
##    s = " shape=\""
# for item in l:
##        s+= str(item) + " "
# s+=("\"")
# return s
