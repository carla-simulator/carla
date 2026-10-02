"""Contract validation: every rejection must carry an actionable message."""

import pytest

from carla_cosmos.contracts import (
    AV_CAMERAS,
    BUILTIN_CONTRACTS,
    COSMOS3_NANO,
    TRANSFER25,
    TRANSFER25_AV,
    CameraManifest,
    ClipManifest,
    ControlInput,
    FrameRule,
    JobRequest,
    RigManifest,
    validate_request,
)


def make_manifest(cameras=("camera:front:wide:120fov",), fps=16, frames=93,
                  kinds=("rgb", "depth", "seg")) -> ClipManifest:
    cams = [CameraManifest(name=n, hfov=90, width=1280, height=720, t_flu=[1.7, 0, 1.44],
                           rpy_flu=[0, 0, 0], attach_ue={}) for n in cameras]
    videos = {f"{k}/{n.replace(':', '_')}": f"{k}_{n.replace(':', '_')}.mp4"
              for n in cameras for k in kinds}
    return ClipManifest(
        clip_id="t", carla_version="0.10.0", map="Town10HD_Opt", weather={},
        rig=RigManifest(name="test", mount="roofline", cameras=cams),
        fps=fps, frames=frames, ego_id=1, ego_type_id="vehicle.lincoln.mkz",
        rear_axle_offset_ue=[-1.4, 0, 0], aovs=["rgb"], videos=videos, scene_dir="scene",
    )


def request(backend, controls, **kw) -> JobRequest:
    return JobRequest(backend=backend, prompt="a rainy street", controls=controls, **kw)


# --------------------------------------------------------------------------- frame rules

def test_frame_rule_arithmetic():
    rule = FrameRule(min=93, base=0, step=93)
    assert rule.allows(93) and rule.allows(186)
    assert not rule.allows(94) and not rule.allows(92)
    assert rule.nearest(100) == [93, 186]

    av = FrameRule(min=29, base=29, step=28)
    assert av.allows(29) and av.allows(57) and av.allows(85)
    assert not av.allows(30) and not av.allows(28)

    wsm = FrameRule(min=101, max=303, base=0, step=101)
    assert wsm.allows(101) and wsm.allows(202) and not wsm.allows(100)
    assert not wsm.allows(404)


# --------------------------------------------------------------------------- happy paths

def test_transfer25_valid():
    m = make_manifest(fps=16, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(blob="b1", weight=0.7),
                                "edge": ControlInput(derive=True)},
                rgb={"camera:front:wide:120fov": "b0"})
    assert validate_request(TRANSFER25, r, m) == []


def test_cosmos3_valid_wsm_scene():
    m = make_manifest(fps=30, frames=303, kinds=("rgb", "depth", "seg"))
    r = request("cosmos3-nano", {"wsm": ControlInput(scene="s1"), "depth": ControlInput(blob="b2")})
    assert validate_request(COSMOS3_NANO, r, m) == []


def test_av_valid_subset_of_views():
    cams = AV_CAMERAS[:3]
    m = make_manifest(cameras=cams, fps=30, frames=87)  # 29 frames at 10 fps
    r = request("transfer2.5-av", {"hdmap_bbox": ControlInput(scene="s1")}, views=list(cams))
    assert validate_request(TRANSFER25_AV, r, m) == []


# --------------------------------------------------------------------------- rejections

def test_wrong_camera_set_for_av():
    m = make_manifest(cameras=("camera:front:wide:120fov",), fps=30, frames=87)
    r = request("transfer2.5-av", {"hdmap_bbox": ControlInput(scene="s1")},
                views=["camera:front:wide:120fov", "camera:top:down:90fov"])
    errors = validate_request(TRANSFER25_AV, r, m)
    assert any("camera:top:down:90fov" in e and "fixed cameras" in e for e in errors)
    assert any("not in the clip" in e for e in errors)


def test_too_many_views_single_view_backend():
    m = make_manifest(cameras=AV_CAMERAS[:2], fps=16, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(blob="b")},
                views=list(AV_CAMERAS[:2]),
                rgb={c: "b" for c in AV_CAMERAS[:2]})
    errors = validate_request(TRANSFER25, r, m)
    assert any("at most 1" in e for e in errors)


