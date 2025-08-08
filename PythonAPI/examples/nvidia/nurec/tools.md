# NUREC Integration Tools

This document describes the utility tools available in the `tools/` directory for the CARLA-NUREC integration.

## Extract Blueprint Sizes Tool

The `extract_blueprint_sizes.py` tool is used to measure the dimensions of all vehicle and walker blueprints available in CARLA. These dimensions are necessary for properly sizing actors when replaying NUREC scenarios in CARLA.

### Purpose

When replaying NUREC scenarios, we need to match real-world object dimensions with CARLA actor dimensions. This tool:

1. Connects to a running CARLA server
2. Spawns each available vehicle and walker blueprint
3. Measures their bounding box dimensions (width, length, height)
4. Saves the results to JSON files that can be used by the NUREC integration

### Usage

```bash
python tools/extract_blueprint_sizes.py [options]
```

#### Prerequisites:

- A running CARLA server
- Sufficient spawn points in the current map for all blueprints

#### Command Line Options:

| Parameter | Long Form | Default | Description |
|-----------|-----------|---------|-------------|
| --host | | 127.0.0.1 | IP address of the CARLA host server |
| -p | --port | 2000 | TCP port for the CARLA server |
| -s | --start | 0.0 | Starting time for measurement |
| -d | --duration | 200.0 | Duration for measurement |
| -f | --recorder-filename | test1.log | Recorder filename |
| -c | --camera | 0 | Camera follows an actor (e.g., 82) |
| -x | --time-factor | 0.2 | Time factor (default 1.0) |
| -i | --ignore-hero | | Ignore hero vehicles |
| --move-spectator | | Move spectator camera |
| --spawn-sensors | | Spawn sensors in the replayed world |

### Output

The tool generates two JSON files:

1. `blueprint_sizes_vehicle.json`: Contains dimensions for all vehicle blueprints
2. `blueprint_sizes_walker.json`: Contains dimensions for all walker blueprints

Each file contains a list of pairs: (blueprint_id, [x_extent, y_extent, z_extent]), where:
- x_extent: Half-length of the actor along its local X-axis
- y_extent: Half-width of the actor along its local Y-axis
- z_extent: Half-height of the actor along its local Z-axis

### Example

```bash
# Connect to a local CARLA server and extract blueprint dimensions
python tools/extract_blueprint_sizes.py --host 127.0.0.1 --port 2000
```

### Notes

- The tool may take several minutes to run as it needs to spawn and measure each blueprint individually
- Make sure to use a map with sufficient spawn points (Town10HD is recommended)
- The tool automatically filters out invalid measurements (infinity or NaN values)
- The generated JSON files should be placed in the NUREC integration directory for use with the replay scripts 