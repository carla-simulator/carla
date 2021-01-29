---
title: Versioning
permalink: /Versioning/
---

# SUMO version numbers and their meaning

SUMO has version numbers in many places, namely

- the release version (visible in releases when calling `sumo -V` or in the about dialog of the GUI applications), something like 1.6.0
- the developer version (visible in the same places for a nightly build or a self compiled developer version), something like v1_6_0+1520-cb474809f6
- the network version (visible in the the beginning of every .net.xml as `<net version="1.0" ...`)
- the TraCI version returned by TraCI's or libsumo's [getVersion](TraCI/Control-related_commands.md) command

## Release version

SUMO loosely follows [Semantic Versioning](https://semver.org) which distinguishes major releases (the first number changes),
minor releases (the second number changes) and patch or bugfix releases (third number).
We release approximately three or four times a year and most of the time we ship a minor release. 
The amount of changes is relatively high (1000+ commits between successive releases)
so there is always some additional feature justifying a new minor version.
Due to the relatively high frequency and the availability of nightly builds we only ship bugfix releases
if the minor release has a severe bug.
Major releases are very rare (we only did this once since 2001) and are more or less a political decision.

We do not release service versions (e.g. a 1.5.1 when a 1.6.0 is already out) or maintain separate release tracks or branches.
There is only one line of development.

## Developer version

It is denoted by a string consisting of a "v", the last release (e.g. "1_6_0"), a plus sign with the number of 
commits since last release (e.g. "+1520") and a minus followed by the first letters of the hash of the last commit to 
the master branch on github (e.g. "-cb474809f6"). For the python packages this is translated into "1.6.0.post1520" to be compliant
with PEP440.

## Network version

This is also a "major.minor" version number simply denoting the SUMO release when this format has been introduced. The [netconvert](netconvert.md)
tool can usually parse older networks as well (at least back to when the versioning scheme in networks has been introduced with 0.25.0 in 2015)
and translate them to the current format.

## TraCI version

Please see the [description of the relevant command](TraCI/Control-related_commands.md).

# Backwards compatibility
## Input
The main input files like the network and the routes are quite stable and we usually only add new elements or attributes there.

## Processing
When introducing new features or changing behavior we usually try to provide options to get the old behavior back
unless we consider the old variant a bug. Since all the models are still under very dynamic development
(and the main development is for research purposes) it may be hard to get identical behavior in complex 
traffic situations with very different SUMO versions. If you ship a scenario to someone it is always a good idea
to mention the SUMO version it is intended to be run with. The version is also mentioned in all output files.

## Output
Output formats are also quite stable but are of course subject to changes in vehicle behavior and general bug fixing.

## GUI
This gets the least attention concerning stability of user experience although we always try not to confuse the user (honestly!).