def test_wrong_frame_count_gives_nearest_valid():
    m = make_manifest(fps=16, frames=100)
    r = request("transfer2.5", {"depth": ControlInput(blob="b")},
                rgb={"camera:front:wide:120fov": "b0"})
    errors = validate_request(TRANSFER25, r, m)
    assert len(errors) == 1
    assert errors[0].startswith("transfer2.5 needs 93, 186, 279, ") and errors[0].endswith("(clip has 100)")


def test_wsm_frame_count_message_lists_all_valid_counts():
    m = make_manifest(fps=10, frames=93, kinds=("rgb", "depth", "seg"))
    r = request("cosmos3-nano", {"wsm": ControlInput(scene="s")})
    errors = validate_request(COSMOS3_NANO, r, m)
    assert errors == ["cosmos3-nano with wsm needs 101, 202 or 303 frames (clip has 93)"]


def test_av_frame_count_message_in_clip_fps():
    cams = AV_CAMERAS[:2]
    m = make_manifest(cameras=cams, fps=30, frames=90)  # 30 at 10 fps: needs 29 + 28k
    r = request("transfer2.5-av", {"hdmap_bbox": ControlInput(scene="s")}, views=list(cams))
    errors = validate_request(TRANSFER25_AV, r, m)
    assert len(errors) == 1
    assert errors[0].startswith("transfer2.5-av needs 87, 171, 255, ")
    assert "at 30 fps (29 + 28*k within >=29 at 10 fps)" in errors[0] and errors[0].endswith("(clip has 90)")


def test_out_of_range_without_step_message():
    m = make_manifest(fps=30, frames=400, kinds=("rgb", "depth"))
    r = request("cosmos3-nano", {"depth": ControlInput(blob="b")})
    errors = validate_request(COSMOS3_NANO, r, m)
    assert errors == ["cosmos3-nano needs 5..300 frames (clip has 400)"]


# --------------------------------------------------------------------------- scene fps

def test_wsm_scene_needs_renderer_compatible_fps():
    # 16 fps is fine for Cosmos 3 in general, but the renderer's 30 fps cannot be decimated to it
    m = make_manifest(fps=16, frames=101, kinds=("rgb", "depth", "seg"))
    r = request("cosmos3-nano", {"wsm": ControlInput(scene="s1")})
    errors = validate_request(COSMOS3_NANO, r, m)
    assert errors == ["wsm with a scene package needs fps in [10, 30] (clip is 16 fps): recapture with --fps 10"]
    # an uploaded wsm video at 16 fps is not subject to the renderer rule
    ok = request("cosmos3-nano", {"wsm": ControlInput(blob="b")})
    assert validate_request(COSMOS3_NANO, ok, m) == []
    # 10 and 30 fps scene packages pass
    for fps in (10, 30):
        m = make_manifest(fps=fps, frames=101, kinds=("rgb", "depth", "seg"))
        assert validate_request(COSMOS3_NANO, r, m) == []


def test_scene_fps_rule_on_builtin_contracts():
    for contract in BUILTIN_CONTRACTS.values():
        for spec in contract.controls:
            if spec.accepts_scene:
                assert spec.scene_fps == [10, 15, 30], (contract.id, spec.name)
                assert all(30 % f == 0 for f in spec.scene_fps)
            else:
                assert spec.scene_fps is None


def test_fps_mismatch_suggests_recapture():
    m = make_manifest(fps=24, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(blob="b")},
                rgb={"camera:front:wide:120fov": "b0"})
    errors = validate_request(TRANSFER25, r, m)
    assert errors == ["clip fps 24 not accepted by 'transfer2.5' (accepted: [16]): recapture with --fps 16"]


def test_wsm_frame_rule_applies_only_with_wsm():
    m = make_manifest(fps=30, frames=120, kinds=("rgb", "depth"))
    ok = request("cosmos3-nano", {"depth": ControlInput(blob="b")})
    assert validate_request(COSMOS3_NANO, ok, m) == []
    bad = request("cosmos3-nano", {"wsm": ControlInput(scene="s")})
    errors = validate_request(COSMOS3_NANO, bad, m)
    assert len(errors) == 1
    assert "wsm" in errors[0] and "101" in errors[0]


