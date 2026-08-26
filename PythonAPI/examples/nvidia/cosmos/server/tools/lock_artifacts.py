"""Generate ``server/artifacts.lock`` from Hugging Face metadata (no downloads).

Every model artifact baked into the image is pinned here: repo, commit
revision, the files that are actually read at runtime, their sizes and (for
LFS files) sha256.  Revisions for the Transfer 2.5 checkpoints are the ones
hard-coded in cosmos-transfer2.5 v1.5.4's checkpoint registry
(``packages/cosmos-oss/cosmos_oss/checkpoints*.py``), so the pipeline's
offline resolution (``hf download <repo> <file> --revision <sha>``) finds
exactly the snapshot we baked.  ``main`` revisions are resolved to a commit
at generation time.

    HF_TOKEN=... python tools/lock_artifacts.py            # rewrites artifacts.lock
    python tools/lock_artifacts.py --check                  # exits 1 if the lock is stale
"""

from __future__ import annotations

import argparse
import fnmatch
import json
import os
import sys
import time
from dataclasses import asdict, dataclass, field
from pathlib import Path

from huggingface_hub import HfApi

LOCK_PATH = Path(__file__).resolve().parents[1] / "artifacts.lock"


@dataclass
class Spec:
    id: str
    repo: str
    revision: str = "main"
    include: list[str] = field(default_factory=lambda: ["*"])
    exclude: list[str] = field(default_factory=list)
    used_by: list[str] = field(default_factory=list)
    images: list[str] = field(default_factory=lambda: ["nano", "full"])
    gated: bool = False
    note: str = ""


TRANSFER25 = "nvidia/Cosmos-Transfer2.5-2B"
COMMON_EXCLUDES = ["*.onnx", "*.msgpack", "*.h5", "*.ot", "*.tflite", "*.gguf", ".gitattributes"]

SPECS: list[Spec] = [
    Spec("cosmos3-nano", "nvidia/Cosmos3-Nano", "411f42a8fdfb8c5b2583cb8786e0938f49796eaa",
         exclude=["assets/*", "images/*"], used_by=["cosmos3"],
         note="pinned to the 2026-08 release we validated the download path with (main moved on 2026-08-27); "
              "transformer, vae, sound_tokenizer, scheduler, text_tokenizer, vision_encoder (not loaded, kept for safety)"),
    Spec("cosmos3-super", "nvidia/Cosmos3-Super", exclude=["assets/*", "images/*"], used_by=["cosmos3"], images=["full"]),
    Spec("qwen3guard", "Qwen/Qwen3Guard-Gen-0.6B", exclude=COMMON_EXCLUDES, used_by=["cosmos3", "transfer25", "transfer25_av"],
         note="text guardrail for Cosmos 3 (cosmos-guardrail) and Transfer 2.5"),
    Spec("cosmos1-guardrail", "nvidia/Cosmos-1.0-Guardrail",
         include=["blocklist/*", "blocklist/**", "face_blur_filter/Resnet50_Final.pth", "README.md"],
         used_by=["cosmos3"], gated=True, note="what cosmos-guardrail 0.3.1 loads; aegis/ and video filter unused"),
    Spec("transfer25-edge", TRANSFER25, "b67b64abda3801a9aceddbff2bdb86126c06db74",
         include=["general/edge/61f5694b-0ad5-4ecd-8ad7-c8545627d125_ema_bf16.pt"], used_by=["transfer25"], gated=True),
    Spec("transfer25-depth", TRANSFER25, "dea7737ca29dd8d9086413c6dc5724b8250a0bb4",
         include=["general/depth/626e6618-bfcd-4d9a-a077-1409e2ce353f_ema_bf16.pt"], used_by=["transfer25"], gated=True),
    Spec("transfer25-blur", TRANSFER25, "eb5325b77d358944da58a690157dd2b8071bbf85",
         include=["general/blur/ba2f44f2-c726-4fe7-949f-597069d9b91c_ema_bf16.pt"], used_by=["transfer25"], gated=True),
    Spec("transfer25-seg", TRANSFER25, "23057a4167b89de89a4a397fdbf3887994d115eb",
         include=["general/seg/5136ef49-6d8d-42e8-8abf-7dac722a304a_ema_bf16.pt"], used_by=["transfer25"], gated=True),
    Spec("transfer25-av-multiview", TRANSFER25, "00c591edab119e8a6ca06e6e091351a04ce0ecc9",
         include=["auto/multiview/4ecc66e9-df19-4aed-9802-0d11e057287a_ema_bf16.pt"], used_by=["transfer25_av"], gated=True),
    Spec("reason1-7b", "nvidia/Cosmos-Reason1-7B", "3210bec0495fdc7a8d3dbb8d58da5711eab4b423", exclude=COMMON_EXCLUDES,
         used_by=["transfer25", "transfer25_av"], gated=True, note="Transfer 2.5 text encoder"),
    Spec("predict25-tokenizer", "nvidia/Cosmos-Predict2.5-2B", "f176dc95b4a70f53ce01c4b302851595e7322b00",
         include=["tokenizer.pth"], used_by=["transfer25", "transfer25_av"], gated=True, note="Wan2.1 VAE"),
    Spec("guardrail1", "nvidia/Cosmos-Guardrail1", "d6d4bfa899a71454a700907664f3e88f503950cf", exclude=[".gitattributes"],
         used_by=["transfer25", "transfer25_av"], gated=True,
         note="Transfer 2.5 guardrails: blocklist, RetinaFace face blur, video content safety filter"),
    Spec("siglip2", "google/siglip2-so400m-patch16-naflex", exclude=COMMON_EXCLUDES, used_by=["transfer25"],
         note="image-context encoder instantiated unconditionally by the Transfer 2.5 conditioner"),
    Spec("sam2-hiera-large", "facebook/sam2-hiera-large", exclude=COMMON_EXCLUDES, used_by=["transfer25"],
         note="server-side seg derivation"),
    Spec("grounding-dino-base", "IDEA-Research/grounding-dino-base", exclude=COMMON_EXCLUDES, used_by=["transfer25"],
         note="server-side seg derivation (prompted boxes)"),
    Spec("video-depth-anything-small", "depth-anything/Video-Depth-Anything-Small", exclude=COMMON_EXCLUDES,
         used_by=["transfer25"], note="server-side depth derivation"),
]


