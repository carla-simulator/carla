# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Run the actual HUD with native pygame fonts, without a CARLA server."""

import importlib.util
import os
from pathlib import Path
import sys
import types
import unittest
from unittest import mock

try:
    import pygame
    import pygame.sysfont
except ImportError:
    pygame = None


@unittest.skipUnless(pygame is not None, 'pygame is required for native font regressions')
class TestManualControlFonts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        pygame.font.init()
        path = Path(__file__).resolve().parents[2] / 'examples' / 'manual_control.py'
        spec = importlib.util.spec_from_file_location('_manual_control_fonts', path)
        cls.module = importlib.util.module_from_spec(spec)
        # HUD construction only needs this CARLA value object. Pygame's real
        # Font, text rendering, Surface and Clock implementations remain active.
        carla = types.ModuleType('carla')
        carla.ColorConverter = types.SimpleNamespace()
        carla.VehicleAckermannControl = lambda: None
        with mock.patch.dict(sys.modules, {'carla': carla}):
            spec.loader.exec_module(cls.module)

    def assert_hud_renders(self):
        hud = self.module.HUD(800, 600)
        surface = hud._font_mono.render('CARLA status 123', True, (255, 255, 255))
        self.assertGreater(surface.get_width(), 0)
        self.assertGreater(surface.get_height(), 0)
        self.assertIsInstance(hud.help.surface, pygame.Surface)
        return hud

    def test_missing_monospaced_fonts_use_the_bundled_font(self):
        with mock.patch.object(pygame.font, 'get_fonts', return_value=[]), \
                mock.patch.object(pygame.font, 'match_font') as lookup:
            self.assert_hud_renders()
        lookup.assert_not_called()

    @unittest.skipUnless(os.name == 'nt', 'The reported registry failure is Windows-specific')
    def test_non_string_windows_font_registry_value_uses_the_bundled_font(self):
        registry = types.SimpleNamespace(
            HKEY_LOCAL_MACHINE=1, HKEY_CURRENT_USER=2,
            OpenKey=lambda *_: object(), QueryInfoKey=lambda _: (0, 1, 0),
            EnumValue=lambda *_: ('Not a font filename', 1, 4))
        with mock.patch.object(pygame.sysfont, '_winreg', registry), \
                mock.patch.object(pygame.sysfont, 'Sysfonts', {}), \
                mock.patch.object(pygame.sysfont, 'Sysalias', {}):
            self.assert_hud_renders()

    def test_working_monospaced_font_choice_is_preserved(self):
        names = pygame.font.get_fonts()
        if not names:
            self.skipTest('No native system font is available for the selection control')
        name = names[0]
        filename = pygame.font.match_font(name)
        if filename is None:
            self.skipTest('The native system font could not be resolved')
        font_name = 'courier' if os.name == 'nt' else 'mono'
        with mock.patch.object(pygame.font, 'get_fonts', return_value=[font_name + 'fixture']), \
                mock.patch.object(pygame.font, 'match_font', return_value=filename) as lookup, \
                mock.patch.object(pygame.font, 'Font', wraps=pygame.font.Font) as create:
            self.assert_hud_renders()
        lookup.assert_called_once_with(font_name + 'fixture')
        self.assertEqual(create.call_args_list[1].args[0], filename)


if __name__ == '__main__':
    unittest.main()
