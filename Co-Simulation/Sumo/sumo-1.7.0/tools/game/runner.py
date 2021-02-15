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

# @file    runner.py
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @date    2010-01-30

"""
This script runs the gaming GUI for the LNdW traffic light game.
It checks for possible scenarios in the current working directory
and lets the user start them as a game. Furthermore it
saves highscores to local disc and to the central highscore server.
"""
from __future__ import absolute_import
from __future__ import print_function
import os
import subprocess
import sys
import re
import pickle
import glob
try:
    import Tkinter
except ImportError:
    import tkinter as Tkinter
from optparse import OptionParser
from xml.dom import pulldom
from collections import defaultdict

SUMO_HOME = os.environ.get('SUMO_HOME',
                           os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))
sys.path.append(os.path.join(SUMO_HOME, 'tools'))
import sumolib  # noqa


_UPLOAD = False if "noupload" in sys.argv else True
_SCOREFILE = "scores.pkl"
if _UPLOAD:
    _TIMEOUT = 5
    _SCORESERVER = "sumo.dlr.de"
    _SCORESCRIPT = "/scores.php?game=TLS&"
_DEBUG = True if "debug" in sys.argv else False
_SCORES = 30

_LANGUAGE_EN = {'title': 'Interactive Traffic Light',
                'cross': 'Simple Junction',
                'cross_demo': 'Simple Junction (Demo)',
                'square': 'Four Junctions',
                'grid6': 'Six Junctions',
                'kuehne': 'Prof. Kühne',
                'bs3d': '3D Junction Virtual World',
                'bs3Dosm': '3D Junction OpenStreetMap',
                'ramp': 'Highway Ramp',
                'corridor': 'Corridor',
                'A10KW': 'Highway Ramp A10',
                'DRT': 'Demand Responsive Transport (new)',
                'DRT2': 'DRT - Advanced (new)',
                'DRT_demo': 'DRT - Demo',
                'high': 'Highscore',
                'reset': 'Reset Highscore',
                'lang': 'Deutsch',
                'quit': 'Quit',
                'Highscore': 'Highscore',
                'Congratulations': 'Congratulations!',
                'your score': 'Your Score',
                'Continue': 'Continue',
                }
_LANGUAGE_DE = {'title': 'Interaktives Ampelspiel',
                'cross': 'Einfache Kreuzung',
                'cross_demo': 'Einfache Kreuzung (Demo)',
                'square': 'Vier Kreuzungen',
                'grid6': 'Sechs Kreuzungen',
                'kuehne': 'Prof. Kühne',
                'bs3d': '3D Forschungskreuzung Virtuelle Welt',
                'bs3Dosm': '3D Forschungskreuzung OpenStreetMap',
                'ramp': 'Autobahnauffahrt',
                'A10KW': 'A10 KW',
                'DRT': 'Bedarfsbus (neu)',
                'DRT2': 'Bedarfsbus für Fortgeschrittene (neu)',
                'DRT_demo': 'Bedarfsbus - Demo',
                'corridor': 'Strecke',
                'high': 'Bestenliste',
                'reset': 'Bestenliste zurücksetzen',
                'lang': 'Englisch',
                'quit': 'Beenden',
                'Highscore': 'Bestenliste',
                'Congratulations': 'Gratulation!',
                'your score': 'Deine Punkte',
                'Continue': 'Weiter',
                }


def printDebug(*args):
    if _DEBUG:
        print("DEBUG:", end=" ")
        for message in args:
            print(message, end=" ")
        print()


if _UPLOAD:
    printDebug("import httplib...")
    try:
        import httplib  # noqa
        printDebug("SUCCESS")
    except ImportError:
        printDebug("FAILED - disabling upload...")
        _UPLOAD = False
if _UPLOAD:
    print("Highscore upload is enabled. To disable call this script with 'noupload' argument.")
else:
    print("Upload is disabled.")


