import os

from cosmos_workers.common.hfcache import materialize_snapshot, snapshot_dir


def _fake_cache(root, repo="nvidia/Cosmos-1.0-Guardrail", rev="abc123"):
    repo_dir = root / "hub" / ("models--" + repo.replace("/", "--"))
    blobs = repo_dir / "blobs"
    snap = repo_dir / "snapshots" / rev
    (snap / "blocklist" / "nltk_data").mkdir(parents=True)
    blobs.mkdir()
    (blobs / "b1").write_bytes(b"wordnet")
    (blobs / "b2").write_text("readme")
    os.symlink("../../../../blobs/b1", snap / "blocklist" / "nltk_data" / "wordnet.zip")
    os.symlink("../../blobs/b2", snap / "README.md")
    (snap / "plain.txt").write_text("already real")
    return snap


def test_materialize_snapshot_replaces_symlinks_with_copies(tmp_path):
    snap = _fake_cache(tmp_path)
    assert snapshot_dir(tmp_path, "nvidia/Cosmos-1.0-Guardrail") == snap
    assert (snap / "README.md").is_symlink()

    assert materialize_snapshot(tmp_path, "nvidia/Cosmos-1.0-Guardrail") == 2
    for rel in ("README.md", "blocklist/nltk_data/wordnet.zip", "plain.txt"):
        assert not (snap / rel).is_symlink()
    assert (snap / "blocklist" / "nltk_data" / "wordnet.zip").read_bytes() == b"wordnet"
    assert (snap / "README.md").read_text() == "readme"
    # idempotent
    assert materialize_snapshot(tmp_path, "nvidia/Cosmos-1.0-Guardrail") == 0
    # unknown repo is a no-op
    assert materialize_snapshot(tmp_path, "nvidia/nope") == 0
