#!/usr/bin/env python3
"""Unit tests for carla_pack.py (standard library only, no engine needed).

    python3 Util/ContentPacks/test_carla_pack.py
"""

import contextlib
import io
import json
import os
import shutil
import sys
import tarfile
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import carla_pack as cp  # noqa: E402


def run(*argv):
    """Run the CLI, returning (exit_code, stdout, stderr)."""
    out, err = io.StringIO(), io.StringIO()
    with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
        rc = cp.main([str(a) for a in argv])
    return rc, out.getvalue(), err.getvalue()


def write(path, data: "bytes | str" = b"x"):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(data, str):
        data = data.encode("utf-8")
    path.write_bytes(data)
    return path


PACK = "TestPack"
RELEASE = "carla-0.10.0-Linux"

VEHICLES = {"Vehicles": [{
    "Make": "Acme", "Model": "Rover",
    "Class": "/TestPack/Blueprints/Vehicles/BP_Rover.BP_Rover_C",
    "NumberOfWheels": 4, "Generation": 3, "ObjectType": "", "BaseType": "car",
    "SpecialType": "", "HasDynamicDoors": False, "HasLights": True,
    "RecommendedColors": [{"R": 0, "G": 0, "B": 0, "A": 0}], "SupportedDrivers": []}]}

PROPS_PACKAGE = {"maps": [{"path": "/TestPack/Maps/TestTown", "name": "TestTown",
                           "use_carla_materials": True}],
                 "props": [{"name": "Cone", "path": "/TestPack/Static/Static/SM_Cone.SM_Cone", "size": "Small"},
                           {"name": "Alien", "path": "/Other/SM_X.SM_X", "size": "Small"}]}

WALKERS = {"Walkers": [{"Id": "9001", "Class": "/TestPack/Blueprints/Walkers/BP_W.BP_W_C",
                        "Gender": "Female", "Age": "Adult",
                        "Speeds": [{"Speed": 0}, {"Speed": 1.7}], "Generation": 2}]}


class PackTestBase(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="carla-pack-test-"))
        self.project_dir = self.tmp / "Unreal" / "CarlaUnreal"
        self.project = write(self.project_dir / "CarlaUnreal.uproject", "{}")
        self.root = self.project_dir / "Plugins" / "Packs"
        self.engine = self.tmp / "Engine"
        write(self.engine / "Engine" / "Build" / "BatchFiles" / "RunUAT.sh", "#!/bin/sh\n")
        write(self.engine / "Engine" / "Build" / "Build.version",
              json.dumps({"MajorVersion": 5, "MinorVersion": 8, "PatchVersion": 0, "Changelist": 0}))
        # release metadata: both a dir and a tarball with <release>/Linux/... members
        self.releases = self.tmp / "Releases"
        write(self.releases / RELEASE / "Linux" / "AssetRegistry.bin", b"base-registry")
        write(self.releases / RELEASE / "Linux" / "Metadata" / "PakList.txt", "pak")
        self.base_tar = self.tmp / (RELEASE + "-release-metadata.tar.gz")
        with tarfile.open(str(self.base_tar), "w:gz") as tar:
            tar.add(str(self.releases / RELEASE), arcname=RELEASE)
        # authored content
        src = self.tmp / "authored"
        self.umap = write(src / "Content" / "Maps" / "TestTown.umap", b"umap")
        write(src / "Content" / "Maps" / "TestTown_BuiltData.uasset", b"built")
        write(src / "Content" / "__ExternalActors__" / "Maps" / "TestTown" / "A" / "B.uasset", b"ea")
        write(src / "Content" / "__ExternalObjects__" / "Maps" / "TestTown" / "C.uasset", b"eo")
        self.xodr = write(src / "TestTown.xodr", "<OpenDRIVE/>")
        self.nav = write(src / "TestTown.bin", b"nav")
        self.vehicles = write(src / "vehicles.json", json.dumps(VEHICLES))
        self.props = write(src / "props.json", json.dumps(PROPS_PACKAGE))
        self.walkers = write(src / "walkers.json", json.dumps(WALKERS))
        self.mesh = write(src / "SM_Cone.uasset", b"mesh")
        self.pack_dir = self.root / PACK

    def tearDown(self):
        shutil.rmtree(str(self.tmp), ignore_errors=True)

    def author_in_pack(self):
        """What the editor does: the map is saved/duplicated into the pack's own content root."""
        c = self.pack_dir / "Content"
        umap = write(c / "Maps" / "TestTown.umap", b"umap")
        write(c / "Maps" / "TestTown_BuiltData.uasset", b"built")
        write(c / "__ExternalActors__" / "Maps" / "TestTown" / "A" / "B.uasset", b"ea")
        write(c / "__ExternalObjects__" / "Maps" / "TestTown" / "C.uasset", b"eo")
        return umap

    def init_and_add(self):
        rc, out, err = run("init", PACK, "--project", self.project, "--carla-version", "0.10.0")
        self.assertEqual(rc, 0, err)
        umap = self.author_in_pack()
        rc, out, err = run("add", PACK, "--project", self.project, "--map", umap,
                           "--xodr", self.xodr, "--nav", self.nav)
        self.assertEqual(rc, 0, err)
        self.assertIn("already in the pack; registering", out)
        rc, out, err = run("add", PACK, "--project", self.project, "--props", self.props,
                           "--vehicles", self.vehicles, "--walkers", self.walkers)
        self.assertEqual(rc, 0, err)
        catalog_err = err
        rc, out, err = run("add", PACK, "--project", self.project, "--asset", self.mesh,
                           "--dest", "Static/")
        self.assertEqual(rc, 0, err)
        return catalog_err

    def fake_staged(self, staged_root, sidecars=True, registry=True):
        """Emulate UAT's verified DLC stage (spike REPORT.md Task 3):
        <staged>/Linux/CarlaUnreal/Plugins/Packs/<Pack>/{<Pack>.uplugin, Content/Paks/Linux/<Pack>CarlaUnreal-Linux.*,
        loose sidecars}, <staged>/Linux/CarlaUnreal/Plugins/<Pack>.upluginmanifest, and the cooked dir
        <pack>/Saved/Cooked/Linux/CarlaUnreal/Plugins/Packs/<Pack>/{AssetRegistry.bin, Metadata/...}."""
        pak_base = "{}CarlaUnreal-Linux".format(PACK)
        plugin = staged_root / "Linux" / "CarlaUnreal" / "Plugins" / "Packs" / PACK
        for ext in (".pak", ".utoc", ".ucas"):
            write(plugin / "Content" / "Paks" / "Linux" / (pak_base + ext), ("data" + ext).encode())
        write(plugin / (PACK + ".uplugin"), "{}")
        write(plugin.parent.parent / (PACK + ".upluginmanifest"), "{}")
        write(staged_root / "Linux" / "Manifest_NonUFSFiles_Linux.txt", "x")
        if sidecars:
            write(plugin / "Content" / "Config" / "TestPack.Package.json", "{staged}")
            write(plugin / "Content" / "Config" / "Vehicles.json", "{staged}")
            write(plugin / "Content" / "Config" / "WalkerParameters.json", "{staged}")
            write(plugin / "Content" / "Maps" / "OpenDrive" / "TestTown.xodr", "<staged/>")
            write(plugin / "Content" / "Maps" / "Nav" / "TestTown.bin", b"staged-nav")
        cooked = self.pack_dir / "Saved" / "Cooked" / "Linux" / "CarlaUnreal" / "Plugins" / "Packs" / PACK
        if registry:
            write(cooked / "AssetRegistry.bin", b"cooked-registry")
        write(cooked / "Metadata" / "DevelopmentAssetRegistry.bin", b"dev-registry")
        return plugin, cooked

    def build_fake(self, out_dir=None, base=None, **extra):
        staged = self.tmp / "Staged"
        plugin, cooked = self.fake_staged(staged)
        out_dir = out_dir or self.tmp / "out"
        argv = ["build", PACK, "--project", self.project, "--base", base or self.base_tar,
                "--engine", self.engine, "--skip-cook", "--staged", staged, "--out", out_dir]
        for k, v in extra.items():
            argv += ["--" + k.replace("_", "-"), v]
        rc, out, err = run(*argv)
        self.assertEqual(rc, 0, out + err)
        tar_path = out_dir / "{}-1.0.0-{}.tar.gz".format(PACK, RELEASE)
        return out_dir / PACK, tar_path, cooked

    def fake_server(self, release: "str | None" = RELEASE):
        """<Package>/Linux/CarlaUnreal layout with BaseRelease at both stamp locations."""
        server = self.tmp / "Package" / "Linux"
        write(server / "CarlaUnreal.sh", "#!/bin/sh\n")
        if release is not None:
            write(server / "CarlaUnreal" / "BaseRelease", release + "\n")
            write(server.parent / "BaseRelease", release + "\n")
        else:
            (server / "CarlaUnreal").mkdir(parents=True, exist_ok=True)
        return server


