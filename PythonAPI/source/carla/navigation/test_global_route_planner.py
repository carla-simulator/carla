import math
import unittest

import carla

from global_route_planner import GlobalRoutePlanner
from global_route_planner import NavEnum
from global_route_planner_dao import GlobalRoutePlannerDAO


class Test_GlobalRoutePlanner(unittest.TestCase):
    """
    Test class for GlobalRoutePlanner class
    """

    def setUp(self):
        # == Utilities test instance without DAO == #
        self.simple_grp = GlobalRoutePlanner(None)

        # == Integration test instance == #
        client = carla.Client('localhost', 2000)
        world = client.get_world()
        integ_dao = GlobalRoutePlannerDAO(world.get_map())
        self.integ_grp = GlobalRoutePlanner(integ_dao)
        self.integ_grp.setup()
        pass

    def tearDown(self):
        self.simple_grp = None
        self.dao_grp = None
        self.integ_grp = None
        pass

    def test_plan_route(self):
        """
        Test for GlobalROutePlanner.plan_route()
        Run this test with carla server running Town03
        """
        plan = self.integ_grp.plan_route((-60, -5), (-77.65, 72.72))
        self.assertEqual(
            plan, [NavEnum.START, NavEnum.LEFT, NavEnum.LEFT,
                   NavEnum.GO_STRAIGHT, NavEnum.LEFT, NavEnum.STOP])

    def test_path_search(self):
        """
        Test for GlobalRoutePlanner.path_search()
        Run this test with carla server running Town03
        """
        self.integ_grp.path_search((191.947, -5.602), (78.730, -50.091))
        self.assertEqual(
            self.integ_grp.path_search((196.947, -5.602), (78.730, -50.091)),
            [256, 157, 158, 117, 118, 59, 55, 230])

    def test_localise(self):
        """
        Test for GlobalRoutePlanner.localise()
        Run this test with carla server running Town03
        """
        x, y = (200, -250)
        segment = self.integ_grp.localise(x, y)
        self.assertEqual(self.integ_grp._id_map[segment['entry']], 5)
        self.assertEqual(self.integ_grp._id_map[segment['exit']], 225)

    def test_unit_vector(self):
        """
        Test for GlobalROutePlanner.unit_vector()
        """
        vector = self.simple_grp.unit_vector((1, 1), (2, 2))
        self.assertAlmostEquals(vector[0], 1 / math.sqrt(2))
        self.assertAlmostEquals(vector[1], 1 / math.sqrt(2))

    def test_dot(self):
        """
        Test for GlobalROutePlanner.test_dot()
        """
        self.assertAlmostEqual(self.simple_grp.dot((1, 0), (0, 1)), 0)
        self.assertAlmostEqual(self.simple_grp.dot((1, 0), (1, 0)), 1)


def suite():
    """
    Gathering all tests
    """

    suite = unittest.TestSuite()
    suite.addTest(Test_GlobalRoutePlanner('test_unit_vector'))
    suite.addTest(Test_GlobalRoutePlanner('test_dot'))
    suite.addTest(Test_GlobalRoutePlanner('test_localise'))
    suite.addTest(Test_GlobalRoutePlanner('test_path_search'))
    suite.addTest(Test_GlobalRoutePlanner('test_plan_route'))

    return suite


if __name__ == '__main__':
    """
    Running test suite
    """
    mySuit = suite()
    runner = unittest.TextTestRunner()
    runner.run(mySuit)
