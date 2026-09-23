"""Pygame dashboard for Alpamayo cameras, trajectory, reasoning, and control."""

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path
from typing import Any

import numpy as np

from alpamayo_rig import (
    CAMERAS_BY_INDEX,
    DASHBOARD_CAMERA_INDICES,
    TRAJECTORY_CAMERA_INDICES,
)


os.environ.setdefault("PYGAME_HIDE_SUPPORT_PROMPT", "hide")
os.environ.setdefault("SDL_AUDIODRIVER", "dummy")


class NullDisplay:
    def draw(self, **_: Any) -> bool:
        return True

    def close(self) -> None:
        pass


class SimulationVideoClock:
    """Convert fixed simulation steps into a stable output-frame cadence."""

    def __init__(self, fps: float) -> None:
        if fps <= 0.0:
            raise ValueError("fps must be positive")
        self.fps = fps
        self.frame_credit = 0.0

    def advance(self, simulation_seconds: float) -> int:
        if simulation_seconds < 0.0:
            raise ValueError("simulation duration cannot be negative")
        self.frame_credit += simulation_seconds * self.fps
        frame_count = int(self.frame_credit + 1e-9)
        self.frame_credit -= frame_count
        return frame_count


class DashboardRecorder:
    """Encode dashboard frames against simulation time using FFmpeg."""

    def __init__(self, pygame: Any, path: str, width: int, height: int, fps: float) -> None:
        self.pygame = pygame
        self.path = Path(path).expanduser().resolve()
        self.path.parent.mkdir(parents=True, exist_ok=True)
        self.fps = fps
        self.clock = SimulationVideoClock(fps)
        self.frames_written = 0
        ffmpeg = shutil.which("ffmpeg")
        if ffmpeg is None:
            try:
                import imageio_ffmpeg

                ffmpeg = imageio_ffmpeg.get_ffmpeg_exe()
            except (ImportError, RuntimeError) as exc:
                raise RuntimeError(
                    "dashboard recording needs FFmpeg or the imageio-ffmpeg package"
                ) from exc
        command = [
            ffmpeg,
            "-y",
            "-loglevel",
            "error",
            "-f",
            "rawvideo",
            "-pixel_format",
            "rgb24",
            "-video_size",
            f"{width}x{height}",
            "-framerate",
            str(fps),
            "-i",
            "-",
            "-an",
            "-c:v",
            "libx264",
            "-preset",
            "veryfast",
            "-crf",
            "20",
            "-pix_fmt",
            "yuv420p",
            "-movflags",
            "+faststart",
            str(self.path),
        ]
        self.process = subprocess.Popen(
            command,
            stdin=subprocess.PIPE,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            start_new_session=True,
        )
        print(f"Recording Alpamayo dashboard to {self.path} at {fps:g} FPS")

    def write(self, surface: Any, simulation_seconds: float) -> None:
        frame_count = self.clock.advance(simulation_seconds)
        if frame_count == 0:
            return
        frame = self.pygame.image.tostring(surface, "RGB")
        if self.process.stdin is None:
            return
        try:
            for _ in range(frame_count):
                self.process.stdin.write(frame)
                self.frames_written += 1
        except BrokenPipeError as exc:
            raise RuntimeError("FFmpeg stopped while recording the dashboard") from exc

    def close(self) -> None:
        if self.process.stdin is not None:
            self.process.stdin.close()
        return_code = self.process.wait(timeout=30)
        if return_code == 0:
            print(
                f"Saved {self.frames_written} dashboard frames to {self.path}"
            )
        else:
            print(f"WARNING: FFmpeg exited with code {return_code}; recording may be invalid")


