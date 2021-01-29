#!/usr/bin/env python3
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

# @file    gtfs2fcd.py
# @author  Michael Behrisch
# @author  Robert Hilbrich
# @date    2018-06-13

"""
Converts GTFS data into separate fcd traces for every distinct trip
"""

from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import pandas as pd
import zipfile

sys.path.append(os.path.join(os.environ["SUMO_HOME"], "tools"))
import sumolib  # noqa
import traceExporter  # noqa


def add_options():
    argParser = sumolib.options.ArgumentParser()
    argParser.add_argument("-r", "--region", help="define the region to process")
    argParser.add_argument("--gtfs", help="define gtfs zip file to load (mandatory)")
    argParser.add_argument("--date", help="define the day to import, format: 'YYYYMMDD'")
    argParser.add_argument("--fcd", help="directory to write / read the generated FCD files to / from")
    argParser.add_argument("--gpsdat", help="directory to write / read the generated gpsdat files to / from")
    argParser.add_argument("--vtype-output", help="file to write the generated vehicle types to")
    argParser.add_argument("--verbose", action="store_true", default=False, help="tell me what you are doing")
    return argParser


def check_options(options):
    if options.gtfs is None or options.date is None:
        sys.exit("Please give a GTFS file using --gtfs FILE and a date using --date YYYYMMDD.")
    if options.region is None:
        options.region = os.path.basename(options.gtfs)[:-4]
    if options.fcd is None:
        options.fcd = os.path.join('fcd', options.region)
    if options.gpsdat is None:
        options.gpsdat = os.path.join('input', options.region)
    return options


def time2sec(s):
    t = s.split(":")
    return int(t[0]) * 3600 + int(t[1]) * 60 + int(t[2])


