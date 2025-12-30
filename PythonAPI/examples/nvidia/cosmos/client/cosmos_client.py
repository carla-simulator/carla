# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

import time
import json
from pathlib import Path
import argparse
import toml
import typing
import gradio_client.client as gradio_client
import gradio_client.utils as gradio_utils
from loguru import logger
import shutil
import random


def validate_specs(config_data):
    """
        {
        "blur_strength": "medium",
        "canny_threshold": "medium",
        "edge": {"input_control": "edge.mp4", "control_weight": 1.0},
        "depth": {"input_control": "depth.mp4", "control_weight": 1.0},
        "seg": {"input_control": "seg.mp4", "control_weight": 1.0},
        "vis": {"control_weight": 0.3},
        "guidance": 7.0,
        "input_video_path": input_video_path,
        "negative_prompt": "...",
        "num_steps": 35,
        "prompt": "...",
        "seed": 1,
        "sigma_max": 70.0,
    }
    """

    specs = config_data
    print(specs)
    
    if not isinstance(specs, dict):
        raise ValueError("The configuration must be a dictionary of specifications.")


    # Required scalar fields
    required_scalar_fields_types = {
        'input_video_path': str,
        'prompt': str,
        'negative_prompt': str,
        'guidance': (int, float),
        'num_steps': int,
        'seed': int,
        'sigma_max': (int, float),
    }
    for field, expected_type in required_scalar_fields_types.items():
        if field not in specs:
            raise ValueError(f"Missing required field '{field}' in the specification.")
        if not isinstance(specs[field], expected_type):
            raise ValueError(f"Field '{field}' must be of type {expected_type}.")

    # Optional scalar fields (validate type if present)
    optional_scalar_fields_types = {
        'blur_strength': str,
        'canny_threshold': str,
    }
    for field, expected_type in optional_scalar_fields_types.items():
        if field in specs and not isinstance(specs[field], expected_type):
            raise ValueError(f"Field '{field}' must be of type {expected_type} if provided.")

    # Controls validation
    def validate_control(name: str, require_input_control: bool):
        if name not in specs:
            raise ValueError(f"Missing required control '{name}' in the specification.")
        ctrl = specs[name]
        if not isinstance(ctrl, dict):
            raise ValueError(f"Control '{name}' must be a dictionary.")
        if 'control_weight' not in ctrl:
            raise ValueError(f"Control '{name}' is missing 'control_weight'.")
        if not isinstance(ctrl['control_weight'], (int, float)):
            raise ValueError(f"Control '{name}'.control_weight must be numeric.")
        if require_input_control:
            if 'input_control' not in ctrl:
                raise ValueError(f"Control '{name}' is missing 'input_control'.")
            if not isinstance(ctrl['input_control'], str):
                raise ValueError(f"Control '{name}'.input_control must be a string (path).")
        else:
            if 'input_control' in ctrl and not isinstance(ctrl['input_control'], str):
                raise ValueError(f"Control '{name}'.input_control must be a string (path) if provided.")

    # Optional controls: seg (requires input_control), vis (input_control optional)
    if 'edge' in specs:
        validate_control('edge', require_input_control=True)
    if 'depth' in specs:
        validate_control('depth', require_input_control=True)
    if 'seg' in specs:
        validate_control('seg', require_input_control=True)
    if 'vis' in specs:
        validate_control('vis', require_input_control=False)

    print("specs validation passed.")


def open_and_validate_config_toml(config_toml):
    with open(config_toml, 'r') as config_toml:
        config_data = toml.load(config_toml)

    validate_specs(config_data)
    return config_data


def extract_options(config_data):
    options = {
       'options': json.dumps(config_data)
    }
    return options

