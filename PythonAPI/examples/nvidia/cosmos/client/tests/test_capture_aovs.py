"""``Capture`` honours the ``aovs`` tuple literally.

``semantic`` writes ``semantic_<camera>.mp4`` (CityScapes palette, the source
``--mask-classes`` needs) and ``instance`` writes the instance-coloured
``seg_<camera>.mp4``; asking for both gives both, from two sensors, frame for
frame.

The CARLA sensors are faked — a queue of synthetic BGRA buffers with the frame
ids the capture loop expects — but the videos are really encoded and decoded,
so the palette round trip through the control encoder is measured here and not
assumed (see :data:`carla_cosmos.mask.MAX_CODEC_L1`).
"""

import numpy as np
import pytest

import carla

from carla_cosmos import controls, mask
from carla_cosmos.capture import AOV_BLUEPRINTS, Capture
from carla_cosmos.clip import Clip
from carla_cosmos.rig import MountedCamera, Rig

W, H, FPS, FRAMES = 64, 48, 10, 4
CAMERA = "camera:front:wide:120fov"

# tags painted into the fake semantic buffer: sky / road / car / pedestrian
TAGS = (11, 1, 14, 12)


# ----------------------------------------------------------------------------- fakes

class FakeImage:
    """What ``sensor.listen`` hands us: a frame id and a raw BGRA buffer."""

    def __init__(self, frame: int, bgra: np.ndarray) -> None:
        self.frame = frame
        self.height, self.width = bgra.shape[:2]
        self.raw_data = bgra.tobytes()


class FakeSensor:
    def __init__(self, type_id: str) -> None:
        self.type_id = type_id
        self.callback = None

    def listen(self, callback) -> None:
        self.callback = callback

    def stop(self) -> None:
        pass

    def destroy(self) -> None:
        pass


class FakeBlueprint:
    def __init__(self, type_id: str) -> None:
        self.id = type_id
        self.attributes: dict[str, str] = {}

    def set_attribute(self, key: str, value: str) -> None:
        self.attributes[key] = value


class FakeWeather:
    cloudiness = 10.0
    precipitation = 0.0
    sun_altitude_angle = 45.0


class FakeMap:
    name = "Carla/Maps/TestTown"


class FakeEgo:
    id = 42
    type_id = "vehicle.test.ego"


class FakeWorld:
    """Just enough world for ``_open_streams`` and ``_build_manifest``."""

    def __init__(self) -> None:
        self.spawned: list[str] = []

    def get_blueprint_library(self):
        return self

    def find(self, type_id: str) -> FakeBlueprint:
        return FakeBlueprint(type_id)

    def spawn_actor(self, bp: FakeBlueprint, transform, attach_to=None) -> FakeSensor:
        self.spawned.append(bp.id)
        return FakeSensor(bp.id)

    def get_weather(self) -> FakeWeather:
        return FakeWeather()

    def get_map(self) -> FakeMap:
        return FakeMap()


# ----------------------------------------------------------------------------- fake frames

def _semantic_bgra(frame: int) -> np.ndarray:
    """Horizontal bands of TAGS, shifted per frame so the videos are not static."""
    bgra = np.zeros((H, W, 4), np.uint8)
    band = H // len(TAGS)
    for i, tag in enumerate(TAGS):
        bgra[i * band:(i + 1) * band, :, 2] = tag
    return np.roll(bgra, frame, axis=0)


