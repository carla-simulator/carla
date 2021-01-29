#!/usr/bin/env python3

""" Complete Scenario Generator from OSM.

    Author: Lara CODECA

    This program and the accompanying materials are made available under the
    terms of the Eclipse Public License 2.0 which is available at
    http://www.eclipse.org/legal/epl-2.0.
"""

import argparse
import cProfile
import io
import logging
import os
import pstats
import shutil
import subprocess
import sys

from xml.etree import ElementTree

import generateParkingAreasFromOSM
import generateTAZBuildingsFromOSM
import generateAmitranFromTAZWeights
import generateDefaultsActivityGen
import activitygen
import sagaActivityReport

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import ptlines2flows
    import generateParkingAreaRerouters
    from visualization import plot_summary
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

def logs():
    """ Log init. """
    stdout_handler = logging.StreamHandler(sys.stdout)
    logging.basicConfig(handlers=[stdout_handler], level=logging.INFO,
                        format='[%(asctime)s] %(levelname)s: %(message)s',
                        datefmt='%m/%d/%Y %I:%M:%S %p')

def get_options(cmd_args=None):
    """ Argument Parser. """
    parser = argparse.ArgumentParser(
        prog='complete.generator.py', usage='%(prog)s [options]',
        description='Complete scenario generator from OSM to the ActivityGen.')
    parser.add_argument(
        '--osm', type=str, dest='osm_file', required=True,
        help='OSM file.')
    parser.add_argument(
        '--out', type=str, dest='out_dir', required=True,
        help='Directory for all the output files.')
    parser.add_argument(
        '--lefthand', dest='left_hand_traffic', action='store_true',
        help='Generate a left-hand traffic scenario.')
    parser.set_defaults(left_hand_traffic=False)
    parser.add_argument(
        '--population', type=int, dest='population', default=1000,
        help='Number of people plans to generate.')
    parser.add_argument(
        '--density', type=float, dest='density', default=3000.0,
        help='Average population density in square kilometers.')
    parser.add_argument(
        '--single-taz', dest='single_taz', action='store_true',
        help='Ignore administrative boundaries and generate only one TAZ.')
    parser.set_defaults(single_taz=False)
    parser.add_argument(
        '--admin-level', type=int, dest='admin_level', default=None,
        help='Select only the administrative boundaries with the given level and generate'
        ' the associated TAZs.')
    parser.add_argument(
        '--taz-plot', type=str, dest='html_filename', default='',
        help='Plots the TAZs to an HTML file as OSM overlay. (Requires folium)')
    parser.add_argument(
        '--processes', type=int, dest='processes', default=1,
        help='Number of processes spawned (when suported) to generate the scenario.')
    parser.add_argument(
        '--from-step', type=int, dest='from_step', default=0,
        help='For successive iteration of the script, it defines from which step it should start: '
             '[ 0 - Copy default files.] '
             '[ 1 - Run netconvert & polyconvert.] '
             '[ 2 - Run ptlines2flows.py.] '
             '[ 3 - Generate parking areas.] '
             '[ 4 - Generate parking area rerouters.] '
             '[ 5 - Extract TAZ from administrative boundaries.] '
             '[ 6 - Generate OD-matrix.] '
             '[ 7 - Generate SUMOActivityGen defaults.] '
             '[ 8 - Run SUMOActivityGen.] '
             '[ 9 - Launch SUMO.] '
             '[10 - Report.] ')
    parser.add_argument(
        '--profiling', dest='profiling', action='store_true',
        help='Enable Python3 cProfile feature.')
    parser.add_argument(
        '--no-profiling', dest='profiling', action='store_false',
        help='Disable Python3 cProfile feature.')
    parser.set_defaults(profiling=False)
    parser.add_argument(
        '--local-defaults', dest='local_defaults', action='store_true',
        help='Uses the default folder and files defined locally. If not enabled, uses the files '
        'contained in the sumo/tools/contributed/saga folder.')
    parser.set_defaults(local_defaults=False)
    return parser.parse_args(cmd_args)