class TestInitAdd(PackTestBase):
    def test_init_creates_skeleton_uplugin_and_manifest(self):
        rc, out, err = run("init", PACK, "--project", self.project, "--carla-version", "0.10.0",
                           "--description", "test pack")
        self.assertEqual(rc, 0, err)
        for sub in cp.CONTENT_SUBDIRS:
            self.assertTrue((self.pack_dir / "Content" / sub).is_dir(), sub)
        self.assertTrue((self.pack_dir / "Content" / "Static" / "Static").is_dir())
        up = json.loads((self.pack_dir / (PACK + ".uplugin")).read_text())
        self.assertEqual(up["FileVersion"], 3)
        self.assertEqual(up["Version"], 1)
        self.assertEqual(up["VersionName"], "1.0.0")
        self.assertEqual(up["FriendlyName"], PACK)
        self.assertEqual(up["Description"], "test pack")
        self.assertEqual(up["Category"], "CARLA Content Packs")
        self.assertTrue(up["CanContainContent"])
        self.assertTrue(up["ExplicitlyLoaded"])  # authored convention; build flips it for the cook
        self.assertFalse(up["EnabledByDefault"])
        self.assertNotIn("Installed", up)      # editor-only marketplace flag, not the tool's business
        self.assertNotIn("Modules", up)
        ini = (self.pack_dir / "Config" / "PluginSettings.ini").read_text()
        self.assertIn("[StageSettings]", ini)
        for d in ("Content/Config", "Content/Maps/OpenDrive", "Content/Maps/Nav", "Content/Maps/TM"):
            self.assertIn("+AdditionalNonUSFDirectories=" + d + "\n", ini)
        m = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertEqual(m["name"], PACK)
        self.assertEqual(m["version"], "1.0.0")
        self.assertEqual(m["carla_version"], "0.10.0")
        self.assertEqual(m["base_release"], "")
        self.assertEqual(m["engine"], {"version": "", "commit": ""})
        self.assertEqual(m["platform"], "Linux")
        self.assertEqual((m["maps"], m["catalogs"], m["files"]), ([], [], {}))

    def test_init_rejects_bad_name_and_duplicates(self):
        rc, _, err = run("init", "9bad-name", "--project", self.project)
        self.assertEqual(rc, 1)
        self.assertIn("invalid pack name", err)
        write(self.project_dir / "Plugins" / "Carla" / "Carla.uplugin", "{}")
        write(self.project_dir / "Plugins" / "Nested" / "MyTool" / "MyTool.uplugin", "{}")
        for name in ("Game", "engine", "Carla", "Script", "Memory", "Temp", "Config", "Paks", "Packs", "MyTool", "mytool"):
            rc, _, err = run("init", name, "--project", self.project)
            self.assertEqual(rc, 1, name)
            self.assertIn("reserved name", err)
            self.assertFalse((self.root / name).exists())
        self.assertEqual(run("init", PACK, "--project", self.project)[0], 0)
        rc, _, err = run("init", PACK, "--project", self.project)
        self.assertEqual(rc, 1)
        self.assertIn("already exists", err)

    def test_add_map_and_catalogs_updates_layout_and_manifest(self):
        err = self.init_and_add()
        c = self.pack_dir / "Content"
        self.assertEqual((c / "Maps" / "TestTown.umap").read_bytes(), b"umap")
        self.assertTrue((c / "Maps" / "TestTown_BuiltData.uasset").is_file())
        self.assertTrue((c / "__ExternalActors__" / "Maps" / "TestTown" / "A" / "B.uasset").is_file())
        self.assertTrue((c / "__ExternalObjects__" / "Maps" / "TestTown" / "C.uasset").is_file())
        self.assertTrue((c / "Maps" / "OpenDrive" / "TestTown.xodr").is_file())
        self.assertTrue((c / "Maps" / "Nav" / "TestTown.bin").is_file())
        self.assertTrue((c / "Config" / "TestPack.Package.json").is_file())
        self.assertTrue((c / "Config" / "Vehicles.json").is_file())
        self.assertTrue((c / "Config" / "WalkerParameters.json").is_file())
        self.assertTrue((c / "Static" / "SM_Cone.uasset").is_file())
        m = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertEqual(m["maps"], [{"name": "TestTown", "package": "/TestPack/Maps/TestTown",
                                      "xodr": "Maps/OpenDrive/TestTown.xodr", "world_partition": True,
                                      "nav": "Maps/Nav/TestTown.bin"}])
        self.assertEqual(m["catalogs"], ["Config/TestPack.Package.json", "Config/Vehicles.json",
                                         "Config/WalkerParameters.json"])
        # the foreign /Other/ mesh path is warned about, the labelled /TestPack/ cone is not
        self.assertIn("/Other/SM_X.SM_X", err)
        self.assertNotIn("SM_Cone", err)

    def test_add_map_twice_does_not_duplicate(self):
        self.init_and_add()
        rc, _, err = run("add", PACK, "--project", self.project, "--map",
                         self.pack_dir / "Content" / "Maps" / "TestTown.umap", "--xodr", self.xodr)
        self.assertEqual(rc, 0, err)
        m = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertEqual(len(m["maps"]), 1)

    FAKE_EDITOR = """#!/bin/sh
# stand-in for UnrealEditor-Cmd: 'Save As' the map into the pack (external actors too),
# re-save the source like the real editor does, record the command line and report ok
set -e
env | grep ^CARLA_PACK_IMPORT_ > "$CARLA_PACK_IMPORT_CONTENT/../Saved/fake-editor.env"
echo "$@" >> "$CARLA_PACK_IMPORT_CONTENT/../Saved/fake-editor.env"
mkdir -p "$(dirname "$CARLA_PACK_IMPORT_DST_FILE")"
cp "$CARLA_PACK_IMPORT_SRC_FILE" "$CARLA_PACK_IMPORT_DST_FILE"
rel=$(dirname "${CARLA_PACK_IMPORT_DST_FILE#$CARLA_PACK_IMPORT_CONTENT/}")
name=$(basename "$CARLA_PACK_IMPORT_DST_FILE" .umap)
mkdir -p "$CARLA_PACK_IMPORT_CONTENT/__ExternalActors__/$rel/$name/0"
echo ea > "$CARLA_PACK_IMPORT_CONTENT/__ExternalActors__/$rel/$name/0/1.uasset"
echo resaved >> "$CARLA_PACK_IMPORT_SRC_FILE"
echo '{"ok": true, "load": true, "save": true, "seconds": 1.0, "error": ""}' > "$CARLA_PACK_IMPORT_RESULT"
"""

    def fake_editor(self, script=None):
        editor = self.engine / "Engine" / "Binaries" / "Linux" / "UnrealEditor-Cmd"
        write(editor, script or self.FAKE_EDITOR)
        editor.chmod(0o755)
        return editor

    def existing_town(self, name="Town12"):
        """A CARLA town as the content repo lays it out: Content/Carla/Maps/<T>/<T>.umap, external
        actors under Content/Carla/__ExternalActors__/Carla/Maps/<T>/, xodr and TM inside the map
        folder, navigation under Maps/Nav."""
        carla = self.project_dir / "Content" / "Carla"
        umap = write(carla / "Maps" / name / (name + ".umap"), b"town")   # nested, like Town12/Town13
        write(carla / "Maps" / name / "Asphalt_Diff.uasset", b"tex")
        write(carla / "__ExternalActors__" / "Carla" / "Maps" / name / "A" / "B.uasset", b"ea")
        write(carla / "Maps" / name / "OpenDrive" / (name + ".xodr"), "<OpenDRIVE/>")
        write(carla / "Maps" / name / "TM" / (name + ".bin"), b"tm")
        write(carla / "Maps" / "Nav" / (name + ".bin"), b"nav")
        return umap

    def test_add_map_imports_an_existing_town_with_the_editor(self):
        run("init", PACK, "--project", self.project)
        town = self.existing_town()
        self.fake_editor()
        rc, out, err = run("add", PACK, "--project", self.project, "--engine", self.engine, "--map", "Town12",
                           "--import")
        self.assertEqual(rc, 0, err)
        c = self.pack_dir / "Content"
        # the editor was asked for a Save As of the /Game package into the pack, with the plugin enabled
        env = (self.pack_dir / "Saved" / "fake-editor.env").read_text()
        self.assertIn("CARLA_PACK_IMPORT_SRC=/Game/Carla/Maps/Town12/Town12\n", env)
        self.assertIn("CARLA_PACK_IMPORT_DST=/TestPack/Maps/Town12/Town12\n", env)
        self.assertIn("-run=pythonscript", env)
        self.assertIn("-EnablePlugins=TestPack", env)
        self.assertIn(str(self.project), env)
        # nested CARLA layout kept, external actors found, sidecars picked up without flags
        self.assertEqual((c / "Maps" / "Town12" / "Town12.umap").read_bytes(), b"town")
        self.assertTrue((c / "__ExternalActors__" / "Maps" / "Town12" / "Town12" / "0" / "1.uasset").is_file())
        self.assertTrue((c / "Maps" / "OpenDrive" / "Town12.xodr").is_file())
        self.assertTrue((c / "Maps" / "TM" / "Town12" / "Town12.bin").is_file())
        self.assertTrue((c / "Maps" / "Nav" / "Town12.bin").is_file())
        m = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertEqual(m["maps"], [{"name": "Town12", "package": "/TestPack/Maps/Town12/Town12",
                                      "xodr": "Maps/OpenDrive/Town12.xodr", "world_partition": True,
                                      "nav": "Maps/Nav/Town12.bin", "tm": "Maps/TM/Town12"}])
        # the source map the editor re-saved is byte-for-byte back, the descriptor too
        self.assertEqual(town.read_bytes(), b"town")
        self.assertIn("restored", out)
        self.assertTrue(json.loads((self.pack_dir / "TestPack.uplugin").read_text())["ExplicitlyLoaded"])
        self.assertTrue((self.pack_dir / "Saved" / "CarlaPack" / "import-Town12.log").is_file())
        # importing a registered map again is refused rather than silently overwriting the pack's copy
        rc, out, err = run("add", PACK, "--project", self.project, "--engine", self.engine,
                           "--map", "/Game/Carla/Maps/Town12/Town12", "--import")
        self.assertEqual(rc, 1)
        self.assertIn("already in the pack as /TestPack/Maps/Town12/Town12", err)
        # the pack's own copy registers without the editor
        rc, out, err = run("add", PACK, "--project", self.project, "--map", c / "Maps" / "Town12" / "Town12.umap",
                           "--xodr", c / "Maps" / "OpenDrive" / "Town12.xodr")
        self.assertEqual(rc, 0, err)
        self.assertEqual(len(json.loads((self.pack_dir / "carla-pack.json").read_text())["maps"]), 1)

    def test_add_map_import_trusts_the_result_file_not_the_exit_code(self):
        """A commandlet exits 1 whenever the session logged an error (Town12's navmesh tile limit
        does); the import counts as done when the result says ok and the map was written."""
        run("init", PACK, "--project", self.project)
        self.existing_town()
        self.fake_editor(self.FAKE_EDITOR.replace('> "$CARLA_PACK_IMPORT_RESULT"\n',
                                                  '> "$CARLA_PACK_IMPORT_RESULT"\nexit 1\n'))
        rc, out, err = run("add", PACK, "--project", self.project, "--engine", self.engine, "--map", "Town12",
                           "--import")
        self.assertEqual(rc, 0, err)
        self.assertIn("exited with code 1", err)
        self.assertEqual(len(json.loads((self.pack_dir / "carla-pack.json").read_text())["maps"]), 1)

    def test_add_map_replaces_an_unregistered_leftover_copy(self):
        run("init", PACK, "--project", self.project)
        self.existing_town()
        self.fake_editor()
        c = self.pack_dir / "Content"
        write(c / "Maps" / "Town12" / "Town12.umap", b"half-written")
        write(c / "__ExternalActors__" / "Maps" / "Town12" / "Town12" / "9" / "stale.uasset", b"stale")
        rc, out, err = run("add", PACK, "--project", self.project, "--engine", self.engine, "--map", "Town12",
                           "--import")
        self.assertEqual(rc, 0, err)
        self.assertIn("unregistered earlier copy", out)
        self.assertEqual((c / "Maps" / "Town12" / "Town12.umap").read_bytes(), b"town")
        self.assertFalse((c / "__ExternalActors__" / "Maps" / "Town12" / "Town12" / "9").exists())
        self.assertTrue((c / "__ExternalActors__" / "Maps" / "Town12" / "Town12" / "0" / "1.uasset").is_file())

    def test_add_map_import_failure_restores_the_source_and_leaves_no_entry(self):
        run("init", PACK, "--project", self.project)
        town = self.existing_town("Town13")
        self.fake_editor("#!/bin/sh\necho broken >> \"$CARLA_PACK_IMPORT_SRC_FILE\"\n"
                         "echo '{\"ok\": false, \"error\": \"load_level(x) failed\"}' > \"$CARLA_PACK_IMPORT_RESULT\"\n"
                         "exit 0\n")
        rc, out, err = run("add", PACK, "--project", self.project, "--engine", self.engine, "--map", "Town13",
                           "--import")
        self.assertEqual(rc, 1)
        self.assertIn("load_level(x) failed", err)
        self.assertEqual(town.read_bytes(), b"town")
        self.assertEqual(json.loads((self.pack_dir / "carla-pack.json").read_text())["maps"], [])
        self.assertFalse((self.pack_dir / "Content" / "Maps" / "Town13").exists())
        self.assertTrue(json.loads((self.pack_dir / "TestPack.uplugin").read_text())["ExplicitlyLoaded"])

    def test_add_map_resolves_names_and_packages(self):
        run("init", PACK, "--project", self.project)
        self.existing_town("Town12")
        write(self.project_dir / "Content" / "Carla" / "Maps" / "TestMaps" / "Town12.umap", b"other")
        rc, _, err = run("add", PACK, "--project", self.project, "--engine", self.engine, "--map", "Town12")
        self.assertEqual(rc, 1)
        self.assertIn("2 maps are called Town12", err)
        rc, _, err = run("add", PACK, "--project", self.project, "--engine", self.engine, "--map", "Nowhere")
        self.assertEqual(rc, 1)
        self.assertIn("no map called Nowhere", err)
        rc, _, err = run("add", PACK, "--project", self.project, "--engine", self.engine,
                         "--map", "/Game/Carla/Maps/Town99/Town99")
        self.assertEqual(rc, 1)
        self.assertIn("not found", err)
        rc, _, err = run("add", PACK, "--project", self.project, "--engine", self.engine,
                         "--map", "/Elsewhere/Maps/Town12")
        self.assertEqual(rc, 1)
        self.assertIn("not the project's /Game", err)
        # without an editor build an --import stops before anything is copied
        rc, _, err = run("add", PACK, "--project", self.project, "--engine", self.engine,
                         "--map", "/Game/Carla/Maps/Town12/Town12", "--import")
        self.assertEqual(rc, 1)
        self.assertIn("UnrealEditor-Cmd", err)
        self.assertFalse((self.pack_dir / "Content" / "Maps" / "Town12").exists())
        self.assertEqual(json.loads((self.pack_dir / "carla-pack.json").read_text())["maps"], [])

    def test_add_map_registers_project_maps_in_place_and_build_cooks_them(self):
        """The default for a CARLA town: no editor, no copy; the manifest names the /Game package,
        the sidecars come along and the DLC cook gets the map on -MapsToCook."""
        run("init", PACK, "--project", self.project, "--carla-version", "0.10.0")
        town = self.existing_town("Town12")
        rc, out, err = run("add", PACK, "--project", self.project, "--map", "Town12")
        self.assertEqual(rc, 0, err)
        self.assertIn("registering it in place as /Game/Carla/Maps/Town12/Town12", out)
        c = self.pack_dir / "Content"
        m = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertEqual(m["maps"], [{"name": "Town12", "package": "/Game/Carla/Maps/Town12/Town12",
                                      "xodr": "Maps/OpenDrive/Town12.xodr", "world_partition": True,
                                      "nav": "Maps/Nav/Town12.bin", "tm": "Maps/TM/Town12"}])
        self.assertTrue((c / "Maps" / "OpenDrive" / "Town12.xodr").is_file())
        self.assertTrue((c / "Maps" / "TM" / "Town12" / "Town12.bin").is_file())
        self.assertFalse((c / "Maps" / "Town12").exists())
        self.assertFalse((c / "__ExternalActors__").exists() and any((c / "__ExternalActors__").iterdir()))
        self.assertEqual(town.read_bytes(), b"town")
        # a second town, then the cook line carries both and the base-content flag
        self.existing_town("Town13")
        rc, out, err = run("add", PACK, "--project", self.project, "--map", "/Game/Carla/Maps/Town13/Town13")
        self.assertEqual(rc, 0, err)
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--engine", self.engine, "--dry-run")
        self.assertEqual(rc, 0, err)
        cmd = out.strip().splitlines()[-1]
        self.assertIn("-MapsToCook=/Game/Carla/Maps/Town12/Town12+/Game/Carla/Maps/Town13/Town13", cmd)
        self.assertIn(" -DLCIncludeEngineContent", cmd)
        self.assertIn("skipping the check", err)      # the fixture registry is not a real one
        # --maps entries are merged, not duplicated
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--engine", self.engine, "--dry-run", "--maps", "Town12")
        self.assertEqual(rc, 0, err)
        self.assertEqual(out.strip().splitlines()[-1].count("/Game/Carla/Maps/Town12/Town12"), 1)
        # a map the base release already cooked is refused
        reg = self.releases / RELEASE / "Linux" / "AssetRegistry.bin"
        fake_registry(reg, ["/Game/Carla/Maps/Town10HD_Opt", "/Game/Carla/Maps/Town13/Town13"])
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.releases / RELEASE,
                           "--engine", self.engine, "--dry-run")
        self.assertEqual(rc, 1)
        self.assertIn("already ships /Game/Carla/Maps/Town13/Town13", err)

    def test_create_is_init_add_build_in_one_command(self):
        for t in ("Town01_Opt", "Town12"):
            self.existing_town(t)
        # the base release is found under <repo>/Build when --base is omitted
        build_dir = self.tmp / "Build" / "Release" / "Package"
        shutil.copy(str(self.base_tar), str(write(build_dir / self.base_tar.name, b"")))
        rc, out, err = run("create", "Towns2", "Town01_Opt", "/Game/Carla/Maps/Town12/Town12",
                           "--project", self.project, "--engine", self.engine, "--dry-run",
                           "--carla-version", "0.10.0")
        self.assertEqual(rc, 0, err)
        self.assertIn("base release: " + str(build_dir / self.base_tar.name), out)
        pack_dir = self.root / "Towns2"
        m = json.loads((pack_dir / "carla-pack.json").read_text())
        self.assertEqual([x["package"] for x in m["maps"]],
                         ["/Game/Carla/Maps/Town01_Opt/Town01_Opt", "/Game/Carla/Maps/Town12/Town12"])
        self.assertTrue((pack_dir / "Content" / "Maps" / "OpenDrive" / "Town12.xodr").is_file())
        cmd = out.strip().splitlines()[-1]
        self.assertIn("-dlcname=" + str(pack_dir / "Towns2.uplugin"), cmd)
        self.assertIn("-MapsToCook=/Game/Carla/Maps/Town01_Opt/Town01_Opt+/Game/Carla/Maps/Town12/Town12", cmd)
        # running it again adds to the existing pack instead of failing on init
        self.existing_town("Town13")
        rc, out, err = run("create", "Towns2", "Town13", "--project", self.project, "--engine", self.engine,
                           "--dry-run")
        self.assertEqual(rc, 0, err)
        self.assertIn("adding to the existing pack", out)
        self.assertEqual(len(json.loads((pack_dir / "carla-pack.json").read_text())["maps"]), 3)
        # no base anywhere: a clear error, nothing else
        shutil.rmtree(str(self.tmp / "Build"))
        rc, out, err = run("create", "Towns3", "Town13", "--project", self.project, "--engine", self.engine,
                           "--dry-run")
        self.assertEqual(rc, 1)
        self.assertIn("no base release found", err)
        # two candidates: refuse to guess
        write(build_dir / "carla-0.10.1-Linux-release-metadata.tar.gz", b"x")
        write(build_dir / "carla-0.10.2-Linux-release-metadata.tar.gz", b"x")
        rc, out, err = run("build", "Towns2", "--project", self.project, "--engine", self.engine, "--dry-run")
        self.assertEqual(rc, 1)
        self.assertIn("several base releases found", err)

    def test_flat_town_takes_only_its_own_sidecars(self):
        """Town01_Opt lives flat in Maps/ next to the shared Maps/TM, Maps/Nav, Maps/OpenDrive folders:
        only its own files may come along (the whole TM folder once did, and the client's Traffic
        Manager loaded another town's cache)."""
        run("init", PACK, "--project", self.project)
        carla = self.project_dir / "Content" / "Carla"
        write(carla / "Maps" / "Town01_Opt.umap", b"flat")
        write(carla / "Maps" / "OpenDrive" / "Town01_Opt.xodr", "<OpenDRIVE/>")
        write(carla / "Maps" / "OpenDrive" / "Town01.xodr", "<OpenDRIVE/>")
        write(carla / "Maps" / "TM" / "Town01_Opt.bin", b"tm1")
        write(carla / "Maps" / "TM" / "Town05_Opt.bin", b"tm5")
        write(carla / "Maps" / "Nav" / "Town01_Opt.bin", b"nav1")
        write(carla / "Maps" / "Nav" / "Town05_Opt.bin", b"nav5")
        rc, out, err = run("add", PACK, "--project", self.project, "--map", "Town01_Opt")
        self.assertEqual(rc, 0, err)
        c = self.pack_dir / "Content"
        m = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertEqual(m["maps"][0]["package"], "/Game/Carla/Maps/Town01_Opt")
        self.assertEqual(m["maps"][0]["tm"], "Maps/TM/Town01_Opt")
        self.assertEqual(sorted(p.name for p in (c / "Maps" / "TM" / "Town01_Opt").iterdir()), ["Town01_Opt.bin"])
        self.assertEqual(sorted(p.name for p in (c / "Maps" / "Nav").iterdir()), ["Town01_Opt.bin"])
        self.assertEqual(sorted(p.name for p in (c / "Maps" / "OpenDrive").iterdir()), ["Town01_Opt.xodr"])

    def test_add_takes_several_maps(self):
        run("init", PACK, "--project", self.project)
        self.existing_town("Town12"); self.existing_town("Town13")
        rc, out, err = run("add", PACK, "--project", self.project, "--map", "Town12", "--map", "Town13",
                           "--xodr", self.xodr)
        self.assertEqual(rc, 1)
        self.assertIn("single --map", err)
        rc, out, err = run("add", PACK, "--project", self.project, "--map", "Town12", "--map", "Town13")
        self.assertEqual(rc, 0, err)
        self.assertEqual(len(json.loads((self.pack_dir / "carla-pack.json").read_text())["maps"]), 2)

    def test_add_map_copy_is_opt_in(self):
        run("init", PACK, "--project", self.project)
        rc, out, err = run("add", PACK, "--project", self.project, "--engine", self.engine,
                           "--map", self.umap, "--xodr", self.xodr)
        self.assertEqual(rc, 1)
        self.assertIn("Unreal editor not found", err)
        self.assertFalse((self.pack_dir / "Content" / "Maps" / "TestTown.umap").exists())
        self.assertEqual(json.loads((self.pack_dir / "carla-pack.json").read_text())["maps"], [])
        rc, out, err = run("add", PACK, "--project", self.project, "--map", self.umap, "--xodr", self.xodr,
                           "--copy")
        self.assertEqual(rc, 0, err)
        self.assertIn("another content root", err)
        self.assertTrue((self.pack_dir / "Content" / "Maps" / "TestTown.umap").is_file())
        # the external folders were found through the exact relative path and copied along
        self.assertTrue((self.pack_dir / "Content" / "__ExternalActors__" / "Maps" / "TestTown" / "A" / "B.uasset").is_file())
        m = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertTrue(m["maps"][0]["world_partition"])

    def test_plugin_settings_merge_keeps_author_content(self):
        run("init", PACK, "--project", self.project)
        ini = self.pack_dir / "Config" / "PluginSettings.ini"
        self.assertTrue(ini.read_text().startswith("; Written by carla-pack"))
        # the author edited the tool-written file: own comment, own section before and
        # after [StageSettings], an extra stage dir, one wanted line in odd case + spacing
        authored = ("; Written by carla-pack (edited by me)\n"
                    "[Other]\nKey=1\n\n"
                    "[StageSettings]\n"
                    "; my comment\n"
                    "+AdditionalNonUSFDirectories=Content/Extra\n"
                    "  +additionalnonusfdirectories=content/config  \n"
                    "\n"
                    "[Zzz]\nLast=true\n")
        ini.write_text(authored)
        out, err = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            changed = cp.write_plugin_settings(self.pack_dir)
        self.assertTrue(changed)
        text = ini.read_text()
        self.assertIn("added 3 stage line(s)", out.getvalue())
        self.assertEqual(err.getvalue(), "")
        expected = ("; Written by carla-pack (edited by me)\n"
                    "[Other]\nKey=1\n\n"
                    "[StageSettings]\n"
                    "; my comment\n"
                    "+AdditionalNonUSFDirectories=Content/Extra\n"
                    "  +additionalnonusfdirectories=content/config  \n"
                    "+AdditionalNonUSFDirectories=Content/Maps/OpenDrive\n"
                    "+AdditionalNonUSFDirectories=Content/Maps/Nav\n"
                    "+AdditionalNonUSFDirectories=Content/Maps/TM\n"
                    "\n"
                    "[Zzz]\nLast=true\n")
        self.assertEqual(text, expected)                                  # inserted inside the section, nothing else touched
        self.assertFalse(cp.write_plugin_settings(self.pack_dir))     # idempotent
        self.assertEqual(ini.read_text(), expected)
        # no [StageSettings] at all: the section is appended after the author's content
        ini.write_text("[Other]\nKey=1")
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertTrue(cp.write_plugin_settings(self.pack_dir))
        self.assertEqual(ini.read_text(), "[Other]\nKey=1\n\n[StageSettings]\n"
                         + "\n".join(cp.STAGE_LINES) + "\n")
        # CRLF files keep their line endings
        ini.write_bytes(b"[StageSettings]\r\n+AdditionalNonUSFDirectories=Content/Config\r\n")
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertTrue(cp.write_plugin_settings(self.pack_dir))
        raw = ini.read_bytes()
        self.assertEqual(raw.count(b"\r\n"), 5)                          # section + Config + 3 added
        self.assertEqual(raw.count(b"\n"), raw.count(b"\r\n"))           # no bare LF crept in
        self.assertIn(b"+AdditionalNonUSFDirectories=Content/Maps/Nav\r\n", raw)
        # build merges too (never rewrites): a pre-existing, complete author file is left byte-identical
        complete = "; mine\n[StageSettings]\n" + "\n".join(cp.STAGE_LINES) + "\n[Mine]\nA=b\n"
        ini.write_text(complete)
        staged = self.tmp / "Staged"
        self.fake_staged(staged)
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out")
        self.assertEqual(rc, 0, err)
        self.assertEqual(ini.read_text(), complete)
        # init over a pre-created pack dir that already holds an author ini merges as well
        shutil.rmtree(str(self.pack_dir))
        write(ini, "[StageSettings]\n+AdditionalNonUSFDirectories=Content/Mine\n")
        rc, out, err = run("init", PACK, "--project", self.project)
        self.assertEqual(rc, 0, err)
        text = ini.read_text()
        self.assertTrue(text.startswith("[StageSettings]\n+AdditionalNonUSFDirectories=Content/Mine\n"))
        for line in cp.STAGE_LINES:
            self.assertEqual(text.count(line + "\n"), 1, line)
        # pure function: unknown content preserved, no duplicates, lists what it added
        merged, added = cp.merge_plugin_settings("[A]\nx=1\n[StageSettings]\n" + cp.STAGE_LINES[0] + "\n")
        self.assertEqual(added, list(cp.STAGE_LINES[1:]))
        self.assertEqual(merged, "[A]\nx=1\n[StageSettings]\n" + "\n".join(cp.STAGE_LINES) + "\n")
        self.assertEqual(cp.merge_plugin_settings(merged), (merged, []))

    def test_add_rejects_bad_catalog_shape(self):
        run("init", PACK, "--project", self.project)
        bad = write(self.tmp / "bad.json", json.dumps({"Vehicles": [{"Make": "x"}]}))
        rc, _, err = run("add", PACK, "--project", self.project, "--vehicles", bad)
        self.assertEqual(rc, 1)
        self.assertIn("missing Model, Class", err)
        rc, _, err = run("add", PACK, "--project", self.project, "--props",
                         write(self.tmp / "bad2.json", "[]"))
        self.assertEqual(rc, 1)
        self.assertIn("props catalog", err)
        self.assertFalse((self.pack_dir / "Content" / "Config" / "Vehicles.json").exists())

    def test_add_props_refuses_untagged_pack_mesh(self):
        run("init", PACK, "--project", self.project)
        untagged = write(self.tmp / "untagged.json", json.dumps(
            {"props": [{"name": "Box", "path": "/TestPack/Static/Props/SM_Box.SM_Box", "size": "Small"}]}))
        rc, _, err = run("add", PACK, "--project", self.project, "--props", untagged)
        self.assertEqual(rc, 1)
        self.assertIn("no semantic segmentation label", err)
        self.assertIn("--allow-untagged", err)
        self.assertFalse((self.pack_dir / "Content" / "Config" / "TestPack.Package.json").exists())
        rc, _, err = run("add", PACK, "--project", self.project, "--props", untagged, "--allow-untagged")
        self.assertEqual(rc, 0, err)
        self.assertIn("no semantic segmentation label", err)
        self.assertTrue((self.pack_dir / "Content" / "Config" / "TestPack.Package.json").is_file())
        # every known label folder is accepted, /Game/ meshes are never checked
        for label in cp.SEMANTIC_LABELS:
            self.assertEqual(cp.semantic_label_of("/TestPack/Static/{}/SM_X.SM_X".format(label)), label)
        ok = write(self.tmp / "ok.json", json.dumps(
            {"props": [{"name": "Base", "path": "/Game/Carla/Static/Props/SM_B.SM_B", "size": "Small"},
                       {"name": "Bush", "path": "/TestPack/Static/Bush/SM_Bush.SM_Bush", "size": "Small"}]}))
        rc, _, err = run("add", PACK, "--project", self.project, "--props", ok)
        self.assertEqual(rc, 0, err)
        self.assertNotIn("semantic", err)
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            with self.assertRaises(SystemExit) as cm:
                cp.main(["add", "--help"])
        self.assertEqual(cm.exception.code, 0)
        self.assertIn("TrafficLight, Static, Dynamic", out.getvalue())

    def test_add_props_parameters_shape(self):
        run("init", PACK, "--project", self.project)
        pp = write(self.tmp / "pp.json", json.dumps(
            {"Props": [{"Name": "Cone", "Mesh": "/Game/Carla/Static/SM_Cone.SM_Cone", "Size": "Odd"}]}))
        rc, _, err = run("add", PACK, "--project", self.project, "--props", pp)
        self.assertEqual(rc, 0, err)
        self.assertTrue((self.pack_dir / "Content" / "Config" / "PropParameters.json").is_file())
        self.assertIn("size 'Odd'", err)

    def test_add_without_pack_or_args_fails(self):
        rc, _, err = run("add", "Nope", "--project", self.project, "--vehicles", self.vehicles)
        self.assertEqual(rc, 1)
        self.assertIn("not found", err)
        run("init", PACK, "--project", self.project)
        rc, _, err = run("add", PACK, "--project", self.project)
        self.assertEqual(rc, 1)
        self.assertIn("nothing to add", err)


