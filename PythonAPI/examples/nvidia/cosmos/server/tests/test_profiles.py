from pathlib import Path

import pytest

from carla_cosmos_server.config import DEFAULT_PROFILES_DIR, default_run_dir, detect_image_variant
from carla_cosmos_server.profiles import (
    AUTO_LATENCY_MAX_GPUS,
    GpuInfo,
    load_profiles,
    nano_parallel_args,
    parallel_of,
    plan_layout,
    resolve_mode,
    select_profile,
    super_tp,
)

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


# ----------------------------------------------------------------------------- latency mode

def test_mode_resolution_and_names():
    assert resolve_mode("auto", 1) == resolve_mode("auto", AUTO_LATENCY_MAX_GPUS) == "latency"
    assert resolve_mode("auto", AUTO_LATENCY_MAX_GPUS + 1) == "throughput"
    assert resolve_mode("throughput", 2) == "throughput" and resolve_mode("latency", 8) == "latency"
    with pytest.raises(ValueError, match="unknown mode"):
        resolve_mode("fast", 2)
    four = plan_layout(gpus(4, RTX6000PRO), "nano", "auto")
    eight = plan_layout(gpus(8, RTX6000PRO), "nano", "auto")
    assert four.mode == "latency" and four.name == "auto-4x96g-nano-latency"
    assert eight.mode == "throughput" and eight.name == "auto-8x96g-nano"          # today's layout, today's name
    assert layout(eight) == layout(plan_layout(gpus(8, RTX6000PRO), "nano", "throughput"))
    assert plan_layout(gpus(4, RTX6000PRO), "nano", "throughput").name == "auto-4x96g-nano"
    # manual YAML profiles are untouched by the mode
    profiles = load_profiles(DEFAULT_PROFILES_DIR)
    assert select_profile(profiles, gpus(8), "av-8gpu", "nano", "latency").mode == "manual"
    assert select_profile(profiles, [], "auto", "nano", "latency").name == "mock"
    assert select_profile(profiles, gpus(4, RTX6000PRO), "auto", "nano", "auto").name == "auto-4x96g-nano-latency"


def test_nano_parallel_args_and_parallel_of():
    assert nano_parallel_args(1) == ["--tp", "1"]
    assert nano_parallel_args(2) == ["--tp", "1", "--cfg-parallel", "2"]
    assert nano_parallel_args(4) == ["--tp", "1", "--cfg-parallel", "2", "--ulysses", "2"]
    assert nano_parallel_args(8) == ["--tp", "1", "--cfg-parallel", "2", "--ulysses", "4"]
    assert parallel_of(["--tp", "1", "--cfg-parallel", "2", "--ulysses", "2"]) == {"cfg": 2, "ulysses": 2}
    assert parallel_of(["--context-parallel-size", "4", "--master-port", "1"]) == {"context": 4}
    assert parallel_of(["--nproc", "4"]) == {"nproc": 4} and parallel_of(["--tp", "1"]) == {}


