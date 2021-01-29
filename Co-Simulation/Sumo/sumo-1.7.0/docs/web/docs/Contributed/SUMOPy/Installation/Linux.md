---
title: Contributed/SUMOPy/Installation/Linux
permalink: /Contributed/SUMOPy/Installation/Linux/
---

Python 2.7 comes with most Linux operating systems. Python 3.x is not
yet supported. All required additional packages are available in
repositories:

```
python-numpy
python-wxgtk2.8
python-opengl
python-imaging
python-matplotlib
python-mpltoolkits.basemap
```

However, often Python 3.x is installed along the older version and may
be the default Python interpreter. So make sure you run the sumopy
scripts with Python 2.7

Another issue may that a newer version of `python-wxgtk2.8` has been
included in the repositories of more recent Linux distributions, as for
example Ubuntu-16.04. In this case, do the following safe operation to
install `python-wxgtk2.8`:

```sh
echo "deb http://archive.ubuntu.com/ubuntu wily main universe"| sudo tee /etc/apt/sources.list.d/wily-copies.list

sudo apt install python-wxgtk2.8

sudo rm /etc/apt/sources.list.d/wily-copies.list

sudo apt update
```