def computeScoreFromWaitingTime(gamename):
    totalDistance = 0
    totalFuel = 0
    totalArrived = 0
    totalWaitingTime = 0
    complete = True
    for line in open(os.path.join(base, "%s.netstate.xml" % start.category)):
        m = re.search('<interval begin="0(.00)?" end="([^"]*)"', line)
        if m and float(m.group(2)) != start.gametime:
            print("error: incomplete output")
            complete = False
        m = re.search('sampledSeconds="([^"]*)".*speed="([^"]*)"', line)
        if m:
            totalDistance += float(m.group(1)) * float(m.group(2))
        m = re.search('fuel_abs="([^"]*)"', line)
        if m:
            totalFuel += float(m.group(1))
        m = re.search('arrived="([^"]*)"', line)
        if m:
            totalArrived += float(m.group(1))
        m = re.search('waitingTime="([^"]*)"', line)
        if m:
            totalWaitingTime += float(m.group(1))
    # doing nothing gives a waitingTime of 6033 for cross and 6700 for
    # square
    score = 10000 - totalWaitingTime
    return score, totalArrived, complete


def computeScoreFromTimeLoss(gamename):
    totalArrived = 0
    timeLoss = None
    departDelay = None
    departDelayWaiting = None
    inserted = None
    running = None
    waiting = None
    completed = False

    for line in open(gamename + ".log"):
        if "Simulation ended at time" in line:
            completed = True
        m = re.search('Inserted: ([0-9]*)', line)
        if m:
            inserted = float(m.group(1))
        m = re.search('Running: (.*)', line)
        if m:
            running = float(m.group(1))
        m = re.search('Waiting: (.*)', line)
        if m:
            waiting = float(m.group(1))
        m = re.search('TimeLoss: (.*)', line)
        if m:
            timeLoss = float(m.group(1))
        m = re.search('DepartDelay: (.*)', line)
        if m:
            departDelay = float(m.group(1))
        m = re.search('DepartDelayWaiting: (.*)', line)
        if m:
            departDelayWaiting = float(m.group(1))
    if not completed or timeLoss is None:
        return 0, totalArrived, False
    else:
        totalArrived = inserted - running
        if _DEBUG:
            print("timeLoss=%s departDelay=%s departDelayWaiting=%s inserted=%s running=%s waiting=%s" % (
                timeLoss, departDelay, departDelayWaiting, inserted, running, waiting))

        score = 10000 - int(100 * ((timeLoss + departDelay) * inserted +
                                   departDelayWaiting * waiting) / (inserted + waiting))
        return score, totalArrived, True


def computeScoreDRT(gamename):
    rideWaitingTime = 0
    rideDuration = 0
    rideStarted = 0
    rideFinished = 0
    tripinfos = gamename + ".tripinfos.xml"
    rideCount = 0
    for ride in sumolib.xml.parse(tripinfos, 'ride'):
        if float(ride.waitingTime) < 0:
            if _DEBUG:
                print("negative waitingTime")
            ride.waitingTime = 10000
        rideWaitingTime += float(ride.waitingTime)
        if float(ride.duration) >= 0:
            rideDuration += float(ride.duration)
            rideStarted += 1
        if float(ride.arrival) >= 0:
            rideFinished += 1

        rideCount += 1

    if rideCount == 0:
        return 0, 0, False
    else:
        avgWT = rideWaitingTime / rideCount
        avgDur = 0 if rideStarted == 0 else rideDuration / rideStarted
        score = 5000 - int(avgWT + avgDur)
        if _DEBUG:
            print("rideWaitingTime=%s rideDuration=%s persons=%s started=%s finished=%s avgWT=%s avgDur=%s" % (
                rideWaitingTime, rideDuration, rideCount, rideStarted, rideFinished, avgWT, avgDur))
        return score, rideCount, True


