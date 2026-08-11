# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
This file contains scripts to start the grpc server to render images. Example command line to start server:
docker run --env "CUDA_VISIBLE_DEVICES=0" --name="nre_1" \
 --gpus all --rm --net=host -v $(pwd):$(pwd) $NUREC_IMAGE \
 serve-grpc --artifact-glob "$(pwd)/scene.usdz" --port=46435 --host=localhost --test-scenes-are-valid

Readiness and reuse verification are protocol-based (get_version /
get_available_scenes over gRPC), not log-scraped: NRE log wording changes
between releases, the gRPC surface does not.
"""

import os
import socket
import subprocess
import logging
import uuid
import threading
import atexit
import time
import select
from typing import Dict, Any, TextIO

import grpc

from scenario import extract_json_from_usdz
from constants import DEFAULT_NUREC_IMAGE, DEFAULT_NUREC_PORT, MAX_MESSAGE_LENGTH
import nre.grpc.protos.common_pb2 as common_pb2
import nre.grpc.protos.sensorsim_pb2_grpc as sensorsim_pb2_grpc

logger = logging.getLogger(__name__)

# Docker labels used to rediscover reusable containers without parsing logs.
LABEL_PORT = "com.carla.nurec.port"
LABEL_SCENE = "com.carla.nurec.scene"
# serve-grpc flags that shape rendering (renderer backend, harmonizer, ...).
# Stamped on the container so reuse is refused when the requested flags differ
# from the ones the running server was started with.
LABEL_ARGS = "com.carla.nurec.server-args"


def find_free_port() -> int:
    """Ask the OS for a free TCP port (containers run with --net=host)."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("localhost", 0))
        return s.getsockname()[1]


def query_server(host: str, port: int, timeout: float = 2.0):
    """
    Single protocol-level probe of a NuRec gRPC server.

    Returns (version_string, scene_ids) or (None, []) if unreachable.
    """
    channel = grpc.insecure_channel(
        f"{host}:{port}",
        options=[
            ("grpc.max_send_message_length", MAX_MESSAGE_LENGTH),
            ("grpc.max_receive_message_length", MAX_MESSAGE_LENGTH),
        ],
    )
    try:
        stub = sensorsim_pb2_grpc.SensorsimServiceStub(channel)
        version = stub.get_version(common_pb2.Empty(), timeout=timeout)
        scenes = stub.get_available_scenes(common_pb2.Empty(), timeout=timeout)
        return version.version_id, list(scenes.scene_ids)
    except grpc.RpcError:
        return None, []
    finally:
        channel.close()

def run_nvidia_smi(image: str) -> int:
    """
    Run nvidia-smi to check for GPU existence.
    
    Args:
        image: Docker image to use, nvidia-smi might not be present outside of docker
        
    Returns:
        int: Number of GPUs found
    """
    cmd = [
        "docker",
        "run",
        "--rm",
        "--gpus",
        "all",
        "--net=host",
        "--entrypoint",
        "nvidia-smi",
        image,
        "--query-gpu=index",
        "--format=csv,noheader,nounits"
    ]
    
    logger.debug(f"Running command: {' '.join(cmd)}")
    result = subprocess.run(cmd, check=True, capture_output=True, text=True)
    logger.debug(f"nvidia-smi output: {result.stdout}")
    
    # Count GPUs
    gpu_count = len([line for line in result.stdout.strip().split('\n') if line.strip()])
    
    logger.debug(f"Found {gpu_count} GPU(s)")
    
    return gpu_count

def get_container_name(uuid_val: str) -> str:
    """
    Get a unique container name in the format "NuRec_uuid_run_<random>".
    """
    random_suffix = str(uuid.uuid4())[:8]
    container_name = f"NuRec_{uuid_val}_run_{random_suffix}"
    return container_name

def check_if_scene_loaded(logs: str) -> bool:
    """
    Check if the scene is loaded by looking for the string "successfully loaded scene" or "done testing gathered scenes" in the logs.
    """
    return "successfully loaded scene" in logs.lower() or "done testing gathered scenes" in logs.lower()


