"""Hybrid-render patch for nre-ga serve-grpc (prototype).

Loaded through PYTHONPATH by the engine's compiled entry point. When ``nre.grpc.serve`` is
imported, ``SensorSimService.render_rgb`` gains a second behaviour: a request whose
``image_quality`` equals the sentinel ``-7`` is rendered with the engine's internal
``fields=[color, distance, opacity]`` API and answered with a packed payload

    b"HYBR" | uint32 h | uint32 w | uint8 rgb (3,h,w) | float32 distance (h,w) | float32 opacity (h,w)

instead of an encoded image. Every other request is untouched.
"""
import importlib.abc
import importlib.util
import struct
import sys
import traceback

SENTINEL = -7.0
FIELDS = ["color_image", "distance_image", "opacity_image"]


def _patch(module):
    import torch
    import grpc
    from nre.grpc.protos.sensorsim_pb2 import RGBRenderReturn

    orig = module.SensorSimService.render_rgb

    def render_rgb(self, request, context):
        if abs(request.image_quality - SENTINEL) > 1e-3:
            return orig(self, request, context)
        try:
            with self.get_backend(request.scene_id) as backend:
                with backend._render_lock:
                    backend._validate_dynamic_object_updates_request(
                        num_dynamic_objects=len(request.dynamic_objects),
                        enable_editing_actors=self.enable_editing_actors)
                    cam = backend.camera_bank._get_camera(request.camera_intrinsics)
                    rb = cam.build_ray_bundle(request)
                    snap = None
                    if self.enable_editing_actors and backend.renderable_model.supports_edit_actors():
                        snap = module.actors_snapshot_from_render_request(request)
                    fr = backend.renderable_model.render_camera_frame_from_ray_bundle(
                        rb, fields=FIELDS, actors_snapshot=snap,
                        frame_start_us=request.frame_start_us, frame_end_us=request.frame_end_us)
                    w, h = rb.rendering_data.w, rb.rendering_data.h
            rgb = (fr.color_image.clamp(0, 1) * 255).to(torch.uint8).permute(2, 0, 1).contiguous().cpu().numpy()
            dist = fr.distance_image.float().contiguous().cpu().numpy().astype("<f4")
            opa = fr.opacity_image.float().contiguous().cpu().numpy().astype("<f4")
            payload = b"HYBR" + struct.pack("<II", h, w) + rgb.tobytes() + dist.tobytes() + opa.tobytes()
            return RGBRenderReturn(image_bytes=payload)
        except Exception as exc:  # noqa: BLE001
            module.log.error("hybrid render failed: %s\n%s", exc, traceback.format_exc())
            context.abort(grpc.StatusCode.UNKNOWN, f"hybrid render failed: {exc}")
            raise

    module.SensorSimService.render_rgb = render_rgb
    module.log.info("[hybrid] render_rgb patched: image_quality == %s returns colour+distance+opacity", SENTINEL)


class _Hook(importlib.abc.MetaPathFinder):
    def find_spec(self, name, path, target=None):
        if name != "nre.grpc.serve":
            return None
        sys.meta_path.remove(self)
        spec = importlib.util.find_spec(name)
        if spec is None or spec.loader is None:
            return spec
        loader = spec.loader
        orig_exec = loader.exec_module

        def exec_module(mod):
            orig_exec(mod)
            try:
                _patch(mod)
            except Exception:  # noqa: BLE001
                sys.stderr.write("[hybrid] patch failed:\n" + traceback.format_exc())

        loader.exec_module = exec_module
        return spec


sys.meta_path.insert(0, _Hook())
sys.stderr.write("[hybrid] sitecustomize armed\n")
