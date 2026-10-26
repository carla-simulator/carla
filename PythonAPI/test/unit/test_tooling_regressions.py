# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Tooling regressions with real files and isolated external-tool boundaries."""

import argparse
import contextlib
import http.server
import importlib.util
import io
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import tarfile
import threading
import types
import unittest
from unittest import mock


ROOT = Path(__file__).resolve().parents[3]


def load_tool(relative):
    path = ROOT / relative
    spec = importlib.util.spec_from_file_location(path.stem, path)
    module = importlib.util.module_from_spec(spec)
    if relative == "Util/Tools/Import.py":
        with mock.patch.dict(sys.modules, {"carla": types.ModuleType("carla")}):
            spec.loader.exec_module(module)
    else:
        spec.loader.exec_module(module)
    return module


IMPORTER = load_tool("Util/Tools/Import.py")
PACK = load_tool("Util/ContentPacks/carla_pack.py")
DOWNLOADER = load_tool("Util/download_from_gdrive.py")
OBJ_PATH = ROOT / "Util/DockerUtils/dist/addOBJ.py"


class ToolingTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="carla tooling ")
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)

    def write(self, name, data=b"data"):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data)
        return path


class TestImportTools(ToolingTest):
    def test_commandlet_propagates_posix_failure(self):
        # Only process launch is inert; exercise the complete production runner.
        failure = subprocess.CalledProcessError(7, "UnrealEditor")
        with mock.patch.dict(os.environ, {"CARLA_UNREAL_ENGINE_PATH": str(self.root)}), \
                mock.patch.object(IMPORTER.os, "name", "posix"), \
                mock.patch.object(IMPORTER.subprocess, "call", return_value=7), \
                mock.patch.object(IMPORTER.subprocess, "check_call", side_effect=failure):
            with self.assertRaises(subprocess.CalledProcessError):
                IMPORTER.invoke_commandlet("ImportAssets", ["-test=value with spaces"])

    def test_commandlet_keeps_arguments_separate(self):
        with mock.patch.dict(os.environ, {"CARLA_UNREAL_ENGINE_PATH": str(self.root)}), \
                mock.patch.object(IMPORTER.subprocess, "call") as call, \
                mock.patch.object(IMPORTER.subprocess, "check_call") as check:
            IMPORTER.invoke_commandlet("ImportAssets", ["-test=value with spaces"])
        launch = check.call_args if check.called else call.call_args
        self.assertEqual(launch.args[0][-1], "-test=value with spaces")
        self.assertFalse(launch.kwargs.get("shell", False))

    def _import_package(self, maps, navigation=None):
        descriptor = self.write("assets/Example.json", json.dumps({"maps": maps, "props": []}).encode())
        settings = []
        def commandlet(name, arguments):
            setting = next(arg for arg in arguments if arg.startswith("-importSettings="))
            path = setting.partition("=")[2].strip('"')
            settings.append(json.loads(Path(path).read_text()))
        with mock.patch.object(IMPORTER, "CARLA_ROOT_PATH", str(self.root)), \
                mock.patch.object(IMPORTER.os, "getcwd", return_value=str(self.root)), \
                mock.patch.object(IMPORTER, "invoke_commandlet", side_effect=commandlet), \
                mock.patch.object(IMPORTER, "build_binary_for_navigation", side_effect=navigation), \
                mock.patch.object(IMPORTER, "move_assets_commandlet"), \
                mock.patch.object(IMPORTER, "prepare_maps_commandlet_for_cooking"), \
                mock.patch.object(IMPORTER, "load_asset_materials_commandlet"), \
                mock.patch.object(IMPORTER, "build_binary_for_tm"):
            IMPORTER.import_assets_from_json_list([(str(descriptor.parent), descriptor.name)], 300)
        return settings

    def test_mixed_maps_keep_per_map_tiling_settings(self):
        self.write("assets/A.fbx")
        self.write("assets/B_Tile_0_0.fbx")
        self.write("assets/C_Tile_0_0.fbx")
        maps = [{"name": "A", "source": "A.fbx"},
                {"name": "B", "tiles": ["B_Tile_0_0.fbx"], "tile_size": 100},
                {"name": "C", "tiles": ["C_Tile_0_0.fbx"], "tile_size": 400}]
        settings = self._import_package(maps)
        groups = [group for setting in settings for group in setting["ImportGroups"]]
        observed = {group["DestinationPath"].split("/")[-1]: group["ImportSettings"]["StaticMeshImportData"]
                    for group in groups}
        self.assertEqual([observed[name]["bForceVerticesRelativeToTile"] for name in "ABC"], [0, 1, 1])
        self.assertEqual([observed[name]["TileSize"] for name in "BC"], [100, 400])
        package = self.root / "Unreal/CarlaUnreal/Content/Example/Config/Example.Package.json"
        self.assertEqual([item["name"] for item in json.loads(package.read_text())["maps"]], list("ABC"))

    def test_standalone_map_after_tiles_does_not_raise_key_error(self):
        self.write("assets/A_Tile_0_0.fbx")
        self.write("assets/B.fbx")
        self._import_package([{"name": "A", "tiles": ["A_Tile_0_0.fbx"]},
                              {"name": "B", "source": "B.fbx"}])

    def test_navigation_without_xodr_key_still_builds_geometry(self):
        commands = self._build_navigation(include_xodr=False, map_name="geometry")
        self.assertTrue(commands)
        nav = self.root / "Unreal/CarlaUnreal/Content/Example/Maps/geometry/Nav/geometry.bin"
        self.assertEqual(nav.read_bytes(), b"test navigation output")

    def test_empty_or_missing_xodr_file_still_builds_geometry(self):
        for xodr in ("", "not-present.xodr"):
            with self.subTest(xodr=xodr):
                commands = self._build_navigation(xodr=xodr, map_name="geometry")
                self.assertTrue(commands)
                nav = self.root / "Unreal/CarlaUnreal/Content/Example/Maps/geometry/Nav/geometry.bin"
                self.assertEqual(nav.read_bytes(), b"test navigation output")

    def test_navigation_worker_failure_reaches_caller(self):
        # The absent build-tools directory causes an actual filesystem failure
        # inside the navigation worker; joining a Thread used to hide it.
        self.write("assets/geometry.fbx")
        self.write("assets/roads.xodr")
        real_builder = IMPORTER.build_binary_for_navigation
        with self.assertRaises(OSError):
            self._import_package([{"name": "Town", "source": "geometry.fbx", "xodr": "roads.xodr"}],
                                 navigation=real_builder)

    def _build_navigation(self, xodr="roads.xodr", include_xodr=True, map_name="DeclaredTown"):
        self.write("assets/geometry.fbx")
        self.write("assets/roads.xodr")
        tools = self.root / "Util/DockerUtils/dist"
        tools.mkdir(parents=True, exist_ok=True)
        commands = []
        def builder(command, **kwargs):
            commands.append(command)
            (tools / "geometry.bin").write_bytes(b"test navigation output")
            return 0
        with mock.patch.object(IMPORTER, "CARLA_ROOT_PATH", str(self.root)), \
                mock.patch.object(IMPORTER.subprocess, "call", side_effect=builder), \
                mock.patch.object(IMPORTER.subprocess, "check_call", side_effect=builder):
            umap = {"name": map_name, "source": "geometry.fbx"}
            if include_xodr:
                umap["xodr"] = xodr
            IMPORTER.build_binary_for_navigation("Example", str(self.root / "assets"), [umap])
        return commands

    def test_navigation_output_uses_declared_map_name(self):
        self._build_navigation()
        nav = self.root / "Unreal/CarlaUnreal/Content/Example/Maps/DeclaredTown/Nav/DeclaredTown.bin"
        self.assertEqual(nav.read_bytes(), b"test navigation output")

    def test_navigation_builder_receives_xodr_stem(self):
        command = self._build_navigation()[-1]
        self.assertEqual(command[-1], "roads")

    def test_zero_batch_size_is_a_usage_error(self):
        with mock.patch.object(IMPORTER, "CARLA_ROOT_PATH", str(self.root)), \
                mock.patch.object(sys, "argv", ["Import.py", "--json-only", "--batch-size=0"]), \
                contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit) as result:
                IMPORTER.main()
        self.assertEqual(result.exception.code, 2)

    def test_direct_tiled_import_rejects_invalid_batch_size(self):
        for size in (0, -1, float("nan"), float("inf")):
            with self.subTest(size=size), self.assertRaises(argparse.ArgumentTypeError):
                IMPORTER.import_assets("Example", str(self.root), [], [{"name": "Town", "tiles": []}], 1, 2000, size)


