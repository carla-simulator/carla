# CARLA Performance Benchmarks

## Running Benchmarks

```bash
# Run all benchmarks
python3 -m pytest Benchmarks/ -v

# Run specific benchmark
python3 -m pytest Benchmarks/test_navigation.py -v
```

## Metrics

- Navigation path planning time
- Collision detection latency
- Sensor data processing rate
- Memory usage per agent

## Results

Results are stored in `Benchmarks/results/` directory.
