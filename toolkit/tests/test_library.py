import json
import os
import sys
import unittest

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))

from bhutan_sim.library import DEFAULT_LIBRARY_PATH, generate_library, library_manifest, load_library  # noqa: E402
from bhutan_sim.scenario import ScenarioTemplate  # noqa: E402
from bhutan_sim.taxonomy import Taxonomy  # noqa: E402
from bhutan_sim import weather  # noqa: E402


class LibraryTests(unittest.TestCase):
    def test_generates_at_least_100_unique_templates(self):
        templates = generate_library()
        self.assertGreaterEqual(len(templates), 100)
        self.assertEqual(len({t.id for t in templates}), len(templates))

    def test_generation_is_deterministic(self):
        first = [t.content_hash() for t in generate_library(base_seed=7)]
        second = [t.content_hash() for t in generate_library(base_seed=7)]
        self.assertEqual(first, second)
        third = [t.content_hash() for t in generate_library(base_seed=8)]
        self.assertNotEqual(first, third)

    def test_covers_at_least_20_families_and_every_group(self):
        taxonomy = Taxonomy.load()
        self.assertGreaterEqual(len(taxonomy.families), 20)
        groups = {f.group for f in taxonomy.families}
        self.assertEqual(groups, set(taxonomy.groups))

    def test_templates_round_trip_through_json(self):
        for template in generate_library()[:10]:
            data = json.loads(template.to_json())
            restored = ScenarioTemplate.from_dict(data)
            self.assertEqual(restored.content_hash(), template.content_hash())
            self.assertEqual(restored.params.route_class, data["route_class"])

    def test_every_weather_preset_resolves(self):
        for template in generate_library():
            resolved = template.params.resolved_weather()
            for field in weather.WEATHER_FIELDS:
                self.assertIn(field, resolved)
            if template.params.time_of_day == "night":
                self.assertLess(resolved["sun_altitude_angle"], 0)

    def test_committed_library_matches_generator(self):
        committed = load_library(DEFAULT_LIBRARY_PATH)
        generated = generate_library()
        self.assertEqual([t.content_hash() for t in committed], [t.content_hash() for t in generated])

    def test_manifest_lists_families(self):
        taxonomy = Taxonomy.load()
        manifest = library_manifest(generate_library(taxonomy), taxonomy.version)
        self.assertEqual(manifest["count"], len(manifest["scenarios"]))
        self.assertEqual(set(manifest["families"]), {f.id for f in taxonomy.families})


if __name__ == "__main__":
    unittest.main()