class TestObjMerge(ToolingTest):
    def _merge(self, destination, source):
        target = self.write("result.obj", destination.encode())
        incoming = self.write("source.obj", source.encode())
        result = subprocess.run([sys.executable, str(OBJ_PATH), str(target), str(incoming)],
                                capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        return target.read_text()

    def test_unused_leading_vertices_do_not_shift_faces(self):
        result = self._merge("v 0 0 0\nv 1 0 0\nv 0 1 0\n",
                             "v 99 99 99\nv 0 0 1\nv 1 0 1\nv 0 1 1\nf 2 3 4\n")
        self.assertIn("f 5 6 7\n", result)

    def test_quad_face_keeps_all_its_vertices(self):
        result = self._merge("", "v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n")
        self.assertIn("f 1 2 3 4\n", result)

    def test_triangle_with_texture_indices_control(self):
        result = self._merge("v 0 0 0\n", "v 0 0 1\nv 1 0 1\nv 0 1 1\nf 1/1/1 2/2/1 3/3/1\n")
        self.assertIn("f 2 3 4\n", result)


class TestPackCopy(ToolingTest):
    def test_bad_second_catalog_leaves_no_unregistered_first_catalog(self):
        pack = self.root / "Pack"
        self.write("Pack/carla-pack.json", json.dumps(PACK.new_manifest("Pack", "1.0.0", "0.10.0")).encode())
        props = self.write("props.json", json.dumps({"props": []}).encode())
        vehicles = self.write("vehicles.json", json.dumps({"Vehicles": [{}]}).encode())
        original = (pack / "carla-pack.json").read_bytes()
        with contextlib.redirect_stderr(io.StringIO()):
            result = PACK.main(["add", str(pack), "--props", str(props), "--vehicles", str(vehicles)])
        self.assertEqual(result, 1)
        self.assertEqual((pack / "carla-pack.json").read_bytes(), original)
        self.assertFalse((pack / "Content/Config/Pack.Package.json").exists())

    def test_tar_manifest_is_found_on_host_platform(self):
        manifest = self.write("Source/Pack/carla-pack.json",
                              json.dumps(PACK.new_manifest("Pack", "1.0.0", "0.10.0")).encode())
        archive = self.root / "Pack.tar.gz"
        with tarfile.open(archive, "w:gz") as tar:
            tar.add(manifest, arcname="Pack/carla-pack.json")
        result, top, sizes = PACK.read_manifest_from_tar(archive)
        self.assertEqual(result["name"], "Pack")
        self.assertEqual(top, "Pack")
        self.assertIn("carla-pack.json", sizes)

    def test_absolute_destination_cannot_escape_pack_content(self):
        source = self.write("mesh.uasset")
        outside = self.root / "outside/mesh.uasset"
        try:
            with self.assertRaises(PACK.PackError):
                PACK.add_asset(self.root / "Pack", {}, source, str(outside))
        finally:
            self.assertFalse(outside.exists())

    def test_copying_directory_onto_itself_preserves_source(self):
        source = self.write("tree/mesh.uasset")
        PACK.copy_any(source.parent, source.parent)
        self.assertEqual(source.read_bytes(), b"data")

    def test_copy_rejects_destination_containing_source(self):
        source = self.write("tree/nested/mesh.uasset")
        with self.assertRaises(PACK.PackError):
            PACK.copy_any(source.parent, self.root / "tree")
        self.assertEqual(source.read_bytes(), b"data")

    def test_relative_asset_destination_control(self):
        source = self.write("mesh.uasset")
        for destination, expected in [("Static/", "Static"), ("./Other/", "Other"), (".", ".")]:
            with self.subTest(destination=destination):
                PACK.add_asset(self.root / "Pack", {}, source, destination)
                self.assertEqual((self.root / "Pack/Content" / expected / "mesh.uasset").read_bytes(), b"data")

    def _check_dotted_directory(self, separator):
        pack = self.root / "Pack"
        manifest = self.write("Pack/carla-pack.json", json.dumps(PACK.new_manifest("Pack", "1.0.0", "0.10.0")).encode())
        first = self.write("first.uasset", b"first asset")
        second = self.write("second.uasset", b"second asset")
        original_manifest = manifest.read_bytes()
        result = PACK.main(["add", str(pack), "--asset", str(first), "--asset", str(second),
                            "--dest", "Version.1" + separator])
        self.assertEqual(result, 0)
        destination = pack / "Content/Version.1"
        self.assertTrue(destination.is_dir())
        self.assertEqual((destination / first.name).read_bytes(), b"first asset")
        self.assertEqual((destination / second.name).read_bytes(), b"second asset")
        self.assertEqual(first.read_bytes(), b"first asset")
        self.assertEqual(second.read_bytes(), b"second asset")
        self.assertEqual(json.loads(manifest.read_bytes()), json.loads(original_manifest))

    def test_dotted_directory_windows_separator_keeps_both_assets(self):
        self._check_dotted_directory("\\")

    def test_dotted_directory_forward_separator_keeps_both_assets(self):
        self._check_dotted_directory("/")


class _DownloadHandler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        body = b"asset data" if self.path == "/ok" else b"x" * 40000
        self.send_response(404 if self.path == "/error" else 200)
        if self.path == "/short":
            self.send_header("Transfer-Encoding", "chunked")
        else:
            self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        if self.path == "/short":
            self.wfile.write(b"13880\r\n")  # promise 80000 bytes, then disconnect halfway
        self.wfile.write(body)

    def log_message(self, *args):
        pass


class TestDriveDownload(ToolingTest):
    def _download(self, mode, destination):
        # Real requests and HTTP responses from a loopback server; no Google call.
        import requests
        server = http.server.ThreadingHTTPServer(("127.0.0.1", 0), _DownloadHandler)
        worker = threading.Thread(target=server.serve_forever)
        worker.start()
        real_get = requests.get
        def get(*args, **kwargs):
            return real_get("http://127.0.0.1:%s/%s" % (server.server_port, mode), stream=True)
        try:
            with mock.patch.object(DOWNLOADER.requests.Session, "get", side_effect=get), \
                    contextlib.redirect_stdout(io.StringIO()):
                DOWNLOADER.download_file_from_google_drive("unused", destination)
        finally:
            server.shutdown()
            server.server_close()
            worker.join()

    def test_http_error_is_not_saved_as_an_asset(self):
        import requests
        destination = self.root / "download.bin"
        with self.assertRaises(requests.HTTPError):
            self._download("error", destination)
        self.assertFalse(destination.exists())

    def test_interrupted_download_preserves_existing_file(self):
        import requests
        destination = self.write("download.bin", b"previous asset")
        with self.assertRaises(requests.RequestException):
            self._download("short", destination)
        self.assertEqual(destination.read_bytes(), b"previous asset")
        self.assertEqual(list(self.root.iterdir()), [destination])

    def test_successful_download_control(self):
        destination = self.write("download.bin", b"previous asset")
        self._download("ok", destination)
        self.assertEqual(destination.read_bytes(), b"asset data")


if __name__ == "__main__":
    unittest.main()