def test_unsupported_control():
    m = make_manifest(fps=16, frames=93)
    r = request("transfer2.5", {"wsm": ControlInput(scene="s")},
                rgb={"camera:front:wide:120fov": "b0"})
    errors = validate_request(TRANSFER25, r, m)
    assert any("'wsm' is not supported by 'transfer2.5'" in e for e in errors)


def test_missing_required_control():
    m = make_manifest(cameras=AV_CAMERAS, fps=30, frames=87)
    r = request("transfer2.5-av", {}, views=list(AV_CAMERAS))
    errors = validate_request(TRANSFER25_AV, r, m)
    assert any("'hdmap_bbox' is required" in e for e in errors)


def test_scene_not_accepted_by_general_control():
    m = make_manifest(fps=16, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(scene="s")},
                rgb={"camera:front:wide:120fov": "b0"})
    errors = validate_request(TRANSFER25, r, m)
    assert any("cannot be rendered from a scene package" in e for e in errors)


def test_weight_out_of_range():
    m = make_manifest(fps=16, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(blob="b", weight=1.5)},
                rgb={"camera:front:wide:120fov": "b0"})
    errors = validate_request(TRANSFER25, r, m)
    assert any("weight 1.5 outside [0.0, 1.0]" in e for e in errors)


def test_fps_mismatch():
    m = make_manifest(fps=30, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(blob="b")},
                rgb={"camera:front:wide:120fov": "b0"})
    errors = validate_request(TRANSFER25, r, m)
    assert any("fps 30 not accepted" in e and "[16]" in e for e in errors)


def test_missing_rgb_when_required():
    m = make_manifest(fps=16, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(blob="b")})
    errors = validate_request(TRANSFER25, r, m)
    assert any("RGB video" in e and "required" in e for e in errors)


def test_control_needs_exactly_one_source():
    m = make_manifest(fps=16, frames=93)
    r = request("transfer2.5", {"depth": ControlInput(blob="b", derive=True)},
                rgb={"camera:front:wide:120fov": "b0"})
    errors = validate_request(TRANSFER25, r, m)
    assert any("exactly one of blob, scene or derive" in e for e in errors)


def test_no_controls():
    r = request("cosmos3-nano", {})
    errors = validate_request(COSMOS3_NANO, r, make_manifest(fps=30, frames=120))
    assert any("no controls given" in e for e in errors)


def test_empty_prompt_rejected():
    with pytest.raises(ValueError, match="prompt"):
        JobRequest(backend="cosmos3-nano", prompt="   ")


def test_builtin_contract_ids():
    assert set(BUILTIN_CONTRACTS) == {"cosmos3-nano", "cosmos3-super", "transfer2.5", "transfer2.5-av"}
    assert TRANSFER25_AV.views == list(AV_CAMERAS)


def test_multiview_control_blobs():
    from carla_cosmos.contracts import AV_CAMERAS, TRANSFER25_AV, ControlInput, JobRequest, validate_request
    from carla_cosmos.contracts import CameraManifest, ClipManifest, RigManifest

    cams = list(AV_CAMERAS[:2])
    manifest = ClipManifest(
        clip_id="c", carla_version="x", map="m", weather={}, fps=30, frames=87, ego_id=1, ego_type_id="v",
        rear_axle_offset_ue=[0, 0, 0], aovs=[], videos={}, scene_dir="scene",
        rig=RigManifest(name="r", mount="exact", cameras=[
            CameraManifest(name=c, hfov=120, width=8, height=8, t_flu=[0, 0, 0], rpy_flu=[0, 0, 0], attach_ue={})
            for c in cams]))
    req = JobRequest(backend="transfer2.5-av", prompt="p", views=cams,
                     controls={"hdmap_bbox": ControlInput(blob="a" * 64)})
    errs = validate_request(TRANSFER25_AV, req, manifest)
    assert any("single blob for 2 views" in e for e in errs)
    req.controls["hdmap_bbox"] = ControlInput(blobs={cams[0]: "a" * 64})
    errs = validate_request(TRANSFER25_AV, req, manifest)
    assert any("no blob for view" in e for e in errs)
    req.controls["hdmap_bbox"] = ControlInput(blobs={c: "a" * 64 for c in cams})
    assert validate_request(TRANSFER25_AV, req, manifest) == []
    assert req.controls["hdmap_bbox"].blob_for(cams[1]) == "a" * 64