def _async_with_upload_example(
    url: str,
    config_data: dict,
    *,
    retry_max_retries: int = 3,
    backoff_initial_seconds: float = 1.5,
    backoff_multiplier: float = 2.0,
    jitter_seconds: float = 0.5,
    poll_interval_seconds: int = 5,
    result_timeout_seconds: int = 120,
):
    """
    Asynchronous inference using config data. Uploads input/edge/depth/seg/vis videos from
    the provided config, replaces their paths with remote paths, and submits the
    full configuration to the server.

    - client.submit(api_name="/upload_file") -> async job per local file
    - wait_for_job(upload_job) -> remote_path
    - client.submit(request_json, api_name="/generate_video") -> async job
    - wait_for_job() -> local_video_path
    """

    logger.info("--------------------------------")
    logger.info("Asynchronous inference with config data + local file uploads")

    client = gradio_client.Client(url)

    # Extract paths from config (support nested or flat schema)
    if 'controlnet_specs' in config_data and isinstance(config_data['controlnet_specs'], dict):
        specs = config_data['controlnet_specs']
    else:
        specs = config_data

    input_video_path = specs.get('input_video_path')
    edge_video = specs.get('edge', {}).get('input_control') if isinstance(specs.get('edge'), dict) else None
    depth_video = specs.get('depth', {}).get('input_control') if isinstance(specs.get('depth'), dict) else None
    seg_video = specs.get('seg', {}).get('input_control') if isinstance(specs.get('seg'), dict) else None
    vis_video = specs.get('vis', {}).get('input_control') if isinstance(specs.get('vis'), dict) else None

    # Helpers for parsing results
    def _parse_upload_result(upload_result: typing.Any) -> str:
        try:
            obj: typing.Any = upload_result
            if isinstance(obj, str):
                obj = json.loads(obj)
            if isinstance(obj, dict) and "path" in obj:
                return typing.cast(str, obj["path"])
            if isinstance(obj, (list, tuple)):
                for item in obj:
                    if isinstance(item, dict) and "path" in item:
                        return typing.cast(str, item["path"])
            raise ValueError(f"Unexpected upload result format: {repr(upload_result)[:200]}")
        except Exception as e:
            raise RuntimeError(f"Failed to parse upload result: {e}")

    def _extract_video_path_from_generate_result(generate_result: typing.Any) -> str:
        # Common cases: list → first item dict with key 'video'; or path string
        if isinstance(generate_result, (list, tuple)):
            for item in generate_result:
                if isinstance(item, dict) and "video" in item:
                    return typing.cast(str, item["video"])
                if isinstance(item, str) and item.lower().endswith(".mp4"):
                    return item
        if isinstance(generate_result, dict) and "video" in generate_result:
            return typing.cast(str, generate_result["video"])
        if isinstance(generate_result, str) and generate_result.lower().endswith(".mp4"):
            return generate_result
        raise RuntimeError(f"Unexpected generate result format: {repr(generate_result)[:200]}")

    # Upload input video (with retries)
    file_descriptor = gradio_utils.handle_file(input_video_path)
    upload_file_result = _submit_with_retry(
        client,
        file_descriptor,
        api_name="/upload_file",
        max_retries=retry_max_retries,
        backoff_initial_seconds=backoff_initial_seconds,
        backoff_multiplier=backoff_multiplier,
        jitter_seconds=jitter_seconds,
        poll_interval_seconds=poll_interval_seconds,
        result_timeout_seconds=result_timeout_seconds,
    )
    remote_input_path = _parse_upload_result(upload_file_result)
    logger.info(f"remote_input_path={remote_input_path}")

    # Optionally upload edge and depth control videos
    remote_edge_path = None
    remote_depth_path = None
    remote_vis_path = None
    remote_seg_path = None

    if edge_video:
        edge_fd = gradio_utils.handle_file(edge_video)
        edge_result = _submit_with_retry(
            client,
            edge_fd,
            api_name="/upload_file",
            max_retries=retry_max_retries,
            backoff_initial_seconds=backoff_initial_seconds,
            backoff_multiplier=backoff_multiplier,
            jitter_seconds=jitter_seconds,
            poll_interval_seconds=poll_interval_seconds,
            result_timeout_seconds=result_timeout_seconds,
        )
        remote_edge_path = _parse_upload_result(edge_result)
        logger.info(f"remote_edge_path={remote_edge_path}")

    if depth_video:
        depth_fd = gradio_utils.handle_file(depth_video)
        depth_result = _submit_with_retry(
            client,
            depth_fd,
            api_name="/upload_file",
            max_retries=retry_max_retries,
            backoff_initial_seconds=backoff_initial_seconds,
            backoff_multiplier=backoff_multiplier,
            jitter_seconds=jitter_seconds,
            poll_interval_seconds=poll_interval_seconds,
            result_timeout_seconds=result_timeout_seconds,
        )
        remote_depth_path = _parse_upload_result(depth_result)
        logger.info(f"remote_depth_path={remote_depth_path}")

    if seg_video:
        seg_fd = gradio_utils.handle_file(seg_video)
        seg_result = _submit_with_retry(
            client,
            seg_fd,
            api_name="/upload_file",
            max_retries=retry_max_retries,
            backoff_initial_seconds=backoff_initial_seconds,
            backoff_multiplier=backoff_multiplier,
            jitter_seconds=jitter_seconds,
            poll_interval_seconds=poll_interval_seconds,
            result_timeout_seconds=result_timeout_seconds,
        )
        remote_seg_path = _parse_upload_result(seg_result)
        logger.info(f"remote_seg_path={remote_seg_path}")

    if vis_video:
        vis_fd = gradio_utils.handle_file(vis_video)
        vis_result = _submit_with_retry(
            client,
            vis_fd,
            api_name="/upload_file",
            max_retries=retry_max_retries,
            backoff_initial_seconds=backoff_initial_seconds,
            backoff_multiplier=backoff_multiplier,
            jitter_seconds=jitter_seconds,
            poll_interval_seconds=poll_interval_seconds,
            result_timeout_seconds=result_timeout_seconds,
        )
        remote_vis_path = _parse_upload_result(vis_result)
        logger.info(f"remote_vis_path={remote_vis_path}")

    # Inject remote paths back into specs (nested or flat)
    specs['input_video_path'] = remote_input_path

    if remote_edge_path is not None:
        specs.setdefault('edge', {})
        specs['edge']['input_control'] = remote_edge_path

    if remote_depth_path is not None:
        specs.setdefault('depth', {})
        specs['depth']['input_control'] = remote_depth_path

    if remote_seg_path is not None:
        specs.setdefault('seg', {})
        specs['seg']['input_control'] = remote_seg_path

    if remote_vis_path is not None:
        specs.setdefault('vis', {})
        specs['vis']['input_control'] = remote_vis_path

    request_text = json.dumps(config_data)

    logger.info(f"generate_video_request: {request_text=}")
    generate_result = _submit_with_retry(
        client,
        request_text,
        api_name="/generate_video",
        max_retries=retry_max_retries,
        backoff_initial_seconds=backoff_initial_seconds,
        backoff_multiplier=backoff_multiplier,
        jitter_seconds=jitter_seconds,
        poll_interval_seconds=poll_interval_seconds,
        result_timeout_seconds=result_timeout_seconds,
    )
    local_video_path = _extract_video_path_from_generate_result(generate_result)

    logger.info(f"Local video path (downloaded to local machine): {local_video_path=}")
    return local_video_path


