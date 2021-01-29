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

# @file    racing.py
# @author  Jakob Erdmann
# @date    2017-06-12

"""
This script runs an aribtrary sumo simulation and controls the specified vehicle
via keyboard input
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import sys
import threading
import math

if sys.version_info.major == 3:
    import queue as Queue
    from tkinter import Button, Frame, Tk
else:
    import Queue
    from Tkinter import Button, Frame, Tk

try:
    sys.path.append(os.path.join(os.environ.get("SUMO_HOME",
                                                os.path.join(os.path.dirname(__file__), '..')), "tools"))
    from sumolib import checkBinary  # noqa
    import traci  # noqa
except ImportError:
    sys.exit(
        "please declare environment variable 'SUMO_HOME' as the root directory of your sumo installation " +
        "(it should contain folders 'bin', 'tools' and 'docs')")

try:
    import autopy  # noqa
except ImportError:
    sys.stderr.write("autopy not installed. Can only use keyboard control.")
    autopy = None


eventQueue = Queue.Queue()
TS = 0.05
VERBOSE = False
MAX_STEER_ANGLE = 5
MIN_SPEED = -5
MAX_OFFROAD_SPEED = 7
OFFROAD_DECEL_TIME = 2
# autopy = None # disable mouse control


def leftKey(event):
    eventQueue.put(('left', None))
    if VERBOSE:
        print("Left key pressed")


def rightKey(event):
    eventQueue.put(('right', None))
    if VERBOSE:
        print("Right key pressed")


def upKey(event):
    eventQueue.put(('up', None))
    if VERBOSE:
        print("Up key pressed")


def downKey(event):
    eventQueue.put(('down', None))
    if VERBOSE:
        print("Down key pressed")


def mouseControl(master, speed, steerAngle, accel=2.6, decel=4.5):
    try:
        centerX = master.winfo_screenwidth() / 2.0
        centerY = master.winfo_screenheight() / 2.0
        x, y = autopy.mouse.location()
        dx = (x - centerX) / centerX
        dy = (y - centerY) / centerY
        if dy < 0:
            speed -= dy * TS * accel
        else:
            speed -= dy * TS * decel
        steerAngle = MAX_STEER_ANGLE * dx
    except Exception:  # as e:
        # print(e)
        pass
    return speed, steerAngle


class RacingClient:

    """
    Launch the main part of the GUI and the worker thread. periodicCall and
    endApplication could reside in the GUI part, but putting them here
    means that you have all the thread controls in a single place.
    """

    def __init__(self, master, sumocfg, egoID):
        self.master = master
        self.sumocfg = sumocfg
        self.egoID = egoID
        self.running = True

        self.thread = threading.Thread(target=self.workerThread)
        self.thread.start()
        # Start the periodic call in the GUI to see if it can be closed
        self.periodicCall()

    def periodicCall(self):
        if not self.running:
            sys.exit(1)
        self.master.after(100, self.periodicCall)

    def workerThread(self):
        try:
            traci.start([checkBinary("sumo-gui"), "-c", self.sumocfg,
                         "--lateral-resolution", "0.32",
                         "--collision.action", "warn",
                         "--step-length", str(TS)])
            # steal focus for keyboard input after sumo-gui has loaded
            # self.master.focus_force() # not working on all platforms
            # make sure ego vehicle is loaded
            traci.simulationStep()
            speed = traci.vehicle.getSpeed(self.egoID)
            angle = traci.vehicle.getAngle(self.egoID)
            traci.vehicle.setSpeedMode(self.egoID, 0)
            steerAngle = 0
            x, y = traci.vehicle.getPosition(self.egoID)
            traci.gui.trackVehicle(traci.gui.DEFAULT_VIEW, self.egoID)
            while traci.simulation.getMinExpectedNumber() > 0:
                try:
                    if eventQueue.qsize() == 0:
                        if steerAngle > 0:
                            steerAngle = max(0, steerAngle - MAX_STEER_ANGLE * TS)
                        else:
                            steerAngle = min(0, steerAngle + MAX_STEER_ANGLE * TS)
                        # print("revert steerAngle=%.2f" % steerAngle)
                    while eventQueue.qsize():
                        try:
                            msg = eventQueue.get(0)
                            if len(msg) == 1:
                                direction = msg
                                val = None
                            else:
                                direction, val = msg

                            if direction == 'up':
                                if val is None:
                                    val = 1
                                speed += val * TS * traci.vehicle.getAccel(self.egoID)
                            if direction == 'down':
                                if val is None:
                                    val = 1
                                speed -= val * TS * traci.vehicle.getDecel(self.egoID)
                            if direction == 'left':
                                if val is None:
                                    steerAngle -= TS * 5
                                else:
                                    steerAngle = val
                            if direction == 'right':
                                if val is None:
                                    steerAngle += TS * 5
                                else:
                                    steerAngle = val
                        except Queue.Empty:
                            pass
                    if autopy:
                        speed, steerAngle = mouseControl(self.master, speed, steerAngle)
                    # move vehicle
                    # posLat = traci.vehicle.getLateralLanePosition(self.egoID)
                    if traci.vehicle.getLaneID(self.egoID) == "":
                        if abs(speed) > MAX_OFFROAD_SPEED:
                            sign = 1 if speed > 0 else -1
                            speed -= TS * sign * (abs(speed) - MAX_OFFROAD_SPEED) / OFFROAD_DECEL_TIME

                    speed = max(MIN_SPEED, min(speed, traci.vehicle.getMaxSpeed(self.egoID)))
                    steerAngle = min(MAX_STEER_ANGLE, max(-MAX_STEER_ANGLE, steerAngle))
                    angle += steerAngle
                    angle = angle % 360
                    rad = -angle / 180 * math.pi + 0.5 * math.pi
                    x2 = x + math.cos(rad) * TS * speed
                    y2 = y + math.sin(rad) * TS * speed
                    traci.vehicle.moveToXY(self.egoID, "dummy", -1, x2, y2, angle, keepRoute=2)
                    traci.vehicle.setSpeed(self.egoID, speed)
                    traci.vehicle.setLine(self.egoID, str(speed))
                    x3, y3 = traci.vehicle.getPosition(self.egoID)
                    x, y = x2, y2
                    traci.simulationStep()
                    if VERBOSE:
                        print(("old=%.2f,%.2f new=%.2f,%.2f found=%.2f,%.2f speed=%.2f steer=%.2f " +
                               "angle=%s rad/pi=%.2f cos=%.2f sin=%.2f") % (
                            x, y, x2, y2, x3, y3, speed, steerAngle, angle, rad / math.pi,
                            math.cos(rad), math.sin(rad)))
                except traci.TraCIException:
                    pass
            traci.close()
        except traci.FatalTraCIError:
            pass
        self.running = False


def main(sumocfg="racing/racing.sumocfg", egoID="ego"):
    root = Tk()
    root.geometry('180x100+0+0')
    frame = Frame(root)
    Button(frame, text="Click here.\nControl with arrow keys").grid(row=0)
    root.bind('<Left>', leftKey)
    root.bind('<Right>', rightKey)
    root.bind('<Up>', upKey)
    root.bind('<Down>', downKey)

    root.winfo_screenwidth()
    root.winfo_screenheight()

    frame.pack()

    RacingClient(root, sumocfg, egoID)
    root.mainloop()


if len(sys.argv) < 3:
    main(*sys.argv[1:])
else:
    print("racing.py <sumocfg> [<egoID>]")
