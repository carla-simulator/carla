#!/usr/bin/env python
import rostest
import sys
import unittest

from carla_ros_bridge.client import main

## A sample python unit test
class TestPublishedTopics(unittest.TestCase):
    def test_camera_topics(self):
        self.assertEquals(1, 1, "1!=1")



if __name__ == '__main__':
    rostest.rosrun(__name__, 'test_published_topics', TestPublishedTopics)
