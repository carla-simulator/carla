#!/usr/bin/env python3

""" Generate TAZ and Buildings weight from OSM.

    Author: Lara CODECA

    This program and the accompanying materials are made available under the
    terms of the Eclipse Public License 2.0 which is available at
    http://www.eclipse.org/legal/epl-2.0.
"""

import argparse
import csv
import logging
import multiprocessing
import os
import sys
import xml.etree.ElementTree

from functools import partial
import pyproj
import numpy

import shapely.geometry as geometry
from shapely.ops import transform

from tqdm import tqdm

if 'SUMO_HOME' in os.environ:
    sys.path.append(os.path.join(os.environ['SUMO_HOME'], 'tools'))
    import sumolib
else:
    sys.exit("please declare environment variable 'SUMO_HOME'")

def logs():
    """ Log init. """
    stdout_handler = logging.StreamHandler(sys.stdout)
    logging.basicConfig(handlers=[stdout_handler], level=logging.WARN,
                        format='[%(asctime)s] %(levelname)s: %(message)s',
                        datefmt='%m/%d/%Y %I:%M:%S %p')

def get_options(cmd_args=None):
    """ Argument Parser """
    parser = argparse.ArgumentParser(
        prog='generateTAZBuildingsFromOSM.py', usage='%(prog)s [options]',
        description='Generate TAZ and Buildings weight from OSM.')
    parser.add_argument('--osm', type=str, dest='osm_file', required=True,
                        help='OSM file.')
    parser.add_argument('--net', type=str, dest='net_file', required=True,
                        help='SUMO network xml file.')
    parser.add_argument('--taz-output', type=str, dest='taz_output', required=True,
                        help='TAZ output file (XML).')
    parser.add_argument('--weight-output', type=str, dest='od_output', required=True,
                        help='TAZ weight output file (CSV).')
    parser.add_argument('--poly-output', type=str, dest='poly_output', required=True,
                        help='Prefix for the POLY output files (CSV).')
    parser.add_argument('--single-taz', dest='single_taz', action='store_true',
                        help='Ignore administrative boundaries and generate only one TAZ.')
    parser.add_argument('--admin-level', type=int, dest='admin_level', default=None,
                        help='Select only the administrative boundaries with the given level '
                        'and generate the associated TAZs.')
    parser.add_argument('--taz-plot', type=str, dest='html_filename', default='',
                        help='Plots the TAZs to an HTML file as OSM overlay. (Requires folium)')
    parser.add_argument('--processes', type=int, dest='processes', default=1,
                        help='Number of processes spawned to associate buildings and edges.')
    parser.set_defaults(single_taz=False)
    return parser.parse_args(cmd_args)

