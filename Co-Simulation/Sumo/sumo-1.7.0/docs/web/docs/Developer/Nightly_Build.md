---
title: Developer/Nightly Build
permalink: /Developer/Nightly_Build/
---

There are two main scripts responsible for the nightly build process
which are [{{SUMO}}/tools/build/dailyUpdateMakeGCC.sh]({{Source}}tools/build/dailyUpdateMakeGCC.sh) (for the linux part) and [{{SUMO}}/tools/build/dailyBuildMSVC.py]({{Source}}tools/build/dailyBuildMSVC.py) for Windows. Essentially they both
perform the following steps:

1.  git pull
2.  do a clean build (release and debug version)
3.  run unittests (currently linux only)
4.  build and upload packages
5.  build and upload documentation (linux only)
6.  run the acceptance tests (texttest)
7.  upload test results

All the results can be found in the [daily dir on the sumo web
space](https://sumo.dlr.de/daily/), also see
[Downloads\#Nightly_Snapshots](../Downloads.md#nightly_snapshots).
Coverage information based on running the nightly tests is generated as
well. The status can be found at [the sumo web
space](https://sumo.dlr.de/daily/lcov/html/).

There is also a local copy in the Sumo/daily directory (including
the most recent game scenarios) the DLR internal tsall NAS. The windows
script uses the sumo-all package from the local dir to build the binary
distributable zip and the game zip.

One final step is to trigger the nightly build on the opensuse build
service. This is achieved via uploading a new spec file via a cronjob,
which triggers an automatic download of the sources and a rebuild. All
errors and test result e-mails are directed to the sumo-tests list at
dlr.de, except for the build service errors which go to a private
account since the build is currently in a private project.

## Continuous integration

There are continuous integration builds on
[Travis](https://travis-ci.org/eclipse/sumo) and on
[AppVeyor](https://ci.appveyor.com/project/eclipsewebmaster/sumo).
Both are configured such that they cancel a running build when a 
new commit occurs, so we do not have build results for all commits
(but we avoid a large backlog this way).

To get build logs click on the marker beside the commit
message (red cross, yellow circle or green tick) and choose details
for the build in question.

The AppVeyor build also provides the binaries as download (currently
for about six months after the commit). To download choose details 
using the marker (as above) for the commit from 
https://github.com/eclipse/sumo/commits/master and then choose the 
Visual Studio version in question (when in doubt use the latest version)
and click on artifacts where a bin.zip should be available.

## Platforms and tests

- "sumo" refers to the vanilla Eclipse SUMO version without optional libraries
- "extra" means the variant with GDAL, OSG and FFMPEG
  running mesoscopic and python3 tests
- the clang build has debugging code enabled via the configure option
  "--enable-debug"
- all start times are local Berlin time

| variant | platform / compiler | start time    | estimated duration |
| ------- | ------------------- | ------------- | ------------------ |
| sumo    | Win32 msvc12        | 01:00         | 1h                 |
| sumo    | Win64 msvc12        | (after 32bit) | 4h                 |
| extra   | Win64 msvc12        | 07:00         | 2h                 |
| sumo    | ubuntu64 gcc        | 00:00         | 3h                 |
| extra   | ubuntu64 gcc        | 07:00         | 1h                 |
| sumo    | ubuntu64 clang      | 03:00         | 4h                 |
| extra   | ubuntu64 clang      | 08:00         | 1.5h               |
| sumo    | macOS clang         | 00:00         | 1h                 |

## Scenario tests

- "scenario" refers to the internal tests available at
source:trunk/tests and includes the default and the daily tests
  - "weekly" are the weekly test suites of the internal tests
    available at source:trunk/tests
  - all scenario tests run with the "extra" variant of sumo

| variant         | platform     | start time         | estimated duration |
| --------------- | ------------ | ------------------ | ------------------ |
| scenario        | 64 bit gcc   | 02:00              | 5h ?               |
| scenario weekly | 64 bit gcc   | 02:00 (only sa)    | 72h ?              |
| scenario        | 64 bit clang | 02:00 (only mo-fr) | 8h ?               |