def _normalize_image_name(image: str) -> str:
    """
    Normalize Docker image names for comparison.
    Docker often omits the leading "docker.io/" in image names when listing containers.
    This strips that prefix for consistent equality checks.
    """
    if not image:
        return image
    prefix = "docker.io/"
    if image.startswith(prefix):
        return image[len(prefix):]
    return image


class ServerMonitor:
    def __init__(self, expected_host, expected_port):
        self.expected_host = expected_host
        self.expected_port = expected_port
        self.server_started = threading.Event()
        self.scene_loaded = threading.Event()
        self.error_occurred = threading.Event()
        self.stop_reading = threading.Event()
        self.lock = threading.Lock()
        self.all_output = []
    
    def process_line(self, line: str) -> None:
        line = line.strip()
        if not line:
            return
            
        with self.lock:
            self.all_output.append(line)
            logger.debug(line)
            
            # Check for server start message
            if not self.server_started.is_set():
                # Look for patterns like "serving on nurec.suffix:port" or "localhost:port"
                if "serving on" in line.lower():
                    if (f"{self.expected_host}:{self.expected_port}" in line or 
                        f"localhost:{self.expected_port}" in line):
                        logger.debug(f"Server started: {line}")
                        self.server_started.set()
            
            # Check for scene loaded message
            if not self.scene_loaded.is_set():
                if check_if_scene_loaded(line):
                    logger.debug(f"Scene loaded: {line}")
                    self.scene_loaded.set()
            
            # Check for critical error conditions that should fail verification
            critical_error_patterns = [
                "failed to create backend",
                "failed to load scene", 
                "failed to initialize",
                "traceback (most recent call last)",
                "critical error",
                "fatal error",
                "segmentation fault",
                "out of memory"
            ]
            
            line_lower = line.lower()
            for pattern in critical_error_patterns:
                if pattern in line_lower:
                    logger.error(f"Critical error detected: {line}")
                    self.error_occurred.set()
                    return
            
            # Check for other potential errors (but don't fail verification)
            if any(keyword in line_lower for keyword in ["error", "failed", "exception"]):
                if "error" in line_lower and "no error" not in line_lower:
                    logger.warning(f"Potential error detected: {line}")
    
    def wait_for_ready(self, timeout: int = 120) -> bool:
        """Wait for both server start and scene load, with timeout"""
        start_time = time.time()
        
        while time.time() - start_time < timeout:
            if self.error_occurred.is_set():
                self.stop_reading.set()  # Signal threads to stop
                return False
            if self.server_started.is_set() and self.scene_loaded.is_set():
                self.stop_reading.set()  # Signal threads to stop
                return True
            time.sleep(0.1)
        
        self.stop_reading.set()  # Signal threads to stop on timeout
        return False

def read_output_thread(output_stream: TextIO, monitor: ServerMonitor) -> None:
    """Read output stream and pass lines to monitor with timeout"""
    try:
        while not monitor.stop_reading.is_set():
            ready, _, _ = select.select([output_stream], [], [], 0.5)  # 0.5 second timeout
            
            if ready:
                line = output_stream.readline()
                if not line:
                    break
                monitor.process_line(line)
            
            # Check if we should stop reading
            if monitor.stop_reading.is_set():
                break
                    
    except Exception as e:
        logger.error(f"Error reading output: {e}")
    
    logger.debug("Output reading thread exiting")