def _async_wait_for_job(
    job: gradio_client.Job,
    poll_interval_seconds: int = 5,
    result_timeout_seconds: int = 20,
) -> typing.Tuple[gradio_client.StatusUpdate, typing.Any, typing.Optional[Exception]]:
    """
    Waits for a job to complete.

    Returns (tuple):
        - job_status: The status of the job
        - result: The result of the job
        - error: The error of the job
    """
    start_time = time.time()
    while not job.done():
        status = job.status()
        code = getattr(status, "code", None)
        code_value = getattr(code, "value", str(code)) if code is not None else "UNKNOWN"
        queue_size = getattr(status, "queue_size", None)
        rank = getattr(status, "rank", None)
        eta = getattr(status, "eta", None)
        progress = None
        progress_data = getattr(status, "progress_data", None)
        if isinstance(progress_data, dict):
            if "progress" in progress_data:
                progress = progress_data["progress"]
            elif (
                "index" in progress_data and "length" in progress_data and progress_data.get("length")
            ):
                try:
                    progress = float(progress_data["index"]) / float(progress_data["length"])  # type: ignore
                except Exception:
                    progress = None

        eta_str = None
        if eta is not None:
            try:
                if isinstance(eta, (int, float)):
                    total_seconds = int(max(eta, 0))
                else:
                    from datetime import datetime

                    if hasattr(eta, "timestamp"):
                        delta = eta - datetime.utcnow()
                        total_seconds = int(max(delta.total_seconds(), 0))
                    else:
                        total_seconds = None  # type: ignore

                if total_seconds is not None:
                    minutes, seconds = divmod(total_seconds, 60)
                    eta_str = f"{minutes}m{seconds:02d}s"
            except Exception:
                eta_str = None

        elapsed = int(time.time() - start_time)

        parts = []
        if code_value:
            parts.append(f"status={code_value}")
        if progress is not None:
            try:
                pct = int(progress * 100) if isinstance(progress, (int, float)) and progress <= 1.0 else int(progress)  # type: ignore
                parts.append(f"progress={pct}%")
            except Exception:
                pass
        if rank is not None and queue_size is not None:
            parts.append(f"queue={rank}/{queue_size}")
        elif queue_size is not None:
            parts.append(f"queue_size={queue_size}")
        if eta_str:
            parts.append(f"eta={eta_str}")
        parts.append(f"elapsed={elapsed}s")

        logger.info("Waiting for job... " + ", ".join(parts))
        time.sleep(max(1, int(poll_interval_seconds)))

    job_status: gradio_client.StatusUpdate = job.status()
    if job_status.success:
        try:
            result = job.result(timeout=max(1, int(result_timeout_seconds)))
            logger.info(f"[SUCCESS] {result=}")
            return job_status, result, None
        except Exception as e:
            logger.error(f"[EXCEPTION] {job_status=} {e=}")
            return job_status, None, e
    else:
        logger.warning(f"[NO_OUTPUT] {job_status=}")
        return job_status, None, None