def computeScoreSquare(gamename):
    rideWaitingTime = 0
    rideDuration = 0
    rideStarted = 0
    rideFinished = 0
    tripinfos = gamename + ".tripinfos.xml"
    rideCount = 0
    for ride in sumolib.xml.parse(tripinfos, 'tripinfo'):
        if float(ride.waitingTime) < 0:
            if _DEBUG:
                print("negative waitingTime")
            ride.waitingTime = 10000
        rideWaitingTime += float(ride.waitingTime)
        if ride.vType.startswith("ev"):
            rideWaitingTime += 10 * float(ride.waitingTime)
        if float(ride.duration) >= 0:
            rideDuration += float(ride.duration)
            rideStarted += 1
        if float(ride.arrival) >= 0:
            rideFinished += 1

        rideCount += 1

    if rideCount == 0:
        return 0, 0, False
    else:
        avgWT = rideWaitingTime / rideCount
        avgDur = 0 if rideStarted == 0 else rideDuration / rideStarted
        score = 1000 - int(avgWT + avgDur)
        if _DEBUG:
            print("rideWaitingTime=%s rideDuration=%s persons=%s started=%s finished=%s avgWT=%s avgDur=%s" % (
                rideWaitingTime, rideDuration, rideCount, rideStarted, rideFinished, avgWT, avgDur))
        return score, rideCount, True


_SCORING_FUNCTION = defaultdict(lambda: computeScoreFromWaitingTime)
_SCORING_FUNCTION.update({
    'A10KW': computeScoreFromTimeLoss,
    'DRT': computeScoreDRT,
    'DRT2': computeScoreDRT,
    'DRT_demo': computeScoreDRT,
    'square': computeScoreSquare,
})


def loadHighscore():
    if _UPLOAD:
        printDebug("try to load highscore from scoreserver...")
        try:
            conn = httplib.HTTPConnection(_SCORESERVER, timeout=_TIMEOUT)
            conn.request("GET", _SCORESCRIPT + "top=" + str(_SCORES))
            response = conn.getresponse()
            if response.status == httplib.OK:
                scores = {}
                for line in response.read().splitlines():
                    category, values = line.split()
                    scores[category] = _SCORES * [("", "", -1.)]
                    for idx, item in enumerate(values.split(':')):
                        name, game, score = item.split(',')
                        scores[category][idx] = (name, game, int(float(score)))
                printDebug("SUCCESS")
                return scores
        except Exception:
            printDebug("FAILED")

    try:
        return pickle.load(open(_SCOREFILE))
    except Exception:
        pass
    return {}


def parseEndTime(cfg):
    cfg_doc = pulldom.parse(cfg)
    for event, parsenode in cfg_doc:
        if event == pulldom.START_ELEMENT and parsenode.localName == 'end':
            return float(parsenode.getAttribute('value'))


class IMAGE:
    pass


