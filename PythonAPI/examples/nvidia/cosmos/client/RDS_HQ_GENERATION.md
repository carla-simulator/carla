# RDS-HQ Data Generation

Generate RDS-HQ format data from CARLA recordings for NVIDIA Cosmos-Drive HDMap rendering.

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

Generates RDS-HQ format data that can be used with the NVIDIA Cosmos-Drive toolkit to render HDMap videos. The output includes camera poses, intrinsics, 3D road elements (lane lines, boundaries, crosswalks, signs), and dynamic objects (vehicles with bounding boxes).

A dataset configuration file is automatically created as `dataset_{session_id}.json` where `{session_id}` follows the format `{logname}_{start_time_us}_{end_time_us}`. For example: `my_recording_0_10000000.json` for a 10-second clip.

## Rendering HDMap Videos

```bash
# Copy config to renderer
cp /output/directory/dataset_{session_id}.json \
   /path/to/cosmos-drive-dreams-toolkits/config/

# Render HDMap videos
cd /path/to/cosmos-drive-dreams-toolkits
python render_from_rds_hq.py \
  -i /output/directory \
  -o /videos/output \
  -d {session_id} \
  -c pinhole
```