class AlpamayoDisplay:
    WIDTH = 1600
    HEIGHT = 900
    CAMERA_COLUMNS = 4
    CAMERA_ROWS = 2
    CELL_WIDTH = WIDTH // CAMERA_COLUMNS
    CELL_HEIGHT = 225
    CAMERA_AREA_HEIGHT = CELL_HEIGHT * CAMERA_ROWS
    TRAJECTORY_WIDTH = 680
    BOTTOM_HEIGHT = HEIGHT - CAMERA_AREA_HEIGHT

    def __init__(
        self,
        screenshot: str | None = None,
        record_video: str | None = None,
        record_fps: float = 20.0,
    ) -> None:
        import pygame

        self.pygame = pygame
        pygame.init()
        pygame.font.init()
        self.screen = pygame.display.set_mode((self.WIDTH, self.HEIGHT))
        pygame.display.set_caption("CARLA — NVIDIA Alpamayo 2 Super")
        self.font = pygame.font.SysFont("sans", 18)
        self.small_font = pygame.font.SysFont("sans", 15)
        self.mono_font = pygame.font.SysFont("monospace", 16)
        self.title_font = pygame.font.SysFont("sans", 27, bold=True)
        self.reasoning_font = pygame.font.SysFont("sans", 23)
        self.reasoning_fonts = [
            self.reasoning_font,
            pygame.font.SysFont("sans", 21),
            pygame.font.SysFont("sans", 19),
            pygame.font.SysFont("sans", 17),
            pygame.font.SysFont("sans", 15),
        ]
        self.metric_font = pygame.font.SysFont("sans", 22, bold=True)
        self._surface_cache: dict[int, tuple[np.ndarray, Any]] = {}
        self._screenshot = Path(screenshot).expanduser() if screenshot else None
        self._screenshot_saved = False
        self.recorder = (
            DashboardRecorder(pygame, record_video, self.WIDTH, self.HEIGHT, record_fps)
            if record_video
            else None
        )

    def _handle_events(self) -> bool:
        pygame = self.pygame
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
            if event.type == pygame.KEYUP and event.key in (pygame.K_ESCAPE, pygame.K_q):
                return False
        return True

    def _surface_for(self, camera_index: int, rgb_chw: np.ndarray):
        cached = self._surface_cache.get(camera_index)
        if cached is not None and cached[0] is rgb_chw:
            return cached[1]
        rgb_hwc = np.transpose(rgb_chw, (1, 2, 0))
        surface = self.pygame.surfarray.make_surface(np.transpose(rgb_hwc, (1, 0, 2)))
        surface = self.pygame.transform.smoothscale(
            surface, (self.CELL_WIDTH, self.CELL_HEIGHT)
        )
        self._surface_cache[camera_index] = (rgb_chw, surface)
        return surface

    def _draw_camera(
        self,
        camera_index: int,
        image: np.ndarray | None,
        column: int,
        row: int,
    ) -> None:
        pygame = self.pygame
        x = column * self.CELL_WIDTH
        y = row * self.CELL_HEIGHT
        pygame.draw.rect(
            self.screen,
            (18, 20, 24),
            (x, y, self.CELL_WIDTH, self.CELL_HEIGHT),
        )
        if image is not None:
            self.screen.blit(self._surface_for(camera_index, image), (x, y))
        overlay = pygame.Surface((self.CELL_WIDTH, 31), pygame.SRCALPHA)
        overlay.fill((8, 11, 15, 215))
        self.screen.blit(overlay, (x, y))
        spec = CAMERAS_BY_INDEX[camera_index]
        if camera_index in TRAJECTORY_CAMERA_INDICES:
            category, color = "ALPAMAYO INPUT", (80, 205, 255)
        elif camera_index == 7:
            category, color = "CARLA VIEW", (100, 230, 155)
        else:
            category, color = "SOURCE CONTEXT", (215, 175, 255)
        label = self.small_font.render(
            f"{spec.display_name.upper()}  ·  {category}", True, (238, 241, 245)
        )
        self.screen.blit(label, (x + 11, y + 7))
        pygame.draw.rect(self.screen, color, (x, y, 5, 31))
        pygame.draw.rect(
            self.screen,
            (8, 10, 13),
            (x, y, self.CELL_WIDTH, self.CELL_HEIGHT),
            2,
        )

    def _draw_trajectory(self, trajectory: np.ndarray | None) -> None:
        pygame = self.pygame
        area = pygame.Rect(
            0,
            self.CAMERA_AREA_HEIGHT,
            self.TRAJECTORY_WIDTH,
            self.BOTTOM_HEIGHT,
        )
        pygame.draw.rect(self.screen, (13, 18, 24), area)
        self.screen.blit(
            self.title_font.render("Predicted path", True, (242, 245, 248)),
            (area.x + 24, area.y + 18),
        )
        self.screen.blit(
            self.small_font.render(
                "6.4 s horizon  ·  ego frame  ·  x forward / y left",
                True,
                (137, 153, 168),
            ),
            (area.x + 25, area.y + 52),
        )

        plot = pygame.Rect(area.x + 42, area.y + 84, area.width - 70, area.height - 112)
        pygame.draw.rect(self.screen, (10, 14, 19), plot, border_radius=9)
        max_forward = 40.0
        max_lateral = 15.0
        if trajectory is not None and len(trajectory):
            max_forward = max(max_forward, float(np.max(trajectory[:, 0])) + 5.0)
            max_lateral = max(
                max_lateral, float(np.max(np.abs(trajectory[:, 1]))) + 3.0
            )
        scale = min(plot.height / max_forward, plot.width / (2.0 * max_lateral))
        origin = np.array([plot.centerx, plot.bottom - 13], dtype=np.float64)

        for distance in range(10, int(max_forward) + 1, 10):
            y = int(round(origin[1] - distance * scale))
            pygame.draw.line(
                self.screen, (35, 45, 55), (plot.left + 8, y), (plot.right - 8, y), 1
            )
            self.screen.blit(
                self.small_font.render(f"{distance} m", True, (91, 108, 122)),
                (plot.left + 10, y - 18),
            )
        for lateral in range(-10, 11, 5):
            x = int(round(origin[0] - lateral * scale))
            pygame.draw.line(
                self.screen, (27, 36, 45), (x, plot.top + 8), (x, plot.bottom - 8), 1
            )
        pygame.draw.line(
            self.screen,
            (92, 112, 128),
            (plot.left + 8, int(origin[1])),
            (plot.right - 8, int(origin[1])),
            1,
        )
        pygame.draw.line(
            self.screen,
            (92, 112, 128),
            (int(origin[0]), plot.top + 8),
            (int(origin[0]), plot.bottom - 8),
            1,
        )
        pygame.draw.polygon(
            self.screen,
            (67, 183, 255),
            [
                (int(origin[0]), int(origin[1] - 13)),
                (int(origin[0] - 8), int(origin[1] + 8)),
                (int(origin[0] + 8), int(origin[1] + 8)),
            ],
        )
        if trajectory is not None and len(trajectory) >= 2:
            points = [
                (
                    int(round(origin[0] - float(point[1]) * scale)),
                    int(round(origin[1] - float(point[0]) * scale)),
                )
                for point in trajectory
            ]
            pygame.draw.lines(self.screen, (255, 190, 65), False, points, 5)
            pygame.draw.circle(self.screen, (255, 92, 82), points[-1], 7)
            endpoint = trajectory[-1]
            endpoint_text = self.small_font.render(
                f"endpoint  {endpoint[0]:.1f} m / {endpoint[1]:+.1f} m",
                True,
                (255, 205, 106),
            )
            self.screen.blit(endpoint_text, (plot.right - endpoint_text.get_width() - 12, plot.top + 10))

    @staticmethod
    def _wrap_text(font: Any, text: str, max_width: int) -> list[str]:
        words = text.split()
        if not words:
            return ["Waiting for the first prediction…"]
        lines: list[str] = []
        line = words[0]
        for word in words[1:]:
            candidate = f"{line} {word}"
            if font.size(candidate)[0] <= max_width:
                line = candidate
            else:
                lines.append(line)
                line = word
        lines.append(line)
        return lines

    def _draw_metric(self, rect: Any, label: str, value: str, accent: tuple[int, int, int]) -> None:
        pygame = self.pygame
        pygame.draw.rect(self.screen, (24, 31, 39), rect, border_radius=8)
        pygame.draw.rect(self.screen, accent, (rect.x, rect.y, 4, rect.height), border_radius=2)
        self.screen.blit(
            self.small_font.render(label.upper(), True, (125, 143, 158)),
            (rect.x + 14, rect.y + 9),
        )
        self.screen.blit(
            self.metric_font.render(value, True, (236, 240, 244)),
            (rect.x + 14, rect.y + 29),
        )

    def _draw_control_bar(
        self,
        x: int,
        y: int,
        width: int,
        label: str,
        value: float,
        color: tuple[int, int, int],
        signed: bool = False,
    ) -> None:
        pygame = self.pygame
        self.screen.blit(
            self.small_font.render(f"{label}  {value:+.2f}" if signed else f"{label}  {value:.2f}", True, (172, 184, 194)),
            (x, y),
        )
        bar = pygame.Rect(x, y + 23, width, 8)
        pygame.draw.rect(self.screen, (35, 44, 53), bar, border_radius=4)
        if signed:
            center = bar.centerx
            extent = int((bar.width / 2) * min(abs(value), 1.0))
            fill = pygame.Rect(center if value >= 0 else center - extent, bar.y, extent, bar.height)
            pygame.draw.line(self.screen, (105, 120, 132), (center, bar.y - 3), (center, bar.bottom + 3), 1)
        else:
            fill = pygame.Rect(bar.x, bar.y, int(bar.width * min(max(value, 0.0), 1.0)), bar.height)
        if fill.width:
            pygame.draw.rect(self.screen, color, fill, border_radius=4)

    def _draw_inference(
        self,
        reasoning: str,
        status: str,
        telemetry: dict[str, Any],
    ) -> None:
        pygame = self.pygame
        area = pygame.Rect(
            self.TRAJECTORY_WIDTH,
            self.CAMERA_AREA_HEIGHT,
            self.WIDTH - self.TRAJECTORY_WIDTH,
            self.BOTTOM_HEIGHT,
        )
        pygame.draw.rect(self.screen, (17, 21, 27), area)
        pygame.draw.line(
            self.screen,
            (42, 52, 62),
            (area.x, area.top + 12),
            (area.x, area.bottom - 12),
            1,
        )
        self.screen.blit(
            self.title_font.render("Alpamayo 2 Super", True, (242, 245, 248)),
            (area.x + 25, area.y + 18),
        )

        status_text = status.upper()
        max_status_width = 430
        while self.small_font.size(status_text)[0] > max_status_width and len(status_text) > 4:
            status_text = status_text[:-2].rstrip() + "…"
        rendered_status = self.small_font.render(status_text, True, (127, 211, 255))
        status_right_margin = 91 if self.recorder is not None else 25
        status_rect = pygame.Rect(
            area.right - rendered_status.get_width() - 26 - status_right_margin,
            area.y + 18,
            rendered_status.get_width() + 26,
            30,
        )
        pygame.draw.rect(self.screen, (25, 53, 69), status_rect, border_radius=15)
        self.screen.blit(rendered_status, (status_rect.x + 13, status_rect.y + 7))
        if self.recorder is not None:
            rec_rect = pygame.Rect(area.right - 76, area.y + 18, 54, 30)
            pygame.draw.rect(self.screen, (79, 29, 33), rec_rect, border_radius=15)
            pygame.draw.circle(self.screen, (255, 75, 75), (rec_rect.x + 14, rec_rect.centery), 5)
            self.screen.blit(
                self.small_font.render("REC", True, (255, 188, 188)),
                (rec_rect.x + 24, rec_rect.y + 7),
            )

        card = pygame.Rect(area.x + 24, area.y + 66, area.width - 48, 150)
        pygame.draw.rect(self.screen, (24, 29, 37), card, border_radius=10)
        self.screen.blit(
            self.small_font.render(
                "CHAIN OF CAUSATION · MODEL REASONING", True, (255, 190, 65)
            ),
            (card.x + 18, card.y + 14),
        )
        compact_font = self.reasoning_fonts[-1]
        lines: list[str] = []
        max_lines = 1
        for candidate_font in self.reasoning_fonts:
            candidate_lines = self._wrap_text(
                candidate_font, reasoning, card.width - 36
            )
            candidate_height = candidate_font.get_linesize()
            candidate_max_lines = max(1, (card.height - 51) // candidate_height)
            compact_font = candidate_font
            lines = candidate_lines
            max_lines = candidate_max_lines
            if len(candidate_lines) <= candidate_max_lines:
                break
        if len(lines) > max_lines:
            lines = lines[:max_lines]
            last = lines[-1]
            while compact_font.size(last + "…")[0] > card.width - 36 and last:
                last = last[:-1]
            lines[-1] = last.rstrip() + "…"
        line_height = compact_font.get_linesize()
        for index, line in enumerate(lines):
            self.screen.blit(
                compact_font.render(line, True, (238, 240, 242)),
                (card.x + 18, card.y + 45 + index * line_height),
            )

        metric_y = area.y + 232
        metric_gap = 10
        metric_width = (area.width - 48 - metric_gap * 3) // 4
        metrics = (
            ("Cycle", str(telemetry.get("cycle", 0)), (106, 204, 255)),
            ("Inference", f"{telemetry.get('inference_seconds', 0.0):.2f} s", (255, 190, 65)),
            ("Ego speed", f"{telemetry.get('speed_mps', 0.0):.2f} m/s", (101, 224, 166)),
            ("Target speed", f"{telemetry.get('target_speed_mps', 0.0):.2f} m/s", (183, 147, 255)),
        )
        for index, (label, value, accent) in enumerate(metrics):
            self._draw_metric(
                pygame.Rect(
                    area.x + 24 + index * (metric_width + metric_gap),
                    metric_y,
                    metric_width,
                    66,
                ),
                label,
                value,
                accent,
            )

        bar_y = area.y + 313
        bar_width = 230
        self._draw_control_bar(
            area.x + 25,
            bar_y,
            bar_width,
            "THROTTLE",
            float(telemetry.get("throttle", 0.0)),
            (79, 211, 143),
        )
        self._draw_control_bar(
            area.x + 25 + bar_width + 42,
            bar_y,
            bar_width,
            "BRAKE",
            float(telemetry.get("brake", 0.0)),
            (255, 95, 86),
        )
        self._draw_control_bar(
            area.x + 25 + (bar_width + 42) * 2,
            bar_y,
            bar_width,
            f"STEER REQ {float(telemetry.get('requested_steer', 0.0)):+.2f} / CMD",
            float(telemetry.get("steer", 0.0)),
            (104, 180, 255),
            signed=True,
        )
        footer = (
            f"MAP  {telemetry.get('map', '')}     ·     "
            "SYNC 10 Hz     ·     ESC / Q TO STOP SAFELY"
        )
        self.screen.blit(
            self.small_font.render(footer, True, (115, 131, 145)),
            (area.x + 25, area.bottom - 36),
        )

    def draw(
        self,
        *,
        images: dict[int, np.ndarray],
        trajectory: np.ndarray | None,
        reasoning: str,
        status: str,
        telemetry: dict[str, Any],
        record_frame: bool = False,
    ) -> bool:
        if not self._handle_events():
            return False
        self.screen.fill((8, 10, 13))
        for position, camera_index in enumerate(DASHBOARD_CAMERA_INDICES):
            self._draw_camera(
                camera_index,
                images.get(camera_index),
                position % self.CAMERA_COLUMNS,
                position // self.CAMERA_COLUMNS,
            )
        self._draw_trajectory(trajectory)
        self._draw_inference(reasoning, status, telemetry)
        self.pygame.display.flip()
        if (
            self._screenshot is not None
            and not self._screenshot_saved
            and trajectory is not None
        ):
            self._screenshot.parent.mkdir(parents=True, exist_ok=True)
            self.pygame.image.save(self.screen, str(self._screenshot))
            self._screenshot_saved = True
            print(f"Saved Alpamayo dashboard to {self._screenshot}")
        if self.recorder is not None and record_frame:
            self.recorder.write(self.screen, simulation_seconds=0.1)
        return True

    def close(self) -> None:
        if self.recorder is not None:
            self.recorder.close()
        self.pygame.quit()