class NuRecRenderService:
    def __init__(self, usdz_path, port=None, image=None, reuse_container=False,
                 renderer=None, extra_server_args=None):
        self.usdz_path = usdz_path
        self.port = port
        self.image: str = image or os.getenv("NUREC_IMAGE") or DEFAULT_NUREC_IMAGE
        self.reuse_container = reuse_container
        # Renderer backend: None (server default), 'gsplat' or 'nrend'.
        # Named renderer_backend because NurecScenario (a subclass) uses
        # self.renderer for its NurecRenderer client object.
        self.renderer_backend = renderer
        # Escape hatch for any other serve-grpc flag, e.g. ["--enable-harmonizer"].
        self.extra_server_args = list(extra_server_args or [])
        self.container_name = None
        self.final_port = None
        self.server_version = None
        self.process = None
        self.monitor = None
        self.stdout_thread = None
        self.stderr_thread = None
        self.container_running = False
        self.container_reused = False
        self._cleanup_registered = False
    
    def _register_cleanup(self):
        """Register cleanup handler with atexit"""
        if not self._cleanup_registered:
            atexit.register(self._cleanup)
            self._cleanup_registered = True
    
    def _cleanup(self):
        """Cleanup function for atexit"""
        if self.container_running and self.container_name and not self.reuse_container:
            logger.info(f"Stopping container {self.container_name} at exit")
            subprocess.run(["docker", "stop", "-t0", self.container_name], 
                         capture_output=True, text=True)
            self.container_running = False
    
    def _find_existing_container(self, uuid_val):
        """Find existing running container with matching UUID"""
        try:
            # List running containers
            result = subprocess.run([
                "docker", "ps", "--format", "{{.Names}}\t{{.Image}}"
            ], capture_output=True, text=True, check=True)
            
            running_containers = result.stdout.strip().split('\n')
            
            # Look for containers matching our pattern
            pattern = f"NuRec_{uuid_val}_run_"
            matching_container = None
            for line in running_containers:
                parts = line.strip().split('\t')
                if line == "":
                    continue
                if len(parts) != 2:
                    logger.warning(f"Skipping container {line} because it has the wrong format")
                    continue
                name, image = parts
                if pattern in name and _normalize_image_name(image) == _normalize_image_name(self.image):
                    matching_container = name
                    break
            
            if matching_container:
                logger.debug(f"Found existing container: {matching_container}")
                return matching_container
            
            return None
        except subprocess.CalledProcessError as e:
            logger.error(f"Error listing containers: {e}")
            return None
    
    def _server_args_signature(self) -> str:
        """Canonical string of the serve-grpc flags that affect rendering."""
        flags = [f"--renderer={self.renderer_backend}"] if self.renderer_backend else []
        flags.extend(self.extra_server_args)
        return " ".join(flags)

    def _get_container_labels(self, container_name):
        """Read the port/scene/args labels this module stamps on containers it starts."""
        try:
            result = subprocess.run([
                "docker", "inspect", container_name,
                "--format", f'{{{{index .Config.Labels "{LABEL_PORT}"}}}}\t{{{{index .Config.Labels "{LABEL_SCENE}"}}}}\t{{{{index .Config.Labels "{LABEL_ARGS}"}}}}'
            ], capture_output=True, text=True, check=True)
            port_str, scene, server_args = (result.stdout.strip("\n").split("\t") + ["", ""])[:3]
            return (int(port_str) if port_str.isdigit() else None), scene, server_args
        except subprocess.CalledProcessError as e:
            logger.error(f"Error inspecting container {container_name}: {e}")
            return None, "", ""

    def _verify_container_grpc(self, container_name, expected_scene):
        """
        Verify a running container over gRPC: server answers get_version and
        the expected scene is among get_available_scenes. Returns the port on
        success, None on failure.
        """
        port, labeled_scene, labeled_args = self._get_container_labels(container_name)
        if port is None:
            logger.warning(f"Container {container_name} has no {LABEL_PORT} label; not reusing")
            return None
        if labeled_scene and labeled_scene != expected_scene:
            logger.warning(f"Container {container_name} serves scene {labeled_scene!r}, wanted {expected_scene!r}")
            return None
        if labeled_args != self._server_args_signature():
            logger.warning(
                f"Container {container_name} was started with server args {labeled_args!r}, "
                f"wanted {self._server_args_signature()!r}; not reusing"
            )
            return None
        version, scene_ids = query_server("localhost", port)
        if version is None:
            logger.warning(f"Container {container_name} not answering gRPC on port {port}")
            return None
        if not any(expected_scene in scene_id for scene_id in scene_ids):
            logger.warning(f"Container {container_name} scenes {scene_ids} do not include {expected_scene!r}")
            return None
        logger.debug(f"Container {container_name} verified: NRE {version}, port {port}, scenes {scene_ids}")
        self.server_version = version
        return port
    
    def _kill_old_nurec_containers(self):
        """Kill any old NuRec containers running the same image"""
        try:
            # List all containers (running and stopped) with their images
            result = subprocess.run([
                "docker", "ps", "-a", "--format", "{{.Names}}\t{{.Image}}"
            ], capture_output=True, text=True, check=True)
            
            container_lines = result.stdout.strip().split('\n')
            
            # Look for NuRec containers
            containers_to_kill = []
            for line in container_lines:
                if not line.strip():
                    continue
                    
                parts = line.split('\t')
                if len(parts) >= 2:
                    container_name = parts[0]
                    container_image = parts[1]
                    
                    # Check if it's a NuRec container with the same image
                    if (container_name.startswith("NuRec_") and 
                        _normalize_image_name(container_image) == _normalize_image_name(self.image)):
                        containers_to_kill.append(container_name)
            
            if containers_to_kill:
                logger.info(f"Killing old NuRec containers with same image: {containers_to_kill}")
                for container in containers_to_kill:
                    result = subprocess.run([
                        "docker", "stop", "-t0", container
                    ], capture_output=True, text=True)
                    
                    if result.returncode == 0:
                        logger.debug(f"Successfully killed container: {container}")
                    else:
                        logger.warning(f"Failed to kill container {container}: {result.stderr}")
            else:
                logger.debug("No old NuRec containers found to kill")
            
        except subprocess.CalledProcessError as e:
            logger.error(f"Error managing containers: {e}")
    
    def start(self) -> Dict[str, Any]:
        """Start the gRPC server"""
        if self.container_running:
            return {
                'container_name': self.container_name,
                'host': 'localhost',
                'port': self.final_port,
                'status': 'already_running'
            }
        
        if not os.path.exists(self.usdz_path):
            raise FileNotFoundError(f"USDZ file not found at {self.usdz_path}")
        
        try:
            json_files = extract_json_from_usdz(self.usdz_path, ["data_info.json"])
            data_info = json_files["data_info.json"]
            sequence_id = data_info["sequence_id"]
            logger.debug(f"Parsed metadata from USDZ file, sequence_id: {sequence_id}")
            uuid_val = str(sequence_id)
        except Exception as e:
            logger.error(f"Error extracting sequence_id from USDZ file: {e}")
            raise e

        expected_scene = os.path.basename(self.usdz_path).replace(".usdz", "")
        
        # Handle container reuse logic
        if self.reuse_container:
            existing_container = self._find_existing_container(uuid_val)

            if existing_container:
                verified_port = self._verify_container_grpc(existing_container, expected_scene)
                if verified_port is not None:
                    logger.debug(f"Reusing existing container: {existing_container}")
                    self.container_name = existing_container
                    self.container_running = True
                    self.container_reused = True
                    self.final_port = verified_port
                    logger.debug(f"Container already ready at localhost:{self.final_port}")

                    return {
                        'container_name': self.container_name,
                        'host': 'localhost',
                        'port': self.final_port,
                        'status': 'reused'
                    }
                else:
                    logger.warning(f"Existing container {existing_container} verification failed")

            # Kill any old NuRec containers with the same image and start fresh
            self._kill_old_nurec_containers()

        # Continue with normal container startup
        usdz_folder = os.path.dirname(os.path.realpath(self.usdz_path))
        self.container_name = get_container_name(uuid_val)

        # Explicit port, or an OS-assigned free one (never CARLA's 2000).
        self.final_port = self.port if self.port else find_free_port()

        gpu_count = run_nvidia_smi(self.image)
        if gpu_count == 0:
            raise RuntimeError("No GPUs found")
        
        # Use GPU 0 unless CUDA_VISIBLE_DEVICES is set
        if 'CUDA_VISIBLE_DEVICES' in os.environ:
            visible_gpu_ids = os.environ['CUDA_VISIBLE_DEVICES']
            logger.debug(f"Using CUDA_VISIBLE_DEVICES from environment: {visible_gpu_ids}")
        else:
            visible_gpu_ids = "0"
            logger.info(f"CUDA_VISIBLE_DEVICES not set, defaulting to GPU 0")
        
        logger.debug(f"Running on GPU(s): {visible_gpu_ids}")

        cmd = [
            "docker",
            "run",
            "--env",
            f"CUDA_VISIBLE_DEVICES={visible_gpu_ids}",
            "--name",
            self.container_name,
            "--label",
            f"{LABEL_PORT}={self.final_port}",
            "--label",
            f"{LABEL_SCENE}={expected_scene}",
            "--label",
            f"{LABEL_ARGS}={self._server_args_signature()}",
            "--gpus",
            "all",
            "--rm",
            "--net=host",
            "-v",
            f"{usdz_folder}:{usdz_folder}:ro",
            self.image,
            # NRE >= 26.04 ships a multi-command CLI; the gRPC server is the
            # serve-grpc subcommand (the old nvidia-nurec-grpc image served
            # directly from its entrypoint).
            "serve-grpc",
            "--artifact-glob",
            f"{os.path.realpath(self.usdz_path)}",
            f"--port={self.final_port}",
            "--host=localhost",
            "--test-scenes-are-valid",
            # NRE >= 26.04 rejects render requests carrying DynamicObject pose
            # updates unless this is set, and the integration sends them every
            # tick (the old nvidia-nurec-grpc server accepted them by default).
            "--enable-editing-actors",
        ]
        if self.renderer_backend:
            cmd.append(f"--renderer={self.renderer_backend}")
        cmd.extend(self.extra_server_args)

        # Register cleanup handler
        if not self.reuse_container:
            self._register_cleanup()

        logger.info(f"Starting container {self.container_name} on localhost:{self.final_port}")
        logger.debug(f"Command: {' '.join(cmd)}")

        # Monitor container output for critical errors; readiness itself is
        # detected over gRPC, not by parsing logs.
        self.monitor = ServerMonitor("localhost", self.final_port)

        self.process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.container_running = True

        # Keep draining output for the container's lifetime: it surfaces
        # runtime errors and stops the pipe from filling up.
        self.stdout_thread = threading.Thread(
            target=read_output_thread, args=(self.process.stdout, self.monitor), daemon=True)
        self.stderr_thread = threading.Thread(
            target=read_output_thread, args=(self.process.stderr, self.monitor), daemon=True)

        self.stdout_thread.start()
        self.stderr_thread.start()

        logger.info("Waiting for gRPC server to answer and scene to be available...")
        if self._wait_ready_grpc(expected_scene, timeout=300):
            logger.info(f"Server ready: NRE {self.server_version} at localhost:{self.final_port}")

            return {
                'container_name': self.container_name,
                'host': 'localhost',
                'port': self.final_port,
                'status': 'ready'
            }
        else:
            if self.process.poll() is None:
                logger.error("Timeout waiting for server to be ready")
                self.process.terminate()
            else:
                logger.error(f"Container exited with code {self.process.returncode}")

            self.monitor.stop_reading.set()
            self.stdout_thread.join(timeout=5)
            self.stderr_thread.join(timeout=5)

            self.container_running = False
            tail = "\n".join(self.monitor.all_output[-30:])
            raise RuntimeError(f"NuRec server failed to start. Last container output:\n{tail}")

    def _wait_ready_grpc(self, expected_scene: str, timeout: int = 300) -> bool:
        """
        Poll the gRPC endpoint until the server answers get_version and
        get_available_scenes contains the expected scene. Aborts early if the
        container process exits or its logs show a critical error.
        """
        start_time = time.time()
        while time.time() - start_time < timeout:
            if self.monitor.error_occurred.is_set():
                logger.error("Critical error in container logs while waiting for readiness")
                return False
            if self.process.poll() is not None:
                logger.error("Container process exited while waiting for readiness")
                return False

            version, scene_ids = query_server("localhost", self.final_port)
            if version is not None:
                self.server_version = version
                if any(expected_scene in scene_id for scene_id in scene_ids):
                    return True
                logger.debug(f"Server up (NRE {version}) but scene {expected_scene!r} "
                             f"not yet in {scene_ids}; still waiting")
            time.sleep(1.0)

        return False
    
    def stop(self) -> None:
        """Stop the server"""
        if self.reuse_container:
            logger.info(f"Not stopping container {self.container_name} because we want to reuse it.")
            return
            
        if self.container_running and self.container_name:
            logger.info(f"Stopping container {self.container_name}")
            subprocess.run(["docker", "stop", "-t0", self.container_name], 
                         capture_output=True, text=True)
            self.container_running = False
    
    def __enter__(self):
        """Context manager entry"""
        self.start()
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Context manager exit"""
        self.stop()
        return False