def test_latency_layouts():
    # 4 x 96 GB :nano — the brev node: Nano cfg-parallel 2 x Ulysses 2 and Transfer 2.5 CP=4 share all four
    # GPUs; AV multiview does not fit beside them (37 + 28 + 22 GiB resident + 12 GiB working set > 86 GiB)
    four = plan_layout(gpus(4, RTX6000PRO, "RTX PRO 6000"), "nano", "latency")
    L = layout(four)
    assert L["cosmos3-nano"] == ([0, 1, 2, 3], ["--tp", "1", "--cfg-parallel", "2", "--ulysses", "2"])
    assert L["transfer25"] == ([0, 1, 2, 3], ["--context-parallel-size", "4", "--master-port", "12342"])
    assert L["wsm-renderer"] == ([3], []) and "transfer25-av" not in L
    assert "AV multiview skipped" in four.description and "GiB" in four.description
    assert four.shared_gpus() == {g: ["cosmos3-nano", "transfer25"] for g in range(4)}
    assert four.validate() == [] and four.mode == "latency"
    # 2 x 96 GB: cfg-parallel only (Ulysses 1), CP=2
    two = layout(plan_layout(gpus(2, RTX6000PRO), "nano", "latency"))
    assert two["cosmos3-nano"] == ([0, 1], ["--tp", "1", "--cfg-parallel", "2"])
    assert two["transfer25"] == ([0, 1], ["--context-parallel-size", "2", "--master-port", "12342"])
    assert two["wsm-renderer"] == ([1], [])
    # a single GPU degenerates to today's layout (nothing to spread a query over), still tagged latency
    one = plan_layout(gpus(1, RTX6000PRO), "nano", "latency")
    assert layout(one) == layout(plan_layout(gpus(1, RTX6000PRO), "nano", "throughput"))
    assert one.mode == "latency" and one.name == "auto-1x96g-nano-latency"
    small = plan_layout(gpus(1, RTX5090, "RTX 5090"), "nano", "latency")
    assert layout(small) == {"transfer25": ([0], ["--default-resolution", "480"]), "wsm-renderer": ([0], [])}
    # three 32 GB cards: Nano does not fit, Transfer 2.5 @480p context-parallel over all three, alone
    three = plan_layout(gpus(3, RTX5090), "nano", "latency")
    assert layout(three)["transfer25"] == ([0, 1, 2], ["--default-resolution", "480", "--context-parallel-size", "3",
                                                       "--master-port", "12342"])
    assert three.shared_gpus() == {}
    # 8 x H100 :full — Super TP=4 x cfg-parallel 2 spans the node; Nano does not fit beside a Super shard on
    # 80 GB, Transfer 2.5 only at 480p
    eight = plan_layout(gpus(8), "full", "latency")
    E = layout(eight)
    assert E["cosmos3-super"] == (list(range(8)), ["--tp", "4", "--cfg-parallel", "2"])
    assert E["transfer25"][0] == list(range(8)) and E["transfer25"][1][:2] == ["--default-resolution", "480"]
    assert "cosmos3-nano" not in E and "Nano skipped" in eight.description
    assert eight.name == "auto-8x80g-full-latency"
    # 4 x 96 GB :full — Super TP=2 x cfg-parallel 2 leaves no room for anything else
    assert set(layout(plan_layout(gpus(4, RTX6000PRO), "full", "latency"))) == {"cosmos3-super", "wsm-renderer"}
    # cards large enough for all three (B200): AV multiview joins with one rank per GPU, 7 views on 4 ranks
    big = plan_layout(gpus(4, B200, "B200"), "nano", "latency")
    B = layout(big)
    assert B["transfer25-av"] == ([0, 1, 2, 3], ["--nproc", "4", "--master-port", "12341"])
    assert big.shared_gpus()[0] == ["cosmos3-nano", "transfer25", "transfer25-av"]
    assert {w.name: w.parallel for w in big.workers} == {
        "cosmos3-nano": {"cfg": 2, "ulysses": 2}, "transfer25": {"context": 4}, "transfer25-av": {"nproc": 4},
        "wsm-renderer": {}}
    # three GPUs: Nano keeps a power of two, Transfer takes all three
    three_big = layout(plan_layout(gpus(3, RTX6000PRO), "nano", "latency"))
    assert three_big["cosmos3-nano"][0] == [0, 1] and three_big["transfer25"][0] == [0, 1, 2]
    # a shipped manual latency layout for the 4-GPU node with AV instead of Transfer 2.5 general
    prof = load_profiles(DEFAULT_PROFILES_DIR)["latency-4gpu-av"]
    assert prof.validate() == [] and prof.mode == "manual"
    assert layout(prof)["transfer25-av"] == ([0, 1, 2, 3], ["--nproc", "4", "--master-port", "12341"])
    assert set(prof.shared_gpus()) == {0, 1, 2, 3}


def test_validate_rejects_shared_master_port():
    from carla_cosmos_server.profiles import Profile, WorkerSpec

    p = Profile(name="x", description="", workers=[
        WorkerSpec("a", "transfer25", ["transfer2.5"], [0, 1], ["--context-parallel-size", "2", "--master-port", "1"]),
        WorkerSpec("b", "transfer25_av", ["transfer2.5-av"], [0, 1], ["--nproc", "2", "--master-port", "1"])])
    assert any("master-port" in e for e in p.validate())
