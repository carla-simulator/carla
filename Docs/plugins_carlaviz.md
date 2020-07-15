# carlaviz

CARLA integrates 

*	[__General information__](#general-information)  
	*	[Description](#description)  
	*	[Support](#support)  
*	[__Get carlaviz__](#get-carlaviz)  
	*	[Prerequisites](#prerequisites)  
	*	[Download the plugin](#download-the-plugin)  
*	[__Run carlaviz__](#run-carlaviz)  
	*	[Examples](#examples)  
	*	[Utilities](#utilities)  

---
## Basic info

*   __Contributor__ — Minjun Xu, also known as [wx9698](https://github.com/wx9698).  
*   __License__ — [MIT](https://en.wikipedia.org/wiki/MIT_License).  
> *Check license??*

### Description

Visualize a simulation in a web browser, hear to sensor data and draw information updates.

### Support

*   __Linux__ — CARLA 0.9.6, 0.9.7, 0.9.8, 0.9.9.  
*   __Windows__ — CARLA 0.9.9.  
*   __Build from source__ — Latest updates.  

---
## Get carlaviz

### Prerequisites

*   __Docker__ — Visit the docs and [install Docker](https://docs.docker.com/get-docker/).  
*   __Operative system__ — Any OS able to run CARLA should work.  
*   __Websocket-client__ — ```pip3 install websocket_client```. Install [pip](https://pip.pypa.io/en/stable/installing/) if it is not already in the system.  

### Download the plugin

Open a terminal and pull the Docker image of carlaviz, based on the CARLA version to be run.  

```bash
# Pull only the image that matches the CARLA package being used
docker pull mjxu96/carlaviz:0.9.6
docker pull mjxu96/carlaviz:0.9.7
docker pull mjxu96/carlaviz:0.9.8
docker pull mjxu96/carlaviz:0.9.9

# Pull this image if working on a CARLA build from source
docker pull carlasim/carlaviz:latest
```

!!! Note
    Currently in Windows there is only support for 0.9.9.  

CARLA up to 0.9.9 (included) is set to be single-stream. For later versions, multi-streaming for sensors is implemented.  
* __In single-stream__, a sensor can only be heard by one client. When a sensor is already being heard by another client, for example when running `manual_control.py`, the *carlaviz* plugin is forced to duplicate the sensor in order to 
* __In multi-stream__, a sensor can be heard by multiple clients, so there is no need to duplicate these and performance does not suffer.  

!!! Note
    Alternatively on Linux, users can build carlaviz following the instructions [here](https://github.com/carla-simulator/carlaviz/blob/master/docs/build.md), but using a Docker image will make things much easier.  

---
## Run carlaviz

__1. Run CARLA.__

*   __a) In a CARLA package__ — Go to the CARLA folder and start the simulation with `CarlaUE4.exe` (Windows) or `./CarlaUE4.sh` (Linux).  

*   __b) In a build from source__ — Go to the CARLA folder, run the UE editor with `make launch` and press `Play`.  

__2. Run carlaviz.__ In another terminal run the following command according to the Docker image that has been downloaded.  

Change `<name_of_Docker_image>` for the name of the image previously downloaded, e.g. `carlasim/carlaviz:latest` or `mjxu96/carlaviz:0.9.9`.  

```sh
docker run -it --network="host" -e CARLAVIZ_HOST_IP=localhost -e CARLA_SERVER_IP=localhost -e CARLA_SERVER_PORT=2000 <name_of_Docker_image>
```

If the everything has been properly set, carlaviz will show a successful message similar to the following.  

![carlaviz_run](img/plugins_carlaviz_run.jpg)

!!! Warning
    Remember to edit the previous command to match the Docker image being used.  


__3. Open the localhost__ Open your web browser and go to `http://127.0.0.1:8080/`. carlaviz runs by default in port `8080`. 

> *Visualization??* 

### Examples

The RSS library 

### Utilities

The RSS library 


!!! Note
    In an automated vehicle controller it might be possible to adapt the planned trajectory to the restrictions. A fast control loop (>1KHz) can be used to ensure these are followed.

---

That sets the basics regarding the RSS sensor in CARLA. Find out more about the specific attributes and parameters in the [sensor reference](ref_sensors.md#rss-sensor). 

Open CARLA and mess around for a while. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