def _sleep_with_backoff(
    attempt_index: int,
    backoff_initial_seconds: float,
    backoff_multiplier: float,
    jitter_seconds: float,
) -> None:
    delay = backoff_initial_seconds * (backoff_multiplier ** attempt_index)
    if jitter_seconds > 0:
        delay += random.uniform(0, jitter_seconds)
    delay = max(0.0, float(delay))
    logger.info(f"Retrying after {delay:.2f}s (attempt {attempt_index + 1})")
    time.sleep(delay)


def _submit_with_retry(
    client: gradio_client.Client,
    payload: typing.Any,
    api_name: str,
    *,
    max_retries: int = 3,
    backoff_initial_seconds: float = 1.5,
    backoff_multiplier: float = 2.0,
    jitter_seconds: float = 0.5,
    poll_interval_seconds: int = 5,
    result_timeout_seconds: int = 120,
) -> typing.Any:
    last_error: typing.Optional[BaseException] = None
    attempts = max(0, int(max_retries)) + 1
    for attempt in range(attempts):
        try:
            job = client.submit(payload, api_name=api_name)
        except Exception as e:
            last_error = e
            logger.warning(f"Submit error on {api_name}: {e}.")
            if attempt < attempts - 1:
                _sleep_with_backoff(attempt, backoff_initial_seconds, backoff_multiplier, jitter_seconds)
                continue
            break

        job_status, result, error = _async_wait_for_job(
            job,
            poll_interval_seconds=poll_interval_seconds,
            result_timeout_seconds=result_timeout_seconds,
        )

        if error:
            last_error = error
            logger.warning(f"Job error on {api_name}: {error}.")
        if getattr(job_status, "success", False) and result is not None:
            return result

        logger.warning(
            f"No output from job (success={getattr(job_status, 'success', None)}). Will retry: {attempt < attempts - 1}"
        )
        if attempt < attempts - 1:
            _sleep_with_backoff(attempt, backoff_initial_seconds, backoff_multiplier, jitter_seconds)
            continue
        break

    details = f"api_name={api_name}, last_error={last_error!r}"
    raise RuntimeError(f"Failed to complete request after {attempts} attempts: {details}")


