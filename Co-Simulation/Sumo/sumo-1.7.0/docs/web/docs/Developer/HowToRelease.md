---
title: Developer/HowToRelease
permalink: /Developer/HowToRelease/
---

## Packages

For an overview of created packages and contents see
[Downloads](../Downloads.md).

## Release steps

Below, a list of steps that should be done in order to publish a new
release is given. All necessary commits which have no ticket of their
own may refer to #563.

### Merge phase

Major changes to the SUMO trunk should end about two weeks before the
release date. This refers especially to merges of project branches into
the trunk. It is also a good idea to inform the developers of dependent
software (Veins, VSimRTI, flow etc.) at this stage.
- send out mail to sumo-dev informing about the upcoming release so that 'contributed'-authors can check their stuff for compatibility

### Freeze phase (Release day - 7)

- check the sources
  - compile, try to remove warnings and commit the patches
  - run [checkStyle](../Developer/CodeStyle.md) and commit
    changed files
  - check the calendar to update copyright statements
  - check whether the TraCI version needs to be incremented
    and rebuild TraCI constants in python
    (tools/traci/rebuildConstants.py)
  - check whether the network version needs to be incremented and
    update the value in NWFrame::MAJOR_VERSION. netconvert tests
    need to be updated afterwards.
  - update author information
- check the regular tests
  - put special attention to the tests which serve as examples, see
    tests/examples.txt\!
  - Win64 and gcc4_64 should have no failing tests, the other
    platforms are less important
  - If there are failing tests, which are not flagged as known bugs,
    save them after careful checking or open a ticket and assign a
    known bug.
  - recheck/rebuild the test networks (if necessary due to
    netconvert changes)
  - check the tests again
- check the documentation
  - update the [ChangeLog](../ChangeLog.md)
  - generate options documentation from configuration templates
    using `tools/build/configTemplateToWiki.py`
  - recheck/rebuild the configuration schemata (if options were
    added) using `tools/xml/rebuildSchemata.py` (use the internal
    build to include all options)
- check the internal tests (same procedure as above), especially the
  (to be) published scenarios
- GitHub
  - add new [milestone](https://github.com/eclipse/sumo/milestones)
    if necessary
  - check all remaining tickets and assign them to later milestones
    or to persons.
- scenarios (optional)
  - prepare scenarios for release if the previous version does not
    run with the current release or significant changes were made to
    the scenarios
- update submodules by running `git submodule update --remote` and committing
  the changes if necessary

The trunk is now frozen. All commits which do not refer to an open
ticket for the upcoming release need to be made to a separate branch.
The freeze phase should not last longer than one week. The goal is to
fix all scenarios and have all failing tests fixed, which are not
assigned to a later milestone.

### Release day - 1

All scenarios should be fixed by now.

- patch the version information
  - in src/config.h.cmake, also disable the HAVE_VERSION_H macro
  - in CMakeLists.txt and build/wix/sumo.wxs
  - commit the changes
- recheck whether submodules changed by doing `git submodule update --remote`
and committing the changes after careful inspection
- check the documentation
  - update the [ChangeLog](../ChangeLog.md) again and include
    version and release date
  - modify the version number (Version) and the release date (ReleaseDate)
    [in mkdocs.yml]({{Source}}docs/web/mkdocs.yml) in the **extra:** section at the end
    to update the [download links](../Downloads.md)
- update the eclipse.org/sumo website
  - modify the version number (Version) [in config.toml](https://github.com/eclipse/sumo.website/blob/master/config.toml) in the **Default Parameters** section at the beginning
- create a new Eclipse release at https://projects.eclipse.org/projects/technology.sumo (after login there should be a "Create Release" button)
  - add an IP Log to the release
  - send an email to the PMC at technology-pmc@eclipse.org asking for review (include links to the release and the IP log)
- add a new version tag
```
> git tag -a v0_13_7 -m "tagging release 0.13.7, refs #563"
> git push --tags
```
- create new macOS application bundles with the new version number (`build/macos/...`) - can only be done on macOS with platypus installed (see {{Source}}build/macos/README.txt)

### Release day

The nightly build should have generated all releasable packages. If not,
delay the release. (The complete documentation, tests and source
distribution build can be achieved via "make dist".) The
following things need to be there:

- the platform independent part of the distribution;
  - source and all inclusive distributions (.tar.gz, .zip) ("make dist")
- the binary part of the distribution
  - windows binary distribution (zip, includes docs)
  - windows installer (msi, Win32 and x64, includes docs)
  - check presence of RPMs on
    <https://build.opensuse.org/package/show/home:behrisch/sumo_nightly>

If everything is fine:

- make a new folder in S:\\Sumo\\Releases
- make new sumo.dlr.de-release
  - copy the folder from S:\Sumo\Releases to the releases dir `scp -r /media/S/Releases/x.y.z delphi@ts-sim-front-ba.intra.dlr.de:docs/releases`
- make new sourceforge-release
  - make a new release within the sumo package (named "version
    x.y.z")
  - add files to the release
  - change default download attributes
  - update files at the [opensuse build
    service](https://build.opensuse.org/package/show?package=sumo&project=home%3Abehrisch)
- update the ubuntu ppa (see
<https://askubuntu.com/questions/642632/how-to-bump-the-version-of-a-package-available-in-another-users-ppa>)
  - this assumes you have the devscripts package as well as all sumo dependencies installed
  - if you try this on Windows Linux Subsystem you will also need to do `sudo update-alternatives --set fakeroot /usr/bin/fakeroot-tcp`
  - download the source release and rename it to `sumo_{{Version}}+dfsg1.orig.tar.gz`
  - unzip the source release
  - copy the debian dir one level up
  - modify the changelog, using `dch` (enter an email address which has write access to the ppa and a valid gpg key)
  - run `dpkg-buildpackage -S` in the sumo dir and `dput -f ppa:sumo/stable sumo_{{Version}}+dfsg1_source.changes` one level up
- scenarios (optional)
  - add files to [the scenario folder](https://sourceforge.net/projects/sumo/files/traffic_data/scenarios/)
  - updated README.txt
- inform the users about the new release
  - post information about the release to sumo-user@eclipse.org and
    sumo-announce@eclipse.org
  - submit news about the release on the Eclipse Newsroom <https://newsroom.eclipse.org/node/add/news>
  - tweet about it, post on Facebook and Instagram stories
  - trigger update of main website at <https://sumo.dlr.de>
- close [the milestone](https://github.com/eclipse/sumo/milestones)
  (retargeting open tickets needs to be done manually for now)
- create a [release on github](https://github.com/eclipse/sumo/releases) to trigger the build of the release python wheels
- publish the wheels on PyPI

### After-release cleanup

The trunk is now open for changes again.

- re-enable HAVE_VERSION_H in src/config.h.cmake
- rename version to "git" in CMakeLists.txt
- insert a new empty "Git master" section at the top of the [ChangeLog](../ChangeLog.md)
- commit changes
- drink your favorite beverage
