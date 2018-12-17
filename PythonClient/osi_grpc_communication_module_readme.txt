Description:

1. The module (osi_grpc_communication_module.py) provides a standardized interface to connect CARLA with third party tools.

2. Module uses the gRPC (google remote procedure call) to send the groundtruth measurement data of the host_vehicle and the non_player_agents (vehicles) with OSI standardization (Open Simulation Interface) at every frame in the episode.

3. All the proto_to_python generated files are present with the module. they can be readily used for the compilation and working. These are in the folder named "osi3". For further manipulation they can be downloaded from: https://github.com/OpenSimulationInterface

4. osi_grpc.proto is present in "proto" folder and is used to generate the .py version of the request with protoc and a special gRPC plugin as per the instructions here: https://grpc.io/docs/quickstart/python.html

5. The generated .py files from the osi_grpc.proto are present in the osi_grpc folder. Changes made in osi_grpc.proto would require regeneration of .py files to be effective in module.

6. test_osi_grpc_server.py is to readily receive the information from carla server and is an example to show the output of the module.

7. The required Licences are provided.