
<h1>C++ Reference </h1>
We use Doxygen to generate the documentation of our C++ code:

[Libcarla/Source](https://carla.org/Doxygen/html/dir_b9166249188ce33115fd7d5eed1849f2.html)<br>
[Unreal/CarlaUE4/Source](https://carla.org/Doxygen/html/dir_733e9da672a36443d0957f83d26e7dbf.html)<br>
[Unreal/CarlaUE4/Carla/Plugins](https://carla.org/Doxygen/html/dir_8fc34afb5f07a67966c78bf5319f94ae.html)

The documentation generated is available at this link **<https://carla.org/Doxygen/html/index.html>**

!!! note
    The documentation updates automatically with the github repository,
    so it will be always updated.

### Create doxygen documentation

!!! important
    [Doxygen](http://www.doxygen.nl/index.html) is required to generate the documentation
    and [Graphviz](https://www.graphviz.org/) for the graph drawing toolkit.

```sh
# linux
> sudo apt-get install doxygen graphviz
```

Once installed, go to the project root folder where the _Doxyfile_ file is situated and
run the following command:

```sh
> doxygen
```

It will start to build the documentation webpage. Then go to Doxygen/html/ and open _index.html_ in your browser.