## netconvert
DEFAULT_NETCONVERT = 'osm.netccfg'
DEFAULT_NET_XML = 'osm.net.xml'
DEFAULT_PT_STOPS_XML = 'osm_stops.add.xml'
DEFAULT_PT_LINES = 'osm_ptlines.xml'
DEFAULT_SIDE_PARKING_XML = 'osm_parking.xml'
DEFAULT_TYPE_FILES = ('{}/data/typemap/osmNetconvert.typ.xml,'
                      '{}/data/typemap/osmNetconvertBicycle.typ.xml,'
                      '{}/data/typemap/osmNetconvertPedestrians.typ.xml'.format(
                          os.environ['SUMO_HOME'], os.environ['SUMO_HOME'],
                          os.environ['SUMO_HOME']))

## ptlines2flows
DEFAULT_PT_FLOWS = 'osm_pt.rou.xml'

## generateParkingAreasFromOSM
DEFAULT_PARKING_AREAS = 'osm_parking_areas.add.xml'

## merged parking files
DEFAULT_COMPLETE_PARKING_XML = 'osm_complete_parking_areas.add.xml'

## generateParkingAreaRerouters
DEFAULT_PARKING_REROUTERS_XML = 'osm_parking_rerouters.add.xml'

## polyconvert
DEFAULT_POLY_XML = 'osm_polygons.add.xml'

## generateTAZBuildingsFromOSM
DEFAULT_TAZ_OUTPUT_XML = 'osm_taz.xml'
DEFAULT_WEIGHT_OUTPUT_CSV = 'osm_taz_weight.csv'
DEFAULT_BUILDINGS_PREFIX = 'buildings/osm_buildings'

## generateAmitranFromTAZWeights
DEFAULT_ODMATRIX_AMITRAN_XML = 'osm_odmatrix_amitran.xml'

## generateDefaultsActivityGen
DEAFULT_GENERIC_AG_CONG = 'activitygen.json'
DEAFULT_SPECIFIC_AG_CONG = 'osm_activitygen.json'

## SUMO
DEFAULT_SUMOCFG = 'osm.sumocfg'
DEFAULT_TRIPINFO_FILE = 'output.tripinfo.xml'
DEFAULT_SUMMARY_FILE = 'output.summary.xml'

## SAGA Report
DEFAULT_REPORT_FILE = 'activities_report.json'

def _call_netconvert(filename, lefthand):
    """ Call netconvert using a subprocess. """
    netconvert_options = ['netconvert',
                          '-c', DEFAULT_NETCONVERT,
                          '--osm', filename,
                          '-o', DEFAULT_NET_XML,
                          '--ptstop-output', DEFAULT_PT_STOPS_XML,
                          '--ptline-output', DEFAULT_PT_LINES,
                          '--parking-output', DEFAULT_SIDE_PARKING_XML,
                          '--type-files', DEFAULT_TYPE_FILES]
    if lefthand:
        netconvert_options.append('--lefthand')
    subprocess.check_call(netconvert_options)

def _call_pt_lines_to_flows():
    """ Call directly ptlines2flows from sumo/tools. """
    pt_flows_options = ptlines2flows.get_options(['-n', DEFAULT_NET_XML,
                                                  '-e', '86400',
                                                  '-p', '600',
                                                  '--random-begin',
                                                  '--seed', '42',
                                                  '--ptstops', DEFAULT_PT_STOPS_XML,
                                                  '--ptlines', DEFAULT_PT_LINES,
                                                  '-o', DEFAULT_PT_FLOWS,
                                                  '--ignore-errors',
                                                  '--vtype-prefix', 'pt_',
                                                  '--verbose'])
    ptlines2flows.main(pt_flows_options)

def _call_generate_parking_areas_from_osm(filename):
    """ Call directly generateParkingAreasFromOSM from SUMOActivityGen. """
    parking_options = ['--osm', filename, '--net', DEFAULT_NET_XML, '--out', DEFAULT_PARKING_AREAS]
    generateParkingAreasFromOSM.main(parking_options)

def _merge_parking_files(side_parking, parking_areas, complete_parking):
    """ Merge the two additional files containing parkings into one. """

    side_parking_struct = ElementTree.parse(side_parking).getroot()
    parking_areas_struct = ElementTree.parse(parking_areas).getroot()

    for element in parking_areas_struct:
        side_parking_struct.append(element)

    merged_parking = ElementTree.ElementTree(side_parking_struct)
    merged_parking.write(open(complete_parking, 'wb'))

