# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
This file contains scripts to start the grpc server to render images. Example command line to start server:
docker run --env "CUDA_VISIBLE_DEVICES=1" --name="nre_1" \
 --gpus 'all,"capabilities=compute,video,utility"' --rm \
 --net=host -v $(pwd):$(pwd) --gpus=all $NUREC_IMAGE \
 --artifact-glob "$(pwd)/carla/clipgt-7f360cc2-371e-4606-9dc9-b9d0822928a8.usdz" --no-enable-nrend --port=46435 --host=0.0.0.0 --test-scenes-are-valid
"""

import os
import sys
import subprocess
import re
import logging
import uuid
import threading
import atexit
import time
import select
from typing import Optional, Dict, Any, Union, TextIO
from scenario import extract_json_from_usdz

logger = logging.getLogger(__name__)

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
    def __init__(self, usdz_path, port=None, image=None, reuse_container=False):
        self.usdz_path = usdz_path
        self.port = port
        self.image = image
        self.reuse_container = reuse_container
        self.container_name = None
        self.final_port = None
        self.process = None
        self.monitor = None
        self.stdout_thread = None
        self.stderr_thread = None
        self.container_running = False
        self.container_reused = False
        self._cleanup_registered = False

        if image is None:
            self.image = os.getenv("NUREC_IMAGE")
            if self.image is None:
                raise ValueError("NUREC_IMAGE environment variable is not set, either pass it as an argument or set it in the environment!")
    
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
    
    def _verify_container_logs(self, container_name, expected_scene, expected_port):
        """Verify container is running correctly and has the right scene and port"""
        try:
            # Get container logs
            result = subprocess.run([
                "docker", "logs", container_name
            ], capture_output=True, text=True, check=True)
            
            logs = result.stdout + result.stderr
            
            # Check for server start
            server_started = "serving on" in logs.lower()
            
            # Check for correct scene
            scene_loaded = check_if_scene_loaded(logs)
            correct_scene = expected_scene in logs
            
            # Check for correct port
            extracted_port = self._extract_port_from_logs(logs)
            correct_port = extracted_port == expected_port if extracted_port else False
            
            logger.debug(f"Container {container_name} verification:")
            logger.debug(f"  Server started: {server_started}")
            logger.debug(f"  Scene loaded: {scene_loaded}")
            logger.debug(f"  Correct scene ({expected_scene}): {correct_scene}")
            logger.debug(f"  Expected port: {expected_port}, Found port: {extracted_port}, Correct port: {correct_port}")
            
            return server_started and scene_loaded and correct_scene and correct_port
            
        except subprocess.CalledProcessError as e:
            logger.error(f"Error getting container logs: {e}")
            return False
    
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
    
    def _extract_port_from_logs(self, logs):
        """Extract port number from container logs"""
        import re
        # Look for patterns like "serving on localhost:2000"
        port_match = re.search(r'serving on (?:localhost|127\.0\.0\.1):(\d+)', logs, re.IGNORECASE)
        if port_match:
            return int(port_match.group(1))
        return None
    
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
            # Determine the expected port first
            expected_port = self.port if self.port else 2000
            
            existing_container = self._find_existing_container(uuid_val)
            
            if existing_container:
                # Verify the existing container
                if self._verify_container_logs(existing_container, expected_scene, expected_port):
                    logger.debug(f"Reusing existing container: {existing_container}")
                    self.container_name = existing_container
                    self.container_running = True
                    self.container_reused = True
                    
                    # Extract port from logs
                    logs_result = subprocess.run([
                        "docker", "logs", existing_container
                    ], capture_output=True, text=True)
                    
                    logs = logs_result.stdout + logs_result.stderr
                    extracted_port = self._extract_port_from_logs(logs)
                    
                    if extracted_port:
                        self.final_port = extracted_port
                    else:
                        self.final_port = expected_port
                    
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
        
        # Set default port if not provided
        if not self.port:
            self.final_port = 2000
        else:
            self.final_port = self.port

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
            "--gpus",
            'all,"capabilities=compute,video,utility"',
            "--rm",
            "--net=host",
            "-v",
            f"{usdz_folder}:{usdz_folder}:ro",
            "--gpus=all",
            self.image,
            "--artifact-glob",
            f"{os.path.realpath(self.usdz_path)}",
            f"--port={self.final_port}",
            "--host=localhost",
            "--test-scenes-are-valid",
        ]

        # Register cleanup handler
        if not self.reuse_container:
            self._register_cleanup()

        logger.info(f"Starting container {self.container_name} on localhost:{self.final_port}")
        logger.debug(f"Command: {' '.join(cmd)}")

        # Create monitor for server readiness
        self.monitor = ServerMonitor("localhost", self.final_port)

        # Start process with live output monitoring
        self.process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        self.container_running = True
        
        # Start threads to read output
        self.stdout_thread = threading.Thread(target=read_output_thread, args=(self.process.stdout, self.monitor))
        self.stderr_thread = threading.Thread(target=read_output_thread, args=(self.process.stderr, self.monitor))
        
        self.stdout_thread.start()
        self.stderr_thread.start()

        # Wait for server to be ready
        logger.info("Waiting for server to start and scene to load...")
        if self.monitor.wait_for_ready(timeout=120):
            logger.info("Server is ready! Both server started and scene loaded successfully.")
            
            return {
                'container_name': self.container_name,
                'host': 'localhost',
                'port': self.final_port,
                'status': 'ready'
            }
        else:
            # Check if process is still running
            if self.process.poll() is None:
                logger.error("Timeout waiting for server to be ready")
                self.process.terminate()
            else:
                logger.error(f"Container exited with code {self.process.returncode}")
            
            # Wait for threads to finish
            self.stdout_thread.join(timeout=5)
            self.stderr_thread.join(timeout=5)
            
            self.container_running = False
            raise RuntimeError("Server failed to start properly")
    
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
