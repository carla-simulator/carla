
# Building Carla in a Docker

Make sure you have installed Python 3.6 or newer, is in path and is called python3.

Run:

```
sudo pip3 install ue4-docker
sudo ue4-docker setup
```

Then use the following commands, each one will take a long time:

```
ue4-docker build 4.22.2 --no-engine --no-minimal
sudo docker build -t carla-prerequisites -f Prerequisites.Dockerfile .
sudo docker build -t carla -f Carla.Dockerfile .
```
