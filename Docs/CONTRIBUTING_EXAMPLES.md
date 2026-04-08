# Contributing Examples

## Good Commit Messages

```
feat: Add new sensor type

- Added RGB-D camera sensor
- Implemented depth map generation
- Added unit tests

Closes #123
```

## Code Style

### Python
```python
def calculate_route(start: carla.Location, end: carla.Location) -> list[carla.Waypoint]:
    """Calculate route between two locations.
    
    Args:
        start: Starting location
        end: Destination location
    
    Returns:
        List of waypoints forming the route
    """
    ...
```

### C++
```cpp
/// @brief Calculate distance between two points
/// @param a First point
/// @param b Second point
/// @return Euclidean distance
float CalculateDistance(const FVector& a, const FVector& b);
```

## Testing

Always include tests for new features:
```python
def test_route_calculation():
    """Test route calculation between two points."""
    route = calculate_route(start, end)
    assert len(route) > 0
```