def _match(path: str, patterns: list[str]) -> bool:
    for pat in patterns:
        if fnmatch.fnmatch(path, pat) or (pat.endswith("/**") and path.startswith(pat[:-3] + "/")) \
                or (pat.endswith("/*") and path.startswith(pat[:-2] + "/")):
            return True
    return False


def build(api: HfApi) -> dict:
    artifacts = []
    for spec in SPECS:
        info = api.model_info(spec.repo, revision=spec.revision, files_metadata=True)
        files = []
        for s in info.siblings or []:
            if not _match(s.rfilename, spec.include) or _match(s.rfilename, spec.exclude):
                continue
            files.append({"path": s.rfilename, "size": s.size or 0,
                          "sha256": s.lfs.sha256 if s.lfs else None, "blob_id": s.blob_id})
        if not files:
            raise SystemExit(f"{spec.id}: no files matched include={spec.include}")
        files.sort(key=lambda f: f["path"])
        artifacts.append({**asdict(spec), "revision": info.sha, "requested_revision": spec.revision,
                          "gated": bool(info.gated) if info.gated is not None else spec.gated,
                          "bytes": sum(f["size"] for f in files), "files": files})
    images = {}
    for name in ("nano", "full"):
        ids = [a["id"] for a in artifacts if name in a["images"]]
        images[name] = {"artifacts": ids, "bytes": sum(a["bytes"] for a in artifacts if a["id"] in ids)}
    return {"version": 1, "generated": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "cosmos_transfer25_version": "v1.5.4", "images": images, "artifacts": artifacts}


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--check", action="store_true")
    p.add_argument("--out", default=str(LOCK_PATH))
    args = p.parse_args()
    api = HfApi(token=os.environ.get("HF_TOKEN"))
    lock = build(api)
    text = json.dumps(lock, indent=1) + "\n"
    if args.check:
        old = json.loads(Path(args.out).read_text())
        for k in ("generated",):
            old.pop(k, None)
            lock.pop(k, None)
        if old != lock:
            print("artifacts.lock is stale", file=sys.stderr)
            return 1
        print("artifacts.lock up to date")
        return 0
    Path(args.out).write_text(text)
    for a in lock["artifacts"]:
        print(f"{a['id']:<28} {a['repo']:<42} {a['revision'][:10]}  {a['bytes'] / 1e9:7.2f} GB  {len(a['files'])} files"
              f"{'  [gated]' if a['gated'] else ''}")
    for name, img in lock["images"].items():
        print(f"image :{name}: {img['bytes'] / 1e9:.1f} GB of artifacts")
    return 0


if __name__ == "__main__":
    sys.exit(main())