def main(options):
    if options.verbose:
        print('Loading GTFS data "%s"' % options.gtfs)
    gtfsZip = zipfile.ZipFile(options.gtfs)
    routes = pd.read_csv(gtfsZip.open('routes.txt'), dtype=str)
    stops = pd.read_csv(gtfsZip.open('stops.txt'), dtype=str)
    stop_times = pd.read_csv(gtfsZip.open('stop_times.txt'),
                             converters={'trip_id': str, 'arrival_time': time2sec, 'departure_time': time2sec,
                                         'stop_id': str, 'stop_sequence': int})
    trips = pd.read_csv(gtfsZip.open('trips.txt'), dtype=str)
    calendar_dates = pd.read_csv(gtfsZip.open('calendar_dates.txt'), dtype=str)

    # currently not used:
    # agency = pd.read_csv(gtfsZip.open('agency.txt'), dtype=str)
    # calendar = pd.read_csv(gtfsZip.open('calendar.txt'), dtype=str)
    # transfers = pd.read_csv(gtfsZip.open('transfers.txt'), dtype=str)

    # Merging the tables
    tmp = pd.merge(trips, calendar_dates, on='service_id')
    trips_on_day = tmp[tmp['date'] == options.date]
    if 'fare_stops.txt' in gtfsZip.namelist():
        zones = pd.read_csv(gtfsZip.open('fare_stops.txt'), dtype=str)
        stops_merged = pd.merge(pd.merge(stops, stop_times, on='stop_id'), zones, on='stop_id')
    else:
        stops_merged = pd.merge(stops, stop_times, on='stop_id')
        stops_merged['fare_zone'] = ''
        stops_merged['fare_token'] = ''
        stops_merged['start_char'] = ''

    trips_routes_merged = pd.merge(trips_on_day, routes, on='route_id')
    full_data_merged = pd.merge(stops_merged,
                                trips_routes_merged,
                                on='trip_id')[['trip_id', 'route_id', 'route_short_name', 'route_type',
                                               'stop_id', 'stop_name', 'stop_lat', 'stop_lon', 'stop_sequence',
                                               'fare_zone', 'fare_token', 'start_char',
                                               'arrival_time', 'departure_time']].drop_duplicates()

    gtfs_modes = {
        # modes according to https://developers.google.com/transit/gtfs/reference/#routestxt
        '0':  'tram',
        '1':  'subway',
        '2':  'rail',
        '3':  'bus',
        '4':  'ship',
        # '5':  'cableTram',
        # '6':  'aerialLift',
        # '7':  'funicular',
        # modes used in Berlin and MDV see https://developers.google.com/transit/gtfs/reference/extended-route-types
        '100':  'rail',        # DB
        '109':  'light_rail',  # S-Bahn
        '400':  'subway',      # U-Bahn
        '700':  'bus',         # Bus
        '714':  'bus',         # SEV
        '715':  'bus',         # Rufbus
        '900':  'tram',        # Tram
        '1000': 'ship',        # Faehre
        # modes used by hafas
        's': 'light_rail',
        'RE': 'rail',
        'RB': 'rail',
        'IXB': 'rail',        # tbd
        'ICE': 'rail_electric',
        'IC': 'rail_electric',
        'IRX': 'rail',        # tbd
        'EC': 'rail',
        'NJ': 'rail',        # tbd
        'RHI': 'rail',        # tbd
        'DPN': 'rail',        # tbd
        'SCH': 'rail',        # tbd
        'Bsv': 'rail',        # tbd
        'KAT': 'rail',        # tbd
        'AIR': 'rail',        # tbd
        'DPS': 'rail',        # tbd
        'lt': 'light_rail',  # tbd
        'BUS': 'bus',        # tbd
        'Str': 'tram',        # tbd
        'DPF': 'rail',        # tbd
    }
    fcdFile = {}
    tripFile = {}
    if not os.path.exists(options.fcd):
        os.makedirs(options.fcd)
    seenModes = set()
    for mode in set(gtfs_modes.values()):
        filePrefix = os.path.join(options.fcd, mode)
        fcdFile[mode] = open(filePrefix + '.fcd.xml', 'w', encoding="utf8")
        sumolib.writeXMLHeader(fcdFile[mode], "gtfs2fcd.py")
        fcdFile[mode].write('<fcd-export>\n')
        if options.verbose:
            print('Writing fcd file "%s"' % fcdFile[mode].name)
        tripFile[mode] = open(filePrefix + '.rou.xml', 'w')
        tripFile[mode].write("<routes>\n")
    timeIndex = 0
    for _, trip_data in full_data_merged.groupby(['route_id']):
        seqs = {}
        for trip_id, data in trip_data.groupby(['trip_id']):
            stopSeq = []
            buf = ""
            offset = 0
            firstDep = None
            for __, d in data.sort_values(by=['stop_sequence']).iterrows():
                arrivalSec = d.arrival_time + timeIndex
                stopSeq.append(d.stop_id)
                departureSec = d.departure_time + timeIndex
                until = 0 if firstDep is None else departureSec - timeIndex - firstDep
                buf += (('    <timestep time="%s"><vehicle id="%s" x="%s" y="%s" until="%s" ' +
                         'name=%s fareZone="%s" fareSymbol="%s" startFare="%s" speed="20"/></timestep>\n') %
                        (arrivalSec - offset, trip_id, d.stop_lon, d.stop_lat, until,
                         sumolib.xml.quoteattr(d.stop_name), d.fare_zone, d.fare_token, d.start_char))
                if firstDep is None:
                    firstDep = departureSec - timeIndex
                offset += departureSec - arrivalSec
            mode = gtfs_modes[d.route_type]
            s = tuple(stopSeq)
            if s not in seqs:
                seqs[s] = trip_id
                fcdFile[mode].write(buf)
                timeIndex = arrivalSec
            tripFile[mode].write('    <vehicle id="%s" route="%s" type="%s" depart="%s" line="%s_%s"/>\n' %
                                 (trip_id, seqs[s], mode, firstDep, d.route_short_name, seqs[s]))
            seenModes.add(mode)
    if options.gpsdat:
        if not os.path.exists(options.gpsdat):
            os.makedirs(options.gpsdat)
        for mode in set(gtfs_modes.values()):
            fcdFile[mode].write('</fcd-export>\n')
            fcdFile[mode].close()
            tripFile[mode].write("</routes>\n")
            tripFile[mode].close()
            if mode in seenModes:
                traceExporter.main(['', '--base-date', '0', '-i', fcdFile[mode].name,
                                    '--gpsdat-output', os.path.join(options.gpsdat, "gpsdat_%s.csv" % mode)])
            else:
                os.remove(fcdFile[mode].name)
                os.remove(tripFile[mode].name)
    if options.vtype_output:
        with open(options.vtype_output, 'w', encoding="utf8") as vout:
            sumolib.xml.writeHeader(vout, os.path.basename(__file__), "additional")
            for mode in sorted(seenModes):
                vout.write('    <vType id="%s" vClass="%s"/>\n' %
                           (mode, "rail_urban" if mode in ("light_rail", "subway") else mode))
            vout.write('</additional>\n')


if __name__ == "__main__":
    main(check_options(add_options().parse_args()))
