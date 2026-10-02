"""Compose a carla-cosmos image in a registry from a code image plus weights-only layer images.

    python tools/compose_image.py --registry localhost:5000 --repo carla-cosmos \
        --code nano-nomodels --layers nano-layer1,nano-layer2,nano-layer3 --variant nano --tag nano
    python tools/compose_image.py --registry localhost:5000 --repo carla-cosmos \
        --code nano-nomodels --layers nano-layer1,nano-layer2,nano-layer3,super-layer1,...,super-layer4 \
        --variant full --tag full

Image layers are content-addressed tarballs that do not depend on what sits below them, so a
weights layer baked once (tools/bake_weights.sh --layers-to) can be stacked on any code image:
a code change means re-pushing the ~66 GB code image and re-running this script — no re-bake,
no local copy of the 100–300 GB result.  The new manifest references the existing blobs; only a
new config blob and a tiny marker layer (``/models/hf/ARTIFACTS_IMAGE``, ``CARLA_COSMOS_IMAGE_VARIANT``)
are uploaded.  Works with any v2 registry (basic or bearer-token auth, cross-repo blob mounts).
"""

from __future__ import annotations

import argparse
import base64
import gzip
import hashlib
import io
import json
import os
import re
import sys
import tarfile
import time
import urllib.error
import urllib.parse
import urllib.request

OCI_MANIFEST = "application/vnd.oci.image.manifest.v1+json"
OCI_INDEX = "application/vnd.oci.image.index.v1+json"
OCI_CONFIG = "application/vnd.oci.image.config.v1+json"
OCI_LAYER = "application/vnd.oci.image.layer.v1.tar+gzip"
DOCKER_MANIFEST = "application/vnd.docker.distribution.manifest.v2+json"
DOCKER_LIST = "application/vnd.docker.distribution.manifest.list.v2+json"
DOCKER_CONFIG = "application/vnd.docker.container.image.v1+json"
DOCKER_LAYER = "application/vnd.docker.image.rootfs.diff.tar.gzip"
ACCEPT = ", ".join([OCI_MANIFEST, OCI_INDEX, DOCKER_MANIFEST, DOCKER_LIST])


