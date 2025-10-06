# RDS-HQ Data Generation

Generate RDS-HQ format data from CARLA recordings for NVIDIA Cosmos-Drive HDMap rendering.

## Setup

Before first use, initialize the Cosmos-Drive-Dreams toolkit submodule:

```bash
git submodule update --init --recursive
```

This will download the NVIDIA Cosmos-Drive-Dreams toolkit (pinned to commit `ada3a74f`) which is used for HD map video rendering.

## Usage

```bash
python carla_cosmos_gen.py \
  -f /path/to/recording.log \
  --sensors cosmos_aov.yaml \
  -c <ego_vehicle_id> \
  -o /output/directory
```

**Required Parameters:**
- `-f`: CARLA recorder log file path
- `--sensors`: Sensor configuration YAML (must include `rds_hq` sensor)
- `-c`: Ego vehicle actor ID
- `-o`: Output directory

**Optional Parameters:**
- `--skip-render-hdmap`: Skip automatic HD map video rendering (export RDS-HQ data only)

## Sensor Configuration

Add `rds_hq` sensor to `cosmos_aov.yaml`:

```yaml
- sensor: rds_hq
  attributes:
    image_size_x: 1920
    image_size_y: 1080
    fov: 110
  transform:
    location: {x: 0.0, y: 0.0, z: 1.8}
    rotation: {pitch: 8, yaw: 0, roll: 0}
```

## Output

Generates RDS-HQ format data in `/output/directory/rds-hq/` including:
- Camera poses and intrinsics
- 3D road elements (lane lines, boundaries, crosswalks, signs, road markings)
- Dynamic objects (vehicles and pedestrians with bounding boxes)
- Dataset configuration file: `dataset_{session_id}.json`

**By default**, HD map videos are automatically rendered to `/output/directory/hdmap/pinhole_rds_hq/` using the integrated Cosmos-Drive-Dreams toolkit.

Where `{session_id}` follows the format `{logname}_{start_time_us}_{end_time_us}`. For example: `my_recording_0_10000000` for a 10-second clip starting at t=0.

## Workflow Options

### Automatic Rendering (Default)

The script automatically exports RDS-HQ data and renders HD map videos:

```bash
python carla_cosmos_gen.py \
  -f recording.log \
  --sensors cosmos_aov.yaml \
  -c 123 \
  -o /output/directory
```

Output structure:
```
/output/directory/
├── rds-hq/                    # RDS-HQ format data
│   ├── 3d_lanelines/
│   ├── 3d_road_boundaries/
│   ├── 3d_crosswalks/
│   ├── 3d_road_markings/
│   ├── pose/
│   ├── pinhole_intrinsic/
│   ├── all_object_info/
│   └── dataset_{session_id}.json
├── hdmap/                     # Rendered HD map videos
│   └── pinhole_rds_hq/
│       └── {session_id}_0.mp4
├── rgb.mp4                    # Other AOV videos
├── depth.mp4
├── semantic_segmentation.mp4
└── ...
```

### Export Only (Skip Rendering)

To only export RDS-HQ data without rendering:

```bash
python carla_cosmos_gen.py \
  -f recording.log \
  --sensors cosmos_aov.yaml \
  -c 123 \
  -o /output/directory \
  --skip-render-hdmap
```

This is useful if you want to render manually later or use custom rendering settings.
