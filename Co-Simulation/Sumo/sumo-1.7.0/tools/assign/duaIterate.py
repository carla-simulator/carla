#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
# Copyright (C) 2008-2020 German Aerospace Center (DLR) and others.
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# https://www.eclipse.org/legal/epl-2.0/
# This Source Code may also be made available under the following Secondary
# Licenses when the conditions for such availability set forth in the Eclipse
# Public License 2.0 are satisfied: GNU General Public License, version 2
# or later which is available at
# https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
# SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later

# @file    duaIterate.py
# @author  Daniel Krajzewicz
# @author  Michael Behrisch
# @author  Jakob Erdmann
# @author  Yun-Pang Floetteroed
# @date    2008-02-13

"""
Run duarouter and sumo alternating to perform a dynamic user assignment.
Based on the Perl script dua_iterate.pl.
"""
from __future__ import print_function
from __future__ import absolute_import
import os
import sys
import subprocess
import shutil
import glob
import argparse
from datetime import datetime
from costMemory import CostMemory

sys.path.append(os.path.dirname(os.path.dirname(os.path.realpath(__file__))))
import sumolib  # noqa
from sumolib.options import get_long_option_names  # noqa


def addGenericOptions(argParser):
    # add options which are used by duaIterate and cadytsIterate
    argParser.add_argument("-w", "--disable-warnings", action="store_true", dest="noWarnings",
                           default=False, help="disables warnings")
    argParser.add_argument("-n", "--net-file", dest="net",
                           help="SUMO network (mandatory)", metavar="FILE")
    argParser.add_argument("-+", "--additional", default="", help="Additional files")
    argParser.add_argument("-b", "--begin",
                           type=float, default=0, help="Set simulation/routing begin")
    argParser.add_argument("-e", "--end",
                           type=float, help="Set simulation/routing end")
    argParser.add_argument("-R", "--route-steps", type=int, default=200, help="Set simulation route steps")
    argParser.add_argument("-a", "--aggregation",
                           type=int, default=900, help="Set main weights aggregation period")
    argParser.add_argument("-m", "--mesosim", action="store_true",
                           default=False, help="Whether mesosim shall be used")
    argParser.add_argument("-p", "--path", help="Path to binaries")
    argParser.add_argument("-y", "--absrand", action="store_true",
                           default=False, help="use current time to generate random number")
    argParser.add_argument("-I", "--nointernal-link", action="store_true", dest="internallink",
                           default=False, help="not to simulate internal link: true or false")
    argParser.add_argument("-j", "--meso-junctioncontrol", action="store_true", default=False,
                           help="Enable mesoscopic traffic light and priority junciton handling")
    argParser.add_argument("-L", "--meso-junctioncontrollimited", action="store_true", default=False,
                           help="Enable mesoscopic traffic light and priority junction handling for saturated links")
    argParser.add_argument("-q", "--meso-multiqueue", action="store_true", default=False,
                           help="Enable multiple queues at edge ends")
    argParser.add_argument("--meso-recheck", type=int, default=0,
                           help="Delay before checking whether a jam is gone. " +
                                "(higher values can lead to a big speed increase)")
    argParser.add_argument("--meso-tls-penalty", type=float,
                           help="Apply scaled time penalties when driving across tls controlled junctions")
    argParser.add_argument("--meso-minor-penalty", type=int,
                           help="Apply fixed time penalty when driving across a minor link; " +
                                "do not use together with --meso-junctioncontrollimited")
    argParser.add_argument("-Q", "--eco-measure", choices=['CO', 'CO2', 'PMx', 'HC', 'NOx', 'fuel', 'noise'],
                           help="define the applied eco measure, e.g. fuel, CO2, noise")
    argParser.add_argument("--eager-insert", action="store_true",
                           default=False, help="eager insertion tests (may slow down the sim considerably)")
    argParser.add_argument("--time-to-teleport", dest="timetoteleport", type=float, default=300,
                           help="Delay before blocked vehicles are teleported (negative value disables teleporting)")
    argParser.add_argument("--time-to-teleport.highways", dest="timetoteleport_highways", type=float, default=0,
                           help="Delay before blocked vehicles are teleported on wrong highway lanes")
    argParser.add_argument("--measure-vtypes", dest="measureVTypes",
                           help="Restrict edgeData measurements to the given vehicle types")
    argParser.add_argument("-7", "--zip", action="store_true",
                           default=False, help="zip old iterations using 7zip")


