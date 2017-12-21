CARLA Benchmark
===============

Running the Benchmark
-------------------------
The "carla" api provides a basic benchmarking system, that allows making
several tests on a certain agent. We already provide the same benchmark
used in the CoRL 2017 paper. By running this benchmark you can compare
the results of your agent to the results obtained by the agents
show in the paper. 

The basic functionality requires only the protobuf module to be installed
    $ sudo apt-get install python3 python3-pip
    $ sudo pip3 install protobuf
However, other operations as handling images require some extra modules, and the
"manual_control.py" example requires pygame
    $ sudo pip3 install numpy Pillow pygame
The script "PythonClient/client_example.py" provides basic functionality for
controlling the vehicle and saving images to disk. Run the help command to see
options available
    $ ./client_example.py --help
The script "PythonClient/manual_control.py" launches a PyGame window with
several views and allows to control the vehicle using the WASD keys.
    $ ./manual_control.py --help