class Registry:
    def __init__(self, host: str, user: str | None, insecure: bool):
        self.base = ("http://" if insecure or host.startswith(("localhost", "127.")) else "https://") + host
        self.user = user
        self.tokens: dict[str, str] = {}

    # -- http -------------------------------------------------------------------------------------
    def _req(self, method: str, path: str, repo: str, data=None, headers=None, scope="pull") -> tuple[int, dict, bytes]:
        h = dict(headers or {})
        tok = self.tokens.get(repo)
        if tok:
            h["Authorization"] = tok
        elif self.user:
            h["Authorization"] = "Basic " + base64.b64encode(self.user.encode()).decode()
        url = path if path.startswith("http") else self.base + path
        req = urllib.request.Request(url, data=data, method=method, headers=h)
        try:
            with urllib.request.urlopen(req, timeout=600) as r:
                return r.status, dict(r.headers), r.read()
        except urllib.error.HTTPError as e:
            if e.code == 401 and repo not in self.tokens and "WWW-Authenticate" in e.headers:
                self._bearer(e.headers["WWW-Authenticate"], repo, scope)
                if repo in self.tokens:
                    return self._req(method, path, repo, data, headers, scope)
            return e.code, dict(e.headers), e.read()

    def _bearer(self, challenge: str, repo: str, scope: str) -> None:
        if not challenge.lower().startswith("bearer"):
            return
        params = dict(re.findall(r'(\w+)="([^"]*)"', challenge))
        q = {"service": params.get("service", ""), "scope": f"repository:{repo}:{scope}"}
        req = urllib.request.Request(params["realm"] + "?" + urllib.parse.urlencode(q))
        if self.user:
            req.add_header("Authorization", "Basic " + base64.b64encode(self.user.encode()).decode())
        with urllib.request.urlopen(req, timeout=60) as r:
            body = json.loads(r.read())
        self.tokens[repo] = "Bearer " + (body.get("token") or body.get("access_token"))

    # -- read ---------------------------------------------------------------------------------------
    def manifest(self, repo: str, ref: str) -> tuple[dict, str]:
        st, hd, body = self._req("GET", f"/v2/{repo}/manifests/{ref}", repo, headers={"Accept": ACCEPT})
        if st != 200:
            raise SystemExit(f"GET {repo}:{ref} manifest -> {st}: {body[:200]!r}")
        m = json.loads(body)
        mt = m.get("mediaType") or hd.get("Content-Type", "")
        if mt in (OCI_INDEX, DOCKER_LIST):
            cands = [d for d in m["manifests"]
                     if d.get("platform", {}).get("os", "linux") == "linux"
                     and d.get("platform", {}).get("architecture", "amd64") == "amd64"
                     and d.get("annotations", {}).get("vnd.docker.reference.type") != "attestation-manifest"]
            if not cands:
                raise SystemExit(f"{repo}:{ref}: no linux/amd64 manifest in index")
            return self.manifest(repo, cands[0]["digest"])
        return m, mt

    def blob_json(self, repo: str, digest: str) -> dict:
        st, _, body = self._req("GET", f"/v2/{repo}/blobs/{digest}", repo)
        if st != 200:
            raise SystemExit(f"GET blob {digest} -> {st}")
        return json.loads(body)

    # -- write --------------------------------------------------------------------------------------
    def has_blob(self, repo: str, digest: str) -> bool:
        st, _, _ = self._req("HEAD", f"/v2/{repo}/blobs/{digest}", repo)
        return st == 200

    def mount_blob(self, repo: str, digest: str, from_repo: str) -> bool:
        if repo == from_repo or self.has_blob(repo, digest):
            return True
        q = urllib.parse.urlencode({"mount": digest, "from": from_repo})
        st, _, _ = self._req("POST", f"/v2/{repo}/blobs/uploads/?{q}", repo, data=b"", scope="pull,push")
        return st == 201

    def put_blob(self, repo: str, data: bytes) -> str:
        digest = "sha256:" + hashlib.sha256(data).hexdigest()
        if self.has_blob(repo, digest):
            return digest
        st, hd, body = self._req("POST", f"/v2/{repo}/blobs/uploads/", repo, data=b"", scope="pull,push")
        if st != 202:
            raise SystemExit(f"blob upload start -> {st}: {body[:200]!r}")
        loc = hd["Location"]
        loc = loc if loc.startswith("http") else self.base + loc
        sep = "&" if "?" in loc else "?"
        st, _, body = self._req("PUT", f"{loc}{sep}digest={digest}", repo, data=data,
                                headers={"Content-Type": "application/octet-stream"}, scope="pull,push")
        if st != 201:
            raise SystemExit(f"blob upload -> {st}: {body[:200]!r}")
        return digest

    def put_manifest(self, repo: str, tag: str, manifest: dict) -> str:
        data = json.dumps(manifest, indent=1).encode()
        st, hd, body = self._req("PUT", f"/v2/{repo}/manifests/{tag}", repo, data=data,
                                 headers={"Content-Type": manifest["mediaType"]}, scope="pull,push")
        if st not in (200, 201):
            raise SystemExit(f"PUT manifest {repo}:{tag} -> {st}: {body[:300]!r}")
        return hd.get("Docker-Content-Digest", "sha256:" + hashlib.sha256(data).hexdigest())


def marker_layer(variant: str) -> tuple[bytes, str]:
    """A one-file layer: /models/hf/ARTIFACTS_IMAGE = <variant>.  Returns (gzip bytes, diff_id)."""
    raw = io.BytesIO()
    with tarfile.open(fileobj=raw, mode="w", format=tarfile.PAX_FORMAT) as tf:
        for d in ("models", "models/hf"):
            ti = tarfile.TarInfo(d); ti.type = tarfile.DIRTYPE; ti.mode = 0o755; ti.mtime = 0
            tf.addfile(ti)
        payload = (variant + "\n").encode()
        ti = tarfile.TarInfo("models/hf/ARTIFACTS_IMAGE"); ti.size = len(payload); ti.mode = 0o644; ti.mtime = 0
        tf.addfile(ti, io.BytesIO(payload))
    tar = raw.getvalue()
    return gzip.compress(tar, mtime=0), "sha256:" + hashlib.sha256(tar).hexdigest()


