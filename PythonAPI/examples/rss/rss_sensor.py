#!/usr/bin/env python
#
# Copyright (c) 2020 Intel Corporation
#

import glob
import os
import sys

try:
    sys.path.append(glob.glob(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))) + '/carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import inspect
import carla
from carla import ad
import math
from rss_visualization import RssDebugVisualizer # pylint: disable=relative-import


# ==============================================================================
# -- RssSensor -----------------------------------------------------------------
# ==============================================================================


class RssStateInfo(object):

    def __init__(self, rss_state, ego_dynamics_on_route, world_model):
        self.rss_state = rss_state
        self.distance = -1
        self.is_dangerous = ad.rss.state.isDangerous(rss_state)
        if rss_state.situationType == ad.rss.situation.SituationType.Unstructured:
            self.actor_calculation_mode = ad.rss.map.RssMode.Unstructured
        else:
            self.actor_calculation_mode = ad.rss.map.RssMode.Structured

        # calculate distance to other vehicle
        object_state = None
        for scene in world_model.scenes:
            if scene.object.objectId == rss_state.objectId:
                object_state = scene.object.state
                break

        if object_state:
            self.distance = math.sqrt((float(ego_dynamics_on_route.ego_center.x) - float(object_state.centerPoint.x))**2 +
                                      (float(ego_dynamics_on_route.ego_center.y) - float(object_state.centerPoint.y))**2)

        self.longitudinal_margin = float(rss_state.longitudinalState.rssStateInformation.currentDistance - rss_state.longitudinalState.rssStateInformation.safeDistance)
        self.margin = max(0, self.longitudinal_margin)
        self.lateral_margin = None
        if rss_state.lateralStateLeft.rssStateInformation.evaluator != "None":
            self.lateral_margin = rss_state.lateralStateLeft.rssStateInformation.currentDistance - rss_state.lateralStateLeft.rssStateInformation.safeDistance
        if rss_state.lateralStateRight.rssStateInformation.evaluator != "None":
            lateral_margin_right = rss_state.lateralStateRight.rssStateInformation.currentDistance - rss_state.lateralStateRight.rssStateInformation.safeDistance
            if self.lateral_margin==None or self.lateral_margin > lateral_margin_right:
                self.lateral_margin=lateral_margin_right
        if self.lateral_margin!=None and self.lateral_margin>0:
            self.margin += self.lateral_margin

    def get_actor(self, world):
        if self.rss_state.objectId == 18446744073709551614:
            return None # "Border Left"
        elif self.rss_state.objectId == 18446744073709551615:
            return None # "Border Right"
        else:
            return world.get_actor(self.rss_state.objectId)

    def __str__(self):
        return "RssStateInfo: object=" + str(self.rss_state.objectId) + " dangerous=" + str(self.is_dangerous)