def main():
    parser = argparse.ArgumentParser(description="Submit and retrieve video processing job")
    parser.add_argument("endpoint", help="Base URL of the FastAPI server (e.g., http://localhost:8080)")
    parser.add_argument("config_toml", help="Path to the TOML file with processing configuration")
    parser.add_argument("--output", help="Path to save the result video", default=None)
    # optional overrides
    parser.add_argument("--input-video", help="Override input video path from config", default=None)
    parser.add_argument("--edge-video", help="Override edge control video path from config", default=None)
    parser.add_argument("--depth-video", help="Override depth control video path from config", default=None)
    parser.add_argument("--seg-video", help="Override seg control video path from config", default=None)
    parser.add_argument("--vis-video", help="Override vis control video path from config", default=None)
    parser.add_argument("--seed", help="Override seed from config", default=None)
    # reliability controls
    parser.add_argument("--retries", type=int, default=3, help="Max retries for uploads and generation")
    parser.add_argument("--backoff-initial", type=float, default=1.5, help="Initial backoff seconds")
    parser.add_argument("--backoff-multiplier", type=float, default=2.0, help="Backoff multiplier")
    parser.add_argument("--jitter", type=float, default=0.5, help="Random jitter added to backoff")
    parser.add_argument("--poll-interval", type=int, default=5, help="Polling interval seconds for job status")
    parser.add_argument("--result-timeout", type=int, default=120, help="Timeout seconds when fetching job result")

    args = parser.parse_args()

    # Load and validate the TOML configuration
    config_data = open_and_validate_config_toml(args.config_toml)

    # Apply video overrides if provided (support nested or flat schema)
    if 'controlnet_specs' in config_data and isinstance(config_data['controlnet_specs'], dict):
        specs = config_data['controlnet_specs']
    else:
        specs = config_data

    if args.input_video:
        specs['input_video_path'] = args.input_video
        print(f"Overriding input video with: {args.input_video}")

    if args.edge_video:
        specs.setdefault('edge', {})
        specs['edge']['input_control'] = args.edge_video
        print(f"Overriding edge video with: {args.edge_video}")

    if args.depth_video:
        specs.setdefault('depth', {})
        specs['depth']['input_control'] = args.depth_video
        print(f"Overriding depth video with: {args.depth_video}")

    if args.seg_video:
        specs.setdefault('seg', {})
        specs['seg']['input_control'] = args.seg_video
        print(f"Overriding seg video with: {args.seg_video}")

    if args.vis_video:
        specs.setdefault('vis', {})
        specs['vis']['input_control'] = args.vis_video
        print(f"Overriding vis video with: {args.vis_video}")

    if args.seed:
        specs.setdefault('seed', {})
        specs['seed'] = int(args.seed)
        print(f"Overriding seed with: {args.seed}")

    local_video_path = _async_with_upload_example(
        args.endpoint,
        config_data,
        retry_max_retries=args.retries,
        backoff_initial_seconds=args.backoff_initial,
        backoff_multiplier=args.backoff_multiplier,
        jitter_seconds=args.jitter,
        poll_interval_seconds=args.poll_interval,
        result_timeout_seconds=args.result_timeout,
    )

    if not local_video_path:
        logger.error("No output video was produced by the job.")
        return

    if args.output:
        target_path = Path(args.output)
        # If a directory is provided (or endswith '/'), use the source filename
        if target_path.is_dir() or str(args.output).endswith('/'):
            target_file = target_path / Path(local_video_path).name
        else:
            target_file = target_path

        target_file.parent.mkdir(parents=True, exist_ok=True)
        try:
            shutil.move(str(local_video_path), str(target_file))
            logger.info(f"Saved output video to: {target_file}")
        except Exception as e:
            logger.error(f"Failed to save output to {target_file}: {e}")


if __name__ == "__main__":
    main()