class GenerateTAZandWeightsFromOSM():
    """ Generate TAZ and Buildings weight from OSM."""
    def __init__(self, parameters):
        self._param = parameters
        self._osm = _parse_xml_file(self._param.osm_file)
        self._net = sumolib.net.readNet(self._param.net_file)

        self._osm_boundaries = {
            'relation': {},
            'way': {},
            'node': {},
        }
        self._osm_buildings = dict()
        self._taz = dict()
        self._center = {
            'lat': 0.0,
            'lon': 0.0,
        }

        logging.info('Filtering administrative boudaries from OSM..')
        self._filter_boundaries_from_osm()
        logging.info("Extracting TAZ from OSM boundaries.")
        self._build_taz_from_osm()
        logging.info("Computing TAZ areas...")
        self._taz_areas()

    def generate_taz(self):
        """ Generate TAZ by filtering edges,
            additionally computing TAZ weight through nodes and area. """
        logging.info("Filtering edges...")
        self._edges_filter()
        logging.info("Filtering nodes...")
        self._nodes_filter()

    def generate_buildings(self):
        """ Generate the buildings weight with edge and TAZ association."""
        logging.info("Filtering buildings...")
        self._filter_buildings_from_osm()
        logging.info("Processing buildings...")
        self._processing_buildings()
        logging.info("Sorting buildings in the TAZ...")
        self._sort_buildings()

    def save_sumo_taz(self, filename):
        """ Save TAZ to file. """
        logging.info("Creation of %s", filename)
        self._write_taz_file(filename)

    def save_taz_weigth(self, filename):
        """ Save weigths to file."""
        logging.info("Creation of %s", filename)
        self._write_csv_file(filename)

    def save_buildings_weigth(self, filename):
        """ Save building weights to file."""
        logging.info("Creation of %s", filename)
        self._write_poly_files(filename)

    def save_taz_to_osm(self, filename):
        """ Plot the boundaries using folium to html file."""
        import folium
        from random import choice
        logging.info("Plotting TAZ to OpenStreetMap in file %s.", filename)
        colors = ['#0000FF', '#0040FF', '#0080FF', '#00FFB0', '#00E000', '#80FF00',
                  '#FFFF00', '#FFC000', '#FF0000']
        osm_map = folium.Map(location=[self._center['lat'], self._center['lon']])
        for _, value in self._taz.items():
            lons, lats = value['convex_hull'].exterior.xy
            points = list(zip(lats, lons))
            folium.PolyLine(points, tooltip=value['name'],
                            color=choice(colors), opacity=0.95).add_to(osm_map)
        osm_map.save(filename)

    @staticmethod
    def _is_boundary(tags):
        """ Check tags to find {'k': 'boundary', 'v': 'administrative'} """
        for tag in tags:
            if tag['k'] == 'boundary' and tag['v'] == 'administrative':
                return True
        return False

    @staticmethod
    def _is_admin_level(tags, level):
        """ Check tags to find {'k': 'admin_level', 'v': '*'} and
            returns True iff the value has the given level.
        """
        for tag in tags:
            if tag['k'] == 'admin_level' and tag['v'] == str(level):
                return True
        return False

    def _filter_boundaries_from_osm(self):
        """ Extract boundaries from OSM structure. """
        for relation in tqdm(self._osm['relation']):
            if 'tag' in relation and self._is_boundary(relation['tag']):
                if self._param.admin_level:
                    if not self._is_admin_level(relation['tag'], self._param.admin_level):
                        ## it's a boundary but from the wrong admin_level
                        continue
                self._osm_boundaries['relation'][relation['id']] = relation
                for member in relation['member']:
                    self._osm_boundaries[member['type']][member['ref']] = {}
        for way in tqdm(self._osm['way']):
            if way['id'] in self._osm_boundaries['way'].keys():
                self._osm_boundaries['way'][way['id']] = way
                for ndid in way['nd']:
                    self._osm_boundaries['node'][ndid['ref']] = {}
        for node in tqdm(self._osm['node']):
            if node['id'] in self._osm_boundaries['node'].keys():
                self._osm_boundaries['node'][node['id']] = node
        logging.info('Found %d administrative boundaries.',
                     len(self._osm_boundaries['relation'].keys()))

    def _build_taz_from_osm(self):
        """ Extract TAZ from OSM boundaries. """
        if not self._param.single_taz:
            for id_boundary, boundary in tqdm(self._osm_boundaries['relation'].items()):

                if not boundary:
                    logging.critical('Empty boundary %s', id_boundary)
                    continue

                list_of_nodes = []
                if 'member' in boundary:
                    for member in boundary['member']:
                        if member['type'] == 'way':
                            if 'nd' in self._osm_boundaries['way'][member['ref']]:
                                for node in self._osm_boundaries['way'][member['ref']]['nd']:
                                    coord = self._osm_boundaries['node'][node['ref']]
                                    list_of_nodes.append((float(coord['lon']), float(coord['lat'])))

                ## --------------------------- Consistency checks ----------------------------------
                if len(list_of_nodes) <= 2:
                    logging.critical('Boundary %s has %d nodes.', id_boundary, len(list_of_nodes))
                    continue
                try:
                    _, _ = geometry.MultiPoint(list_of_nodes).convex_hull.exterior.coords.xy
                except AttributeError:
                    logging.critical('Impossible to create the convex hull for boundary %s.',
                                     id_boundary)
                    continue
                # ----------------------------------------------------------------------------------

                name = None
                ref = None
                for tag in boundary['tag']:
                    if tag['k'] == 'name':
                        name = tag['v']
                    elif tag['k'] == 'ref':
                        ref = tag['v']
                if not name:
                    name = id_boundary
                if not ref:
                    ref = id_boundary
                self._taz[id_boundary] = {
                    'name': name,
                    'ref': ref,
                    'convex_hull': geometry.MultiPoint(list_of_nodes).convex_hull,
                    'raw_points': geometry.MultiPoint(list_of_nodes),
                    'edges': set(),
                    'nodes': set(),
                    'buildings': set(),
                    'buildings_cumul_area': 0,
                }

            logging.info('Generated %d TAZ from OSM boundaries.', len(self._taz.keys()))

        if not self._taz:
            ## Generate only one taz with everything in it.
            list_of_nodes = []
            for node in self._osm['node']:
                if node:
                    list_of_nodes.append((float(node['lon']), float(node['lat'])))
            self._taz['all'] = {
                'name': 'all',
                'ref': 'all',
                'convex_hull': geometry.MultiPoint(list_of_nodes).convex_hull,
                'raw_points': geometry.MultiPoint(list_of_nodes),
                'edges': set(),
                'nodes': set(),
                'buildings': set(),
                'buildings_cumul_area': 0,
            }
            logging.info('Generated 1 TAZ containing everything.')
            self._param.single_taz = True

    def _taz_areas(self):
        """ Compute the area in "shape" for each TAZ """
        for id_taz in tqdm(self._taz.keys()):
            x_coords, y_coords = self._taz[id_taz]['convex_hull'].exterior.coords.xy
            length = len(x_coords)
            poly = []
            for pos in range(length):
                x_coord, y_coord = self._net.convertLonLat2XY(x_coords[pos], y_coords[pos])
                poly.append((x_coord, y_coord))
            self._taz[id_taz]['area'] = geometry.Polygon(poly).area

    def _edges_filter(self):
        """ Sort edges to the right TAZ """
        for edge in tqdm(self._net.getEdges()):
            if self._param.single_taz:
                self._taz['all']['edges'].add(edge.getID())
            else:
                for coord in edge.getShape():
                    lon, lat = self._net.convertXY2LonLat(coord[0], coord[1])
                    for id_taz in list(self._taz.keys()):
                        if self._taz[id_taz]['convex_hull'].contains(geometry.Point(lon, lat)):
                            self._taz[id_taz]['edges'].add(edge.getID())
        empty_taz = set()
        for id_taz, taz in self._taz.items():
            if not taz['edges']:
                empty_taz.add(id_taz)
        for id_taz in empty_taz:
            del self._taz[id_taz]
        logging.info('Saved %d TAZ after the edges filtering.', len(self._taz.keys()))

    def _nodes_filter(self):
        """ Sort nodes to the right TAZ """
        points = []
        for node in tqdm(self._osm['node']):
            points.append([float(node['lat']), float(node['lon'])])
            if self._param.single_taz:
                self._taz['all']['nodes'].add(node['id'])
            else:
                for id_taz in list(self._taz.keys()):
                    if self._taz[id_taz]['convex_hull'].contains(
                            geometry.Point(float(node['lon']), float(node['lat']))):
                        self._taz[id_taz]['nodes'].add(node['id'])
        lat, lon = numpy.mean(numpy.array(points), axis=0)
        self._center['lat'] = lat
        self._center['lon'] = lon

    @staticmethod
    def _is_building(way):
        """ Return if a way is a building """
        if 'tag' not in way:
            return False
        for tag in way['tag']:
            if tag['k'] == 'building':
                return True
        return False

    def _filter_buildings_from_osm(self):
        """ Extract buildings from OSM structure. """
        nodes = dict()
        for node in tqdm(self._osm['node']):
            nodes[node['id']] = node
        for way in tqdm(self._osm['way']):
            if not self._is_building(way):
                continue
            self._osm_buildings[way['id']] = way
            self._osm_buildings[way['id']]['nodes'] = list()
            for ndid in way['nd']:
                self._osm_buildings[way['id']]['nodes'].append(nodes[ndid['ref']])
        logging.info('Found %d buildings.', len(self._osm_buildings.keys()))

    def _processing_buildings(self):
        """ Compute centroid and approximated area for each building (if necessary). """
        with multiprocessing.Pool(processes=self._param.processes) as pool:
            buildings = list(self._osm_buildings.values())
            for res in tqdm(pool.imap_unordered(self._processing_buildings_parallel, buildings)):
                self._osm_buildings[res['id']] = res

    @staticmethod
    def _processing_buildings_parallel(building):
        """ Compute centroid and approximated area for each building (if necessary). """
        # compute the centroid
        points = []
        for node in building['nodes']:
            points.append([float(node['lat']), float(node['lon'])])
        centroid = numpy.mean(numpy.array(points), axis=0)
        building['tag'].append({
            'k':  'centroid',
            'v':  '{}, {}'.format(centroid[0], centroid[1])
            })
        # compute the approximated area
        approx = geometry.MultiPoint(points).convex_hull
        # http://openstreetmapdata.com/info/projections
        proj = partial(pyproj.transform, pyproj.Proj('epsg:4326'),
                       pyproj.Proj('epsg:3857'))
        converted_approximation = transform(proj, approx)
        area = 0.0
        if not numpy.isnan(converted_approximation.area):
            area = converted_approximation.area
        building['tag'].append({
            'k':  'approx_area',
            'v':  area
            })
        return building

    def _sort_buildings(self):
        """ Multiprocess helper to sort buildings to the right TAZ based on centroid. """
        with multiprocessing.Pool(processes=self._param.processes) as pool:
            list_parameters = list()
            slices = numpy.array_split(list(self._osm_buildings.values()), self._param.processes)
            logging.info('Preprocessing for multiprocessing...')
            for buildings in slices:
                parameters = {
                    'buildings': buildings,
                    'all_in_one': self._param.single_taz,
                    'taz': self._taz,
                    'net_file': self._param.net_file,
                }
                list_parameters.append(parameters)
            logging.info('Buildings to TAZ multiprocessing...')
            for res in pool.imap_unordered(self._sort_buildings_into_taz, list_parameters):
                for id_taz, assocs in res.items():
                    self._taz[id_taz]['buildings'] |= assocs['buildings']
                    self._taz[id_taz]['buildings_cumul_area'] += assocs['buildings_cumul_area']

    @staticmethod
    def _sort_buildings_into_taz(parameters):
        """ Sort buildings to the right TAZ based on centroid. """
        ## global sumolib network
        sumo_net = sumolib.net.readNet(parameters['net_file'])

        def _get_centroid(building):
            """ Return the lat lon of the centroid. """
            for tag in building['tag']:
                if tag['k'] == 'centroid':
                    splitted = tag['v'].split(',')
                    return splitted[0], splitted[1]
            return None

        def _get_approx_area(building):
            """ Return approximated area of the building. """
            for tag in building['tag']:
                if tag['k'] == 'approx_area':
                    return float(tag['v'])
            return None

        def _building_to_edge(id_taz, x_coord, y_coord):
            """ Given the coords of a building, return te closest edge """
            centroid = (x_coord, y_coord)

            pedestrian_edge_info = None
            pedestrian_dist_edge = sys.float_info.max

            generic_edge_info = None
            generic_dist_edge = sys.float_info.max

            radius = 50.0
            while not pedestrian_edge_info or not generic_edge_info:
                neighbours = sumo_net.getNeighboringEdges(x_coord, y_coord, r=radius)
                for edge, _ in neighbours:
                    if edge.getID() not in parameters['taz'][id_taz]['edges']:
                        continue
                    if edge.allows('rail'):
                        continue
                    _, _, dist = edge.getClosestLanePosDist(centroid)
                    if edge.allows('passenger') and dist < generic_dist_edge:
                        generic_edge_info = edge
                        generic_dist_edge = dist
                    if edge.allows('pedestrian') and dist < pedestrian_dist_edge:
                        pedestrian_edge_info = edge
                        pedestrian_dist_edge = dist
                radius += 50.0

            if generic_edge_info and generic_dist_edge > 500.0:
                logging.info("A building entrance [passenger] is %d meters away.",
                             generic_dist_edge)
            if pedestrian_edge_info and pedestrian_dist_edge > 500.0:
                logging.info("A building entrance [pedestrian] is %d meters away.",
                             pedestrian_dist_edge)

            return generic_edge_info, pedestrian_edge_info

        def _associate_building_to_edges(id_taz, x_coord, y_coord):
            """ Adds a building to the specific TAZ. """
            generic_edge, pedestrian_edge = _building_to_edge(id_taz, x_coord, y_coord)
            if generic_edge or pedestrian_edge:
                gen_id = None
                ped_id = None
                if generic_edge:
                    gen_id = generic_edge.getID()
                if pedestrian_edge:
                    ped_id = pedestrian_edge.getID()
                return gen_id, ped_id
            return None

        associations = dict()
        for id_taz in list(parameters['taz'].keys()):
            associations[id_taz] = {
                'buildings': set(),
                'buildings_cumul_area': 0
            }
        for building in tqdm(parameters['buildings']):
            area = int(_get_approx_area(building))
            if area <= 0:
                ## there have been problems with the building conversion
                continue
            lat, lon = _get_centroid(building)
            x_coord, y_coord = sumo_net.convertLonLat2XY(lon, lat)

            if parameters['all_in_one']:
                ret = _associate_building_to_edges('all', x_coord, y_coord)
                if ret:
                    gen_id, ped_id = ret
                    associations['all']['buildings'].add((building['id'], area, gen_id, ped_id))
                    associations['all']['buildings_cumul_area'] += area
            else:
                for id_taz in list(parameters['taz'].keys()):
                    if parameters['taz'][id_taz]['convex_hull'].contains(
                            geometry.Point(float(lon), float(lat))):
                        ret = _associate_building_to_edges(id_taz, x_coord, y_coord)
                        if ret:
                            gen_id, ped_id = ret
                            associations[id_taz]['buildings'].add(
                                (building['id'], area, gen_id, ped_id))
                            associations[id_taz]['buildings_cumul_area'] += area
        return associations

    _TAZS = """
<tazs> {list_of_tazs}
</tazs>
"""

    _TAZ = """
    <!-- id="{taz_id}" name="{taz_name}" -->
    <taz id="{taz_id}" edges="{list_of_edges}"/>"""

    def _write_taz_file(self, filename):
        """ Write the SUMO file. """
        with open(filename, 'w') as outfile:
            string_of_tazs = ''
            for value in self._taz.values():
                string_of_edges = ''
                for edge in value['edges']:
                    string_of_edges += str(edge) + ' '
                string_of_edges = string_of_edges.strip()
                string_of_tazs += self._TAZ.format(
                    taz_id=value['ref'], taz_name=value['name'], #.encode('utf-8'),
                    list_of_edges=string_of_edges)
            outfile.write(self._TAZS.format(list_of_tazs=string_of_tazs))

    def _write_poly_files(self, prefix):
        """ Write the CSV file. """
        for value in self._taz.values():
            if not value['buildings']:
                continue
            filename = '{}.{}.csv'.format(prefix, value['ref'])
            with open(filename, 'w') as csvfile:
                csvwriter = csv.writer(csvfile, delimiter=',')
                csvwriter.writerow(['TAZ', 'Poly', 'Area', 'Weight', 'GenEdge', 'PedEdge'])
                for poly, area, g_edge, p_edge in value['buildings']:
                    if not value['buildings_cumul_area']:
                        continue
                    csvwriter.writerow([value['ref'], poly, area,
                                        area/value['buildings_cumul_area'], g_edge, p_edge])

    def _write_csv_file(self, filename):
        """ Write the CSV file. """
        with open(filename, 'w') as csvfile:
            csvwriter = csv.writer(csvfile, delimiter=',')
            csvwriter.writerow(['TAZ', 'Name', '#Nodes', 'Area'])
            for value in self._taz.values():
                num_nodes = len(value['nodes'])
                if num_nodes <= 0: # the area is empty
                    continue
                csvwriter.writerow([value['ref'], value['name'], num_nodes, value['area']])