class StartDialog(Tkinter.Frame):

    def __init__(self, parent, lang):
        Tkinter.Frame.__init__(self, parent)
        # variables for changing language
        self.parent = parent
        self._language_text = lang
        self.buttons = []
        # misc variables
        self.name = ''
        # setup gui
        self.parent.title(self._language_text['title'])
        self.parent.minsize(250, 50)
        self.category = None

        # we use a grid layout with 4 columns
        COL_DLRLOGO, COL_START, COL_HIGH, COL_SUMOLOGO = range(4)
        # there is one column for every config, +2 more columns for control
        # buttons
        configs = sorted(glob.glob(os.path.join(base, "*.sumocfg")))
        numButtons = len(configs) + 3
        # button dimensions
        bWidth_start = 30
        bWidth_high = 10
        bWidth_control = 41

        self.gametime = 0
        self.ret = 0
        # some pretty images
        Tkinter.Label(self, image=IMAGE.dlrLogo).grid(
            row=0, rowspan=numButtons, column=COL_DLRLOGO)
        Tkinter.Label(self, image=IMAGE.sumoLogo).grid(
            row=0, rowspan=numButtons, column=COL_SUMOLOGO)

        # 2 button for each config (start, highscore)
        for row, cfg in enumerate(configs):
            if "bs3" in cfg and not haveOSG:
                continue
            category = self.category_name(cfg)
            # lambda must make a copy of cfg argument
            button = Tkinter.Button(self, width=bWidth_start,
                                    command=lambda cfg=cfg: self.start_cfg(cfg))
            self.addButton(button, category)
            button.grid(row=row, column=COL_START)

            button = Tkinter.Button(self, width=bWidth_high,
                                    command=lambda cfg=cfg: ScoreDialog(self, [], None, self.category_name(cfg),
                                                                        self._language_text)
                                    )  # .grid(row=row, column=COL_HIGH)
            self.addButton(button, 'high')
            button.grid(row=row, column=COL_HIGH)

        # control buttons
        button = Tkinter.Button(
            self, width=bWidth_control, command=high.clear)
        self.addButton(button, 'reset')
        button.grid(row=numButtons - 3, column=COL_START, columnspan=2)

        button = Tkinter.Button(
            self, width=bWidth_control, command=sys.exit)
        self.addButton(button, 'quit')
        button.grid(row=numButtons - 1, column=COL_START, columnspan=2)

        button = Tkinter.Button(
            self, width=bWidth_control, command=lambda: self.change_language())
        self.addButton(button, 'lang')
        button.grid(row=numButtons - 2, column=COL_START, columnspan=2)

        self.grid()
        # The following three commands are needed so the window pops
        # up on top on Windows...
        self.parent.iconify()
        self.parent.update()
        self.parent.deiconify()

    def addButton(self, button, text):
        button["text"] = self._language_text.get(text, text)
        self.buttons.append((text, button))

    def change_language(self):
        if self._language_text == _LANGUAGE_DE:
            self._language_text = _LANGUAGE_EN
        else:
            self._language_text = _LANGUAGE_DE
        for text, button in self.buttons:
            button["text"] = self._language_text[text]

    def category_name(self, cfg):
        return os.path.basename(cfg)[:-8]

    def start_cfg(self, cfg):
        # remember which which cfg was launched
        self.category = self.category_name(cfg)
        if _DEBUG:
            print("starting", cfg)
        self.gametime = parseEndTime(cfg)
        self.ret = subprocess.call(
            [guisimPath, "-S", "-G", "-Q", "-c", cfg, '-l', 'log',
                '--output-prefix', "%s." % self.category,
                '--duration-log.statistics',
                '--tripinfo-output.write-unfinished'], stderr=sys.stderr)

        if _DEBUG:
            print("ended", cfg)

        # compute score
        score, totalArrived, complete = _SCORING_FUNCTION[self.category](self.category)

        # parse switches
        switch = []
        lastProg = {}
        tlsfile = os.path.join(base, "%s.tlsstate.xml" % start.category)
        if os.path.exists(tlsfile):
            for line in open(tlsfile):
                m = re.search(r'tlsstate time="(\d+(.\d+)?)" id="([^"]*)" programID="([^"]*)"', line)
                if m:
                    tls = m.group(3)
                    program = m.group(4)
                    if tls not in lastProg or lastProg[tls] != program:
                        lastProg[tls] = program
                        switch += [m.group(3), m.group(1)]

        lang = start._language_text
        if _DEBUG:
            print(switch, score, totalArrived, complete)
        if complete:
            ScoreDialog(self, switch, score, self.category, lang)

        # if ret != 0:
        # quit on error
        #    sys.exit(start.ret)