def _instance_bgra(frame: int) -> np.ndarray:
    bgra = np.zeros((H, W, 4), np.uint8)
    bgra[:, :, 1] = 1 + frame          # id high byte
    bgra[H // 2:, :, 0] = 7            # id low byte on the lower half
    return bgra


def _depth_bgra(frame: int) -> np.ndarray:
    bgra = np.zeros((H, W, 4), np.uint8)
    bgra[:, :, 2] = np.linspace(1, 255, W, dtype=np.uint8)[None, :]
    bgra[:, :, 1] = frame
    return bgra


def _rgb_bgra(frame: int) -> np.ndarray:
    bgra = np.zeros((H, W, 4), np.uint8)
    bgra[:, :, :3] = (17 * (frame + 1)) % 256
    return bgra


_BUFFERS = {"rgb": _rgb_bgra, "depth": _depth_bgra,
            "semantic": _semantic_bgra, "instance": _instance_bgra}


def run_fake_capture(tmp_path, aovs, seg_mode="instance", edge=False, frames=FRAMES):
    """Drive the capture loop over fake sensors; returns ``(capture, streams, clip_dir)``."""
    world = FakeWorld()
    rig = Rig.single(width=W, height=H)
    cap = Capture(world, FakeEgo(), rig, frames=frames, fps=FPS, aovs=aovs,
                  seg_mode=seg_mode, edge=edge)
    mounted = MountedCamera(camera=rig.cameras[0], t_flu=(0.0, 0.0, 1.5), rpy_flu=(0.0, 0.0, 0.0),
                            attach=carla.Transform(), shifted=False)
    clip_dir = tmp_path / "clip"
    clip_dir.mkdir(parents=True, exist_ok=True)
    tmp_dir = clip_dir / "tmp"
    tmp_dir.mkdir()
    stream = cap._open_streams(mounted, clip_dir, tmp_dir)
    for i in range(frames):
        fid = 1000 + i
        for aov in stream.queues:
            stream.queues[aov].put(FakeImage(fid, _BUFFERS[aov](i)))
        cap._process_frame(stream, i, fid, first=(i == 0))
    for writer in stream.writers.values():
        writer.close()
    cap._encode_depth([stream], clip_dir)
    return cap, mounted, stream, clip_dir


def build_clip(cap, mounted, clip_dir) -> Clip:
    manifest = cap._build_manifest("fake_clip", [mounted], np.zeros(3), seed=1,
                                   recorder=None, carla_version="0.10.0")
    manifest.scene_dir = None
    clip = Clip(path=clip_dir, manifest=manifest)
    clip.save_manifest()
    return clip


# ----------------------------------------------------------------------------- tests

def test_semantic_and_instance_aovs_are_both_written(tmp_path):
    cap, mounted, stream, clip_dir = run_fake_capture(
        tmp_path, ("rgb", "depth", "semantic", "instance"))

    assert sorted(stream.sensors) == ["depth", "instance", "rgb", "semantic"]
    assert set(cap.world.spawned) == {AOV_BLUEPRINTS[a] for a in ("rgb", "depth", "semantic", "instance")}

    names = ["rgb", "depth", "seg", "semantic"]
    for kind in names:
        assert (clip_dir / f"{kind}_camera_front_wide_120fov.mp4").is_file(), kind
    counts = {kind: controls.probe_video(clip_dir / f"{kind}_camera_front_wide_120fov.mp4")["frames"]
              for kind in names}
    assert set(counts.values()) == {FRAMES}, counts

    # the seg control keeps the random instance palette, the semantic AOV is CityScapes
    assert not mask.is_palette_video(clip_dir / "seg_camera_front_wide_120fov.mp4")
    assert mask.is_palette_video(clip_dir / "semantic_camera_front_wide_120fov.mp4")


def test_semantic_video_survives_the_control_encoder(tmp_path):
    """Class ids come back exactly, and no pixel is further from its palette
    colour than the guard :data:`mask.MAX_CODEC_L1` allows."""
    _, _, _, clip_dir = run_fake_capture(tmp_path, ("semantic",))

    worst = 0
    with controls.VideoReader(clip_dir / "semantic_camera_front_wide_120fov.mp4") as reader:
        for i, frame in enumerate(reader):
            worst = max(worst, int(mask._palette_distance(frame).max()))
            expected = controls.semantic_tags(_semantic_bgra(i))
            assert (mask.tags_from_frame(frame) == expected).all(), f"frame {i}"
    assert worst < mask.MAX_CODEC_L1, f"codec L1 {worst} is not under the guard"


def test_manifest_registers_the_semantic_video(tmp_path):
    cap, mounted, _, clip_dir = run_fake_capture(tmp_path, ("rgb", "depth", "semantic", "instance"))
    clip = build_clip(cap, mounted, clip_dir)

    assert clip.manifest.videos["semantic/camera_front_wide_120fov"] == \
        "semantic_camera_front_wide_120fov.mp4"
    assert clip.manifest.video("seg", CAMERA) == "seg_camera_front_wide_120fov.mp4"
    assert "semantic" in clip.manifest.aovs

    loaded = Clip.load(clip_dir)
    assert loaded.manifest == clip.manifest
    assert loaded.validate() == []
    assert loaded.validate(for_masking=True) == []
    assert mask.semantic_video(loaded, CAMERA).name == "semantic_camera_front_wide_120fov.mp4"


def test_masking_a_clip_without_the_semantic_aov_is_reported(tmp_path):
    """An instance-only clip (what every clip captured before this AOV existed
    looks like) still loads and validates — masking is what it cannot do."""
    cap, mounted, stream, clip_dir = run_fake_capture(tmp_path, ("rgb", "depth", "instance"))
    assert "semantic" not in stream.sensors
    assert not (clip_dir / "semantic_camera_front_wide_120fov.mp4").exists()

    clip = build_clip(cap, mounted, clip_dir)
    assert "semantic/camera_front_wide_120fov" not in clip.manifest.videos
    assert clip.validate() == []

    errors = clip.validate(for_masking=True)
    assert len(errors) == 1
    assert "no semantic class information" in errors[0]
    assert "semantic_camera_front_wide_120fov.mp4" in errors[0]
    assert "'semantic' AOV" in errors[0]


def test_semantic_seg_mode_still_colours_the_seg_control(tmp_path):
    """``seg_mode='semantic'`` keeps working: seg and the semantic AOV agree."""
    _, _, stream, clip_dir = run_fake_capture(tmp_path, ("rgb", "semantic"), seg_mode="semantic")
    assert sorted(stream.sensors) == ["rgb", "semantic"]
    for kind in ("seg", "semantic"):
        assert mask.is_palette_video(clip_dir / f"{kind}_camera_front_wide_120fov.mp4")


def test_seg_mode_semantic_requires_the_semantic_aov():
    with pytest.raises(ValueError, match="needs the 'semantic' AOV"):
        Capture(FakeWorld(), FakeEgo(), Rig.single(), frames=4, fps=FPS,
                aovs=("rgb", "instance"), seg_mode="semantic")
