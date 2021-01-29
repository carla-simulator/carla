---
title: Contributed/SUMOPy
permalink: /Contributed/SUMOPy/
---

This document describes the capabilities and basic usage of the software
SUMOPy. SUMOPy is intended to expand the user-base of the traffic
micro-simulator SUMO by providing a user-friendly, yet flexible
simulation suite. The original publication related to SUMOPy can be
found at the [University of Bologna](http://campus.unibo.it/200538/1/Research-A-Choudhry-657079-.pdf)
and in the [proceedings of the SUMO2013](http://sumo.dlr.de/2013/SUMO2013_15-17May%202013_Berlin-Adlershof.pdf).

A further scope of SUMOPy is to manage the huge amount of data necessary
to run complex multi-modal simulations. This includes different demand
generation methods such as support for OD matrices, trurnflows and a
syntetic (or virtual population. Also different services such as
Personal Rapid Transit (PRT) or self-driving taxis are supported.

Essentially, SUMOPy consists of a GUI interface, network editor as well
as a simple to use scripting language which facilitates the use of SUMO.

# Introduction

SUMO rapidly developed into a flexible and powerful open-source
micro-simulator for multi-modal urban traffic networks . The features
and the number of tools provided are constantly increasing, making
simulations ever more realistic. However, the different functionalities
consist at the present state of a large number of binaries and scripts
that act upon a large number of files, containing information on the
network, the vehicles, districts, trips routes, configurations, and many
other parameters. Scripts (mostly written in Python), binaries and data
files exist in a dispersed manner. In practice, a master script is
necessary to hold all processes and data together in order run a
simulation of a specific scenario in a controlled way. This approach is
extremely flexible, but it can become very time consuming and error
prone to find the various tools, combine their input and output and
generate the various configuration files. Furthermore, it reduces the
user-base of SUMO to those familiar with scripting and command line
interfaces. Instead, SUMO has the potential to become a
multi-disciplinary simulation platform if it becomes more accessible to
disciplines and competences.Scripts (mostly written in Python), binaries
and data files exist in a dispersed manner.

This problem has been recognized and different graphical user interfaces
have been developed. The *traffic modeller* (also named *traffic
generator*) is a tool written in Java which helps to manage files, to
configure simulations and to evaluate and visualize results.

*SUMOPy* is written entirely in the object-oriented script language
*Python*, it uses *wxWindows* with *PyOPENGL* as GUI interface and
*NumPy* for fast numerical array-type calculations. It is similar to the
*traffic generator* in that it simplifies the use of SUMO through a GUI.
But SUMOPy is more than just a GUI, it is a *suite* that allows to
access SUMO tools and binaries in a simple unified fashion. The
distinguishing features are:

- SUMOPy has Python instances that can make direct use of tools
  already available as Python code.
- SUMOPy has a Python command line interface that allows direct and
  interactive manipulation of SUMOPy instances.
- SUMOPy provides a library that greatly simplifies the scripting.

# Installation

SUMOPy is a directory with python scripts. It is sufficient to unzip the
latest version and copy it in a directory of your choice. Since
SUMO-0.28, SUMOPy is inside the SUMO distribution and located in
`SUMOHOME/tools/contributed`.

However, SUMOPy makes extensive use of Python packages which need to be
installed before. The *required* packages to be installed are:

1.  Python 2.7
2.  numpy-1.10 or newer
3.  wxPython2.8 or wxPython2.9 (wxPython3.x is currently not properly
    working with PyOpenGL-3.0.x)
4.  PyOpenGL-3.0.x

The following packages are optional:

1.  matplotlib-1.4 or newer, for high quality graphical output in
    different file formats.
2.  basemap or pyproj for geographic coordinate projection
3.  PIL-1.1.7 or newer and basemap-1.0 (or pyproj) for downloading
    background maps from mapservers.

The exact choice of package-versions and installation methods depend on
the operating system. Below we give short recommendations regarding the
choice of packages for different operating systems. In general, the
32-bit version is preferred as there are more pre-compiled packages
available, but this may change over time.

- [Installation requirements for Windows](../Contributed/SUMOPy/Installation/Windows.md)
- [Installation requirements for Linux](../Contributed/SUMOPy/Installation/Linux.md)

# The graphical user interface

The graphical user interface of SUMOPY cane be launched by running the
script sumopy_gui.py, which is located in SUMOPy's main directory. The
following documents describe the basic functioning of the GUI.

- [Getting started\!](../Contributed/SUMOPy/GUI/Getting_Started.md)
- [Demand modelling with GUI](../Contributed/SUMOPy/GUI/Demand_Modelling.md)
  - [Zone to zone demand flows](../Contributed/SUMOPy/Demand/Zone_To_Zone.md)
  - [Turn flows](../Contributed/SUMOPy/Demand/Turn_Flows.md)

# Simulation processes

This section addresses the different simulation methods.

- [Simulating Sublanes](../Contributed/SUMOPy/Simulation/Sublanes.md)

# Tutorials

[Opening and simulating a scenario](../Contributed/SUMOPy/Tutorials/Open_and_Simulate.md)