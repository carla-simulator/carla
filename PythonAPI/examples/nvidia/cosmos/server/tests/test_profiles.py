from pathlib import Path

import pytest

from carla_cosmos_server.config import DEFAULT_PROFILES_DIR, default_run_dir
from carla_cosmos_server.profiles import GpuInfo, load_profiles, select_profile


def gpus(n, mib=81559):
    return [GpuInfo(i, "H100", mib) for i in range(n)]


def test_builtin_profiles_valid_and_auto_selection():
    profiles = load_profiles(DEFAULT_PROFILES_DIR)
    assert {"mock", "nano-1gpu", "nano-2gpu", "full-8gpu", "av-8gpu"} <= set(profiles)
    for p in profiles.values():
        assert p.validate() == [], p.name
    assert select_profile(profiles, [], "auto").name == "mock"
    assert select_profile(profiles, gpus(1), "auto").name == "nano-1gpu"
    assert select_profile(profiles, gpus(2), "auto").name == "nano-2gpu"
    assert select_profile(profiles, gpus(4), "auto").name == "nano-2gpu"
    assert select_profile(profiles, gpus(8), "auto").name == "full-8gpu"
    assert select_profile(profiles, gpus(8), "av-8gpu").name == "av-8gpu"  # manual only
    with pytest.raises(ValueError, match="no profile matches"):
        select_profile(profiles, gpus(1, mib=32607), "auto")  # a 32 GB consumer GPU
    with pytest.raises(ValueError, match="unknown profile"):
        select_profile(profiles, [], "nope")


def test_run_dir_shortening(tmp_path):
    short = Path("/state")
    assert default_run_dir(short) == Path("/state/run")
    long = tmp_path / ("x" * 120)
    rd = default_run_dir(long)
    assert len(str(rd)) <= 80 and rd != long / "run"