class TestBuild(PackTestBase):
    def test_dry_run_prints_uat_command(self):
        self.init_and_add()
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--engine", self.engine, "--dry-run")
        self.assertEqual(rc, 0, err)
        cmd = out.strip().splitlines()[-1]
        self.assertTrue(cmd.startswith(str(self.engine / "Engine/Build/BatchFiles/RunUAT.sh") + " BuildCookRun"))
        for flag in ("-project=" + str(self.project), " -nocompileeditor ", " -nop4 ", " -skipbuild ",
                     " -cook ", " -stage ", " -pak ", " -iostore ", "-clientconfig=Development",
                     "-TargetPlatform=Linux", "-Platform=Linux",
                     "-dlcname=" + str(self.pack_dir / "TestPack.uplugin"),
                     "-basedonreleaseversion=" + RELEASE, "-stagingdirectory=",
                     "-AdditionalCookerOptions=-EnablePlugins=TestPack"):
            self.assertIn(flag, cmd + " ")
        self.assertNotIn("-MapsToCook", cmd)       # a DLC cook takes everything under /<Pack>/
        self.assertNotIn("-iterate", cmd)          # refused together with -basedonreleaseversion
        self.assertIn(" -DLCIncludeEngineContent", cmd)   # existing towns reference uncooked /Game assets
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--engine", self.engine, "--dry-run", "--no-base-content")
        self.assertEqual(rc, 0, err)
        self.assertNotIn("-DLCIncludeEngineContent", out.strip().splitlines()[-1])
        # the tarball was extracted and the root passed is the Releases dir itself
        root = [c for c in cmd.split() if c.startswith("-basedonreleaseversionroot=")][0].split("=", 1)[1]
        self.assertTrue((Path(root) / RELEASE / "Linux" / "AssetRegistry.bin").is_file(), root)
        self.assertTrue(root.endswith("/Releases"))

    def test_dry_run_with_dir_base_maps_config_and_extra_args(self):
        self.init_and_add()
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.releases / RELEASE,
                           "--engine", self.engine, "--dry-run", "--config", "Shipping",
                           "--maps", "TestTown,/TestPack/Maps/Other", "--uat-arg=-DLCIncludeEngineContent")
        self.assertEqual(rc, 0, err)
        cmd = out.strip().splitlines()[-1]
        self.assertIn("-clientconfig=Shipping", cmd)
        self.assertIn("-MapsToCook=/TestPack/Maps/TestTown+/TestPack/Maps/Other", cmd)
        self.assertIn("-basedonreleaseversionroot=" + str(self.releases), cmd)
        self.assertTrue(cmd.endswith("-DLCIncludeEngineContent"))
        self.assertEqual(cmd.count("-DLCIncludeEngineContent"), 1)   # not added twice

    def test_build_needs_engine_and_valid_base(self):
        self.init_and_add()
        os.environ.pop(cp.ENGINE_ENV, None)
        rc, _, err = run("build", PACK, "--project", self.project, "--base", self.base_tar, "--dry-run")
        self.assertEqual(rc, 1)
        self.assertIn(cp.ENGINE_ENV, err)
        rc, _, err = run("build", PACK, "--project", self.project, "--base", self.tmp / "authored",
                         "--engine", self.engine, "--dry-run")
        self.assertEqual(rc, 1)
        self.assertIn("not a release-metadata tarball", err)

    def test_packaging_normalises_staged_output(self):
        self.init_and_add()
        (self.pack_dir / "Config" / "PluginSettings.ini").unlink()
        out_pack, tar_path, cooked = self.build_fake()
        self.assertTrue((self.pack_dir / "Config" / "PluginSettings.ini").is_file())  # build ensures it
        self.assertTrue(out_pack.is_dir())
        self.assertTrue(tar_path.is_file())
        paks = out_pack / "Content" / "Paks" / "Linux"
        # UAT's container names are kept (mount verified only with them)
        self.assertEqual(sorted(p.name for p in paks.iterdir()),
                         ["TestPackCarlaUnreal-Linux.pak", "TestPackCarlaUnreal-Linux.ucas",
                          "TestPackCarlaUnreal-Linux.utoc"])
        self.assertEqual((paks / "TestPackCarlaUnreal-Linux.utoc").read_bytes(), b"data.utoc")
        # the cooked-dir registry, not the stripped/Development one
        self.assertEqual((out_pack / "AssetRegistry.bin").read_bytes(), b"cooked-registry")
        up = json.loads((out_pack / "TestPack.uplugin").read_text())
        self.assertTrue(up["ExplicitlyLoaded"])  # distributed copy: mounted on demand
        self.assertFalse(up["EnabledByDefault"])
        self.assertTrue(up["CanContainContent"])
        self.assertNotIn("Modules", up)
        self.assertNotIn("Installed", up)      # never forced
        self.assertFalse((out_pack / "TestPack.upluginmanifest").exists())
        # loose sidecars come from the stage
        self.assertEqual((out_pack / "Content" / "Config" / "Vehicles.json").read_text(), "{staged}")
        self.assertEqual((out_pack / "Content" / "Maps" / "OpenDrive" / "TestTown.xodr").read_text(), "<staged/>")
        m = json.loads((out_pack / "carla-pack.json").read_text())
        self.assertEqual(m["base_release"], RELEASE)
        self.assertEqual(m["platform"], "Linux")
        self.assertEqual(m["engine"]["version"], "5.8.0")
        self.assertEqual(m["maps"][0]["package"], "/TestPack/Maps/TestTown")
        expected_files = {"TestPack.uplugin", "AssetRegistry.bin",
                          "Content/Paks/Linux/TestPackCarlaUnreal-Linux.pak",
                          "Content/Paks/Linux/TestPackCarlaUnreal-Linux.utoc",
                          "Content/Paks/Linux/TestPackCarlaUnreal-Linux.ucas",
                          "Content/Config/TestPack.Package.json", "Content/Config/Vehicles.json",
                          "Content/Config/WalkerParameters.json",
                          "Content/Maps/OpenDrive/TestTown.xodr", "Content/Maps/Nav/TestTown.bin"}
        self.assertEqual(set(m["files"]), expected_files)
        for rel, digest in m["files"].items():
            self.assertEqual(digest, cp.sha256_file(out_pack / rel))
            self.assertTrue(digest.startswith("sha256:") and len(digest) == 7 + 64)
        with tarfile.open(str(tar_path)) as tar:
            names = set(tar.getnames())
        self.assertIn("TestPack/carla-pack.json", names)
        self.assertIn("TestPack/Content/Paks/Linux/TestPackCarlaUnreal-Linux.pak", names)
        self.assertNotIn("TestPack/TestPack.upluginmanifest", names)
        # the source manifest is untouched
        src = json.loads((self.pack_dir / "carla-pack.json").read_text())
        self.assertEqual(src["files"], {})

    def test_packaging_falls_back_to_authored_sidecars(self):
        self.init_and_add()
        staged = self.tmp / "Staged"
        self.fake_staged(staged, sidecars=False)
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out")
        self.assertEqual(rc, 0, err)
        self.assertIn("was not staged", err)
        out_pack = self.tmp / "out" / PACK
        self.assertEqual((out_pack / "Content" / "Maps" / "OpenDrive" / "TestTown.xodr").read_text(), "<OpenDRIVE/>")
        self.assertEqual(json.loads((out_pack / "Content" / "Config" / "Vehicles.json").read_text()), VEHICLES)
        self.assertEqual((out_pack / "Content" / "Maps" / "Nav" / "TestTown.bin").read_bytes(), b"nav")

    def test_packaging_asset_registry_order(self):
        self.init_and_add()
        staged = self.tmp / "Staged"
        plugin, cooked = self.fake_staged(staged, registry=False)
        write(plugin / "AssetRegistry.bin", b"staged-registry")
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out")
        self.assertEqual(rc, 0, err)
        self.assertEqual((self.tmp / "out" / PACK / "AssetRegistry.bin").read_bytes(), b"staged-registry")
        write(cooked / "AssetRegistry.bin", b"cooked-registry")
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out")
        self.assertEqual(rc, 0, err)
        self.assertEqual((self.tmp / "out" / PACK / "AssetRegistry.bin").read_bytes(), b"cooked-registry")
        override = write(self.tmp / "my.bin", b"override")
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out2",
                           "--asset-registry", override)
        self.assertEqual(rc, 0, err)
        self.assertEqual((self.tmp / "out2" / PACK / "AssetRegistry.bin").read_bytes(), b"override")
        # Development registry only as a last resort, with a warning
        (cooked / "AssetRegistry.bin").unlink()
        (plugin / "AssetRegistry.bin").unlink()
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out3")
        self.assertEqual(rc, 0, err)
        self.assertIn("DevelopmentAssetRegistry.bin", err)
        self.assertEqual((self.tmp / "out3" / PACK / "AssetRegistry.bin").read_bytes(), b"dev-registry")

    def test_packaging_rename_is_opt_in(self):
        self.init_and_add()
        staged = self.tmp / "Staged"
        self.fake_staged(staged)
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out", "--rename")
        self.assertEqual(rc, 0, err)
        self.assertIn("renamed containers TestPackCarlaUnreal-Linux.* -> TestPack-Linux.*", out)
        self.assertNotIn("unverified", out + err)   # verified on the packaged server (2026-08-30)
        paks = self.tmp / "out" / PACK / "Content" / "Paks" / "Linux"
        self.assertEqual(sorted(p.name for p in paks.iterdir()),
                         ["TestPack-Linux.pak", "TestPack-Linux.ucas", "TestPack-Linux.utoc"])
        m = json.loads((self.tmp / "out" / PACK / "carla-pack.json").read_text())
        self.assertIn("Content/Paks/Linux/TestPack-Linux.pak", m["files"])
        self.assertNotIn("unverified", cp.build_parser().format_help())
        with contextlib.redirect_stdout(io.StringIO()) as help_out:
            with self.assertRaises(SystemExit):
                cp.main(["build", "--help"])
        self.assertNotIn("unverified", help_out.getvalue())

    def test_runtime_descriptor_is_minimal_and_refuses_modules(self):
        self.init_and_add()
        uplugin = self.pack_dir / "TestPack.uplugin"
        staged = self.tmp / "Staged"
        self.fake_staged(staged)
        # authored keys pass through (Installed:false stays false, no FileVersion -> 3 added first)
        uplugin.write_text(json.dumps({"FriendlyName": "T", "Installed": False, "Custom": 1,
                                       "ExplicitlyLoaded": False, "CanContainContent": False,
                                       "EnabledByDefault": True, "Modules": []}))
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out")
        self.assertEqual(rc, 0, err)
        shipped = json.loads((self.tmp / "out" / PACK / "TestPack.uplugin").read_text())
        self.assertEqual(list(shipped)[0], "FileVersion")
        self.assertEqual(shipped["FileVersion"], 3)
        self.assertIs(shipped["Installed"], False)
        self.assertEqual(shipped["Custom"], 1)
        self.assertIs(shipped["ExplicitlyLoaded"], True)
        self.assertIs(shipped["CanContainContent"], True)
        self.assertIs(shipped["EnabledByDefault"], False)
        self.assertNotIn("Modules", shipped)
        # a code module cannot ship in a content pack
        uplugin.write_text(json.dumps({"FileVersion": 3, "Modules": [{"Name": "MyCode", "Type": "Runtime"}]}))
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--skip-cook", "--staged", staged, "--out", self.tmp / "out2")
        self.assertEqual(rc, 1)
        self.assertIn("1 code module(s) (MyCode)", err)
        self.assertFalse((self.tmp / "out2" / PACK / "TestPack.uplugin").exists())

    def fake_uat(self, exit_code):
        """A RunUAT.sh that records the .uplugin as the cooker would see it, then exits."""
        record = self.tmp / "uplugin-during-cook.json"
        write(self.engine / "Engine" / "Build" / "BatchFiles" / "RunUAT.sh",
              '#!/bin/sh\nfor a in "$@"; do case "$a" in -dlcname=*) cp "${a#-dlcname=}" "%s";; esac; done\n'
              'exit %d\n' % (record, exit_code))
        os.chmod(str(self.engine / "Engine" / "Build" / "BatchFiles" / "RunUAT.sh"), 0o755)
        return record

    def test_build_flips_descriptor_for_the_cook_and_restores_it(self):
        self.init_and_add()
        uplugin = self.pack_dir / "TestPack.uplugin"
        original = uplugin.read_bytes()
        self.assertTrue(json.loads(original.decode())["ExplicitlyLoaded"])
        staged = self.tmp / "Staged"
        self.fake_staged(staged)
        record = self.fake_uat(0)
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--engine", self.engine, "--staged", staged, "--out", self.tmp / "out")
        self.assertEqual(rc, 0, err)
        seen = json.loads(record.read_text())
        self.assertFalse(seen["ExplicitlyLoaded"])
        self.assertFalse(seen["EnabledByDefault"])
        self.assertEqual(uplugin.read_bytes(), original)               # restored byte-for-byte
        shipped = json.loads((self.tmp / "out" / PACK / "TestPack.uplugin").read_text())
        self.assertTrue(shipped["ExplicitlyLoaded"])
        # dry-run never touches the file nor runs UAT
        record.unlink()
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--engine", self.engine, "--dry-run")
        self.assertEqual(rc, 0, err)
        self.assertFalse(record.exists())
        self.assertEqual(uplugin.read_bytes(), original)

    def test_build_restores_descriptor_when_uat_fails_or_is_interrupted(self):
        self.init_and_add()
        uplugin = self.pack_dir / "TestPack.uplugin"
        original = b'{\n  "FileVersion": 3, "ExplicitlyLoaded": true, "Custom": "keep me"\n}\n'
        uplugin.write_bytes(original)
        record = self.fake_uat(7)
        rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                           "--engine", self.engine, "--out", self.tmp / "out")
        self.assertEqual(rc, 1)
        self.assertIn("exit code 7", err)
        self.assertFalse(json.loads(record.read_text())["ExplicitlyLoaded"])
        self.assertEqual(uplugin.read_bytes(), original)
        # Ctrl-C while UAT runs
        real_run = cp.subprocess.run

        def interrupted(cmd, *a, **k):
            if "BuildCookRun" not in cmd:          # engine_info()'s git call
                return real_run(cmd, *a, **k)
            self.assertFalse(json.loads(uplugin.read_text())["ExplicitlyLoaded"])
            raise KeyboardInterrupt()
        cp.subprocess.run = interrupted
        try:
            rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                               "--engine", self.engine, "--out", self.tmp / "out")
        finally:
            cp.subprocess.run = real_run
        self.assertEqual(rc, 130)
        self.assertEqual(uplugin.read_bytes(), original)

    def test_build_descriptor_write_failure_leaves_original(self):
        self.init_and_add()
        uplugin = self.pack_dir / "TestPack.uplugin"
        original = uplugin.read_bytes()
        self.fake_uat(0)
        real_save = cp.save_json

        def failing_save(path, data):
            if Path(path).name == "TestPack.uplugin" and Path(path).parent == self.pack_dir:
                raise OSError(28, "No space left on device")
            return real_save(path, data)
        cp.save_json = failing_save
        try:
            rc, out, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                               "--engine", self.engine, "--out", self.tmp / "out")
        finally:
            cp.save_json = real_save
        self.assertEqual(rc, 1)
        self.assertIn("No space left", err)
        self.assertEqual(uplugin.read_bytes(), original)
        self.assertEqual([p.name for p in self.pack_dir.iterdir() if p.name.endswith(".tmp")], [])

    def test_build_refuses_out_inside_packs_root(self):
        self.init_and_add()
        staged = self.tmp / "Staged"
        self.fake_staged(staged)
        for out in (self.root, self.pack_dir, self.root / "sub", self.pack_dir / "Content"):
            rc, _, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                             "--skip-cook", "--staged", staged, "--out", out)
            self.assertEqual(rc, 1, out)
            self.assertIn("--out", err)
            self.assertTrue((self.pack_dir / "TestPack.uplugin").is_file())
            self.assertTrue((self.pack_dir / "Content" / "Maps" / "TestTown.umap").is_file())
        # an existing <out>/<Pack> that is not a previous build output is never deleted
        foreign = write(self.tmp / "out" / PACK / "precious.txt", "keep")
        rc, _, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                         "--skip-cook", "--staged", staged, "--out", self.tmp / "out")
        self.assertEqual(rc, 1)
        self.assertIn("refusing to delete", err)
        self.assertTrue(foreign.is_file())
        # the tool's default (<pack>/Saved/CarlaPack/out) and anything under Saved/ stay allowed
        rc, _, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                         "--skip-cook", "--staged", staged, "--out", self.pack_dir / "Saved" / "mine")
        self.assertEqual(rc, 0, err)

    def test_packaging_fails_without_staged_output(self):
        self.init_and_add()
        rc, _, err = run("build", PACK, "--project", self.project, "--base", self.base_tar,
                         "--skip-cook", "--staged", self.tmp / "nowhere", "--out", self.tmp / "out")
        self.assertEqual(rc, 1)
        self.assertIn("no staged output", err)


