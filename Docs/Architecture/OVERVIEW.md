# CARLA Architecture Overview

## Components

### Core Components
- **CARLA Server**: Main simulation server
- **Python API**: Python bindings for CARLA
- **UE4/UE5 Plugin**: Unreal Engine integration

### Python API Structure
```
PythonAPI/
├── carla/              # Core CARLA module
├── agents/             # Navigation agents
│   ├── navigation/     # Route planners, controllers
│   └── tools/          # Helper utilities
├── examples/           # Example scripts
├── test/               # Test suite
└── util/               # Utility scripts
```

## Communication
- Client-server architecture
- TCP/IP communication
- MessagePack serialization

## Threading
- Main simulation thread
- Async sensor data handling
- Thread-safe API calls
