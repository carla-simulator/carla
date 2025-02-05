# C++ Reference 

## C++ client

The C++ client can be built with `make` on Linux and `cmake` in Windows. An C++ client example is provided in the repository in `CARLA_ROOT/Examples/CppClient/main.cpp`. This example shows how to connect the C++ client to the CARLA server and use the API for some simple tasks. 

To build the example C++ client, open a terminal in the `CARLA_ROOT/Examples/CppClient` directory in the repository. Run `make` in this folder and then execute `./bin/cpp_client` to run the example. The example will choose a random map from those available then load it. It will then spawn a vehicle and apply a control to the vehicle. 

Please see the [C++ client example](adv_cpp_client.md) for more details on this example script.

## C++ documentation

We use Doxygen to generate the documentation of our C++ code:

[Libcarla/Source](http://carla.org/Doxygen/html/dir_b9166249188ce33115fd7d5eed1849f2.html)<br>
[Unreal/CarlaUE4/Source](http://carla.org/Doxygen/html/dir_733e9da672a36443d0957f83d26e7dbf.html)<br>
[Unreal/CarlaUE4/Carla/Plugins](http://carla.org/Doxygen/html/dir_8fc34afb5f07a67966c78bf5319f94ae.html)

The generated documentation is available at this link **<http://carla.org/Doxygen/html/index.html>**

!!! note
    Document updates are done automatically by GitHub.

### Create doxygen documentation

!!! important
    [Doxygen](http://www.doxygen.nl/index.html) is required to generate the documentation
    and [Graphviz](https://www.graphviz.org/) for the graph drawing toolkit.

1- Install doxygen and graphviz with the following command:

```sh
# linux
> sudo apt-get install doxygen graphviz
```

2- Once installed, go to the project root folder where the _Doxyfile_ file is situated and
run the following command:

```sh
> doxygen
```

It will start to build the documentation webpage.  
The resulting webpage can be found at Doxygen/html/

3- Open _index.html_ in a browser. Now you have your local cpp documentation!