def split_ref(ref: str, default_repo: str) -> tuple[str, str]:
    return (ref.split(":", 1)[0], ref.split(":", 1)[1]) if ":" in ref else (default_repo, ref)


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("--registry", required=True, help="host[:port]")
    p.add_argument("--repo", required=True, help="target repository, e.g. carla-cosmos or team/carla-cosmos")
    p.add_argument("--code", required=True, help="code image tag (or repo:tag) — the -nomodels image")
    p.add_argument("--layers", required=True,
                   help="comma-separated weights-layer image tags (or repo:tag), bottom first; `tag@N` skips N base layers")
    p.add_argument("--variant", required=True, choices=["nano", "full"])
    p.add_argument("--tag", default=None, help="target tag (default: the variant)")
    p.add_argument("--user", default=os.environ.get("REGISTRY_AUTH"), help="user:password (or $REGISTRY_AUTH)")
    p.add_argument("--insecure", action="store_true", help="plain http")
    args = p.parse_args()
    reg = Registry(args.registry, args.user, args.insecure)
    tag = args.tag or args.variant

    code_repo, code_tag = split_ref(args.code, args.repo)
    code_m, code_mt = reg.manifest(code_repo, code_tag)
    code_cfg = reg.blob_json(code_repo, code_m["config"]["digest"])
    oci = code_mt == OCI_MANIFEST
    layer_mt, config_mt, manifest_mt = (OCI_LAYER, OCI_CONFIG, OCI_MANIFEST) if oci else (DOCKER_LAYER, DOCKER_CONFIG, DOCKER_MANIFEST)

    layers = [dict(d, mediaType=layer_mt) for d in code_m["layers"]]
    diff_ids = list(code_cfg["rootfs"]["diff_ids"])
    history = list(code_cfg.get("history", []))
    for d in layers:
        reg.mount_blob(args.repo, d["digest"], code_repo) or sys.exit(f"cannot mount {d['digest']} from {code_repo}")
    print(f"code  {code_repo}:{code_tag}: {len(layers)} layers, {sum(d['size'] for d in layers) / 1e9:.1f} GB")

    for ref in args.layers.split(","):
        ref, _, skip = ref.strip().partition("@")
        repo, ltag = split_ref(ref, args.repo)
        m, _ = reg.manifest(repo, ltag)
        cfg = reg.blob_json(repo, m["config"]["digest"])
        # a weights-layer image = some base (python:slim, or the code image) + the layers bake_weights.sh
        # committed on top; the base's layer count comes from the label the bake script sets, or from
        # an explicit `tag@N` suffix (N = layers to skip, e.g. the code image's count).
        n_base = int(skip or (cfg.get("config", {}).get("Labels") or {}).get("com.carla.cosmos.base_layers", "0"))
        new = m["layers"][n_base:]
        new_ids = cfg["rootfs"]["diff_ids"][n_base:]
        hist = [h for h in cfg.get("history", []) if not h.get("empty_layer")][n_base:]
        if not new:
            sys.exit(f"{repo}:{ltag}: no weight layers above its base (label com.carla.cosmos.base_layers={n_base})")
        for d in new:
            reg.mount_blob(args.repo, d["digest"], repo) or sys.exit(f"cannot mount {d['digest']} from {repo}")
        layers += [dict(d, mediaType=layer_mt) for d in new]
        diff_ids += new_ids
        history += hist or [{"created_by": f"carla-cosmos weights layer {ltag}"} for _ in new]
        print(f"layer {repo}:{ltag}: +{len(new)} layers, {sum(d['size'] for d in new) / 1e9:.1f} GB")

    gz, diff_id = marker_layer(args.variant)
    digest = reg.put_blob(args.repo, gz)
    layers.append({"mediaType": layer_mt, "size": len(gz), "digest": digest})
    diff_ids.append(diff_id)
    now = time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())
    history.append({"created": now, "created_by": f"carla-cosmos compose_image.py variant={args.variant}"})

    cfg = json.loads(json.dumps(code_cfg))
    cfg["rootfs"]["diff_ids"] = diff_ids
    cfg["history"] = history
    cfg["created"] = now
    c = cfg.setdefault("config", {})
    env = [e for e in c.get("Env", []) if not e.startswith("CARLA_COSMOS_IMAGE_VARIANT=")]
    c["Env"] = env + [f"CARLA_COSMOS_IMAGE_VARIANT={args.variant}"]
    labels = c.setdefault("Labels", {}) or {}
    labels.update({"com.carla.cosmos.variant": args.variant, "com.carla.cosmos.weights": args.variant,
                   "com.carla.cosmos.composed_from": f"{code_repo}:{code_tag}+{args.layers}"})
    c["Labels"] = labels
    cfg_bytes = json.dumps(cfg, indent=1).encode()
    cfg_digest = reg.put_blob(args.repo, cfg_bytes)

    manifest = {"schemaVersion": 2, "mediaType": manifest_mt,
                "config": {"mediaType": config_mt, "size": len(cfg_bytes), "digest": cfg_digest},
                "layers": layers}
    dg = reg.put_manifest(args.repo, tag, manifest)
    total = sum(d["size"] for d in layers)
    print(f"pushed {args.registry}/{args.repo}:{tag} = {len(layers)} layers, {total / 1e9:.1f} GB compressed, {dg}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