class TestInspectVerifyInstall(PackTestBase):
    def test_round_trip(self):
        self.init_and_add()
        out_pack, tar_path, _ = self.build_fake()
        server = self.fake_server()

        rc, out, err = run("inspect", tar_path, "--server", server)
        self.assertEqual(rc, 0, err)
        self.assertIn("Compatible:    yes", out)
        self.assertIn("/TestPack/Maps/TestTown", out)
        rc, out, err = run("inspect", tar_path, "--server", server, "--json")
        self.assertEqual(rc, 0, err)
        report = json.loads(out)
        self.assertTrue(report["server"]["compatible"])
        self.assertEqual(report["server"]["base_release"], RELEASE)
        self.assertEqual(report["manifest"]["name"], PACK)
        rc, out, _ = run("inspect", out_pack)
        self.assertEqual(rc, 0)
        self.assertIn("Base release:  " + RELEASE, out)
        self.assertIn("0.0 MB  Content/Paks/Linux/TestPackCarlaUnreal-Linux.ucas", out)
        self.assertEqual(report["sizes"]["AssetRegistry.bin"], len(b"cooked-registry"))

        self.assertEqual(run("verify", out_pack)[0], 0)
        rc, out, _ = run("verify", tar_path)
        self.assertEqual(rc, 0, out)
        self.assertIn("OK: 10 files verified", out)
        # one shape for every caller: (manifest, top dir, top-relative sizes)
        manifest, top, sizes = cp.read_manifest_from_tar(tar_path)
        self.assertEqual((manifest["name"], top), (PACK, PACK))
        self.assertEqual(sizes["AssetRegistry.bin"], len(b"cooked-registry"))
        self.assertEqual(set(sizes), set(manifest["files"]) | {"carla-pack.json"})

        rc, out, err = run("install", tar_path, "--server", server)
        self.assertEqual(rc, 0, err)
        installed = server / "CarlaUnreal" / "Packs" / PACK
        self.assertTrue((installed / "carla-pack.json").is_file())
        self.assertTrue((installed / "Content" / "Paks" / "Linux" / "TestPackCarlaUnreal-Linux.utoc").is_file())
        self.assertTrue((installed / "Content" / "Config" / "Vehicles.json").is_file())
        self.assertEqual(run("verify", installed)[0], 0)
        self.assertIn("mount_content_pack", out)
        self.assertEqual([p.name for p in (server / "CarlaUnreal" / "Packs").iterdir()], [PACK])

        rc, _, err = run("install", tar_path, "--server", server)
        self.assertEqual(rc, 1)
        self.assertIn("already installed", err)
        self.assertEqual(run("install", tar_path, "--server", server, "--force")[0], 0)
        # --server accepts <Package>, <Package>/Linux and <Package>/Linux/CarlaUnreal
        for s_dir in (server.parent, server, server / "CarlaUnreal"):
            rc, out, err = run("inspect", tar_path, "--server", s_dir, "--json")
            self.assertEqual(rc, 0, err)
            self.assertEqual(json.loads(out)["server"]["path"], str(server / "CarlaUnreal"))
        shutil.rmtree(str(installed))
        self.assertEqual(run("install", tar_path, "--server", server.parent)[0], 0)
        self.assertTrue((installed / "carla-pack.json").is_file())

    def test_base_release_mismatch(self):
        self.init_and_add()
        _, tar_path, _ = self.build_fake()
        server = self.fake_server("carla-0.10.1-Linux")
        rc, out, err = run("inspect", tar_path, "--server", server)
        self.assertEqual(rc, 3)
        self.assertIn("Compatible:    NO", out)
        self.assertIn("base release mismatch", out)
        rc, _, err = run("install", tar_path, "--server", server)
        self.assertEqual(rc, 1)
        self.assertIn("base release mismatch", err)
        self.assertIn("carla-0.10.1-Linux", err)
        self.assertFalse((server / "CarlaUnreal" / "Packs" / PACK).exists())
        rc, _, err = run("install", tar_path, "--server", server, "--force")
        self.assertEqual(rc, 0, err)
        self.assertIn("ignored: --force", err)

    def test_server_without_stamp(self):
        self.init_and_add()
        _, tar_path, _ = self.build_fake()
        server = self.fake_server(None)
        rc, _, err = run("install", tar_path, "--server", server)
        self.assertEqual(rc, 1)
        self.assertIn("no BaseRelease stamp", err)
        rc, _, err = run("install", tar_path, "--server", self.tmp / "authored")
        self.assertEqual(rc, 1)
        self.assertIn("does not look like a CARLA package", err)

    def test_verify_and_install_detect_corruption(self):
        self.init_and_add()
        out_pack, tar_path, _ = self.build_fake()
        (out_pack / "Content" / "Paks" / "Linux" / "TestPackCarlaUnreal-Linux.ucas").write_bytes(b"corrupt")
        (out_pack / "AssetRegistry.bin").unlink()
        write(out_pack / "stray.txt", "x")
        rc, out, _ = run("verify", out_pack)
        self.assertEqual(rc, 1)
        self.assertIn("MISMATCH   Content/Paks/Linux/TestPackCarlaUnreal-Linux.ucas", out)
        self.assertIn("MISSING    AssetRegistry.bin", out)
        self.assertIn("UNLISTED   stray.txt", out)
        self.assertIn("1 missing, 1 mismatched, 1 unlisted", out)
        # a tarball rebuilt from the corrupted folder is refused by install
        bad_tar = self.tmp / "bad.tar.gz"
        with tarfile.open(str(bad_tar), "w:gz") as tar:
            tar.add(str(out_pack), arcname=PACK)
        server = self.fake_server()
        rc, _, err = run("install", bad_tar, "--server", server)
        self.assertEqual(rc, 1)
        self.assertIn("checksum verification failed", err)
        self.assertFalse((server / "CarlaUnreal" / "Packs" / PACK).exists())
        self.assertEqual([p for p in (server / "CarlaUnreal" / "Packs").iterdir()], [])

    def test_unlisted_files_fail_verify_and_install(self):
        self.init_and_add()
        out_pack, tar_path, _ = self.build_fake()
        write(out_pack / "Content" / "Paks" / "Linux" / "Extra-Linux.pak", b"rogue")
        rc, out, _ = run("verify", out_pack)
        self.assertEqual(rc, 1)
        self.assertIn("UNLISTED   Content/Paks/Linux/Extra-Linux.pak", out)
        bad_tar = self.tmp / "unlisted.tar.gz"
        with tarfile.open(str(bad_tar), "w:gz") as tar:
            tar.add(str(out_pack), arcname=PACK)
        server = self.fake_server()
        rc, _, err = run("install", bad_tar, "--server", server)
        self.assertEqual(rc, 1)
        self.assertIn("not listed in its manifest", err)
        self.assertIn("Extra-Linux.pak", err)
        self.assertEqual(list((server / "CarlaUnreal" / "Packs").iterdir()), [])

    def hostile_tar(self, name, members):
        """A pack tarball with a valid manifest plus the given (TarInfo, data) members."""
        path = self.tmp / (name + ".tar.gz")
        manifest = json.dumps(cp.new_manifest(PACK, "1.0.0", "0.10.0"))
        manifest = json.loads(manifest)
        manifest["base_release"] = RELEASE
        data = json.dumps(manifest).encode()
        with tarfile.open(str(path), "w:gz") as tar:
            ti = tarfile.TarInfo(PACK + "/carla-pack.json")
            ti.size = len(data)
            tar.addfile(ti, io.BytesIO(data))
            for ti, payload in members:
                if payload is not None:
                    ti.size = len(payload)
                    tar.addfile(ti, io.BytesIO(payload))
                else:
                    tar.addfile(ti)
        return path

    @staticmethod
    def hostile_cases():
        """name -> members of a tarball that must be refused."""
        def d(name):
            ti = tarfile.TarInfo(name)
            ti.type = tarfile.DIRTYPE
            ti.mode = 0o644
            return ti, None

        def f(name):
            return tarfile.TarInfo(name), b"x"
        fifo = tarfile.TarInfo(PACK + "/fifo")
        fifo.type = tarfile.FIFOTYPE
        link = tarfile.TarInfo(PACK + "/link")
        link.type = tarfile.SYMTYPE
        link.linkname = "/etc/passwd"
        hard = tarfile.TarInfo(PACK + "/hard")
        hard.type = tarfile.LNKTYPE
        hard.linkname = "/etc/passwd"
        cases = {
            "absolute": [f("/tmp/evil")],
            "dotdot": [f(PACK + "/a/../../x")],
            "trailing-dotdot": [d(PACK + "/sub/.."), d(PACK + "/..")],
            "fifo": [(fifo, None)],
            "symlink": [(link, None), f(PACK + "/link/x")],
            "hardlink": [(hard, None)],
            "device": [(tarfile.TarInfo(PACK + "/dev"), None)],
        }
        cases["device"][0][0].type = tarfile.CHRTYPE
        return cases

    def assert_hostile_refused(self, server):
        packs = server / "CarlaUnreal" / "Packs"
        for name, members in self.hostile_cases().items():
            path = self.hostile_tar(name, members)
            rc, out, err = run("install", path, "--server", server)
            self.assertEqual(rc, 1, name)
            self.assertIn("carla-pack: error:", err, name)
            self.assertTrue("unsafe" in err or "non-regular" in err or "cannot extract" in err, (name, err))
            self.assertEqual(list(packs.iterdir()) if packs.exists() else [], [], name)
            rc, out, err = run("verify", path)
            self.assertEqual(rc, 1, name)

    def test_hostile_tarballs_are_refused_and_leave_nothing_behind(self):
        server = self.fake_server()
        packs = server / "CarlaUnreal" / "Packs"
        self.assert_hostile_refused(server)
        # a manifest whose files{} keys escape the pack is rejected before anything is hashed
        evil = self.tmp / "evilkeys"
        m = cp.new_manifest(PACK, "1.0.0", "0.10.0")
        m["files"] = {"../../../etc/passwd": "sha256:0"}
        write(evil / "carla-pack.json", json.dumps(m))
        rc, _, err = run("verify", evil)
        self.assertEqual(rc, 1)
        self.assertIn("not a normalised relative path", err)
        # `tar czf x .` style members (./Pack/...) are fine
        dot_tar = self.tmp / "dot.tar.gz"
        self.init_and_add()
        out_pack, _, _ = self.build_fake()
        with tarfile.open(str(dot_tar), "w:gz") as tar:
            tar.add(str(out_pack), arcname="./" + PACK)
        self.assertEqual(run("verify", dot_tar)[0], 0)
        self.assertEqual(run("install", dot_tar, "--server", server)[0], 0)

    def odd_modes_tar(self):
        """A valid pack tarball whose members carry modes the 'data' filter must sanitise."""
        self.init_and_add()
        out_pack, tar_path, _ = self.build_fake()
        odd = self.tmp / "odd-modes.tar.gz"
        with tarfile.open(str(odd), "w:gz") as tar:
            def fix(ti):
                if ti.isdir():
                    ti.mode = 0o000                     # unreadable dir: extraction would fail unsanitised
                elif ti.name.endswith(".ucas"):
                    ti.mode = 0o6777                    # setuid/setgid + world-writable
                else:
                    ti.mode = 0o600
                ti.uid = ti.gid = 12345
                return ti
            tar.add(str(out_pack), arcname=PACK, filter=fix)
        return odd

    def assert_sanitised_install(self, server, tar_path):
        rc, out, err = run("install", tar_path, "--server", server, "--force")
        self.assertEqual(rc, 0, err)
        installed = server / "CarlaUnreal" / "Packs" / PACK
        self.assertEqual(run("verify", installed)[0], 0)
        ucas = installed / "Content" / "Paks" / "Linux" / "TestPackCarlaUnreal-Linux.ucas"
        self.assertEqual(ucas.stat().st_mode & 0o7777, 0o755)          # no setuid/setgid, no g/o write
        dir_mode = (installed / "Content").stat().st_mode & 0o7777     # was 0o000 in the archive
        self.assertEqual(dir_mode & 0o700, 0o700, oct(dir_mode))       # usable again (filter: umask default; fallback: 0o755)
        self.assertEqual(dir_mode & 0o7000, 0, oct(dir_mode))

    @unittest.skipUnless(hasattr(tarfile, "data_filter"), "tarfile has no 'data' filter on this interpreter")
    def test_extract_uses_the_data_filter_when_available(self):
        import warnings
        server = self.fake_server()
        odd = self.odd_modes_tar()
        seen = []
        real_extractall = tarfile.TarFile.extractall

        def spy(self_, *a, **k):
            seen.append(k.get("filter"))
            return real_extractall(self_, *a, **k)
        tarfile.TarFile.extractall = spy  # type: ignore[method-assign]
        try:
            with warnings.catch_warnings(record=True) as caught:
                warnings.simplefilter("always")
                self.assert_hostile_refused(server)
                self.assert_sanitised_install(server, odd)
        finally:
            tarfile.TarFile.extractall = real_extractall
        self.assertTrue(seen and all(f == "data" for f in seen), seen)
        # no "Python 3.14 will, by default, filter extracted tar archives" warning from any call
        self.assertEqual([str(w.message) for w in caught if issubclass(w.category, DeprecationWarning)], [])

    def test_extract_fallback_without_the_data_filter(self):
        """Python 3.8-3.11 without the backport: the vetted member list plus hand-sanitised modes."""
        import warnings
        server = self.fake_server()
        odd = self.odd_modes_tar()
        had_filter = hasattr(tarfile, "data_filter")
        saved = getattr(tarfile, "data_filter", None)
        seen = []
        real_extractall = tarfile.TarFile.extractall

        def spy(self_, *a, **k):
            seen.append(("filter" in k, [(m.name, m.mode) for m in k.get("members", [])]))
            return real_extractall(self_, *a, **k)
        if had_filter:
            del tarfile.data_filter
        tarfile.TarFile.extractall = spy  # type: ignore[method-assign]
        try:
            with warnings.catch_warnings():
                # 3.12/3.13 warn when extractall runs without a filter; that is the
                # path older interpreters take silently, and it is what is under test
                warnings.simplefilter("ignore", DeprecationWarning)
                self.assert_hostile_refused(server)
                self.assert_sanitised_install(server, odd)
        finally:
            tarfile.TarFile.extractall = real_extractall
            if had_filter:
                tarfile.data_filter = saved
        self.assertTrue(seen)
        self.assertTrue(all(not with_filter for with_filter, _ in seen), seen)
        modes = dict(m for _, members in seen for m in members)
        self.assertEqual(modes[PACK + "/Content/Paks/Linux/TestPackCarlaUnreal-Linux.ucas"], 0o755)
        self.assertEqual(modes[PACK + "/Content"], 0o755)
        self.assertEqual(modes[PACK + "/carla-pack.json"], 0o600)
        # the sanitiser itself, member by member
        for mode, expect in ((0o6777, 0o755), (0o000, 0o600), (0o644, 0o644), (0o4755, 0o755), (0o666, 0o644), (0o710, 0o710)):
            ti = tarfile.TarInfo("f")
            ti.mode = mode
            self.assertEqual(cp.sanitize_member_mode(ti).mode, expect, oct(mode))
        ti = tarfile.TarInfo("d")
        ti.type = tarfile.DIRTYPE
        ti.mode = 0
        self.assertEqual(cp.sanitize_member_mode(ti).mode, 0o755)

    def test_platform_mismatch(self):
        self.init_and_add()
        out_pack, _, _ = self.build_fake()
        m = json.loads((out_pack / "carla-pack.json").read_text())
        m["platform"] = "Win64"
        (out_pack / "carla-pack.json").write_text(json.dumps(m))
        win_tar = self.tmp / "win.tar.gz"
        with tarfile.open(str(win_tar), "w:gz") as tar:
            tar.add(str(out_pack), arcname=PACK)
        server = self.fake_server()
        rc, out, err = run("inspect", win_tar, "--server", server, "--json")
        self.assertEqual(rc, 3)
        self.assertIn("platform mismatch: pack is for 'Win64', server is 'Linux'",
                      json.loads(out)["server"]["problems"][0])
        rc, _, err = run("install", win_tar, "--server", server)
        self.assertEqual(rc, 1)
        self.assertIn("platform mismatch", err)

    def test_inspect_unbuilt_source_pack(self):
        self.init_and_add()
        rc, out, _ = run("inspect", self.pack_dir)
        self.assertEqual(rc, 0)
        self.assertIn("(not built)", out)
        rc, _, err = run("inspect", self.tmp / "missing.tar.gz")
        self.assertEqual(rc, 1)
        self.assertIn("no such pack", err)


