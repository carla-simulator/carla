# carla-cosmos-transfer

Welcome to the integration of CARLA and Cosmos Transfer1. 
In practice, this integration is based on 3 components:

1. A CARLA log playback script `client/carla_cosmos_gen.py`. This tool can read back a log (e.g. `client/example_data/iai_carla_synthetic_log_1731622446_actorPOV4641_startTime3.7s_log`) and generate the required artifacts to control Cosmos Transfer1 using the sensor rig defined by `client/cosmos_aov.yaml`. You can also play with the semantic labels used by modifying the file `client/filter_semantic_classes.yaml`.

2. A rest client `client/cosmos_client.py` that will send queries to a remote Cosmos Transfer1 service, based on your query (toml file). An example of this can be seen in `client/example_data/prompts/rain.toml`.

3. A docker image that includes Cosmos Transfer1 and a server enabled to answer queries. See the `server` directory to get familiar with the different components:
- Dockerfile.server: Creates the docker image with Cosmos Transfer1 and the rest of components. It expects to find a Cosmos Transfer1 installation + all these scripts in the root folder.
- supervisord.conf: Enables the Docker container to keep processes alive

## Getting started

First, you need to setup the server. Please refer to [server/README_SERVER.md](server/README_SERVER.md).

When the server is up and running, you can continue to setup the client. Please refer to [server/README_CLIENT.md](client/README_CLIENT.md).