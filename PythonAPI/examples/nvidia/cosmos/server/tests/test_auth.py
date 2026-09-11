import json

from carla_cosmos_server.auth import TokenStore, bootstrap

from conftest import submission, upload


def test_token_store_roundtrip(tmp_path):
    store = TokenStore(tmp_path / "tokens.json")
    tok = store.create("ci")
    assert tok.startswith("cc_")
    tid = tok.split("_")[1]
    assert store.verify(tok) == tid
    assert store.verify(tok[:-1] + ("a" if tok[-1] != "a" else "b")) is None
    assert store.verify("cc_deadbeef_nope") is None
    assert store.verify("") is None
    # persisted hashed, never raw
    on_disk = json.loads((tmp_path / "tokens.json").read_text())
    assert tok.split("_")[2] not in json.dumps(on_disk)
    assert TokenStore(tmp_path / "tokens.json").verify(tok) == tid
    assert store.revoke(tid) and store.verify(tok) is None


def test_bootstrap_env_token_and_initial(tmp_path):
    store = TokenStore(tmp_path / "tokens.json")
    bootstrap(store, tmp_path / "initial.txt", "my-opaque-secret")
    assert store.verify("my-opaque-secret") is not None
    assert not (tmp_path / "initial.txt").exists()  # env token present -> nothing minted
    store2 = TokenStore(tmp_path / "t2.json")
    bootstrap(store2, tmp_path / "initial2.txt", None)
    tok = (tmp_path / "initial2.txt").read_text().strip()
    assert store2.verify(tok) is not None
    assert oct((tmp_path / "initial2.txt").stat().st_mode)[-3:] == "600"


def test_initial_token_logged_once_when_minted(tmp_path, caplog):
    import logging

    path = tmp_path / "initial.txt"
    with caplog.at_level(logging.INFO, logger="carla_cosmos_server.auth"):
        store = TokenStore(tmp_path / "tokens.json")
        bootstrap(store, path, None)
        tok = path.read_text().strip()
        minted = [r for r in caplog.records if r.levelno == logging.INFO and "first-boot API token" in r.getMessage()]
        assert len(minted) == 1
        assert minted[0].getMessage() == f"first-boot API token (also in {path}): {tok}"
        caplog.clear()
        bootstrap(TokenStore(tmp_path / "tokens.json"), path, None)  # restart: token exists, nothing minted
        assert not any("first-boot API token" in r.getMessage() for r in caplog.records)


def test_public_and_protected_routes(server):
    anon = server.client
    saved = dict(anon.headers)
    anon.headers.pop("Authorization")
    try:
        assert anon.get("/v1/health/live").status_code == 200
        assert anon.get("/v1/health/ready").status_code == 200
        assert anon.get("/ui").status_code == 200
        r = anon.get("/v1/models")
        assert r.status_code == 401 and r.headers["www-authenticate"] == "Bearer"
        # upload rejected without reading the body
        r = anon.put(f"/v1/blobs/{'0' * 64}", content=b"x" * (1 << 20))
        assert r.status_code == 401
        assert server.store.blob_stats() == (0, 0)
        assert anon.get("/v1/jobs", headers={"Authorization": "Bearer cc_nope_nope"}).status_code == 401
        assert anon.get("/v1/jobs", headers={"Authorization": "Basic abc"}).status_code == 401
    finally:
        anon.headers.update(saved)
    assert anon.get("/v1/models").status_code == 200


def test_token_id_recorded_on_job(server, clip16):
    rgb = upload(server, clip16.video("rgb", clip16.manifest.camera_names[0]))
    depth = upload(server, clip16.video("depth", clip16.manifest.camera_names[0]))
    body = submission(clip16, "cosmos3-nano", {"depth": {"blob": depth}}, rgb={clip16.manifest.camera_names[0]: rgb})
    r = server.client.post("/v1/jobs", json=body)
    assert r.status_code == 202, r.text
    assert r.json()["token_id"] == server.token_id
    other = server.tokens.create("other")
    mine = server.client.get("/v1/jobs?mine=true", headers={"Authorization": f"Bearer {other}"}).json()
    assert mine == []
    assert len(server.client.get("/v1/jobs?mine=true").json()) == 1