def fake_registry(path, names, wide=()):
    """An AssetRegistry.bin with the real header + SaveNameBatch layout (see cp.registry_names)."""
    import struct
    entries = []
    for n in list(names) + list(wide):
        if n in wide:
            b = n.encode("utf-16-le")
            entries.append((0x80 | (len(n) >> 8), len(n) & 0xFF, b))
        else:
            b = n.encode("latin-1")
            entries.append((len(n) >> 8, len(n) & 0xFF, b))
    strings = b"".join(e[2] for e in entries)
    data = (bytes(range(16)) + struct.pack("<II", 24, 1)
            + struct.pack("<IIQ", len(entries), len(strings), cp.NAME_BATCH_HASH_VERSION)
            + b"".join(struct.pack("<Q", 0x1234 + i) for i in range(len(entries)))
            + b"".join(bytes([e[0], e[1]]) for e in entries)
            + strings + b"\x00" * 64 + b"/TestPack/NotAName" + b"\x00" * 8)   # body: bytes, not names
    return write(path, data)


class TestAddValidation(PackTestBase):
    def test_add_validates_before_copying_anything(self):
        pack = self.tmp / "Packs" / PACK
        run("init", PACK, "--root", str(self.tmp / "Packs"))
        umap = self.tmp / "MyTown.umap"
        write(umap, b"umap")
        # A directory where a catalog file is expected: the map must not be
        # half-added and the manifest must stay untouched.
        rc, _, err = run("add", PACK, "--root", str(self.tmp / "Packs"),
                         "--map", str(umap), "--props", str(self.tmp))
        self.assertNotEqual(rc, 0)
        self.assertIn("catalog JSON file, not a directory", err)
        manifest = json.loads((pack / "carla-pack.json").read_text())
        self.assertEqual(manifest["maps"], [])
        self.assertFalse((pack / "Content" / "Maps" / "MyTown.umap").exists())


