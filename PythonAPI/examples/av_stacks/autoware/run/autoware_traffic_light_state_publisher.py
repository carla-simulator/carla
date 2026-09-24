#!/usr/bin/env python3
"""Publish CARLA ground-truth traffic-light states to Autoware's external input."""

import argparse
import json
import signal
import time

import rclpy
from autoware_perception_msgs.msg import (
    TrafficLightElement,
    TrafficLightGroup,
    TrafficLightGroupArray,
)
from rclpy.node import Node

# Wire-contract color codes shared with carla_traffic_light_state_source.py (a
# plain CARLA client that cannot import this ROS message package). Sourced
# directly from the message definition so this side is always correct
# regardless of the enum's actual values.
TL_UNKNOWN = TrafficLightElement.UNKNOWN
TL_RED = TrafficLightElement.RED
TL_AMBER = TrafficLightElement.AMBER
TL_GREEN = TrafficLightElement.GREEN

# Wire integer (as written by carla_traffic_light_state_source.py) -> real
# enum value. Routes every color through an explicit, fail-safe lookup
# instead of trusting the JSON integer directly: an unrecognized wire value
# falls back to TL_UNKNOWN rather than forwarding a nonsense color.
WIRE_COLOR_TO_ELEMENT = {0: TL_UNKNOWN, 1: TL_RED, 2: TL_AMBER, 3: TL_GREEN}

PUBLISH_PERIOD_SEC = 0.1  # 10 Hz


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", required=True)
    # Default assumes max_age = 5 * carla_traffic_light_state_source.py's
    # WRITE_INTERVAL_SEC (0.2s), leaving margin so only a real tick stall
    # trips the stale-UNKNOWN fail-safe. Keep the two in sync.
    parser.add_argument("--max-age", type=float, default=1.0)
    return parser.parse_args()


def unknown_element():
    element = TrafficLightElement()
    element.color = TL_UNKNOWN
    element.shape = TrafficLightElement.CIRCLE
    element.status = TrafficLightElement.SOLID_OFF
    element.confidence = 1.0
    return element


class StatePublisher(Node):
    def __init__(self, input_path, max_age):
        super().__init__(
            "carla_traffic_light_state_publisher",
            parameter_overrides=[rclpy.parameter.Parameter("use_sim_time", value=True)],
        )
        self.input_path = input_path
        self.max_age = max_age
        self.known_relation_ids = set()
        self.publisher = self.create_publisher(
            TrafficLightGroupArray,
            "/perception/traffic_light_recognition/external/traffic_signals",
            10,
        )
        self.create_timer(PUBLISH_PERIOD_SEC, self.publish_snapshot)

    def publish_unknown_for_known_relations(self):
        # Read failure with no prior successful read: nothing is known yet
        # (source not started), skip this one publish rather than fabricate
        # an empty array.
        if not self.known_relation_ids:
            return
        message = TrafficLightGroupArray()
        message.stamp = self.get_clock().now().to_msg()
        for relation_id in self.known_relation_ids:
            group = TrafficLightGroup()
            group.traffic_light_group_id = relation_id
            group.elements = [unknown_element()]
            message.traffic_light_groups.append(group)
        self.publisher.publish(message)

    def publish_snapshot(self):
        try:
            with open(self.input_path, encoding="utf-8") as stream:
                payload = json.load(stream)
        except (FileNotFoundError, json.JSONDecodeError, OSError):
            # The source process dying (and possibly cleaning up its output
            # file) is exactly when the fail-safe must still fire: publish
            # UNKNOWN for every relation last seen instead of publishing
            # nothing.
            self.publish_unknown_for_known_relations()
            return

        # Fail-safe: a stale snapshot (source died, matched actor gone, etc.)
        # must not leave stale colors on the topic, since it isn't latched and
        # subscribers otherwise keep whatever they last received forever.
        stale = time.time() - payload.get("written_at", 0.0) > self.max_age

        message = TrafficLightGroupArray()
        message.stamp = self.get_clock().now().to_msg()
        current_relation_ids = set()
        for signal_entry in payload.get("signals", []):
            try:
                relation_id = int(signal_entry["relation_id"])
            except (KeyError, TypeError, ValueError) as exc:
                self.get_logger().warning(
                    f"skipping signal entry with unparseable relation_id {signal_entry!r}: {exc}"
                )
                continue
            current_relation_ids.add(relation_id)

            color = None
            if not stale:
                try:
                    color = int(signal_entry["color"])
                except (KeyError, TypeError, ValueError) as exc:
                    self.get_logger().warning(
                        f"relation {relation_id}: unparseable color in {signal_entry!r}, "
                        f"reporting UNKNOWN: {exc}"
                    )

            group = TrafficLightGroup()
            group.traffic_light_group_id = relation_id
            if color is None:
                element = unknown_element()
            else:
                element = TrafficLightElement()
                element.color = WIRE_COLOR_TO_ELEMENT.get(color, TL_UNKNOWN)
                element.shape = TrafficLightElement.CIRCLE
                element.status = (
                    TrafficLightElement.SOLID_ON
                    if element.color != TL_UNKNOWN
                    else TrafficLightElement.SOLID_OFF
                )
                element.confidence = 1.0
            group.elements = [element]
            message.traffic_light_groups.append(group)

        self.known_relation_ids = current_relation_ids
        self.publisher.publish(message)


def main():
    args = parse_args()
    rclpy.init()
    node = StatePublisher(args.input, args.max_age)

    stop = {"flag": False}

    def handle_signal(signum, _frame):
        print(
            f"autoware_traffic_light_state_publisher.py: received signal {signum}, "
            "shutting down"
        )
        stop["flag"] = True

    signal.signal(signal.SIGTERM, handle_signal)
    signal.signal(signal.SIGINT, handle_signal)

    try:
        while not stop["flag"]:
            rclpy.spin_once(node, timeout_sec=0.5)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
