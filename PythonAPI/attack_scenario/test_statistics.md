# Test statistics


|                  |             | Static Attack | Dynamic Attack | Postprocess Attack |
|------------------|-------------|---------------|----------------|--------------------|
| Pretrained patch | Camera High | 3.1%          | 16.9%          | 28.2%              |
|                  | Camera Low  | 6.2%          | 17.9%          | 16.5%              |
| New patch        | Camera High | 7.9%          | 41.6%          | 46.6%              |
|                  | Camera Low  | 11.6%         | 39.8%          | 46.0%              |


New way of rendering patch
|                  |             | Static Attack |
|------------------|-------------|---------------|
| Pretrained patch | Camera High | 5.4%          |
| New patch        | Camera High | 13.6%         |


Double Attack:
|                  |             | Static Attack | Dynamic Attack | Postprocess Attack |
|------------------|-------------|---------------|----------------|--------------------|
| Pretrained patch | Camera High | 13.2%         | 27.1%          | 45.6%              |
| New patch        | Camera High | 19.2%         | 50.1%          | 50.3%              |

Increased size to 0.447m (from 0.4m)
|                  |             | Static Attack |
|------------------|-------------|---------------|
| New patch        | Single      | 19.5%         |
| New patch        | Double      | 25.7%         |

Spawning patch at beginning + increased size to 0.447m (from 0.4m)
|                  |             | Static Attack |
|------------------|-------------|---------------|
| New patch        | Single      | 21.7%         |
| New patch        | Double      | 28.5%         |

Spawning patch at beginning + increased size to 0.447m (from 0.4m) + new create folder script
|                  |             | Static Attack |
|------------------|-------------|---------------|
| New patch        | Single      | 22.0%         |
| New patch        | Double      | 28.5%         |

## Latest Results:
|                  |             | Static Attack | Dynamic Attack | Postprocess Attack |
|------------------|-------------|---------------|----------------|--------------------|
| New patch        | Single      | 22.0%         | 41.6%          | 46.6%              |
| New patch        | Double      | 28.5%         | 50.1%          | 50.3%              |


TODO:
- check performance of static without spawning at beginning but keeping everything else the same
- run tests with new script on dyndamic and post process attacks
- pipline for training "in simulation"
- presentation!!!!
