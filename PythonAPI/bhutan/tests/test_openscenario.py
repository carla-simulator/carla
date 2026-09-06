import os
import re
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.library import generate_library  # noqa: E402
from bhutan_sim.openscenario import scenario_to_xosc  # noqa: E402
from bhutan_sim.taxonomy import Taxonomy  # noqa: E402


class OpenScenarioTests(unittest.TestCase):
    def test_every_template_exports_well_formed_xosc(self):
        templates = generate_library(Taxonomy.load())
        for template in templates[:15]:
            xml = scenario_to_xosc(template)
            self.assertIn("<OpenSCENARIO", xml)
            self.assertIn('revMajor="1" revMinor="2"', xml)
            opened = len(re.findall(r"<([A-Za-z]+)[\s>/]", xml))
            closed = len(re.findall(r"</([A-Za-z]+)>", xml)) + len(re.findall(r"/>", xml))
            self.assertEqual(opened, closed, template.id)

    def test_actors_weather_and_content_are_present(self):
        templates = generate_library(Taxonomy.load())
        template = next(t for t in templates if t.family == "landslide_debris")
        xml = scenario_to_xosc(template)
        self.assertIn("hero", xml)
        for spec in template.actors:
            self.assertIn(spec.role, xml)
        self.assertIn(template.content_hash(), xml)
        self.assertIn("<Precipitation", xml)

    def test_special_characters_are_escaped(self):
        templates = generate_library(Taxonomy.load())
        template = templates[0]
        template.description = 'Debris & "rocks" <on> the road'
        xml = scenario_to_xosc(template)
        self.assertIn("&amp;", xml)
        self.assertIn("&quot;", xml)
        self.assertIn("&lt;", xml)


if __name__ == "__main__":
    unittest.main()