class RssSensor(object):

    def __init__(self, parent_actor, world, unstructured_scene_visualizer, bounding_box_visualizer, state_visualizer, routing_targets=None):
        self.sensor = None
        self.unstructured_scene_visualizer = unstructured_scene_visualizer
        self.bounding_box_visualizer = bounding_box_visualizer
        self._parent = parent_actor
        self.timestamp = None
        self.response_valid = False
        self.proper_response = None
        self.rss_state_snapshot = None
        self.situation_snapshot = None
        self.world_model = None
        self.individual_rss_states = []
        self._allowed_heading_ranges = []
        self.ego_dynamics_on_route = None
        self.current_vehicle_parameters = self.get_default_parameters()
        self.route = None
        self.debug_visualizer = RssDebugVisualizer(parent_actor, world)
        self.state_visualizer = state_visualizer
        self.change_to_unstructured_position_map = dict()

        # get max steering angle
        physics_control = parent_actor.get_physics_control()
        self._max_steer_angle = 0.0
        for wheel in physics_control.wheels:
            if wheel.max_steer_angle > self._max_steer_angle:
                self._max_steer_angle = wheel.max_steer_angle
        self._max_steer_angle = math.radians(self._max_steer_angle)

        world = self._parent.get_world()
        bp = world.get_blueprint_library().find('sensor.other.rss')
        self.sensor = world.spawn_actor(bp, carla.Transform(carla.Location(x=0.0, z=0.0)), attach_to=self._parent)
        # We need to pass the lambda a weak reference to self to avoid circular
        # reference.

        def check_rss_class(clazz):
            return inspect.isclass(clazz) and "RssSensor" in clazz.__name__

        if not inspect.getmembers(carla, check_rss_class):
            raise RuntimeError('CARLA PythonAPI not compiled in RSS variant, please "make PythonAPI.rss"')

        self.log_level = carla.RssLogLevel.warn
        self.map_log_level = carla.RssLogLevel.warn

        self.set_default_parameters()

        self.sensor.register_actor_constellation_callback(self._on_actor_constellation_request)

        self.sensor.listen(self._on_rss_response)
        self.sensor.set_log_level(self.log_level)
        self.sensor.set_map_log_level(self.map_log_level)

        # only relevant if actor constellation callback is not registered
        # self.sensor.ego_vehicle_dynamics = self.current_vehicle_parameters

        self.sensor.road_boundaries_mode = carla.RssRoadBoundariesMode.Off

        self.sensor.reset_routing_targets()
        if routing_targets:
            for target in routing_targets:
                self.sensor.append_routing_target(target)

    def _on_actor_constellation_request(self, actor_constellation_data):
        # print("_on_actor_constellation_request: ", str(actor_constellation_data))

        actor_constellation_result = carla.RssActorConstellationResult()
        actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.NotRelevant
        actor_constellation_result.restrict_speed_limit_mode = ad.rss.map.RssSceneCreation.RestrictSpeedLimitMode.IncreasedSpeedLimit10
        actor_constellation_result.ego_vehicle_dynamics = self.current_vehicle_parameters
        actor_constellation_result.actor_object_type = ad.rss.world.ObjectType.Invalid
        actor_constellation_result.actor_dynamics = self.current_vehicle_parameters

        actor_id = -1
        # actor_type_id = "none"
        if actor_constellation_data.other_actor != None:
            actor_id = actor_constellation_data.other_actor.id
            # actor_type_id = actor_constellation_data.other_actor.type_id

            ego_on_the_sidewalk = False
            ego_on_routeable_road = False
            for occupied_region in actor_constellation_data.ego_match_object.mapMatchedBoundingBox.laneOccupiedRegions:
                lane = ad.map.lane.getLane(occupied_region.laneId)
                if lane.type == ad.map.lane.LaneType.PEDESTRIAN:
                    # if not ego_on_the_sidewalk:
                    #   print ( "ego-{} on lane of lane type {} => sidewalk".format(actor_id, lane.type))
                    ego_on_the_sidewalk = True
                elif ad.map.lane.isRouteable(lane):
                    # if not ego_on_routeable_road:
                    #   print ( "ego-{} on lane of lane type {} => road".format(actor_id, lane.type))
                    ego_on_routeable_road = True

            if 'walker.pedestrian' in actor_constellation_data.other_actor.type_id:
                # determine if the pedestrian is walking on the sidewalk or on the road
                pedestrian_on_the_road = False
                pedestrian_on_the_sidewalk = False
                for occupied_region in actor_constellation_data.other_match_object.mapMatchedBoundingBox.laneOccupiedRegions:
                    lane = ad.map.lane.getLane(occupied_region.laneId)
                    if lane.type == ad.map.lane.LaneType.PEDESTRIAN:
                        # if not pedestrian_on_the_sidewalk:
                        #   print ( "pedestrian-{} on lane of lane type {} => sidewalk".format(actor_id, lane.type))
                        pedestrian_on_the_sidewalk = True
                    else:
                        # if not pedestrian_on_the_road:
                        #    print ( "pedestrian-{} on lane of lane type {} => road".format(actor_id, lane.type))
                        pedestrian_on_the_road = True
                if ego_on_routeable_road and not ego_on_the_sidewalk and not pedestrian_on_the_road and pedestrian_on_the_sidewalk:
                    # pedestrian is not on the road, but on the sidewalk: then common sense is that vehicle has priority
                    # This analysis can and should be done more detailed, but this is a basic starting point for the decision
                    # In addition, the road network has to be correct to work best
                    # (currently there are no sidewalks in intersection areas)
                    # print ( "pedestrian-{} Off".format(actor_id))
                    actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.NotRelevant
                else:
                    # print ( "pedestrian-{} Unstructured".format(actor_id))
                    actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.Unstructured
                actor_constellation_result.actor_object_type = ad.rss.world.ObjectType.Pedestrian
                actor_constellation_result.actor_dynamics = self.get_pedestrian_parameters()
            elif 'vehicle' in actor_constellation_data.other_actor.type_id:
                actor_constellation_result.actor_object_type = ad.rss.world.ObjectType.OtherVehicle

                # set the response time of others vehicles to 2 seconds; the rest stays the same
                actor_constellation_result.actor_dynamics.responseTime = 2.0

                # per default, if ego is not on the road -> unstructured
                if ego_on_routeable_road:
                    actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.Structured
                else:
                    # print("vehicle-{} unstructured: reason other ego not on routeable road".format(actor_id))
                    actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.Unstructured

                # special handling for vehicles standing still
                actor_vel = actor_constellation_data.other_actor.get_velocity()
                actor_speed = math.sqrt(actor_vel.x**2 + actor_vel.y**2 + actor_vel.z**2)
                if actor_speed < 0.01:
                    # reduce response time
                    actor_constellation_result.actor_dynamics.responseTime = 1.0
                    # still in structured?
                    if actor_constellation_result.rss_calculation_mode == ad.rss.map.RssMode.Structured:

                        actor_distance = math.sqrt(float(actor_constellation_data.ego_match_object.enuPosition.centerPoint.x -
                                                         actor_constellation_data.other_match_object.enuPosition.centerPoint.x)**2 +
                                                   float(actor_constellation_data.ego_match_object.enuPosition.centerPoint.y -
                                                         actor_constellation_data.other_match_object.enuPosition.centerPoint.y)**2)
                        # print("vehicle-{} unstructured check: other distance {}".format(actor_id, actor_distance))

                        if actor_constellation_data.ego_dynamics_on_route.ego_speed < 0.01:
                            # both vehicles stand still, so we have to analyze in detail if we possibly want to use
                            # unstructured mode to cope with blockades on the road...

                            if actor_distance < 10:
                                # the other has to be near enough to trigger a switch to unstructured
                                other_outside_routeable_road = False
                                for occupied_region in actor_constellation_data.other_match_object.mapMatchedBoundingBox.laneOccupiedRegions:
                                    lane = ad.map.lane.getLane(occupied_region.laneId)
                                    if not ad.map.lane.isRouteable(lane):
                                        other_outside_routeable_road = True

                                if other_outside_routeable_road:
                                    # if the other is somewhat outside the standard routeable road (e.g. parked at the side, ...)
                                    # we immediately decide for unstructured
                                    # print("vehicle-{} unstructured: reason other outside routeable
                                    # road".format(actor_id))
                                    actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.Unstructured
                                else:
                                    # otherwise we have to look in the orientation delta in addition to get some basic idea of the
                                    # constellation (we don't want to go into unstructured if we both waiting
                                    # behind a red light...)
                                    heading_delta = abs(float(actor_constellation_data.ego_match_object.enuPosition.heading -
                                                              actor_constellation_data.other_match_object.enuPosition.heading))
                                    if heading_delta > 0.2:  # around 11 degree
                                        # print("vehicle-{} unstructured: reason heading delta
                                        # {}".format(actor_id, heading_delta))
                                        actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.Unstructured
                                        self.change_to_unstructured_position_map[
                                            actor_id] = actor_constellation_data.other_match_object.enuPosition
                        else:
                            # ego moves
                            if actor_distance < 10:
                                # if the ego moves, the other actor doesn't move an the mode was
                                # previously set to unstructured, keep it
                                try:
                                    if self.change_to_unstructured_position_map[actor_id] == actor_constellation_data.other_match_object.enuPosition:
                                        heading_delta = abs(float(actor_constellation_data.ego_match_object.enuPosition.heading -
                                                                  actor_constellation_data.other_match_object.enuPosition.heading))
                                        if heading_delta > 0.2:
                                            actor_constellation_result.rss_calculation_mode = ad.rss.map.RssMode.Unstructured
                                        else:
                                            del self.change_to_unstructured_position_map[actor_id]
                                except (AttributeError, KeyError):
                                    pass
                            else:
                                if actor_id in self.change_to_unstructured_position_map:
                                    del self.change_to_unstructured_position_map[actor_id]

                    # still in structured?
                    if actor_constellation_result.rss_calculation_mode == ad.rss.map.RssMode.Structured:
                        # in structured case we have to cope with not yet implemented lateral intersection checks in core RSS implementation
                        # if the other is standing still, we don't assume that he will accelerate
                        # otherwise if standing at the intersection the acceleration within reaction time
                        # will allow to enter the intersection which current RSS implementation will immediately consider
                        # as dangerous
                        # print("_on_actor_constellation_result({}) setting accelMax to
                        # zero".format(actor_constellation_data.other_actor.id))
                        actor_constellation_result.actor_dynamics.alphaLon.accelMax = 0.
                        actor_constellation_result.actor_dynamics.alphaLat.accelMax = 0.
        else:
            # store route for debug drawings
            self.route = actor_constellation_data.ego_route
        # since the ego vehicle is controlled manually, it is easy possible that the ego vehicle
        # accelerates far more in lateral direction than the ego_dynamics indicate
        # in an automated vehicle this would be considered by the low-level controller when the RSS restriction
        # is taken into account properly
        # but the simple RSS restrictor within CARLA is not able to do so...
        # So we should at least tell RSS about the fact that we acceleration more than this
        # to be able to react on this
        abs_avg_route_accel_lat = abs(float(actor_constellation_data.ego_dynamics_on_route.avg_route_accel_lat))
        if abs_avg_route_accel_lat > actor_constellation_result.ego_vehicle_dynamics.alphaLat.accelMax:
            # print("!! Route lateral dynamics exceed expectations: route:{} expected:{} !!".format(abs_avg_route_accel_lat,
            #                                                                                      actor_constellation_result.ego_vehicle_dynamics.alphaLat.accelMax))
            actor_constellation_result.ego_vehicle_dynamics.alphaLat.accelMax = min(20., abs_avg_route_accel_lat)

        # print("_on_actor_constellation_result({}-{}): ".format(actor_id,
        # actor_type_id), str(actor_constellation_result))
        return actor_constellation_result

    def destroy(self):
        if self.sensor:
            print("Stopping RSS sensor")
            self.sensor.stop()
            print("Deleting Scene Visualizer")
            self.unstructured_scene_visualizer = None
            print("Destroying RSS sensor")
            self.sensor.destroy()
            print("Destroyed RSS sensor")

    def toggle_debug_visualization_mode(self):
        self.debug_visualizer.toggleMode()

    def increase_log_level(self):
        print("inccrease {}".format(self.log_level))
        if self.log_level < carla.RssLogLevel.off:
            self.log_level = self.log_level+1
        self.sensor.set_log_level(self.log_level)

    def decrease_log_level(self):
        print("decrease {}".format(self.log_level))
        if self.log_level > carla.RssLogLevel.trace:
            self.log_level = self.log_level-1
        self.sensor.set_log_level(self.log_level)

    def increase_map_log_level(self):
        if self.map_log_level < carla.RssLogLevel.off:
            self.map_log_level = self.map_log_level+1
        self.sensor.set_map_log_level(self.map_log_level)

    def decrease_map_log_level(self):
        if self.map_log_level > carla.RssLogLevel.trace:
            self.map_log_level = self.map_log_level-1
        self.sensor.set_map_log_level(self.map_log_level)

    @staticmethod
    def get_default_parameters():
        ego_dynamics = ad.rss.world.RssDynamics()
        ego_dynamics.alphaLon.accelMax = 5
        ego_dynamics.alphaLon.brakeMax = -8
        ego_dynamics.alphaLon.brakeMin = -4
        ego_dynamics.alphaLon.brakeMinCorrect = -3
        ego_dynamics.alphaLat.accelMax = 0.2
        ego_dynamics.alphaLat.brakeMin = -0.8
        ego_dynamics.lateralFluctuationMargin = 0.1
        ego_dynamics.responseTime = 0.5
        ego_dynamics.maxSpeedOnAcceleration = 100
        ego_dynamics.unstructuredSettings.pedestrianTurningRadius = 2.0
        ego_dynamics.unstructuredSettings.driveAwayMaxAngle = 2.4
        ego_dynamics.unstructuredSettings.vehicleYawRateChange = 1.3
        ego_dynamics.unstructuredSettings.vehicleMinRadius = 3.5
        ego_dynamics.unstructuredSettings.vehicleTrajectoryCalculationStep = 0.2
        ego_dynamics.unstructuredSettings.vehicleFrontIntermediateYawRateChangeRatioSteps = 4
        ego_dynamics.unstructuredSettings.vehicleBackIntermediateYawRateChangeRatioSteps = 0
        ego_dynamics.unstructuredSettings.vehicleContinueForwardIntermediateAccelerationSteps = 3
        ego_dynamics.unstructuredSettings.vehicleBrakeIntermediateAccelerationSteps = 3
        ego_dynamics.unstructuredSettings.pedestrianTurningRadius = 2.0
        ego_dynamics.unstructuredSettings.pedestrianContinueForwardIntermediateHeadingChangeRatioSteps = 3
        ego_dynamics.unstructuredSettings.pedestrianContinueForwardIntermediateAccelerationSteps = 0
        ego_dynamics.unstructuredSettings.pedestrianBrakeIntermediateAccelerationSteps = 3
        ego_dynamics.unstructuredSettings.pedestrianFrontIntermediateHeadingChangeRatioSteps = 4
        ego_dynamics.unstructuredSettings.pedestrianBackIntermediateHeadingChangeRatioSteps = 0
        return ego_dynamics

    def set_default_parameters(self):
        print("Use 'default' RSS Parameters")
        self.current_vehicle_parameters = self.get_default_parameters()

    @staticmethod
    def get_pedestrian_parameters():
        pedestrian_dynamics = ad.rss.world.RssDynamics()
        pedestrian_dynamics.alphaLon.accelMax = 2.0
        pedestrian_dynamics.alphaLon.brakeMax = -2.0
        pedestrian_dynamics.alphaLon.brakeMin = -2.0
        pedestrian_dynamics.alphaLon.brakeMinCorrect = -2.0
        pedestrian_dynamics.alphaLat.accelMax = 0.001
        pedestrian_dynamics.alphaLat.brakeMin = -0.001
        pedestrian_dynamics.lateralFluctuationMargin = 0.1
        pedestrian_dynamics.responseTime = 0.8
        pedestrian_dynamics.maxSpeedOnAcceleration = 10
        pedestrian_dynamics.unstructuredSettings.pedestrianTurningRadius = 2.0
        pedestrian_dynamics.unstructuredSettings.driveAwayMaxAngle = 2.4
        pedestrian_dynamics.unstructuredSettings.pedestrianContinueForwardIntermediateHeadingChangeRatioSteps = 3
        pedestrian_dynamics.unstructuredSettings.pedestrianContinueForwardIntermediateAccelerationSteps = 0
        pedestrian_dynamics.unstructuredSettings.pedestrianBrakeIntermediateAccelerationSteps = 3
        pedestrian_dynamics.unstructuredSettings.pedestrianFrontIntermediateHeadingChangeRatioSteps = 4
        pedestrian_dynamics.unstructuredSettings.pedestrianBackIntermediateHeadingChangeRatioSteps = 0

        #not used:
        pedestrian_dynamics.unstructuredSettings.vehicleYawRateChange = 1.3
        pedestrian_dynamics.unstructuredSettings.vehicleMinRadius = 3.5
        pedestrian_dynamics.unstructuredSettings.vehicleTrajectoryCalculationStep = 0.2
        pedestrian_dynamics.unstructuredSettings.vehicleFrontIntermediateYawRateChangeRatioSteps = 4
        pedestrian_dynamics.unstructuredSettings.vehicleBackIntermediateYawRateChangeRatioSteps = 0
        pedestrian_dynamics.unstructuredSettings.vehicleContinueForwardIntermediateAccelerationSteps = 3
        pedestrian_dynamics.unstructuredSettings.vehicleBrakeIntermediateAccelerationSteps = 3
        return pedestrian_dynamics

    def get_steering_ranges(self):
        ranges = []
        for heading_range in self._allowed_heading_ranges:
            ranges.append(
                (
                    (float(self.ego_dynamics_on_route.ego_heading) - float(heading_range.begin)) / self._max_steer_angle,
                    (float(self.ego_dynamics_on_route.ego_heading) - float(heading_range.end)) / self._max_steer_angle)
            )
        return ranges

    def _on_rss_response(self, response):
        if not self or not response:
            return
        delta_time = 0.1
        if self.timestamp:
            delta_time = response.timestamp - self.timestamp
        if delta_time > -0.05:
            self.timestamp = response.timestamp
            self.response_valid = response.response_valid
            self.proper_response = response.proper_response
            self.ego_dynamics_on_route = response.ego_dynamics_on_route
            self.rss_state_snapshot = response.rss_state_snapshot
            self.situation_snapshot = response.situation_snapshot
            self.world_model = response.world_model

            # calculate the allowed heading ranges:
            if response.proper_response.headingRanges:
                heading = float(response.ego_dynamics_on_route.ego_heading)
                heading_ranges = response.proper_response.headingRanges
                steering_range = ad.rss.state.HeadingRange()
                steering_range.begin = - self._max_steer_angle + heading
                steering_range.end = self._max_steer_angle + heading
                ad.rss.unstructured.getHeadingOverlap(steering_range, heading_ranges)
                self._allowed_heading_ranges = heading_ranges
            else:
                self._allowed_heading_ranges = []

            if self.unstructured_scene_visualizer:
                self.unstructured_scene_visualizer.tick(response.frame, response, self._allowed_heading_ranges)

            new_states = []
            for rss_state in response.rss_state_snapshot.individualResponses:
                new_states.append(RssStateInfo(rss_state, response.ego_dynamics_on_route, response.world_model))
            if len(new_states) > 0:
                new_states.sort(key=lambda rss_states: rss_states.distance)
            self.individual_rss_states = new_states
            if self.bounding_box_visualizer:
                self.bounding_box_visualizer.tick(response.frame, self.individual_rss_states)
            if self.state_visualizer:
                self.state_visualizer.tick(self.individual_rss_states)
            self.debug_visualizer.tick(self.route, not response.proper_response.isSafe,
                                       self.individual_rss_states, self.ego_dynamics_on_route)

        else:
            print("ignore outdated response {}".format(delta_time))
