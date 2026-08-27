from pathlib import Path

import pytest

from carla_cosmos_server.config import DEFAULT_PROFILES_DIR, default_run_dir, detect_image_variant
from carla_cosmos_server.profiles import GpuInfo, load_profiles, plan_layout, select_profile, super_tp

H100, RTX6000PRO, B200, RTX5090 = 81559, 97887, 183359, 32607   # MiB as nvidia-smi reports them


def gpus(n, mib=H100, name="H100"):
    return [GpuInfo(i, name, mib) for i in range(n)]


def layout(profile):
    """{worker name: (gpus, args)} for compact assertions."""
    return {w.name: (w.gpus, w.args) for w in profile.workers}


def test_builtin_profiles_valid_and_selection():
    profiles = load_profiles(DEFAULT_PROFILES_DIR)
    assert {"mock", "nano-1gpu", "nano-2gpu", "full-8gpu", "av-8gpu"} <= set(profiles)
    for p in profiles.values():
        assert p.validate() == [], p.name
    assert select_profile(profiles, [], "auto", "nano").name == "mock"          # 0 GPUs: the only matching YAML
    assert select_profile(profiles, gpus(8), "av-8gpu").name == "av-8gpu"       # manual by name
    assert select_profile(profiles, gpus(1), "nano-1gpu").name == "nano-1gpu"
    assert select_profile(profiles, gpus(8), "auto", "full").name.startswith("auto-8x80g-full")  # planned
    with pytest.raises(ValueError, match="unknown profile"):
        select_profile(profiles, [], "nope")
    with pytest.raises(ValueError, match="no model weights"):
        select_profile(profiles, gpus(2), "auto", "none")     # a -nomodels image on a GPU host


def test_super_tp_from_gpu_memory():
    assert super_tp(H100 / 1024) == 4        # 80 GB: 123.5/4 + 16 fits, /2 does not
    assert super_tp(RTX6000PRO / 1024) == 2  # 96 GB
    assert super_tp(B200 / 1024) == 1
    assert super_tp(RTX5090 / 1024) is None


def test_plan_layout_beefy_and_many_gpus():
    # one big GPU: Nano only, renderer beside it so scene controls still work
    one = plan_layout(gpus(1, RTX6000PRO, "RTX PRO 6000"), "nano")
    assert layout(one) == {"cosmos3-nano": ([0], ["--tp", "1"]), "wsm-renderer": ([0], [])}
    # two: Nano | Transfer 2.5 + renderer
    two = layout(plan_layout(gpus(2, RTX6000PRO), "nano"))
    assert two["cosmos3-nano"] == ([0], ["--tp", "1"]) and two["transfer25"] == ([1], []) and two["wsm-renderer"] == ([1], [])
    # four x 96 GB with :full: Super first (TP=2), then Transfer 2.5, then Nano
    four = layout(plan_layout(gpus(4, RTX6000PRO), "full"))
    assert four["cosmos3-super"] == ([0, 1], ["--tp", "2"]) and four["transfer25"][0] == [2]
    assert four["cosmos3-nano"][0] == [3] and "transfer25-av" not in four
    # :full on few GPUs keeps Super (the reason to pull it): 2 x 96 GB -> Super only, renderer beside it
    two_full = layout(plan_layout(gpus(2, RTX6000PRO), "full"))
    assert set(two_full) == {"cosmos3-super", "wsm-renderer"} and two_full["wsm-renderer"][0] == [0]
    assert set(layout(plan_layout(gpus(1, B200, "B200"), "full"))) == {"cosmos3-super", "wsm-renderer"}
    # the same four GPUs with :nano never start Super; the spare pair does AV multiview
    four_nano = layout(plan_layout(gpus(4, RTX6000PRO), "nano"))
    assert "cosmos3-super" not in four_nano and four_nano["transfer25-av"] == ([2, 3], ["--nproc", "2"])
    # eight H100 :full: Nano | Transfer | Super TP=4 | AV x2 — every GPU busy
    eight = plan_layout(gpus(8), "full")
    e = layout(eight)
    assert e["cosmos3-super"] == ([0, 1, 2, 3], ["--tp", "4"]) and e["transfer25"][0] == [4]
    assert e["cosmos3-nano"][0] == [5] and e["transfer25-av"] == ([6, 7], ["--nproc", "2"])
    assert sorted(g for w in eight.workers for g in w.gpus if w.type != "wsm_renderer") == list(range(8))
    assert "idle" not in eight.description
    # eight x 96 GB :nano: AV multiview takes the six spare GPUs (6 views)
    assert layout(plan_layout(gpus(8, RTX6000PRO), "nano"))["transfer25-av"] == ([2, 3, 4, 5, 6, 7], ["--nproc", "6"])
    # 32 GB consumer card: Nano does not fit, Transfer 2.5 runs at 480p
    small = layout(plan_layout(gpus(1, RTX5090, "RTX 5090"), "nano"))
    assert small == {"transfer25": ([0], ["--default-resolution", "480"]), "wsm-renderer": ([0], [])}
    # three 32 GB cards: one Transfer worker each, AV needs 40 GiB per rank so none
    three_small = layout(plan_layout(gpus(3, RTX5090), "nano"))
    assert set(three_small) == {"transfer25", "wsm-renderer", "transfer25-2", "transfer25-3"}
    # heterogeneous: the biggest GPU goes to Nano
    mixed = plan_layout([GpuInfo(0, "RTX 5090", RTX5090), GpuInfo(1, "RTX PRO 6000", RTX6000PRO)], "nano")
    assert layout(mixed)["cosmos3-nano"] == ([1], ["--tp", "1"]) and layout(mixed)["transfer25"][0] == [0]
    for prof in (one, eight, mixed):
        assert prof.validate() == []


def test_detect_image_variant(tmp_path, monkeypatch):
    monkeypatch.delenv("CARLA_COSMOS_IMAGE_VARIANT", raising=False)
    assert detect_image_variant(tmp_path) == "none"
    monkeypatch.setenv("CARLA_COSMOS_IMAGE_VARIANT", "nano")
    assert detect_image_variant(tmp_path) == "nano"           # env only outside the image
    (tmp_path / "hf").mkdir()
    (tmp_path / "hf" / "ARTIFACTS_IMAGE").write_text("none\n")
    assert detect_image_variant(tmp_path) == "none"           # a -nomodels image: the marker wins over the env
    (tmp_path / "hf" / "ARTIFACTS_IMAGE").write_text("full\n")
    assert detect_image_variant(tmp_path) == "full"


def test_run_dir_shortening(tmp_path):
    short = Path("/state")
    assert default_run_dir(short) == Path("/state/run")
    long = tmp_path / ("x" * 120)
    rd = default_run_dir(long)
    assert len(str(rd)) <= 80 and rd != long / "run"