class ScoreDialog:

    def __init__(self, parent, switch, score, category, lang):
        self.root = Tkinter.Toplevel(parent)
        # self.root.transient(parent)
        self.name = None
        self.switch = switch
        self.score = score
        self.category = category
        self.root.title(lang["Highscore"])
        self.root.minsize(250, 50)
        haveHigh = False

        if category not in high:
            high[category] = _SCORES * [("", "", -1.)]
        idx = 0
        for n, g, p in high[category]:
            if not haveHigh and p < score:
                Tkinter.Label(
                    self.root, text=(str(idx + 1) + '. ')).grid(row=idx)
                self.name = Tkinter.Entry(self.root)
                self.name.grid(row=idx, sticky=Tkinter.W, column=1)
                self.scoreLabel = Tkinter.Label(self.root, text=str(score),
                                                bg="pale green").grid(row=idx, column=2)
                self.idx = idx
                haveHigh = True
                self.root.title(lang["Congratulations"])
                idx += 1
            if p == -1 or idx == _SCORES:
                break
            Tkinter.Label(self.root, text=(str(idx + 1) + '. ')).grid(row=idx)
            Tkinter.Label(self.root, text=n, padx=5).grid(
                row=idx, sticky=Tkinter.W, column=1)
            Tkinter.Label(self.root, text=str(p)).grid(row=idx, column=2)
            idx += 1
        if not haveHigh:
            if score is not None:  # not called from the main menue
                Tkinter.Label(self.root, text=lang['your score'], padx=5,
                              bg="indian red").grid(row=idx, sticky=Tkinter.W, column=1)
                Tkinter.Label(self.root, text=str(score),
                              bg="indian red").grid(row=idx, column=2)
                idx += 1
        Tkinter.Button(self.root, text=lang["Continue"], command=self.save).grid(
            row=idx, column=2)

        # add QR-code for LNDW
        Tkinter.Label(self.root, image=IMAGE.qrCode).grid(
            row=1, column=3, rowspan=22)

        self.root.grid()
        self.root.bind("<Return>", self.save)
        # self.root.wait_visibility()
        # self.root.grab_set()
        if self.name:
            self.name.focus_set()
        # The following three commands are needed so the window pops
        # up on top on Windows...
        # self.root.iconify()
        # self.root.update()
        # self.root.deiconify()
        # self.root.mainloop()

    def save(self, event=None):
        if self.name and self.name.get():
            name = self.name.get()
            high[self.category].insert(
                self.idx, (name, self.switch, self.score))
            high[self.category].pop()
            self.name.destroy()
            self.name = None
            Tkinter.Label(self.root, text=name, padx=5,
                          bg="pale green").grid(row=self.idx, sticky=Tkinter.W, column=1)
            try:
                f = open(_SCOREFILE, 'w')
                pickle.dump(high, f)
                f.close()
            except Exception:
                pass

            if _UPLOAD:
                printDebug("try to upload score...")
                try:
                    conn = httplib.HTTPConnection(_SCORESERVER, timeout=_TIMEOUT)
                    conn.request("GET", _SCORESCRIPT + "category=%s&name=%s&instance=%s&points=%s" % (
                        self.category, name, "_".join(self.switch), self.score))
                    if _DEBUG:
                        r1 = conn.getresponse()
                        print(r1.status, r1.reason, r1.read())
                    printDebug("SUCCESS")
                except BaseException:
                    printDebug("FAILED")
        self.quit()

    def quit(self, event=None):
        self.root.destroy()


stereoModes = (
    'ANAGLYPHIC', 'QUAD_BUFFER', 'VERTICAL_SPLIT', 'HORIZONTAL_SPLIT')
optParser = OptionParser()
optParser.add_option("-s", "--stereo", metavar="OSG_STEREO_MODE",
                     help="Defines the stereo mode to use for 3D output; unique prefix of %s" % (
                           ", ".join(stereoModes)))
options, args = optParser.parse_args()

base = os.path.dirname(sys.argv[0])
high = loadHighscore()


guisimPath = sumolib.checkBinary("sumo-gui")
haveOSG = "OSG" in subprocess.check_output(sumolib.checkBinary("sumo"), universal_newlines=True)

if options.stereo:
    for m in stereoModes:
        if m.lower().startswith(options.stereo.lower()):
            os.environ["OSG_STEREO_MODE"] = m
            os.environ["OSG_STEREO"] = "ON"
            break

lang = _LANGUAGE_EN
if "OSG_FILE_PATH" in os.environ:
    os.environ["OSG_FILE_PATH"] += os.pathsep + \
        os.path.join(os.environ.get("SUMO_HOME", ""), "data", "3D")
else:
    os.environ["OSG_FILE_PATH"] = os.path.join(
        os.environ.get("SUMO_HOME", ""), "data", "3D")

root = Tkinter.Tk()
IMAGE.dlrLogo = Tkinter.PhotoImage(file='dlr.gif')
IMAGE.sumoLogo = Tkinter.PhotoImage(file='sumo_logo.gif')
IMAGE.qrCode = Tkinter.PhotoImage(file='qr_sumo.dlr.de.gif')
start = StartDialog(root, lang)
root.mainloop()