def initOptions():
    argParser = argparse.ArgumentParser(
        description=""" Any options of the form sumo--long-option-name will be passed to sumo.
        These must be given after all the other options
        example: sumo--step-length 0.5 will add the option --step-length 0.5 to sumo.""",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    addGenericOptions(argParser)

    argParser.add_argument("-C", "--continue-on-unbuild", action="store_true", dest="continueOnUnbuild",
                           default=False, help="continues on unbuild routes")
    argParser.add_argument("-t", "--trips",
                           help="trips in step 0 (either trips, flows, or routes have to be supplied)", metavar="FILE")
    argParser.add_argument("-r", "--routes",
                           help="routes in step 0 (either trips, flows, or routes have to be supplied)", metavar="FILE")
    argParser.add_argument("-F", "--flows",
                           help="flows in step 0 (either trips, flows, or routes have to be supplied)", metavar="FILE")
    argParser.add_argument("-A", "--gA",
                           type=float, default=.5, help="Sets Gawron's Alpha")
    argParser.add_argument("-B", "--gBeta",
                           type=float, default=.9, help="Sets Gawron's Beta")
    argParser.add_argument("-E", "--disable-summary", "--disable-emissions", action="store_true", dest="noSummary",
                           default=False, help="No summaries are written by the simulation")
    argParser.add_argument("-T", "--disable-tripinfos", action="store_true", dest="noTripinfo",
                           default=False, help="No tripinfos are written by the simulation")
    argParser.add_argument("--tripinfo-filter", dest="tripinfoFilter",
                           help="filter tripinfo attributes")
    argParser.add_argument("--inc-start", dest="incStart",
                           type=float, default=0, help="Start for incrementing scale")
    argParser.add_argument("--inc-max", dest="incMax",
                           type=float, default=1, help="Maximum for incrementing scale")
    argParser.add_argument("--inc-base", dest="incBase", type=int, default=-1,
                           help="Give the incrementation base. Negative values disable incremental scaling")
    argParser.add_argument("--incrementation", dest="incValue",
                           type=int, default=1, help="Give the incrementation")
    argParser.add_argument("--time-inc", dest="timeInc",
                           type=int, default=0, help="Give the time incrementation")
    argParser.add_argument("-f", "--first-step", dest="firstStep",
                           type=int, default=0, help="First DUA step")
    argParser.add_argument("-l", "--last-step", dest="lastStep",
                           type=int, default=50, help="Last DUA step")
    argParser.add_argument("--convergence-iterations", dest="convIt",
                           type=int, default=10, help="Number of iterations to use for convergence calculation")
    argParser.add_argument("--max-convergence-deviation", dest="convDev",
                           type=float, help="Maximum relative standard deviation in travel times")
    argParser.add_argument("-D", "--districts", help="use districts as sources and targets", metavar="FILE")
    argParser.add_argument("-x", "--vehroute-file",  dest="routefile",
                           choices=['None', 'routesonly', 'detailed'],
                           default='None', help="choose the format of the route file")
    argParser.add_argument("-z", "--output-lastRoute",  action="store_true", dest="lastroute",
                           default=False, help="output the last routes")
    argParser.add_argument("-K", "--keep-allroutes", action="store_true", dest="allroutes",
                           default=False, help="save routes with near zero probability")
    argParser.add_argument("--routing-algorithm", default="dijkstra", help="select the routing algorithm")
    argParser.add_argument("--max-alternatives", default=5, help="prune the number of alternatives to INT")
    argParser.add_argument("--skip-first-routing", action="store_true", dest="skipFirstRouting",
                           default=False, help="run simulation with demands before first routing")
    argParser.add_argument("--logit", action="store_true", dest="logit",
                           default=False, help="use the logit model for route choice")
    argParser.add_argument("-g", "--logitbeta", type=float, dest="logitbeta",
                           default=0.15, help="use the c-logit model for route choice; logit model when beta = 0")
    argParser.add_argument("-i", "--logitgamma", type=float, dest="logitgamma",
                           default=1., help="use the c-logit model for route choice")
    argParser.add_argument("-G", "--logittheta", type=float, dest="logittheta",
                           help="parameter to adapt the cost unit")
    argParser.add_argument("-J", "--addweights", dest="addweights",
                           help="Additional weightes for duarouter")
    argParser.add_argument("--router-verbose", action="store_true",
                           default=False, help="let duarouter print some statistics")
    argParser.add_argument("-M", "--external-gawron", action="store_true", dest="externalgawron",
                           default=False, help="use the external gawron calculation")
    argParser.add_argument("-N", "--calculate-oldprob", action="store_true", dest="caloldprob",
                           default=False, help="calculate the old route probabilities with the free-flow " +
                                               "travel times when using the external gawron calculation")
    argParser.add_argument("--weight-memory", action="store_true", default=False, dest="weightmemory",
                           help="smooth edge weights across iterations")
    argParser.add_argument("--pessimism", default=1, type=float,
                           help="give traffic jams a higher weight when using option --weight-memory")
    argParser.add_argument("--clean-alt", action="store_true", dest="clean_alt",
                           default=False, help="Whether old rou.alt.xml files shall be removed")
    argParser.add_argument("--binary", dest="gzip", action="store_true", default=False,
                           help="alias for --gzip")
    argParser.add_argument("--gzip", action="store_true", default=False,
                           help="writing intermediate and resulting route files in gzipped format")
    argParser.add_argument("--dualog", default="dua.log", help="log file path (default 'dua.log')")
    argParser.add_argument("--log", default="stdout.log", help="log file path (default 'dua.log')")
    argParser.add_argument("remaining_args", nargs='*')
    return argParser


def call(command, log):
    command = [str(c) for c in command]
    print("-" * 79, file=log)
    print(command, file=log)
    log.flush()
    retCode = subprocess.call(command, stdout=log, stderr=log)
    if retCode != 0:
        print(("Execution of %s failed. Look into %s for details.") %
              (command, log.name), file=sys.stderr)
        sys.exit(retCode)


def prepend_relative(prefix, path):
    if os.path.isabs(path):
        return path
    else:
        return "%s/%s" % (prefix, path)


def writeRouteConf(duarouterBinary, step, options, dua_args, file,
                   output, routesInfo):
    filename = os.path.basename(file)
    filename = filename.split('.')[0]
    cfgname = "%s/iteration_%03i_%s.duarcfg" % (step, step, filename)
    args = [
        '--net-file', prepend_relative("..", options.net),
        '--route-files', prepend_relative("..", file),
        '--output-file', output,
        '--exit-times', str(routesInfo == "detailed"),
        '--ignore-errors', str(options.continueOnUnbuild),
        '--with-taz', str(options.districts is not None),
        '--gawron.beta', str(options.gBeta),
        '--gawron.a', str(options.gA),
        '--keep-all-routes', str(options.allroutes),
        '--routing-algorithm', options.routing_algorithm,
        '--max-alternatives', str(options.max_alternatives),
        '--weights.expand',
        '--logit', str(options.logit),
        '--logit.beta', str(options.logitbeta),
        '--logit.gamma', str(options.logitgamma),
        '--random', str(options.absrand),
        '--begin', str(options.begin),
        '--verbose', str(options.router_verbose),
        '--no-step-log',
        '--no-warnings', str(options.noWarnings),
    ]
    if options.districts:
        args += ['--additional-files', options.districts]
    if step > 0:
        args += ['--weight-files', os.path.join('..', str(step-1), get_weightfilename(options, step - 1, "dump"))]
    if options.eco_measure:
        args += ['--weight-attribute', options.eco_measure]
    if 'CH' in options.routing_algorithm:
        args += ['--weight-period', str(options.aggregation)]
    if options.logittheta:
        args += ['--logit.theta', str(options.logittheta)]
    if options.end:
        args += ['--end', str(options.end)]

    args += ["--save-configuration", cfgname]

    subprocess.call([duarouterBinary] + args + dua_args)
    return cfgname


def get_scale(options, step):
    # compute scaling factor for simulation
    # using incValue = 1 (default) and incBase = 10 would produce
    # iterations with increasing scale 0.1, 0.2, ... 0.9, 1, 1, 1, ...
    if options.incBase > 0:
        return min(options.incStart + options.incValue * float(step + 1) / options.incBase, options.incMax)
    else:
        return options.incMax


def get_dumpfilename(options, step, prefix):
    # the file to which edge costs (traveltimes) are written
    return "%s_%03i_%s.xml" % (prefix, step, options.aggregation)


def get_weightfilename(options, step, prefix):
    # the file from which edge costs are loaded
    # this defaults to the dumpfile writen by the simulation but may be
    # different if one of the options --addweights, --memory-weights or
    # --cost-modifier is used
    if options.addweights:
        prefix = "%s,%s" % (options.addweights, prefix)
    if options.weightmemory:
        prefix = "memory_" + prefix
    return get_dumpfilename(options, step, prefix)


def writeSUMOConf(sumoBinary, step, options, additional_args, route_files):
    detectorfile = "dua_dump_%03i.add.xml" % step
    add = [detectorfile]
    if options.additional != '':
        add += [prepend_relative("..", f) for f in options.additional.split(',')]

    sumoCmd = [sumoBinary,
               '--save-configuration', "%s/iteration_%03i.sumocfg" % (step, step),
               '--log', "iteration_%03i.sumo.log" % step,
               '--net-file', prepend_relative("..", options.net),
               '--route-files', route_files,
               '--additional-files', ",".join(add),
               '--no-step-log',
               '--random', options.absrand,
               '--begin', options.begin,
               '--route-steps', options.route_steps,
               '--no-internal-links', options.internallink,
               '--eager-insert', options.eager_insert,
               '--time-to-teleport', options.timetoteleport,
               '--time-to-teleport.highways', options.timetoteleport_highways,
               '--verbose',
               '--no-warnings', options.noWarnings,
               ] + additional_args

    if hasattr(options, "noSummary") and not options.noSummary:
        sumoCmd += ['--summary-output', "summary_%03i.xml" % step]
    if hasattr(options, "noTripinfo") and not options.noTripinfo:
        sumoCmd += ['--tripinfo-output', "tripinfo_%03i.xml" % step]
        if options.eco_measure:
            sumoCmd += ['--device.hbefa.probability', '1']
    if hasattr(options, "routefile"):
        if options.routefile == "routesonly":
            sumoCmd += ['--vehroute-output', "vehroute_%03i.xml" % step,
                        '--vehroute-output.route-length']
        elif options.routefile == "detailed":
            sumoCmd += ['--vehroute-output', "vehroute_%03i.xml" % step,
                        '--vehroute-output.route-length',
                        '--vehroute-output.exit-times']
    if hasattr(options, "lastroute") and options.lastroute:
        sumoCmd += ['--vehroute-output.last-route', options.lastroute]
    if hasattr(options, "timeInc") and options.timeInc:
        sumoCmd += ['--end', int(options.timeInc * (step + 1))]
    elif options.end:
        sumoCmd += ['--end', options.end]

    if hasattr(options, "incBase") and options.incBase > 0:
        sumoCmd += ['--scale', get_scale(options, step)]
    if options.mesosim:
        sumoCmd += ['--mesosim',
                    '--meso-recheck', options.meso_recheck]
        if options.meso_multiqueue:
            sumoCmd += ['--meso-multi-queue']
        if options.meso_junctioncontrol:
            sumoCmd += ['--meso-junction-control']
        if options.meso_junctioncontrollimited:
            sumoCmd += ['--meso-junction-control.limited']
        if options.meso_tls_penalty:
            sumoCmd += ['--meso-tls-penalty', options.meso_tls_penalty]
        if options.meso_minor_penalty:
            sumoCmd += ['--meso-minor-penalty', options.meso_minor_penalty]

    # make sure all arguments are strings
    sumoCmd = list(map(str, sumoCmd))
    # use sumoBinary to write a config file
    subprocess.call(sumoCmd, stdout=subprocess.PIPE)

    # write detectorfile
    with open(os.path.join(str(step), detectorfile), 'w') as fd:
        vTypes = ' vTypes="%s"' % ' '.join(options.measureVTypes.split(',')) if options.measureVTypes else ""
        suffix = "_%03i_%s" % (step, options.aggregation)
        print("<a>", file=fd)
        print('    <edgeData id="dump%s" freq="%s" file="%s" excludeEmpty="true" minSamples="1"%s/>' % (
            suffix, options.aggregation, get_dumpfilename(options, step, "dump"), vTypes), file=fd)
        if options.eco_measure:
            print(('    <edgeData id="eco%s" type="hbefa" freq="%s" file="dump%s.xml" ' +
                   'excludeEmpty="true" minSamples="1"%s/>') %
                  (suffix, options.aggregation, suffix, vTypes), file=fd)
        print("</a>", file=fd)


def filterTripinfo(step, attrs):
    attrs.add("id")
    inFile = "%s%stripinfo_%03i.xml" % (step, os.sep, step)
    if os.path.exists(inFile):
        out = open(inFile + ".filtered", 'w')
        print("<tripinfos>", file=out)
        hadOutput = False
        for line in open(inFile):
            if "<tripinfo " in line:
                if hadOutput:
                    print("/>", file=out)
                print("    <tripinfo", end=' ', file=out)
                for a in attrs:
                    pos = line.find(a)
                    if pos >= 0:
                        pos += len(a) + 2
                        print(
                            '%s="%s"' % (a, line[pos:line.find('"', pos)]), end=' ', file=out)
                hadOutput = True
            if "<emission" in line:
                for a in attrs:
                    pos = line.find(a)
                    if pos >= 0:
                        pos += len(a) + 2
                        print(
                            '%s="%s"' % (a, line[pos:line.find('"', pos)]), end=' ', file=out)
        if hadOutput:
            print("/>", file=out)
        print("</tripinfos>", file=out)
        out.close()
        os.remove(inFile)
        os.rename(out.name, inFile)


def assign_remaining_args(application, prefix, args):
    # assign remaining args [ prefix--o1 a1 prefix--o2 prefix--o3 a3  ...]
    # only handles long options!
    assigned = []
    # split into options and arguments
    items = []
    item = None
    for arg in args:
        if "--" in arg:
            if item is not None:
                items.append(item)
            item = [arg]
        else:
            if item is None:
                sys.exit(
                    'Encounted argument "%s" without a preceeding option' % arg)
            item.append(arg)
    if item is not None:
        items.append(item)

    # assign to programs
    valid_options = set(get_long_option_names(application))
    for item in items:
        prefixed = item[0]
        if prefixed[0:len(prefix)] == prefix:
            option = prefixed[len(prefix):]
            if option in valid_options:
                assigned.append(option)
                assigned += item[1:]
            else:
                sys.exit('"%s" is not a valid option for "%s"' %
                         (option, application))

    return assigned


def get_basename(demand_file):
    basename = os.path.basename(demand_file)
    if 'alt' in basename:
        return basename[:-12]
    elif 'trips' in basename:
        return basename[:-10]
    else:
        return basename[:basename.find(".")]


def main(args=None):
    argParser = initOptions()

    options = argParser.parse_args(args=args)

    if not options.net:
        argParser.error("Option --net-file is mandatory")
    if (not options.trips and not options.routes and not options.flows) or (options.trips and options.routes):
        argParser.error(
            "Either --trips, --flows, or --routes have to be given!")
    duaBinary = sumolib.checkBinary("duarouter", options.path)
    sumoBinary = sumolib.checkBinary("sumo", options.path)
    if options.addweights and options.weightmemory:
        argParser.error(
            "Options --addweights and --weight-memory are mutually exclusive.")

    # make sure BOTH binaries are callable before we start
    try:
        subprocess.call(duaBinary, stdout=subprocess.PIPE)
    except OSError:
        sys.exit(
            ("Error: Could not locate duarouter (%s).\nMake sure its on the search path or set environment " +
             "variable DUAROUTER_BINARY\n") % duaBinary)
    try:
        subprocess.call(sumoBinary, stdout=subprocess.PIPE)
    except OSError:
        sys.exit(
            ("Error: Could not locate sumo (%s).\nMake sure its on the search path or set environment " +
             "variable SUMO_BINARY\n") % sumoBinary)

    sumo_args = assign_remaining_args(
        sumoBinary, 'sumo', options.remaining_args)
    dua_args = assign_remaining_args(
        duaBinary, 'duarouter', options.remaining_args)
    index = -1
    for i in range(len(dua_args)):
        if dua_args[i] == '--additional-files':
            index = i
    if index > -1:
        dua_args[index+1] = ','.join([prepend_relative("..", f) for f in dua_args[index+1].split(',')])
    sys.stdout = sumolib.TeeFile(sys.stdout, open(options.log, "w+"))
    log = open(options.dualog, "w+")
    if options.zip:
        if options.clean_alt:
            sys.exit(
                "Error: Please use either --zip or --clean-alt but not both.")
        try:
            subprocess.call("7z", stdout=open(os.devnull, 'wb'))
        except Exception:
            sys.exit(
                "Error: Could not locate 7z, please make sure its on the search path.")
        zipProcesses = {}
        zipLog = open("7zip.log", "w+")
    starttime = datetime.now()
    if options.trips:
        input_demands = options.trips.split(",")
    elif options.flows:
        input_demands = options.flows.split(",")
    else:
        input_demands = options.routes.split(",")
    if options.externalgawron:
        # avoid dependency on numpy for normal duaIterate
        from routeChoices import getRouteChoices, calFirstRouteProbs
        print('use externalgawron')
        edgesMap = {}
    if options.weightmemory:
        costmemory = CostMemory('traveltime', pessimism=options.pessimism, network_file=options.net
                                )
    routesSuffix = ".xml"
    if options.gzip:
        routesSuffix = ".gz"

    if options.weightmemory and options.firstStep != 0:
        # load previous dump files when continuing a run
        print(">> Reassembling cost-memory from previous iteration steps")
        for step in range(0, options.firstStep):
            dumpfile = get_dumpfilename(options, step, "dump")
            print(">>> Loading %s" % dumpfile)
            costmemory.load_costs(dumpfile, step, get_scale(options, step))

    avgTT = sumolib.miscutils.Statistics()
    for step in range(options.firstStep, options.lastStep):
        current_directory = os.getcwd()
        final_directory = os.path.join(current_directory, str(step))
        if not os.path.exists(final_directory):
            os.makedirs(final_directory)
        btimeA = datetime.now()
        print("> Executing step %s" % step)

        router_demands = input_demands
        simulation_demands = input_demands
        # demand files have regular names based on the basename and the step
        if not (options.skipFirstRouting and step == 0):
            simulation_demands = [
                get_basename(f) + "_%03i.rou%s" % (step, routesSuffix) for f in input_demands]
        if not ((options.skipFirstRouting and step == 1) or step == 0):
            router_demands = [str(step-1) + os.sep + get_basename(
                f) + "_%03i.rou.alt%s" % (step - 1, routesSuffix) for f in input_demands]

        if not (options.skipFirstRouting and step == options.firstStep):
            # call duarouter
            for router_input, output in zip(router_demands, simulation_demands):
                print(">> Running router on %s" % router_input)
                btime = datetime.now()
                print(">>> Begin time: %s" % btime)
                cfgname = writeRouteConf(duaBinary, step, options, dua_args, router_input,
                                         output, options.routefile)
                log.flush()
                sys.stdout.flush()
                call([duaBinary, "-c", cfgname], log)
                if options.clean_alt and router_input not in input_demands:
                    os.remove(router_input)
                etime = datetime.now()
                print(">>> End time: %s" % etime)
                print(">>> Duration: %s" % (etime - btime))
                print("<<")
                # use the external gawron
                if options.externalgawron:
                    basename = get_basename(router_input)
                    if ((step > 0 and not options.skipFirstRouting) or step > 1):
                        basename = basename[:-4]
                    print('basename', basename)
                    ecomeasure = None
                    if options.eco_measure:
                        ecomeasure = options.eco_measure
                    if step == options.firstStep + 1 and options.skipFirstRouting:
                        if options.caloldprob:
                            calFirstRouteProbs("dump_000_%s.xml" % (
                                options.aggregation), basename + "_001.rou.alt.xml", options.addweights,
                                ecomeasure)
                        else:
                            shutil.copy(str(1) + os.sep +
                                        basename + "_001.rou.alt.xml", ".." + os.sep + basename + "_001.rou.galt.xml")
                            shutil.copy(str(step) + os.sep +
                                        basename + "_001.rou.xml", str(step) + os.sep + basename + "_001.grou.xml")
                    if step == options.firstStep and not options.skipFirstRouting:
                        shutil.copy(str(step) + os.sep +
                                    basename + "_000.rou.alt.xml", ".." + os.sep + basename + "_000.rou.galt.xml")
                        shutil.copy(str(step) + os.sep +
                                    basename + "_000.rou.xml", ".." + os.sep + basename + "_000.grou.xml")
                    else:
                        print('step:', step)
                        print('get externalgawron')
                        dumpfile = str(step-1)+os.sep+"dump_%03i_%s.xml" % (
                            step - 1, options.aggregation)
                        shutil.copy(str(step-1) + os.sep + basename + "_00%s.rou.alt.xml" % (step-1),
                                    basename + "_00%s.rou.galt.xml" % (step-1))
                        if (not options.skipFirstRouting) or (options.skipFirstRouting and step > 1):
                            output, edgesMap = getRouteChoices(
                                edgesMap, dumpfile, str(step)+os.sep + basename +
                                "_%03i.rou.alt.xml" % step, options.net,
                                options.addweights, options.gA, options.gBeta, step, ecomeasure)

        # simulation
        print(">> Running simulation")
        btime = datetime.now()
        print(">>> Begin time: %s" % btime)
        writeSUMOConf(sumoBinary, step, options, sumo_args,
                      ",".join(simulation_demands))  # todo: change 'grou.xml'
        log.flush()
        sys.stdout.flush()
        call([sumoBinary, "-c", "%s%siteration_%03i.sumocfg" % (step, os.sep, step)], log)
        if options.tripinfoFilter:
            filterTripinfo(step, set(options.tripinfoFilter.split(",")))
        etime = datetime.now()
        print(">>> End time: %s" % etime)
        print(">>> Duration: %s" % (etime - btime))
        print("<<")

        if options.weightmemory:
            print(">> Smoothing edge weights")
            costmemory.load_costs(
                str(step) + os.sep + get_dumpfilename(options, step, "dump"), step, get_scale(options, step))
            costmemory.write_costs(str(step) + os.sep + get_weightfilename(options, step, "dump"))
            print(">>> Updated %s edges" % costmemory.loaded())
            print(">>> Decayed %s unseen edges" % costmemory.decayed())
            print(">>> Error avg:%.12g mean:%.12g" %
                  (costmemory.avg_error(), costmemory.mean_error()))
            print(">>> Absolute Error avg:%.12g mean:%.12g" %
                  (costmemory.avg_abs_error(), costmemory.mean_abs_error()))

        if options.zip and step - options.firstStep > 1:
            # this is a little hackish since we zip and remove all files by glob, which may have undesired side effects
            # also note that the 7z file does not have an "_" before the
            # iteration number in order to be not picked up by the remove
            for s in list(zipProcesses.keys()):
                if zipProcesses[s].poll() is not None:
                    for f in glob.glob("*_%03i*" % s):
                        try:
                            os.remove(f)
                        except Exception:
                            print("Could not remove %s" % f, file=zipLog)
                    del zipProcesses[s]
            zipStep = step - 2
            zipProcesses[zipStep] = subprocess.Popen(
                ["7z", "a", "iteration%03i.7z" % zipStep] + glob.glob("*_%03i*" % zipStep), stdout=zipLog,
                stderr=zipLog)

        converged = False
        if options.convDev:
            sum = 0.
            count = 0
            for t in sumolib.output.parse_fast(str(step) + os.sep + "tripinfo_%03i.xml" % step,
                                               'tripinfo', ['duration']):
                sum += float(t.duration)
                count += 1
            avgTT.add(sum / count)
            relStdDev = avgTT.relStdDev(options.convIt)
            print("< relative travel time deviation in the last %s steps: %.05f" % (
                min(avgTT.count(), options.convIt), relStdDev))
            if avgTT.count() >= options.convIt and relStdDev < options.convDev:
                converged = True

        print("< Step %s ended (duration: %s)" %
              (step, datetime.now() - btimeA))
        print("------------------\n")

        log.flush()
        sys.stdout.flush()
        if converged:
            break
    if options.zip:
        for s in zipProcesses.keys():
            zipProcesses[s].wait()
            for f in glob.glob("*_%03i*" % s):
                try:
                    os.remove(f)
                except Exception:
                    print("Could not remove %s" % f, file=zipLog)
        zipLog.close()
    print("dua-iterate ended (duration: %s)" % (datetime.now() - starttime))

    log.close()


if __name__ == "__main__":
    main()
