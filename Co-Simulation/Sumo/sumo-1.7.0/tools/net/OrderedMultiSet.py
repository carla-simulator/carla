# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2011-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    OrderedMultiSet.py
# @author  Jakob Erdmann
# @author  Michael Behrisch
# @date    2011-10-04

"""
multi set with insertion-order iteration
based on OrderedSet by Raymond Hettinger (c) , MIT-License
[http://code.activestate.com/recipes/576694/]
"""
from __future__ import absolute_import

import collections
KEY, PREV, NEXT = range(3)


class OrderedMultiSet(collections.MutableSet):

    def __init__(self, iterable=None):
        self.end = end = []
        # sentinel node for doubly linked list
        end += [None, end, end]
        # key --> [(key, prev1, next1), (key, prev2, next2), ...]
        self.map = collections.defaultdict(collections.deque)
        self.size = 0
        if iterable is not None:
            self |= iterable

    def __len__(self):
        return self.size

    def __contains__(self, key):
        return key in self.map

    def add(self, key):
        self.size += 1
        end = self.end
        curr = end[PREV]
        new = [key, curr, end]
        curr[NEXT] = end[PREV] = new
        self.map[key].append(new)

    def discard(self, key):
        if key in self.map:
            self.size -= 1
            deque = self.map[key]
            key, prev, next = deque.popleft()
            prev[NEXT] = next
            next[PREV] = prev
            if len(deque) == 0:
                self.map.pop(key)

    def __iter__(self):
        end = self.end
        curr = end[NEXT]
        while curr is not end:
            yield curr[KEY]
            curr = curr[NEXT]

    def __reversed__(self):
        end = self.end
        curr = end[PREV]
        while curr is not end:
            yield curr[KEY]
            curr = curr[PREV]

    def pop(self, last=True):
        if not self:
            raise KeyError('set is empty')
        key = next(reversed(self)) if last else next(iter(self))
        self.discard(key)
        return key

    def __repr__(self):
        if not self:
            return '%s()' % (self.__class__.__name__,)
        return '%s(%r)' % (self.__class__.__name__, list(self))

    def __eq__(self, other):
        if isinstance(other, self.__class__):
            return len(self) == len(other) and list(self) == list(other)
        return set(self) == set(other)

    def __del__(self):
        self.clear()                    # remove circular references

    def __sub__(self, other):
        result = self.__class__()
        for x in self:
            result.add(x)
        for x in other:
            result.discard(x)
        return result

    def __or__(self, other):
        result = self.__class__()
        for x in self:
            result.add(x)
        for x in other:
            result.add(x)
        return result
