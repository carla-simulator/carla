# NVIDIA Neural Reconstruction Engine gRPC Protocol Buffers

This package contains the protocol buffer definitions and Python generation tools for the NVIDIA Neural Reconstruction Engine (NuRec) gRPC API.

## Contents

- `protos/`: Protocol buffer definitions (.proto files)
  - `common.proto`: Common data types and structures
  - `sensorsim.proto`: Sensor simulation service definitions
- `update_generated.py`: Python script to generate Python code from proto files
- `requirements.txt`: Required Python dependencies

## Quick Start

### 1. Install Dependencies

Using pip:
```bash
pip install -r requirements.txt
```

### 2. Generate Python Code

Run the generation script to create Python files from the proto definitions:

```bash
python PythonAPI/examples/nvidia/grpc/update_generated.py
```

This will generate the following Python files in `PythonAPI/examples/nvidia/grpc/protos/`:
- `common_pb2.py` and `common_pb2.pyi`: Python code for common.proto
- `sensorsim_pb2.py` and `sensorsim_pb2.pyi`: Python code for sensorsim.proto  
- `common_pb2_grpc.py` and `sensorsim_pb2_grpc.py`: gRPC service code

### 3. Use in Your Code

After generating the Python files, you can import and use them in your Python code:

```python
# Import the generated modules
import nre.grpc.protos.common_pb2 as common_pb2
import nre.grpc.protos.sensorsim_pb2 as sensorsim_pb2
import nre.grpc.protos.sensorsim_pb2_grpc as sensorsim_pb2_grpc

# Create a gRPC channel and stub
import grpc
channel = grpc.insecure_channel('localhost:50051')
stub = sensorsim_pb2_grpc.SensorsimServiceStub(channel)

# Use the service
request = sensorsim_pb2.RGBRenderRequest()
# ... configure your request
response = stub.render_rgb(request)
```

## Service Overview

The NuRec gRPC API provides the following main services:

### SensorsimService

- `render_rgb`: Render RGB images from a scene
- `render_lidar`: Render lidar point clouds  
- `get_version`: Get version information
- `get_available_scenes`: List available scenes
- `get_available_cameras`: List available cameras for a scene
- `get_available_trajectories`: List available trajectories for a scene
- `get_available_ego_masks`: List available ego masks
- `shut_down`: Gracefully shutdown the service

## Development

When modifying the .proto files, always regenerate the Python code:

```bash
python PythonAPI/examples/nvidia/nre/grpc/update_generated.py
```

## Support

For questions about this API, please contact NVIDIA support or refer to the official documentation.

## License

Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