def _call_generate_parking_area_rerouters(processes):
    """ Call directly generateParkingAreaRerouters from sumo/tools. """
    rerouters_options = ['-a', DEFAULT_COMPLETE_PARKING_XML,
                         '-n', DEFAULT_NET_XML,
                         '--max-number-alternatives', '10',
                         '--max-distance-alternatives', '1000.0',
                         '--min-capacity-visibility-true', '50',
                         '--max-distance-visibility-true', '1000.0',
                         '--processes', str(processes),
                         '-o', DEFAULT_PARKING_REROUTERS_XML, '--tqdm']
    generateParkingAreaRerouters.main(rerouters_options)

def _call_polyconvert(filename):
    """ Call polyconvert using a subprocess. """
    polyconvert_options = ['polyconvert',
                           '--osm', filename,
                           '--net', DEFAULT_NET_XML,
                           '-o', DEFAULT_POLY_XML]
    subprocess.check_call(polyconvert_options)

def _call_generate_taz_buildings_from_osm(filename, single_taz, processes, admin_level, plot):
    """ Call directly generateTAZBuildingsFromOSM from SUMOActivityGen. """
    taz_buildings_options = ['--osm', filename,
                             '--net', DEFAULT_NET_XML,
                             '--taz-output', DEFAULT_TAZ_OUTPUT_XML,
                             '--weight-output', DEFAULT_WEIGHT_OUTPUT_CSV,
                             '--processes', str(processes),
                             '--poly-output', DEFAULT_BUILDINGS_PREFIX]
    if single_taz:
        taz_buildings_options.append('--single-taz')
    if admin_level:
        taz_buildings_options.append('--admin-level')
        taz_buildings_options.append(str(admin_level))
    if plot:
        taz_buildings_options.append('--taz-plot')
        taz_buildings_options.append(plot)
    generateTAZBuildingsFromOSM.main(taz_buildings_options)

def _call_generate_amitran_from_taz_weights(density):
    """ Call directly generateAmitranFromTAZWeights from SUMOActivityGen. """
    odmatrix_options = ['--taz-weights', DEFAULT_WEIGHT_OUTPUT_CSV,
                        '--out', DEFAULT_ODMATRIX_AMITRAN_XML,
                        '--density', str(density)]
    generateAmitranFromTAZWeights.main(odmatrix_options)

def _call_generate_defaults_activitygen(population):
    """ Call directly generateDefaultsActivityGen from SUMOActivityGen. """
    default_options = ['--conf', DEAFULT_GENERIC_AG_CONG,
                       '--od-amitran', DEFAULT_ODMATRIX_AMITRAN_XML,
                       '--out', DEAFULT_SPECIFIC_AG_CONG,
                       '--population', str(population)]
    generateDefaultsActivityGen.main(default_options)

def _call_activitygen():
    """ Call directly activitygen from SUMOActivityGen. """
    activitygen_options = ['-c', DEAFULT_SPECIFIC_AG_CONG]
    activitygen.main(activitygen_options)

def _add_rou_to_default_sumocfg():
    """ Load the configuration file used by activitygen and add the newly generated routes. """
    route_files = ''
    for item in os.listdir():
        if '.rou.xml' in item:
            route_files += item + ','
    route_files = route_files.strip(',')

    xml_tree = ElementTree.parse(DEFAULT_SUMOCFG).getroot()
    for field in xml_tree.iter('route-files'):
        ## it should be only one, tho.
        field.attrib['value'] = route_files
    new_sumocfg = ElementTree.ElementTree(xml_tree)
    new_sumocfg.write(open(DEFAULT_SUMOCFG, 'wb'))

def _call_sumo():
    """ Call SUMO using a subprocess. """
    subprocess.check_call(['sumo', '-c', DEFAULT_SUMOCFG])

def _call_saga_activity_report():
    """ Call directly sagaActivityReport from SUMOActivityGen.. """
    report_options = ['--tripinfo', DEFAULT_TRIPINFO_FILE,
                      '--out', DEFAULT_REPORT_FILE]
    sagaActivityReport.main(report_options)

def _call_plot_summary():
    """ Call directly plot_summary from sumo/tools """
    plot_options = ['-i', DEFAULT_SUMMARY_FILE,
                    '-o', 'summary.png',
                    '--xtime1', '--verbose', '--blind']
    plot_summary.main(plot_options)

