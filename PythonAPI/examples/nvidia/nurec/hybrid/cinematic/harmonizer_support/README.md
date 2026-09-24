# Tested Harmonizer compatibility files

External repository: https://github.com/NVIDIA/harmonizer at `dd5799e50855c5bcb1f6ef52a77b5b644b4798c0`.
Clone it as a sibling of CARLA named `harmonizer`. Apply `harmonizer-local.patch`
with `git apply --unidiff-zero`, then copy `text2image_patched.py` into that checkout's root.
The latter is the existing Cosmos runtime override mounted by the batch runner;
it retains its original license header. Build `harmonizer-cosmos-env` using that
repository's Dockerfile.cosmos and obtain the checkpoints under their licenses.
Weights and datasets are not bundled. The model checkpoint belongs at
`harmonizer/models/diffusion_harmonizer.pkl`; the tokenizer belongs in the
checkpoint location expected by the upstream runner.

Single-frame inference uses the existing NuRec container, `/tmp/hybrid/runpy`
bootstrap prepared by `hybrid_run.py`, and `engine_patch/harmonize.py`.
`diffuse_multicam.py` currently expects container `nurec-cinematic-mvp`, the
workspace mounted at `/work`, and the existing local Python environment.
These are prototype deployment assumptions, not a standalone installer.
