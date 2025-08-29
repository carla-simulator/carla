# Getting started with CARLA-Cosmos-Transfer Client

This README provides instructions for setting up a resimulation with CARLA using logs, rendering the necessary control inputs for Cosmos-Transfer, and making requests to the pre-deployed CARLA-Cosmos-Transfer server. For more information on the server setup, please refer to `server/README_SERVER.md`.

We use a REST client, located at `client/cosmos_client.py` that sends queries to a remote Cosmos-Transfer1 service based on your TOML configuration file. An example configuration can be found at `client/example_data/prompts/rain.toml`.

## Setup Python Environment

1. Install `conda`, see [instructions](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html). You can skip this step if conda is already installed on your system.

2. Create the `carla-cosmos-client` conda environment and install all dependencies:

    ```bash
    cd client
    # Create the carla-cosmos-client conda environment.
    conda env create --file carla-cosmos-client.yaml
    # Activate the carla-cosmos-client conda environment.
    conda activate carla-cosmos-client
    # Install the dependencies.
    pip install -r requirements_client.txt
    ```

3. To install CARLA Python Client navigate to the PythonAPI/carla/dist folder within the CARLA installation. Locate the .whl file corresponding to your Python version (e.g., carla-0.9.16-cp310-cp310-linux_x86_64.whl):

    ```bash
    # Replace carla-0.9.16-cp310-cp310-linux_x86_64.whl with your actual filename
    pip install carla-0.9.16-cp310-cp310-linux_x86_64.whl
    ```

## Generating Cosmos-Transfer Control Inputs 

1. Start CARLA by navigating to the folder of your CARLA installation and execute:

    ```bash
    ./CarlaUE4.sh
    ```

2. If you want to generate new control inputs, you can run the `PythonAPI/examples/nvidia/cosmos-transfer/client/carla_cosmos_gen.py` with the `PythonAPI/examples/nvidia/cosmos-transfer/client/example_data/logs/inverted_ai/iai_carla_synthetic_log_1731622446_actorPOV4641_startTime3.7s_log`. A typical invocation will look like this:

    ```bash
    cd client
    # Replace /full_path_to_log/your_log.log with the absolute path to your log file and output_path with your path to store the results (can be a relative path)
    python carla_cosmos_gen.py -f /full_path_to_log/your_log.log --sensors cosmos_aov.yaml --class-filter-config filter_semantic_classes.yaml -c ego_sim_id -s 0.0 -d 5.0 -o output_path
    ```

    **Note**: For the example log, please replace ego_sim_id with 4641.

## Making Requests

Once you have a set of artifacts and the server has been deployed and is active, use the `cosmos_client.py` to make queries.

```bash
cd client
# Replace https://url_to_server with the URL to your CARLA-Cosmos-Transfer1 server
python cosmos_client.py http://url_to_server:port example_data/prompts/rain.toml
```

You can optionally override some fields from the TOML on the command line and choose where to save the output:

```bash
python cosmos_client.py http://url_to_server:port \
  example_data/prompts/rain.toml \
  --output outputs/ \
  --input-video example_data/artifacts/rgb.mp4 \
  --edge-video example_data/artifacts/edges.mp4 \
  --depth-video example_data/artifacts/depth.mp4 \ # optional
  --seg-video example_data/artifacts/semantic_segmentation.mp4 \
  --vis-video example_data/artifacts/vis_control.mp4 \ # optional
  --seed 2048
```

## Cosmos-Transfer1 Configuration

This section describes the TOML configuration (see `example_data/prompts/rain.toml`). The client accepts a flat schema as shown below, and also supports the same keys nested under a top-level `controlnet_specs` table.

### Required fields

| Field                | Type    | Description |
|----------------------|---------|-------------|
| `prompt`             | string  | Text describing the desired scene |
| `input_video_path`   | string  | Path to the input video file |
| `negative_prompt`    | string  | Text describing what to avoid in the output |
| `num_steps`          | int     | Number of diffusion steps |
| `guidance`           | float   | CFG guidance scale |
| `sigma_max`          | float   | Partial noise added to input; [0, 80]. `>= 80` ignores the input video |
| `seed`               | int     | Random seed for reproducibility |

### Optional scalar fields

| Field              | Type   | Description |
|--------------------|--------|-------------|
| `blur_strength`    | string | Blur strength for preparing the `vis` control input. One of: `very_low`, `low`, `medium`, `high`, `very_high` |
| `canny_threshold`  | string | Optional threshold preset used when generating edges externally |

### Controls (all optional)

If present, controls must follow these rules:

| Control | Required keys                              | Notes |
|---------|--------------------------------------------|-------|
| `edge`  | `input_control` (string), `control_weight` (number) | Path typically points to an edges video |
| `depth` | `input_control` (string), `control_weight` (number) | Path points to a depth video |
| `seg`   | `input_control` (string), `control_weight` (number) | Path points to a semantic segmentation video |
| `vis`   | `control_weight` (number)                  | `input_control` is optional |

#### Validation constraints

The client validates before sending:

1. All required scalar fields above must be present and of the correct type.
2. Controls are optional. If `edge`, `depth`, or `seg` are provided, both `control_weight` and `input_control` must be present. If `vis` is provided, `control_weight` must be present and `input_control` is optional.

Example TOMLs are provided in `client/example_data/prompts/`.

### Command-line arguments

`cosmos_client.py` accepts the following arguments:

| Argument             | Type    | Description |
|----------------------|---------|-------------|
| `endpoint`           | string  | Base URL of the server (e.g., `http://localhost:8080`) |
| `config_toml`        | string  | Path to the TOML configuration |
| `--output`           | string  | File or directory path to save the result video |
| `--input-video`      | string  | Override `input_video_path` from the TOML |
| `--edge-video`       | string  | Override `edge.input_control` |
| `--depth-video`      | string  | Override `depth.input_control` |
| `--seg-video`        | string  | Override `seg.input_control` |
| `--vis-video`        | string  | Override `vis.input_control` |
| `--seed`             | int     | Override `seed` |
| `--retries`          | int     | Max retries for uploads and generation (default: 3) |
| `--backoff-initial`  | float   | Initial backoff seconds (default: 1.5) |
| `--backoff-multiplier` | float | Exponential backoff multiplier (default: 2.0) |
| `--jitter`           | float   | Random jitter added to backoff (default: 0.5) |
| `--poll-interval`    | int     | Poll interval in seconds for job status (default: 5) |
| `--result-timeout`   | int     | Timeout in seconds when fetching job results (default: 120) |