def main(cmd_args):
    """ Complete Scenario Generator. """

    args = get_options(cmd_args)
    logging.info('%s', args)

    ## ========================              PROFILER              ======================== ##
    if args.profiling:
        profiler = cProfile.Profile()
        profiler.enable()
    ## ========================              PROFILER              ======================== ##

    os.makedirs(args.out_dir, exist_ok=True)

    if args.from_step <= 0:
        logging.info('Copying default configuration files to destination.')
        shutil.copy(args.osm_file, args.out_dir)
        if args.local_defaults:
            shutil.copy('defaults/activitygen.json', args.out_dir)
            shutil.copy('defaults/basic.vType.xml', args.out_dir)
            shutil.copy('defaults/duarouter.sumocfg', args.out_dir)
            shutil.copy('defaults/osm.netccfg', args.out_dir)
            shutil.copy('defaults/osm.sumocfg', args.out_dir)
        else:
            shutil.copy(
                '{}/contributed/saga/defaults/activitygen.json'.format(os.environ['SUMO_TOOLS']),
                args.out_dir)
            shutil.copy(
                '{}/contributed/saga/defaults/basic.vType.xml'.format(os.environ['SUMO_TOOLS']),
                args.out_dir)
            shutil.copy(
                '{}/contributed/saga/defaults/duarouter.sumocfg'.format(os.environ['SUMO_TOOLS']),
                args.out_dir)
            shutil.copy(
                '{}/contributed/saga/defaults/osm.netccfg'.format(os.environ['SUMO_TOOLS']),
                args.out_dir)
            shutil.copy(
                '{}/contributed/saga/defaults/osm.sumocfg'.format(os.environ['SUMO_TOOLS']),
                args.out_dir)

    args.osm_file = os.path.basename(args.osm_file)
    os.chdir(args.out_dir)

    if args.from_step <= 1:
        logging.info('Generate the net.xml with all the additional components '
                     '(public transports, parkings, ..)')
        _call_netconvert(args.osm_file, args.left_hand_traffic)
        logging.info('Generate polygons using polyconvert.')
        _call_polyconvert(args.osm_file)

    if args.from_step <= 2:
        logging.info('Generate flows for public transportation using ptlines2flows.')
        _call_pt_lines_to_flows()

    if args.from_step <= 3:
        logging.info('Generate parking area location and possibly merge it with the one provided '
                     'by netconvert.')
        _call_generate_parking_areas_from_osm(args.osm_file)
        _merge_parking_files(DEFAULT_SIDE_PARKING_XML, DEFAULT_PARKING_AREAS,
                             DEFAULT_COMPLETE_PARKING_XML)

    if args.from_step <= 4:
        logging.info('Generate parking area rerouters using tools/generateParkingAreaRerouters.py')
        _call_generate_parking_area_rerouters(args.processes)

    if args.from_step <= 5:
        logging.info('Generate TAZ from administrative boundaries, TAZ weights using buildings and '
                     'PoIs and the buildings infrastructure.')
        os.makedirs('buildings', exist_ok=True)
        _call_generate_taz_buildings_from_osm(args.osm_file, args.single_taz, args.processes,
                                              args.admin_level, args.html_filename)

    if args.from_step <= 6:
        logging.info('Generate the default OD-Matrix in Amitran format. ')
        _call_generate_amitran_from_taz_weights(args.density)

    if args.from_step <= 7:
        logging.info('Generate the default values for the activity based mobility generator. ')
        _call_generate_defaults_activitygen(args.population)

    if args.from_step <= 8:
        logging.info('Mobility generation using SUMOActivityGen.')
        _call_activitygen()
        _add_rou_to_default_sumocfg()

    if args.from_step <= 9:
        logging.info('Launch sumo.')
        _call_sumo()
    
    if args.from_step <= 10:
        logging.info('Report.')
        _call_saga_activity_report()
        _call_plot_summary()

    ## ========================              PROFILER              ======================== ##
    if args.profiling:
        profiler.disable()
        results = io.StringIO()
        pstats.Stats(profiler, stream=results).sort_stats('cumulative').print_stats(25)
        print(results.getvalue())
    ## ========================              PROFILER              ======================== ##

    logging.info('Done.')

if __name__ == '__main__':
    logs()
    main(sys.argv[1:])
