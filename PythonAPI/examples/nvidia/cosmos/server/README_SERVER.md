
# Getting started with CARLA-Cosmos-Transfer Server

This readme provides comprehensive instructions for setting up a server for Cosmos-Transfer1. We use a Docker image that includes Cosmos-Transfer1 and a server enabled to handle queries. Refer to the `server` directory to understand the different components:
     - `Dockerfile.server`: Creates the Docker image with Cosmos-Transfer1 and the other components. It expects to find a Cosmos-Transfer1 installation and all required scripts in the root folder.
     - `supervisord.conf`: Keeps processes alive within the Docker container.

## Setup Python Environment

1. Install `conda`, see [instructions](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html). You can skip this step if conda is already installed on your system.

2. Build the docker image. Go for a long coffee; this is going to take 1-2 hours. You will have to provide your HuggingFace token to download the different checkpoints.
```bash
cd server
./make_docker.sh --download-checkpoints
```

3. Deploy your docker image in your favorite environment. We recommend a cluster with at least 8 x H100 GPUs, althought 1 x H100 GPU should be enough.

```bash
docker run -d --shm-size 96g --gpus=all --ipc=host -p 8080:8080 cosmos-transfer1-carla

```

4. Refer to client/README_CLIENT.md on how to send requests to the server.

**Note**: You can build the docker image in dev mode. In dev mode, the checkpoints are not copied into the Docker image and need to be mounted manually when starting a new container:

```bash
cd server
./make_docker.sh --dev
docker run -d -v /full/path/to/carla-cosmos-transfer/server/repos/cosmos-transfer1/checkpoints:/workspace/checkpoints --shm-size 96g --gpus=all --ipc=host -p 8080:8080 cosmos-transfer1-carla
```