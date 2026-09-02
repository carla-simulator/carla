import hashlib


def test_blob_upload_dedup_and_download(server):
    data = b"hello cosmos" * 1000
    bid = hashlib.sha256(data).hexdigest()
    r = server.client.put(f"/v1/blobs/{bid}", content=data, headers={"Content-Type": "video/mp4", "X-Filename": "a.mp4"})
    assert r.status_code == 201 and r.json() == {"id": bid, "size": len(data), "existed": False}
    r = server.client.put(f"/v1/blobs/{bid}", content=data)
    assert r.status_code == 200 and r.json()["existed"] is True
    assert server.store.blob_stats() == (1, len(data))
    r = server.client.post("/v1/blobs/check", json={"ids": [bid, "f" * 64]})
    assert r.json() == {"present": [bid], "missing": ["f" * 64]}
    r = server.client.get(f"/v1/blobs/{bid}")
    assert r.status_code == 200 and r.content == data and r.headers["content-type"].startswith("video/mp4")
    assert server.client.get(f"/v1/blobs/{'a' * 64}").status_code == 404


def test_blob_hash_mismatch_and_bad_id(server):
    r = server.client.put(f"/v1/blobs/{'0' * 64}", content=b"not matching")
    assert r.status_code == 400 and "does not match" in r.text
    assert not list(server.settings.blobs_dir.rglob(".upload-*"))
    assert server.client.put("/v1/blobs/short", content=b"x").status_code == 400
    assert server.store.blob_stats() == (0, 0)
