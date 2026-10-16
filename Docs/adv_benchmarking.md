# Benchmarking Performance

We provide a benchmarking script to enable users to easily analyze the performance of CARLA in their own environment. The script can be configured to run a number of scenarios that combine different maps, sensors and weather conditions. It reports the average and standard deviation of FPS under the requested scenarios. 

In this section we detail the requirements to run the benchmark, where to find the script, the flags available to customize the scenarios that are run and examples on how to run the commands.

We have also included our results of a separate benchmark which measures performance in CARLA in a specific environment when using different combinations of number of vehicles, enabling physics and/or enabling Traffic Manager. The results are presented alongside the CARLA version used and the environment the test was performed in.

- [__The benchmark script__](#the-benchmark-script)
    - [__Before you begin__](#before-you-begin)
    - [__Synopsis__](#synopsis)
        - [__Flags__](#flags)
- [__CARLA performance report__](#carla-performance-report)


---
## The benchmark script

The benchmark script can be found in `PythonAPI/util`. It has several flags available to customize the scenarios to be tested which are detailed in the synopsis below.


### Before you begin

The benchmarking script requires some dependencies to be installed before you can run it:

```python
python -m pip install -U py-cpuinfo==5.0.0 psutil python-tr gpuinfo GPUtil
```

### Synopsis

`python3` [`performance_benchmark.py`](https://github.com/carla-simulator/carla/blob/master/PythonAPI/util/performance_benchmark.py) [`[--host HOST]`](#-host-ip_address) [`[--port PORT]`](#-port-port) [`[--file FILE]`](#-file-filenamemd) [`[--tm]`](#-tm)
[`[--ticks TICKS]`](#-ticks) [`[--sync]`](#-sync) [`[--async]`](#-async))
[`[--fixed_dt FIXED_DT]`](#-fixed_dt) [`[--render_mode]`](#-render_mode))
[`[--no_render_mode]`](#-no_render_mode) [`[--show_scenarios]`](#-show_scenarios))
[`[--sensors SENSORS [SENSORS ...]]`](#-sensors-integer))
[`[--maps MAPS [MAPS ...]]`](#-maps-townname))
[`[--weather WEATHER [WEATHER ...]]`](#-weather-integer)



#### Flags

* `--host`: __IP_ADDRESS__
__Default__: Localhost.  
Configures the host of the server.

* `--port`: __PORT__  
__Default__: 2000  
Configures the TCP port to listen to.

* `--file`: __filename.md__
__Default__: benchmark.md  
Writes results in markdown table format to a file.

* `--tm`
Switch to Traffic Manager benchmark

* `--ticks`
__Default__: 100  
Sets the number of ticks to use for each scenario.

* `--sync`
__Default mode.__  
Runs benchmark in synchronous mode. 

* `--async`
Runs benchmark in asynchronous mode.

* `--fixed_dt`
__Default__: 0.05  
For use with synchronous mode if you would like to set the delta timestep.

* `--render_mode`
Runs benchmark in rendering mode.

* `--no_render_mode`
__Default mode.__  
Runs benchmark in non-rendering mode.

* `--show_scenarios`
When the script is run with only this flag you will see a list of all the scenario parameters available. When combined with other flags you will see a preview of the scenarios that will be run without actually executing them.

* `--sensors`: INTEGER
>> __Default__: All

>> Sensors to be used in the benchmark. Chose between LIDAR and RGB camera:

>> * __`0`__: cam-300x200
>> * __`1`__: cam-800x600
>> * __`2`__: cam-1900x1080
>> * __`3`__: cam-300x200 cam-300x200 (two cameras)
>> * __`4`__: LIDAR: 100k
>> * __`5`__: LIDAR: 500k
>> * __`6`__: LIDAR: 1M


* `--maps`: __TownName__
__Default__: All maps  
All [CARLA maps][carla_maps], both layered and sub-layered, are available.

[carla_maps]: https://carla.readthedocs.io/en/latest/core_map/#carla-maps

* `--weather`: __INTEGER__
__Default__: All weather conditions
Change the weather conditions:
* __`0`__: ClearNoon
* __`1`__: CloudyNoon
* __`2`__: SoftRainSunset

## How to run the benchmark

1. Start CARLA:

        # Linux:
        ./CarlaUnreal.sh
        # Windows:
        CarlaUnreal.exe
        # Source:
        cmake --build Build --target launch


2. In a separate terminal navigate to `PythonAPI/util` to find the `performance_benchmark.py` script:

>> * Show all possible scenarios without running them:
```shell
python3 performance_benchmark.py --show_scenarios
```

>> * Show what scenarios will run when configurations are applied without actually executing them:
```shell
python3 performance_benchmark.py --sensors 2 5 --maps Town03 Town05 --weather 0 1 --show_scenarios`
```

>> * Execute the performance benchmark for those scenarios:
```shell
python3 performance_benchmark.py --sensors 2 5 --maps Town03 Town05 --weather 0 1
```

>> * Perform the benchmark for asynchronous mode and rendering mode:
```shell
python3 performance_benchmark.py --async --render_mode
```

---

<div class="build-buttons">
<!-- Latest release button -->
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</p>
</div>