class TestCheckBase(PackTestBase):
    BASE_NAMES = ["/Game/Carla/Maps/Town10HD_Opt", "Town10HD_Opt", "/Game/Carla/Static/TestPack/SM_TestPack",
                  "SM_TestPack", "TestPack", "/Game/Carla/Static/GenericMaterials/Textures/Other", "Other",
                  "/Game/Carla/Maps/TestMaps/EmptyMap"]

    def test_registry_names_parses_the_name_batch(self):
        reg = fake_registry(self.tmp / "AssetRegistry.bin", self.BASE_NAMES, wide=["Größe/Ünïcode"])
        self.assertEqual(cp.registry_names(reg), self.BASE_NAMES + ["Größe/Ünïcode"])
        with self.assertRaises(cp.PackError):
            cp.registry_names(write(self.tmp / "junk.bin", b"\x00" * 100))
        rc, _, err = run("check-base", self.tmp / "junk.bin", "--pack", PACK)
        self.assertEqual(rc, 1)
        self.assertIn("not a cooked asset registry", err)

    def test_check_base_only_matches_the_mount_root(self):
        rel = self.releases / RELEASE / "Linux"
        fake_registry(rel / "AssetRegistry.bin", self.BASE_NAMES)
        fake_registry(rel / "Metadata" / "DevelopmentAssetRegistry.bin", self.BASE_NAMES)
        # folder/asset-name forms of TestPack and the base folder .../Other are not leaks
        rc, out, err = run("check-base", rel.parent, "--pack", PACK, "--pack", "Other")
        self.assertEqual(rc, 0, err)
        self.assertEqual(out.count("ok    "), 4)                      # 2 registries x 2 packs
        self.assertIn("OK: no /<Pack>/ names for TestPack, Other in 2 registry file(s)", out)
        # a leaked pack: names starting with /TestPack/
        fake_registry(rel / "AssetRegistry.bin", self.BASE_NAMES + ["/TestPack/Maps", "/TestPack/Maps/TestMap", "TestMap"])
        rc, out, err = run("check-base", rel.parent, "--pack", PACK, "--pack", "Other")
        self.assertEqual(rc, 1)
        self.assertIn("LEAK  " + str(rel / "AssetRegistry.bin") + ": 2 package name(s) under /TestPack/ (e.g. /TestPack/Maps, /TestPack/Maps/TestMap)", out)
        self.assertIn("ok    " + str(rel / "Metadata" / "DevelopmentAssetRegistry.bin"), out)
        self.assertIn("FAILED: 1 pack/registry combination(s) leaked", out)
        # single file form
        rc, out, _ = run("check-base", rel / "AssetRegistry.bin", "--pack", "Other")
        self.assertEqual(rc, 0)
        rc, out, _ = run("check-base", rel / "AssetRegistry.bin", "--pack", PACK)
        self.assertEqual(rc, 1)

    def test_check_base_discovers_packs_under_root(self):
        rel = self.releases / RELEASE / "Linux"
        fake_registry(rel / "AssetRegistry.bin", self.BASE_NAMES + ["/Second/Maps/X"])
        rc, out, err = run("check-base", rel, "--project", self.project)
        self.assertEqual(rc, 0, err)
        self.assertIn("no packs under", out)
        run("init", PACK, "--project", self.project)
        rc, out, err = run("check-base", rel, "--project", self.project)
        self.assertEqual(rc, 0, err)
        self.assertIn("for TestPack in 1 registry file(s)", out)
        write(self.root / "Second" / "Second.uplugin", "{}")          # a pack without a manifest counts too
        write(self.root / "notapack" / "README", "x")
        rc, out, err = run("check-base", rel, "--project", self.project)
        self.assertEqual(rc, 1)
        self.assertIn("under /Second/", out)
        self.assertEqual(cp.packs_under(self.root), ["Second", PACK])   # sorted, notapack skipped
        rc, _, err = run("check-base", self.tmp / "missing", "--pack", PACK)
        self.assertEqual(rc, 1)
        self.assertIn("--base not found", err)
        rc, _, err = run("check-base", rel, "--pack", "bad-name")
        self.assertEqual(rc, 1)
        self.assertIn("invalid pack name", err)


if __name__ == "__main__":
    unittest.main(verbosity=2)
