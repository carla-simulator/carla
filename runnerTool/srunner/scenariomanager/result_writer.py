#!/usr/bin/env python

# Copyright (c) 2018-2019 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module contains the result gatherer and write for CARLA scenarios.
It shall be used from the ScenarioManager only.
"""

from __future__ import print_function

import time
import json
from tabulate import tabulate


class ResultOutputProvider(object):

    """
    This module contains the _result gatherer and write for CARLA scenarios.
    It shall be used from the ScenarioManager only.
    """

    def __init__(self, data, result, stdout=True, filename=None, junitfile=None, jsonfile=None):
        """
        Setup all parameters
        - _data contains all scenario-related information
        - _result is overall pass/fail info
        - _stdout (True/False) is used to (de)activate terminal output
        - _filename is used to (de)activate file output in tabular form
        - _junit is used to (de)activate file output in junit form
        - _json is used to (de)activate file output in json form
        """
        self._data = data
        self._result = result
        self._stdout = stdout
        self._filename = filename
        self._junit = junitfile
        self._json = jsonfile

        self._start_time = time.strftime('%Y-%m-%d %H:%M:%S',
                                         time.localtime(self._data.start_system_time))
        self._end_time = time.strftime('%Y-%m-%d %H:%M:%S',
                                       time.localtime(self._data.end_system_time))

    def write(self):
        """
        Public write function
        """
        if self._junit is not None:
            self._write_to_junit()
        if self._json is not None:
            self._write_to_reportjson()

        output = self.create_output_text()
        if self._filename is not None:
            with open(self._filename, 'w', encoding='utf-8') as fd:
                fd.write(output)
        if self._stdout:
            print(output)

    def create_output_text(self):
        """
        Creates the output message
        """
        output = "\n"
        output += " ======= Results of Scenario: {} ---- {} =======\n".format(
            self._data.scenario_tree.name, self._result)
        end_line_length = len(output) - 3
        output += "\n"

        # Lis of all the actors
        output += " > Ego vehicles:\n"
        for ego_vehicle in self._data.ego_vehicles:
            output += "{}; ".format(ego_vehicle)
        output += "\n\n"

        output += " > Other actors:\n"
        for actor in self._data.other_actors:
            output += "{}; ".format(actor)
        output += "\n\n"

        # Simulation part
        output += " > Simulation Information\n"

        system_time = round(self._data.scenario_duration_system, 2)
        game_time = round(self._data.scenario_duration_game, 2)
        ratio = round(self._data.scenario_duration_game / self._data.scenario_duration_system, 3)

        list_statistics = [["Start Time", "{}".format(self._start_time)]]
        list_statistics.extend([["End Time", "{}".format(self._end_time)]])
        list_statistics.extend([["Duration (System Time)", "{}s".format(system_time)]])
        list_statistics.extend([["Duration (Game Time)", "{}s".format(game_time)]])
        list_statistics.extend([["Ratio (System Time / Game Time)", "{}s".format(ratio)]])

        output += tabulate(list_statistics, tablefmt='fancy_grid')
        output += "\n\n"

        # Criteria part
        output += " > Criteria Information\n"
        header = ['Actor', 'Criterion', 'Result', 'Actual Value', 'Expected Value']
        list_statistics = [header]

        for criterion in self._data.scenario.get_criteria():
            name_string = criterion.name
            if criterion.optional:
                name_string += " (Opt.)"
            else:
                name_string += " (Req.)"

            actor = "{} (id={})".format(criterion.actor.type_id[8:], criterion.actor.id)
            criteria = name_string
            result = "FAILURE" if criterion.test_status == "RUNNING" else criterion.test_status
            actual_value = criterion.actual_value
            expected_value = criterion.expected_value_success

            list_statistics.extend([[actor, criteria, result, actual_value, expected_value]])

        # Timeout
        actor = ""
        criteria = "Timeout (Req.)"
        result = "SUCCESS" if self._data.scenario_duration_game < self._data.scenario.timeout else "FAILURE"
        actual_value = round(self._data.scenario_duration_game, 2)
        expected_value = round(self._data.scenario.timeout, 2)

        list_statistics.extend([[actor, criteria, result, actual_value, expected_value]])

        # Global and final output message
        list_statistics.extend([['', 'GLOBAL RESULT', self._result, '', '']])

        output += tabulate(list_statistics, tablefmt='fancy_grid')
        output += "\n"
        output += " " + "=" * end_line_length + "\n"

        return output

    def _write_to_reportjson(self):
        """
        Write a machine-readable report to JSON

        The resulting report has the following format:
        {
            criteria: [
                {
                    name: "CheckCollisions",
                    expected: "0",
                    actual: "2",
                    optional: false,
                    success: false
                }, ...
            ]
        }
        """
        json_list = []

        def result_dict(name, actor, optional, expected, actual, success):
            """
            Convenience function to convert its arguments into a JSON-ready dict
            :param name: Name of the test criterion
            :param actor: Actor ID as string
            :param optional: If the criterion is optional
            :param expected: The expected value of the criterion (eg 0 for collisions)
            :param actual: The actual value
            :param success: If the test was passed
            :return: A dict data structure that will be written to JSON
            """
            return {
                "name": name,
                "actor": actor,
                "optional": optional,
                "expected": expected,
                "actual": actual,
                "success": success,
            }

        for criterion in self._data.scenario.get_criteria():
            json_list.append(
                result_dict(
                    criterion.name,
                    "{}-{}".format(criterion.actor.type_id[8:], criterion.actor.id),
                    criterion.optional,
                    criterion.expected_value_success,
                    criterion.actual_value,
                    criterion.test_status in ["SUCCESS", "ACCEPTABLE"]
                )
            )

        # add one entry for duration
        timeout = self._data.scenario.timeout
        duration = self._data.scenario_duration_game
        json_list.append(
            result_dict(
                "Duration", "all", False, timeout, duration, duration <= timeout
            )
        )

        result_object = {
            "scenario": self._data.scenario_tree.name,
            "success": self._result in ["SUCCESS", "ACCEPTABLE"],
            "criteria": json_list
        }

        with open(self._json, "w", encoding='utf-8') as fp:
            json.dump(result_object, fp, indent=4)

    def _write_to_junit(self):
        """
        Writing to Junit XML
        """
        test_count = 0
        failure_count = 0
        for criterion in self._data.scenario.get_criteria():
            test_count += 1
            if criterion.test_status != "SUCCESS":
                failure_count += 1

        # handle timeout
        test_count += 1
        if self._data.scenario_duration_game >= self._data.scenario.timeout:
            failure_count += 1

        with open(self._junit, "w", encoding='utf-8') as junit_file:

            junit_file.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")

            test_suites_string = ("<testsuites tests=\"%d\" failures=\"%d\" disabled=\"0\" "
                                  "errors=\"0\" timestamp=\"%s\" time=\"%5.2f\" "
                                  "name=\"Simulation\" package=\"Scenarios\">\n" %
                                  (test_count,
                                   failure_count,
                                   self._start_time,
                                   self._data.scenario_duration_system))
            junit_file.write(test_suites_string)

            test_suite_string = ("  <testsuite name=\"%s\" tests=\"%d\" failures=\"%d\" "
                                 "disabled=\"0\" errors=\"0\" time=\"%5.2f\">\n" %
                                 (self._data.scenario_tree.name,
                                  test_count,
                                  failure_count,
                                  self._data.scenario_duration_system))
            junit_file.write(test_suite_string)

            for criterion in self._data.scenario.get_criteria():
                testcase_name = criterion.name + "_" + \
                    criterion.actor.type_id[8:] + "_" + str(criterion.actor.id)
                result_string = ("    <testcase name=\"{}\" status=\"run\" "
                                 "time=\"0\" classname=\"Scenarios.{}\">\n".format(
                                     testcase_name, self._data.scenario_tree.name))
                if criterion.test_status != "SUCCESS":
                    result_string += "      <failure message=\"{}\"  type=\"\"><![CDATA[\n".format(
                        criterion.name)
                    result_string += "  Actual:   {}\n".format(
                        criterion.actual_value)
                    result_string += "  Expected: {}\n".format(
                        criterion.expected_value_success)
                    result_string += "\n"
                    result_string += "  Exact Value: {} = {}]]></failure>\n".format(
                        criterion.name, criterion.actual_value)
                else:
                    result_string += "  Exact Value: {} = {}\n".format(
                        criterion.name, criterion.actual_value)
                result_string += "    </testcase>\n"
                junit_file.write(result_string)

            # Handle timeout separately
            result_string = ("    <testcase name=\"Duration\" status=\"run\" time=\"{}\" "
                             "classname=\"Scenarios.{}\">\n".format(
                                 self._data.scenario_duration_system,
                                 self._data.scenario_tree.name))
            if self._data.scenario_duration_game >= self._data.scenario.timeout:
                result_string += "      <failure message=\"{}\"  type=\"\"><![CDATA[\n".format(
                    "Duration")
                result_string += "  Actual:   {}\n".format(
                    self._data.scenario_duration_game)
                result_string += "  Expected: {}\n".format(
                    self._data.scenario.timeout)
                result_string += "\n"
                result_string += "  Exact Value: {} = {}]]></failure>\n".format(
                    "Duration", self._data.scenario_duration_game)
            else:
                result_string += "  Exact Value: {} = {}\n".format(
                    "Duration", self._data.scenario_duration_game)
            result_string += "    </testcase>\n"
            junit_file.write(result_string)

            junit_file.write("  </testsuite>\n")
            junit_file.write("</testsuites>\n")