def _parse_xml_file(xml_file):
    """ Extract all info from an OSM file. """
    xml_tree = xml.etree.ElementTree.parse(xml_file).getroot()
    dict_xml = {}
    for child in xml_tree:
        parsed = {}
        for key, value in child.attrib.items():
            parsed[key] = value

        for attribute in child:
            if attribute.tag in list(parsed.keys()):
                parsed[attribute.tag].append(attribute.attrib)
            else:
                parsed[attribute.tag] = [attribute.attrib]

        if child.tag in list(dict_xml.keys()):
            dict_xml[child.tag].append(parsed)
        else:
            dict_xml[child.tag] = [parsed]
    return dict_xml

def main(cmd_args):
    """ Generate TAZ and Buildings weight from OSM. """

    args = get_options(cmd_args)
    taz_generator = GenerateTAZandWeightsFromOSM(args)
    taz_generator.generate_taz()
    taz_generator.save_sumo_taz(args.taz_output)
    taz_generator.save_taz_weigth(args.od_output)
    taz_generator.generate_buildings()
    taz_generator.save_buildings_weigth(args.poly_output)
    if args.html_filename:
        taz_generator.save_taz_to_osm(args.html_filename)

    logging.info("Done.")

if __name__ == "__main__":
    logs()
    main(sys.argv[1:])
