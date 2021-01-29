---
title: FAQ
permalink: /FAQ/
---
## General

### What is SUMO?

  SUMO is a traffic simulation package. It is meant to be used to
  simulate networks of a city's size, but you can of course use it for
  smaller networks and larger, too, if your computer power is large
  enough.

### What does "SUMO" mean?

  "SUMO" is an acronym for "Simulation of Urban MObility".

### What kind of a traffic simulation is SUMO?

  SUMO is mainly a **microscopic**, space-continuous road traffic
  simulation. It supports multi-modal and inter-modal ground based
  traffic. SUMO models individual vehicles and their interactions
  using models for car-following, lane-changing and intersection
  behavior. It also uses [pedestrian models](Simulation/Pedestrians.md) to simulate the movement
  of persons and their interactions with vehicles.

  To allow for the efficient simulation of very large scenarios, it is
  also possible to run SUMO as a [**mesoscopic** simulation](Simulation/Meso.md).
  
  SUMO also supports **macroscopic** traffic assignment using the
  [marouter](marouter.md) application.

### What is the best way to read the documentation and find out about specific topics

  The main page for getting an overview over the various topics is the
  outline
  [SUMO User Documentation](index.md). If
  you do not find the topic of interest listed there use the site search.
  Alternatively, use a general purpose search engine as this will also include the mailing list archives.

### How can I contribute to SUMO?

- Tell us about your extensions on the [developer mailing list](https://accounts.eclipse.org/mailing-list/sumo-dev)
- Send us patches (bug fixes as well as extensions) either to the same
  list or as pull requests at <https://github.com/eclipse/sumo> (also
  see next question).
- Report bugs (crashes, surprising behavior) or invalid documentation
  at [sumo-user](https://accounts.eclipse.org/mailing-list/sumo-user)
  or at <https://github.com/eclipse/sumo/issues>
- The main development team at the [DLR](http://sumo.dlr.de) is always
  looking for project partners. [Contact us](Contact.md) to
  discuss your proposals.
- Make your SUMO simulation scenarios publicly available
- [Cite us](Publications.md) in your publications and tell
  other people about SUMO.
- Answer questions on the [sumo-user mailing list](Contact.md)
  whenever you know the answer
- Contribute to this wiki ([contact us](Contact.md), so we can
  give you editing rights)
- Create a video tutorial and tell us about it
- Join us at the annual [SUMO User Conference](https://eclipse.org/sumo/conference/)

### How do code contributions work?

  We need to make sure that you have the necessary rights on the code
  and that you agree that your contribution is put under the Eclipse
  Public License v2. The easiest way to do achieve this is

- Create a GitHub account
- Configure the git client on your computer to use the email address
  that you used when registering for your account:

```
git config --global user.email "email@example.com"
```

- Create an [Eclipse account](https://accounts.eclipse.org/user/register) if you do not
  already have one. Make sure to use the same email address as for
  GitHub
- Fill out a simple online form, called the Eclipse Contributor
  Agreement (ECA, this is mandatory)
- [Fork](https://help.github.com/en/articles/fork-a-repo) the sumo
  repository on GitHub
- Clone the forked repository to your computer

```
git clone https://github.com/yourgithubid/sumo.git
```

- Modify the files in the created sumo clone directory
- From within this directory Commit your changes with

```
git add src
git commit -m "your change message" -s
```

!!! caution
    the option -s must be used so the contribution can be accepted in accordance with Eclipse rules

- Push your changes to your fork on GitHub with

```
git push
```

- Send us a [pull request on GitHub](https://help.github.com/en/articles/creating-a-pull-request-from-a-fork)
- Delete your fork, once the pull request is accepted or declined.
- (If you wish to keep your fork consider adding a topic branch to your local copy and push it before submitting the pull request.)
- More info may be found at
  <https://wiki.eclipse.org/Development_Resources/Contributing_via_Git>

### How do I contribute to the documentation?

The documentation is part of the code repository so the same rules as in the previous question apply. 
If you just want to have a simple typo fixed you can always drop us a line at sumo@dlr.de or at sumo-dev@eclipse.org.

### How do I cite SUMO

  Cite using ["Microscopic Traffic Simulation using SUMO"](https://elib.dlr.de/124092/); Pablo Alvarez Lopez, Michael
  Behrisch, Laura Bieker-Walz, Jakob Erdmann, Yun-Pang Flötteröd,
  Robert Hilbrich, Leonhard Lücken, Johannes Rummel, Peter Wagner, and
  Evamarie Wießner. IEEE Intelligent Transportation Systems Conference
  (ITSC), 2018.

### How do I unsubscribe from the mailing list?

Go to <https://www.eclipse.org/mailman/listinfo/sumo-user>. At the
bottom of the page you will find a form that allows you to enter your
email address to unsubscribe it from the list.

## General Problem Solving

### An application crashed. What should I do?

The applications in the SUMO suite may crash when running out of memory.
Monitor the memory usage of your applications to check whether that may
be your problem. 32bit-applications may only use 2GB of RAM. Use the
64bit version and ensure sufficient memory on your machine in order to
work with larger files. If this does not fix the crash, please report
this as a bug [as explained below](#how_do_i_report_erroneous_behavior_of_a_sumo_application).

### Why does SUMO not behave as documented in this wiki?

This wiki documents the behavior of the [latest development version](Downloads.md#nightly_snapshots). This is usually quite
close the the [latest release](Downloads.md) (differences are
explicitly listed in the [ChangeLog](ChangeLog.md). If you are
using an older version of SUMO, you need to refer to [the documentation that is packaged with that version](Downloads.md#sumo_-_older_releases). Note that we do
not back-port bugfixes to older version of SUMO. If possible you should
always use the latest version of SUMO.

### There is an error message but I don't know what I did wrong

  Activate [XML-Validation](XMLValidation.md) on your input
  files. If that does not solve your problem feel free to ask on the
  mailing list but **please** remember to copy / screenshot the error
  message itself and attach it to your question. Attaching (zipped\!)
  input files for reproducing the error message is also a good idea
  (see question above).

### How do I report erroneous behavior of a SUMO application?

  If you suspect a bug in one of the applications you should report
  your findings by sending the following items to the [mailing list](Contact.md):

- a description of the SUMO-version and the operating system you are
  using
- a screenshot or error output showing the unexpected behavior (this
  may allow us to diagnose the problem at a single glance)
- the complete input files for reproducing the error (i.e. a .sumocfg
  and all files referenced therein) in a zip-archive. Please remove
  unnecessary inputs (i.e. only 2 vehicles instead of 2000) and try to
  find the minimum input example which still shows the problem.
- a description at which time step (for simulations) and on which
  edge/junction the problem occurs

!!! note
    If you are using an older version of SUMO, please also report whether the problem shows up with the [latest release](Downloads.md).

!!! note
    If your attachments are rejected by the mailing list due to their size **(max 100Kb)**, upload them to a file hosting service and send the link. Alternatively, send them directly, rather than via the list.

### SUMO does not do what I want and I don't know how to solve the issue

  See above but **please** give enough details when asking on the
  mailing list.

### What should I do to get helpful answers on the mailing list?

- Make your question specific
  - avoid vague terms.
  - Always name the SUMO version to which your question applies
  - Include relevant warnings/errors/stack-traces in your question
    (please copy the text and avoid screenshots to reproduce text
    messages)
- Phrase your question using familiar terms (not everyone is an expert
in your domain).
- Don't ask for too many things in a single post.
- Do some research on your own before you post the question (otherwise
you may appear to be lazy).
  - read the FAQ
  - read the documentation
  - check out the [Tutorials](Tutorials.md)
  - do a web search (past questions and answers from the mailing
    list can be found by google)
- Do not ask the same thing twice in a short span of time. If you are
in a hurry and cannot get an answer, try to change your question
according to the above suggestions.
- Be polite
- Good Example questions:
  - How can I get data X out of SUMO?
  - How can I influence aspect Y of a simulation?
  - My simulation does Z though I do not expect it to. How do I fix that?

### I asked a question on the mailing list and did not get an answer within X days. Why?

  Community support is provided on a best effort basis. Questions are
  mostly answered by the developers when they find time between their
  projects. Thus, response times can vary between hours and weeks
  (especially during holidays). Sometimes, when there is a high volume
  of questions on the mailing list, your question may have been lost.
  Feel free to repost your question if you did not get an answer
  within a week. Response times also vary with the difficulty of the
  question (because the person answering it needs to find a bigger
  time slice to tackle the problem). Also, follow the advice given
  above when phrasing your questions to increase the chance for quick
  answers.

### I want to do a project with SUMO can you help me with the implementation?

  Unfortunately, we do not have the resources to do other peoples
  projects for free.
  [Contact](http://www.dlr.de/ts/en/desktopdefault.aspx/tabid-1231/mailcontact-30303/)
  us for paid consultancy.
  We try to help out with bugs and give pointers to the relevant
  documentation but this free support is limited to what we can do in
  our spare time.

## TraCI

### My [TraCI](TraCI.md)-program is not working as intended. Can you help me debug it?

  Unfortunately, we do not have the resources to debug other peoples
  code. If you suspect a bug in TraCI itself, the [general rules of bug-reporting](FAQ.md#how_do_i_report_erroneous_behavior_of_a_sumo_application)
  apply. Feel free to post your code to the mailing list as there may
  be other programmers present who could help. Be aware that someone
  who wants to reproduce your problem needs all your input files to do
  so.

### My [TraCI](TraCI.md)-program is not working as intended. How do I debug it?

When using TraCI there are two processes that can raise errors: SUMO and
the TraCI script being run. Here are some guidelines for figuring out
what is happening:

- The SUMO error *connection reset by peer* indicates a problem in the
script. The python strack trace should point to the problem (in the
simplest case, the script ends without calling *traci.close()*)
- The error **TraCIFatalError** *connection closed by SUMO* indicates
a problem with SUMO
- The error **TraCIException** indicates a minor problem with the
script. (e.g. asking for a vehicle that does not exist). This type
of error can be safely handled by the script with *try/except*
- It is a good idea to run [sumo](sumo.md) with the option **--log** {{DT_FILE}} to
figure out what went wrong in a simulation run or why the simulation
aborted.
- If SUMO crashes (just stops instead of quitting with an error
message) here is how to debug it:
  - in the options to *traci.start()* add *\['--save-configuration',
    'debug.sumocfg'\]*
  - run your script (sumo will not start and the script will try to
    connect repeatedly)
  - run the debug version of sumo with the saved configuration in a
    debugger while the script is still trying to connect

```
gdb --args sumoD -c debug.sumocfg
```

### Error: tcpip::Storage::readIsSafe: want to read 8 bytes from Storage, but only 4 remaining

The TraCI protocol changed in version 1.0.0. Please make sure that TraCI
client version and SUMO version match. When using SUMO version 1.0.0 or larger you cannot use traci version 17 or lower (check by calling `traci.getVersion()`)

## Features

### Does SUMO support traffic within the junctions?

  Yes, SUMO supports inner-junction traffic since version 0.9.5.

### Is it possible to connect SUMO to an external application (f.e. ns-2)?

  There are several approaches to do this, see
  [TraCI](TraCI.md) and [Topics/V2X](Topics/V2X.md)

### Can SUMO simulate lefthand traffic?

  Yes. It is supported since version 0.24.0. To create a new network for
  lefthand traffic, the option **--lefthand** must be set.
  
  To convert an existing network to lefthand driving, there are two options. Abstract networks (no geo-reference, coordinates do not matter much) can be processed with netconvert:
```
    netconvert -s righthand.net.xml --flip-y-axis -o lefthand.net.xml
```

   To convert an existing network and preserve coordinates, the network must first be disaggregated into nodes and edges and then re-assembled:
```
    netconvert -s righthand.net.xml --plain-output-prefix righthand
    netconvert -e righthand.edg.xml -n righthand.nod.xml --lefthand -o lefthand.net.xml
```

### Can SUMO generate movement traces?

  Yes. This is accomplished by using the
  [traceExporter](Tools/TraceExporter.md) tool to convert SUMO
  outputs into the appropriate format. Also see
  [Topics/V2X](Topics/V2X.md)

### Can SUMO simulate heterogeneous traffic / lane-free traffic ?

  Yes (since version 0.27.0). The
  [Sublane-Model](Simulation/SublaneModel.md) is activated by
  setting the option **--lateral-resolution** {{DT_FLOAT}}.

### Can SUMO simulate driving in reverse?

  No. While it is possible to move a vehicle backwards using
  [TraCI](TraCI.md), other vehicles will not react in a
  sensible manner to this.

### Can SUMO simulate driving through the oncoming lane?

  Yes. (since version 0.27.0). The [opposite-direction-driving feature is activated by using a network with additional information](Simulation/OppositeDirectionDriving.md).

### Can SUMO be run in parallel (on multiple cores or computers)?

  The simulation itself runs on a single core. However, routing
  in [sumo](sumo.md) or [duarouter](duarouter.md) can
  be parallelized by setting the option **--device.rerouting.threads** {{DT_INT}} and **--routing-threads** {{DT_INT}} respectively.
  When these options are are used, multiple cores on the machine are used.
  
  There is no support for multi-node parallelization.
  
  When running [sumo-gui](sumo-gui.md), an additional thread is used for visualization.
  
  The python TraCI library allows controlling multiple simulations
  from a single script either by calling *traci.connect* and storing
  the returned connection object or by calling
  *traci.start(label=...)* and retrieving the connection object with
  *traci.getConnection(label)*.
  
  The work to make the core (microscopic) simulation run in parallel is ongoing (Issue #4767). 
  Some parts of the simulation can already be run in parallel when setting option **--threads** but this does not lead to meaningful speedup yet.

## Building / Installation

### How do I access the code repository?

Since 2018-04-10 SUMO moved to [the organizational Eclipse account at GitHub](https://github.com/eclipse/sumo/). You have the choice to
access the repository using git or subversion. There are plenty of
clients for all platforms. If you use the command line client, you
can checkout sumo using the following command (for git):

```
git clone --recursive https://github.com/eclipse/sumo
```

If you want to see the full project history in your git checkout please
change to the created directory and call

```
git fetch origin refs/replace/*:refs/replace/*
```

For later updates go inside the sumo directory, which has been created,
and simply type `git pull`.

### Is there further documentation on Git and Subversion?

  There are the [Git book](https://git-scm.com/book/de/v1) and the
  [Subversion book](http://svnbook.red-bean.com/) and the [GitHub help](https://help.github.com/) is also worth reading.

### How to get an older version of SUMO?

  see
  [Downloads\#SUMO_-_older_releases](Downloads.md#sumo_-_older_releases).
  On Linux, older versions [must be built from source](Installing/Linux_Build.md).

### How to check out revision 5499 (or any other outdated sumo)?

  You can use the subversion option "**-r <REVISION_NUMBER\>**"
  together with the checkout on the command line. If you are using git
  you can find the correct hash using the git log. You have to consult
  your client's documentation if you use a graphical interface. Please
  be aware of the fact that we can only give very limited support for
  older versions.

### Which platforms are supported?

  We compile regularly under Windows 7 and Windows Server 2012 R2
  using Visual Studio 2013 (32bit and 64bit) and have daily builds on
  Linux (openSUSE Leap 42.1 (64bit) and openSUSE 13.1 (32bit)).
  Furthermore there are nightly builds on the [open build service](https://build.opensuse.org/package/show?package=sumo_nightly&project=home%3Abehrisch).
  SUMO can be installed on macOS via Homebrew (or built from source).
  We would be happy to hear about successful builds on other
  platforms. We already heard about successful builds on Solaris and
  Cygwin.

### Can I run multiple versions of SUMO alongside each other?

  Different versions of SUMO generally do not interfere with each
  other. When setting environment variables such as *SUMO_HOME* or
  *PATH*, care must be taken to reference the desired directory. On
  linux, the package manager will generally only offer a single
  version. [All other versions must be built from source](Installing/Linux_Build.md).

### Letters and words are represented as squares in sumo-gui and netedit

  Make sure that your computer supports 3D Acceleration and graphical
  drivers are correctly installed and configured.

### Troubleshooting

  See [Installing/Linux Build](Installing/Linux_Build.md) or
  [Installing/Windows Build](Installing/Windows_Build.md).

### Uninstalling

  In most cases you can simply delete the sumo folder. If you
  installed sumo via a package manager on linux, uninstall it via the
  package manager as well.

## Basic Usage

### What measures are used/represented?

  All time values are given in seconds. All length values are given in
  meters. This means that speed should be given in m/s, etc.
  Currently, the default simulation time step is one second long (to
  be exact: in each simulation step one second real time is
  simulated).

### What does the following error mean?

  Warning: No types defined, using defaults... Error: An exception
  occurred\! Type:RuntimeException, Message:The primary document
  entity could not be opened. Id=<PATH\> Error: (At line/column 1/0).
  Error: Quitting (conversion failed).
  Answer: Simply that the file you try to use (<PATH\>) does not exist.
  This is xerces' way to say "file not found".

### How do I work around a file not found / command not found error?

  To run the command-line programs your operating system must be able
  to find them.
  If you are using Windows please consult [Basics/Basic Computer Skills\#running_programs_from_the_command_line](Basics/Basic_Computer_Skills.md#running_programs_from_the_command_line)
  If you are using Linux run `export PATH=$PATH:/path/to/sumo/bin`
  (replace /path/to/sumo/bin with the path to the bin directory of
  your sumo installation)

### How do I work around missing dll errors on windows?

  Install [MSVC2013 Redistributable](http://www.microsoft.com/download/en/details.aspx?id=5555)
  (Old versions of SUMO may also require the MSVC2010 Redistributable)

### What is the meaning of the different exit codes (linux command line)

  0 is success, 1 is a recognized error and anything above is an
  unorderly termination (crash). If you see exit codes other than 0 or
  1 [please tell us about it](FAQ.md#how_do_i_report_erroneous_behavior_of_a_sumo_application)

### What's the deal with *schema resolution* warnings / xsd errors?

  Since version 0.20.0 sumo performs
  [XML-validation](XMLValidation.md) on input files with
  schema information. This helps to detect common mistakes during
  manual preparation of XML input files. Files generated by sumo
  applications such as [duarouter](duarouter.md) always add
  schema information. The schema files which are needed for checking
  are retrieved from the local sumo installation if [the environment variable **SUMO_HOME** is set](Basics/Basic_Computer_Skills.md#additional_environment_variables).
  Otherwise the files will be retrieved from
  [sumo.dlr.de](https://sumo.dlr.de) which is slower. Validation
  can be disabled by using the option **--xml-validation never** or by deleting the schema
  information at the top of the XML input file(s).

### What causes ''Error: unable to resolve host/address 'sumo.dlr.de' ''?

  This is related to [XML-validation](XMLValidation.md) (see
  above). When the [the environment variable **SUMO_HOME**](Basics/Basic_Computer_Skills.md#additional_environment_variables)
  is not set, applications will try to retrieve the xsd schema files
  online. If there is no internet connection. The above error results.
  The solution is to either

- disable validation or
- declare **SUMO_HOME** or
- ensure internet connectivity

### Why can my input-XML files not be read even though they look OK to me?

  Errors such as

```
Error: attribute name expected at
At line/column 10/46
```

  can be caused by non-printing characters in the XML-file. Open your
  XML-file in a document editor and activate the option for showing
  line-breaks and other non-printing characters to find them.

### Why do I get errors about *missing files* / *file not found* even though the file exists?

  SUMO versions before 1.0.0 use the space character to separate file paths. This means it
  will not be able to load files with a path such as *C:\\Program
  Files\\foo.xml* even when adding quotation marks. Since 1.0.0 the ',' (comma) is used so these paths should work.
  Additionally, SUMO may fail to load files if the name contains characters outside
  the basic [ASCII set](https://en.wikipedia.org/wiki/Ascii).
  
### Windows 10 is blocking the execution of sumo-gui and netedit. How come?
  Executable files downloaded from the internet may trigger the windows 10 security warning. You can work around this by clicking on the 'More Info' Text and then selecting 'Run Anyway' in the subsequent dialog.

## netconvert

### I made changes to the *.net.xml*-file but it did not work as expected. Why?

  As a general rule, you should never modify the *.net.xml* file
  directly nor try to generate one with a custom process. There are
  lots of subtle interdependencies between network elements which are
  hard to get right with manual modifications. Even if your .net.xml
  can be loaded by [sumo](sumo.md) it could fail in ways that
  are less obvious:

- By having right-of-way rules that work somewhat differently than
  expected (this part of the *.net.xml* file is particularly complex)
- By losing properties when later being edited with
  [netedit](netedit.md)
- By failing to work with updated versions of [sumo](sumo.md)

If you need to modify a network there are several possibilities:

1.  Edit the network with [netedit](netedit.md)
2.  Modify the original input files and then rebuild the net with
    [netconvert](netconvert.md).
3.  Patch the network with [netconvert](netconvert.md). You can
    [load a *.net.xml* file](Networks/Import/SUMO_Road_Networks.md) together with
    [small XML-files](Networks/PlainXML.md)
    to patch individual edges, nodes and connections. Additional
    possibilities are described
    [here](Tutorials/ScenarioGuide.md#modifying_the_network).
    For changing traffic light plans or timings see
    [Simulation/Traffic_Lights](Simulation/Traffic_Lights.md).
    If you cannot figure out how to accomplish the desired changes this
    way, [contact us\!](Contact.md).
4.  Use [netconvert](netconvert.md)-option **--plain-output-prefix** to convert the
    network into [it's plain XML representation](Networks/PlainXML.md).
    Then modify these files and rebuild the network

If you need to build a network from custom input data it is recommended
to generate
[*plain-xml*-files](Networks/PlainXML.md)
(nodes, edges, connections) with a custom process and build your network
with [netconvert](netconvert.md) from these files. These input
files allow for many customization of geometrical and structural network
properties. If you cannot get them to represent the network you need,
ask for help on the mailing list. Alternatively you could also [add another import module to netconvert](Developer/How_To/Net_Importer.md) (which we would be
happy to integrate if possible).

!!! caution
    In contrast, generating a network directly has been a dead-end for more than one project. You have been warned.

### My network looks ugly, All the junction shapes are wrong. How come?

  This currently happens up to version 0.23.0 if you import networks
  with left-handed traffic. See
  [#Can_SUMO_simulate_lefthand_traffic?](#can_sumo_simulate_lefthand_traffic).

### Some junctions in my network look ugly. What can I do?

  See [Joining Junctions](Networks/Import/OpenStreetMap.md#junctions). If
  that doesn't help, check for invalid edge geometry. A sharp turn in
  front of an intersection (even if the segment is only centimeters
  long) may mess up the shape computation. Read the
  [netconvert](netconvert.md) warnings about sharp angles and
  consider using [netconvert](netconvert.md) option **--geometry.min-radius.fix**.
  If that still doesn't help you may specify the shape of the junction
  [manually using the `shape`-attribute](Networks/PlainXML.md#node_descriptions).

### Can I import the free network of Osnabrück "Frida"?

  Yes and no. You can import it using netconvert, a description is
  available at
  [Networks/Import/ArcView](Networks/Import/ArcView.md#frida_network_city_of_osnabruck).
  This may be a good gis-network but lacks some needed information in
  order to be usable for simulations (see discussion at the link
  above).

### Are there any other free networks available I can use?

  See [Networks/Import/OpenStreetMap](Networks/Import/OpenStreetMap.md)

### The application hangs after a while (few memory consumption, most of the system time) (Windows)

  You are probably running a program compiled in the debug-mode. This
  yields in at least the triple of normal memory usage and your system
  may not be able to solve. Try to use the normal version, build in
  Release-mode (or buy more RAM :-) ).

### Many errors of the form *Edge's 'x' from- and to-node are at the same position*

  You are probably trying to import a network with geo-coordinates
  without specifying a geo-projection. Add option **--proj.utm**.

### Many errors of the form *Error: Type 'x' used by edge 'y' was not defined*

  You are probably trying to re-import an OSM-network written to plain
  XML data. Add **--ignore-errors.edge-type** or provide a type file.

### Error: SUMO was compiled without GDAL support

The default windows release is currently provided without GDAL support
due to EPL2 Licensing issues. If you need to import shapefiles, either
use the Linux version or download the [nightly-extra version](http://sumo.dlr.de/daily/sumo-msvc12extrax64-git.zip)

## netedit

### How can I obtain netedit?

  [netedit](netedit.md) is available as part of the regular
  distribution since version 0.25.0.

## Traffic Demand Generation

### How do I generate random routes?

  First of all you should know that random routes are probably quite
  unrealistic.
  If you wish to use them anyway, use the script
  {{SUMO}}/tools/trip/randomTrips.py to generate random trips as explained
  here
  [Tools/Trip\#randomTrips.py](Tools/Trip.md#randomtripspy).

  You can also call the script without options to get additional help.
  See [Demand/Shortest or Optimal Path Routing\#Usage_Examples](Demand/Shortest_or_Optimal_Path_Routing.md#usage_examples)
  for more information on turning trips into routes.

### How do I maintain a constant number of vehicles in the net?

  There are different methods for accomplishing this. In either case
  the simulation itself should be constraint using options **--begin**, **--end**.

- You can use [rerouters](Simulation/Rerouter.md) in the
  simulation. Rerouters, assign a new route for vehicles driving
  across them and thus prevent them from leaving the network. For an
  example with a simple circle see [{{SUMO}}/tests/sumo/cf_model/drive_in_circles]({{Source}}tests/sumo/cf_model/drive_in_circles)
  - The tool [generateContinuousRerouters.py](Tools/Misc.md#generatecontinuousrerouterspy) can be used to generate
    rerouters for continuous operation with configurable turning ratios.
  - If the network is not circular to begin with (i.e a single
    road) you can make the network circular in a non-geometrical way
    by adding a return edge and declaring it's length to be very
    short (minimum 0.1m). The return edge should have a sensible geometry (i.e. a detour loop) but the length can be made very short so that it does not affect vehicle routes.
- You can generate long trips going around the network with lots of
  detours. This can be accomplished using
  [randomTrips.py](Tools/Trip.md#randomtripspy) by setting
  the options below. If the network contains disconnected parts, not
  all random trips will be viable. In this case, simply generate more
  trips and delete superfluous vehicles from the output route file.
  - **--begin** <your desired begin time\>
  - **--end** <begin + time in which vehicles shall be inserted to then network\>
  - **--period** <(end - begin) / number of vehicles desired\>
  - **--intermediate** {{DT_INT}} set to a large value to ensure that vehicles remain in the simulation long enough
  - **-r** <output route file\>

- You can use [jtrrouter](jtrrouter.md) to [generate vehicles which drive randomly around the network with configurable turning ratios](Tutorials/Manhattan.md#generating_vehicles)

- You can use the option **--max-num-vehicles** to set the desired number. Vehicle
  insertions are delayed whenever this number would be exceeded. To
  avoid a large number of delayed vehicles it is recommended to also
  use the option **--max-depart-delay**. When using this approach you must ensure that there
  is a sufficient number of vehicles that are ready for insertion at
  all times. Note, that the number of distinct vehicle IDs over the
  whole simulation is much larger the specified value because some
  vehicles leave the simulation and new vehicles with distinct IDs are
  inserted to replace them.

!!! caution
    Up to version 0.24.0, option **--max-num-vehicles** terminates the simulation when exceeding the specified number


### A vehicle cannot reach its target or takes a circuitous route. Why?

  Unexpected routes are often caused by invalid lane-to-lane
  connections or
  [`vClass`-restrictions](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#abstract_vehicle_class).
  Even if there are some lanes on every edge that permit the vehicle,
  they may not be connected. If the vehicle has the default type, it
  will ignore access restrictions and travel on all lanes. The
  [netcheck tool](Tools/Net.md#netcheckpy) can be used to
  visualize connectivity from a starting edge and to discover missing
  connections. It may be necessary to [delete some detour edges](netconvert.md#edge_removal) from [the network](Networks/Import/SUMO_Road_Networks.md) to discover
  the relevant connectivity boundaries.

  To investigate connectivity [sumo-gui](sumo-gui.md) can be
  made to show lane-to-lane connections when customizing the
  visualization options for junctions. To investigate lane-permission
  issues, the menu option *Edit-\>Select lanes which allow...* can be
  used for visualizing selected permissions.

### How do I generate SUMO routes from GPS traces?

  The answer largely depends on the quality of your input data mainly
  on the frequency / distance of your location updates and how precise
  the locations fit your street network. In the best case there is a
  location update on each edge of the route in the street network and
  you can simply read off the route by mapping the location to the
  street. This mapping can be done using the python sumolib coming
  with sumo, see
  [Tools/Sumolib\#locate_nearby_edges_based_on_the_geo-coordinate](Tools/Sumolib.md#locate_nearby_edges_based_on_the_geo-coordinate).

  This will fail when there is an edge in the route which did not get
  hit by a data point or if you have a mismatch (for instance matching
  an edge which goes in the "wrong" direction). In the former case you
  can easily repair the route using [duarouter](duarouter.md)
  with **--repair**. Another small script which helps here is
  [Tools/Routes\#tracemapper.py](Tools/Routes.md#tracemapperpy).
  You can also use the map matching plugin from
  [Contributed/SUMOPy](Contributed/SUMOPy.md) here.

  For more complex cases (i.e. large temporal gaps or spatial errors)
  the problem is known as [Map Matching](https://en.wikipedia.org/wiki/Map_matching). Open source
  tools exist to facilitate this
  ([MatchGPX2OSM](http://wiki.openstreetmap.org/wiki/Routing/Travel_Time_Analysis/MatchGPX2OSM)
  [graphhopper](https://github.com/graphhopper/map-matching)).

  To exactly reproduce high-resolution trajectories, it is possible to
  map exact vehicle locations (including lateral positioning) and also
  to move vehicles beyond the road space by using the [TraCI moveToXY function](TraCI/Change_Vehicle_State.md#move_to_xy_0xb4).

## Simulation

### How to simulate an accident

SUMO automatically detects [vehicle collisions](Simulation/Safety.md#collisions). Since the default
model aims to be accident free, some effort must be taken to [create accidents](Simulation/Safety.md#deliberately_causing_collisions).

Often, the effects of an accident are required instead of the accident
itself. Without using [TraCI](TraCI.md) the following approaches
may be useful:

1. Let a vehicle halt on the lane for some time (see [Definition of Vehicles, Vehicle Types, and Routes\#Stops](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops)).
This works quite nice for simulating accidents.
2. Put a variable speed sign of the lane where the accident is meant to
be and let it reduce the speed (see [Simulation/Variable Speed Signs](Simulation/Variable_Speed_Signs.md)). This method will
reduce the throughput on the lane, but further dynamics will rather not
fit to what one would expect from an accident situation.
3. You may of course combine both approaches. Within a project we
simulated traffic incidents by letting vehicles stop on one lane and
reducing the speed on the other lanes.
4. Alternatively, you may [close one or more of the lanes on an edge](Simulation/Rerouter.md#closing_a_lane)
5. If you [close the whole edge, rerouting may be triggered as well](Simulation/Rerouter.md#closing_a_street)
6. By setting the option **--collision.stoptime**, traffic jams may be created after a
registered
[collision](Simulation/Safety.md#deliberately_causing_collisions).

### I have changed my network and now SUMO does not load it.

  Actually, SUMO-networks are not meant to be edited by hand. You have
  to describe everything within your input properly and let netconvert
  build your network. Editing networks by hand is very complicated and
  error-prone.

### How do I change the duration of cycles and phases?

  use [netedit](netedit.md#traffic_lights)

### I can not see a vehicle moving in my simulation

  There may be several reasons why you do not see the cars.

- The simulation is not yet running (click the "play" button (![Image:play.gif](images/Play.gif "Image:play.gif")), see [sumo-gui#Usage_Description](sumo-gui.md#usage_description))
- If your simulation area is too big, cars will not be displayed
  unless you zoom into the net. Cars are simply to small when looking
  from far away. To change this you may also set the option *Draw with
  constant size when zoomed out* (see [visualization settings for vehicles](sumo-gui.md#changing_the_appearancevisualisation_of_the_simulation)).
- The simulation is too fast so that all vehicles disappear before
  being seen. To avoid this, you may [increase the *Delay*-value](sumo-gui.md#usage_description) to slow down
  the simulation.
- [You might have outdated graphic card drivers](sumo-gui.md#windows_and_buttons_appear_but_no_netcars_are_visible_vehicles_are_not_visible_or_flicker_roads_are_drawn_on_top_of_vehicles)

### Different departure times with different time step size

  Vehicles are entering the network earlier with decreasing time step
  lengths.
  The reason for this behavior is that a vehicle is emitted at the
  end of a time step. For one second time steps, this means that a
  vehicle which has the depart time of 0 will be inserted into the
  network at the end of the time step between 0 and 1, this means
  almost on second 1 (0.99...). If time steps of 0.1 seconds are used,
  the same vehicle is inserted into the network at the end of the time
  step between 0 and 0.1, this means almost on 0.1 (0.099...).

### How to save a simulation state and proceed later and/or differently

  See [Simulation/SaveAndLoad](Simulation/SaveAndLoad.md)

### How can I make the simulation run faster?

- make sure there are [no unwanted jams](#the_simulation_has_lots_of_jamsdeadlocks_what_can_i_do)
  The more vehicles there are in the simulion, the slower it gets.
- if your network is very large, build it with option
  --no-internal-links ([simplified intersection model](Simulation/Intersections.md#internal_links))
- If you are using trips or periodic rerouting, enable [parallel routing](Demand/Automatic_Routing.md#parallelization)
- use the default step-lenght of 1s (using **--step-length 0.1** slows down the simulation
  by a factor of 10)
- if your simulation is very larger. consider using the [mesoscopic simulation model](Simulation/Meso.md)
- see below for further tips on slow-down

### The simulation gets slow with many vehicles waiting for insertion

  SUMO keeps checking continuously for possible vehicle insertions. If
  the network is jammed the number of necessary checks grows quickly.
  The option **--max-depart-delay** {{DT_TIME}} may be used to discard vehicles which could not be
  inserted within {{DT_TIME}} seconds.

### The simulation runs slow on the command line

  when running on the windows operating system, the command-line
  output which is refreshed every simulation step slows the simulation
  down significantly. Use option **--no-step-log** to avoid this.

### The simulation has lots of jams/deadlocks. What can I do?

Deadlocks in a scenario can have many causes:

1.  invalid network
  - invalid lane numbers
  - missing turning lanes
  - invalid connections
  - invalid junctions [(big clusters of small junctions should be joined)](Networks/PlainXML.md#joining_nodes)
2.  invalid traffic lights (see [Improving generated traffic lights](Simulation/Traffic_Lights.md#improving_generated_programs_with_knowledge_about_traffic_demand))
3.  invalid demand (too many vehicles overall, too many vehicles
    starting on the same edge).
4.  invalid routing
  - only shortest path were used instead of [a user assignment algorithm](Demand/Dynamic_User_Assignment.md)
  - to many vehicles start/end their route with a [turn-around](Simulation/Turnarounds.md).
5.  invalid insertion (vehicles being inserted on the wrong lane close
    to the end of an edge where they need to change to another turn
    lane). This can be fixed by setting the vehicle attribute `departLane="best"`

If the network was imported from OpenStreetMap, it is highly recommended
to use the [recommended import options](Networks/Import/OpenStreetMap.md#recommended_netconvert_options).
The best course of action typically is to observe the simulation using
[sumo-gui](sumo-gui.md) and figure out where the first jam
develops.

### Why do the vehicles perform unexpected lane-changing maneuvers?

  This may be caused by invalid lane-to-lane connections. Check the
  connections in [sumo-gui](sumo-gui.md) by activating
  *Junctions-\>show lane to lane connections* in the [gui settings dialog](sumo-gui.md#changing_the_appearancevisualisation_of_the_simulation).

### How do I get high flows/vehicle densities?

By default, insertion flow is [limited by the time resolution of the simulation](Simulation/VehicleInsertion.md#forcing_insertion_avoiding_depart_delay) (vehicles are only inserted every full second) and by the default insertion speed of 0.

The following definition will allow flows in the range of 2500 vehicles/hour/lane:

`<vehicle departSpeed="max" departPos="last" departLane="best" ../>`

Instead of setting `departPos="last"`, the option **--extrapolate-departpos** can also be used.

!!! caution
    When using *departLane* values *best*, *free* or *random_free* high flows on multi-lane roads require insertion edges that are somewhat longer than the braking distance of the fastest vehicles. This is because the lane selection checks vehicles on the insertion edge to determine a suitable insertion lane and short edges give insufficient information.

To increase flows even further the following settings can be used
(potentially sacrificing some realism)

- `<vType sigma="0" minGap="1" length="3" .../>`
- `<vType tau="0.5" .../>` (should not be lower than step-length)

### How do I insert vehicles with a fixed density?

To start a simulation with 65 evenly-spaced vehicles on a specific lane
of a 1km highway, do the following:

- create an edge that is 1km long
- compute the space for each vehicle: 1000m / 65 = 15.385m
- subtract the minimum space requirement (5m vehicle length + 2.5m
  minimum gap by default)
- this gives you 7.885m as remaining space between vehicles.
- at a default value of tau=1 (desired time headway excluding minGap)
  this means vehicles will be in a stationary state at 7.885m/s
- you can use insertion method 'last' which places vehicles at their
  desired headway for a given speed (repeat for every lane):

```
<flow id="lane0" from="startEdge" to="destEdge" begin="0" end="1" number="65" departPos="last" departSpeed="7.885" departLane="0"/>
```

To let vehicles enter the simulation continuously with a specific
density:

- compute the time period between successive vehicles using the
  equilibrium speed from above: 15.385m / 7.885m/s = 1.951s
- use the equilibrium speed as departSpeed

```
<flow id="lane0" from="startEdge" to="destEdge" begin="0" end="3600" period="1.951" departPos="base" departSpeed="7.885" departLane="0"/>
```

### How do I force a lane change?

  There are several options to force a vehicle onto a lane:

- setting vehicle attributes ['departLane' and 'arrivalLane'](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#a_vehicles_depart_and_arrival_parameter)
- setting a
  [<stop\>](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md#stops)
  on the desired lane (also affects speed)
- setting a route that requires lane-changing to a specific lane for
  continuation
- setting the vehicle [class of the vehicle and forbidding that class from using the other lanes of an edge](Simulation/VehiclePermissions.md)
- trigger the lane-change via
  [TraCI](TraCI/Change_Vehicle_State.md#change_lane_0x13)

!!! note
    A further method is planned but not yet implemented: #1020

### Are there any limits in regard to the size of a simulation scenario?

- There are no hard limits on the number of streets, intersections or
  vehicles in SUMO. Make sure that you have enough RAM when trying to
  build or run scenarios beyond city-size. It may be necessary to use
  the 64bit version of SUMO to make use of available RAM.
- The maximum length of a simulation scenario is 285 million years (or
  278 thousand years at millisecond time resolution). If you need to
  simulate longer time periods you must [save and reload the simulation state](#how_to_save_a_simulation_state_and_proceed_later_andor_differently)

### How fast can SUMO run?

This really depends on how many vehicles you have in your simulation at any given time (and to some extent on how strongly vehicles interact at intersections and how many lanes each vehicle has to chose during lane-changing). 
When running in verbose mode, SUMO will tell you the UPS metric. This is vehicle **up**dates **p**er **s**econd. A number of x tells you that you could run x vehicles in real-time at a step length of 1 second. 
Running time is inversely proportional to step-length (**--step-length 0.1** computes 10 times as many steps compared to the default of 1s and thus takes 10 times as long).

Some examples on an average desktop PC:

- vehicles on a long single lane: 700k UPS
- vehicles on a long four-lane road: 240k UPS
- complex grid network: 80k UPS

In a city simulation of one day running with 80k UPS where a vehicle spends on average 30 minutes driving, 3.8 million vehicles can be simulated in 24h wall-clock time. However, the simulation would run slower than real-time whenever there are more than 80k Vehicles in the network at the same time because rush hours and low-traffic times would average out.

Calculated as ` 24 * 3600 * 80000 / 1800 = 3840000 `

## Visualisation

### sumo-gui breaks

  Sometimes [sumo-gui](sumo-gui.md) terminates with no reason.
  In most cases, an update of the opengl-driver solves this problem.

### Display flickers in the area of the mouse pointer (Windows)

  Newer Windows-Versions seem to cache the area under the mouse to
  apply the mouse shadow afterwards. To avoid this, go to your
  Systemmenu, then Mouse-\>Pointers and disable the mouse shadows.
  That's the only solution so far. (Origin: Till Oliver Knoll, via QT
  Interest List)

### sumo-gui windows and buttons appear but no net/cars are visible / Vehicles are not visible or flicker / Roads are drawn on top of vehicles

  This problem may occur when using outdated/wrong openGL drivers.
  Please try to update the drivers for you graphics hardware.

### Building videos from sumo-gui

There are several ways to build videos from your SUMO simulation. You
can use screen capturing tools like VLC Player
[\[1\]](http://www.videolan.org/vlc). The disadvantage of this approach is
the requirement of a (very) fast CPU to capture the video in real time,
it depends on the chosen resolution and screen size of the simulation.

The second approach needs a sumo-gui version which has support for
ffmpeg compiled in (for the windows downloads this is the extra
version). When in doubt open the about dialog (press F2) and read the
line mentioning the compiled features (like Proj) whether it includes
FFmpeg. If so, you can use the screenshot button to start video
recording. Just enter a filename ending with ".h264" or ".hevc"
depending on the encoder you want. Please be aware that this will slow
down your simulation sometimes to the point of halting.

Another approach is the use of single images for every simulation
timestep. This can be done with an additional traci command for building
screenshots. A small python snippet shows the usage of this TraCI
command to get a new image for each new timestep.

```
    for i in range(duration):
        traci.simulationStep()
        traci.gui.screenshot("View #0", "images/"+str(i)+".png")
```

Next you have to glue the images together. This job can be done
graphically with virtualdub [\[2\]](http://www.virtualdub.org) or via
commandline with ffmpeg [\[3\]](http://www.ffmpeg.org). An example command
for ffmpeg is shown below.

```
    ffmpeg -r 10 -i images/%d.png -b:v 1500k -vcodec wmv2 sumo.wmv
```

The parameters -r set the value for the fps and -b:v the bitrate for the
video.

### Incomplete output

When using the GUI or TraCI the output files seem sometimes incomplete
because the files are only flushed when everything is closed. For the
GUI you can either use the close item from the menu or close the whole
GUI. In TraCI it is always a good idea to wait for the sumo process to
be finished using some kind of
[process.wait()](https://docs.python.org/2/library/subprocess.html#popen-objects)
mechanism

### Incompatibility with DisplayLink devices

Drivers of [DisplayLink](http://www.DisplayLink.com) devices are
incompatibles with Fox Library. If sumo-gui or netedit presents graphics
problem like [this](http://sumo.dlr.de/docs/images/DisplayLinkError.png)
during the execution, DisplayLink drivers must be uninstalled.

### Distorted view with green lines all over the network

If your sumo-gui looks [like this](https://github.com/eclipse/sumo/issues/7511)
it is probably again a problem with your display driver. It seems to occur most
of the time with on board (Intel) graphics together with a dedicated nvidia card.
To fix it install the latest nvidia drivers, open the nvidia system control panel
(should be available from a right click on the desktop background) go to 3D settings
and select nvidia as your preferred graphics processor. If this does not fix it, try
to set the integrated graphics as preferred. (Thanks @palvarezlopez for finding out.)

## Upgrading

### How do I upgrade SUMO?

  The easiest way is to download the [latest sumo release](https://sumo.dlr.de/docs/Downloads.php) or the [latest development version](https://sumo.dlr.de/docs/Downloads.php#sumo_-_latest_development_version) to a
  directory of your choices and start using it from there. You may
  have to update the environment variables *PATH* and *SUMO_HOME* to
  ensure that the correct version of the application is called from
  the command line (see
  [Basics/Basic_Computer_Skills\#Configuring_Path_Settings](Basics/Basic_Computer_Skills.md#configuring_path_settings)).
  Confirm that the values are update correctly by calling `echo %SUMO_HOME%` (Linux: `echo $SUMO_HOME`) 
  from a command line that you wish to use for calling applications and traci scripts.

### [sumo](sumo.md) warns about deprecated networks or fails to load them

  In major versions (whenever the 1st or 2nd digit of the version
  number changes) there may be changes to the network format. In this
  case we provide scripts in  {{SUMO}}*/tools/net* which can be used to
  upgrade your networks automatically. The current version of
  [netconvert](netconvert.md) can also be used to upgrade most
  network files to the current version. Simply call `netconvert -s old.net.xml -o new.net.xml`

## Python tools

### How do I run the python tools?

  See [here](Basics/Using_the_Command_Line_Applications.md#using_python_tools_from_the_command_line)

### tools fail with a SyntaxError or ImportError or some TypeError concerning "\>\>"

  Many python tools require python version 2.7. We aim to have Python
  3 compatibility for [sumolib](Tools/Sumolib.md) and
  [traci](TraCI/Interfacing_TraCI_from_Python.md) however.

### tools fail with an ImportError

  Make sure to set the environment variable SUMO_HOME to point at the
  base directory of your SUMO installation (the directory that
  contains *tools* and *bin*).

### how do I import [traci](TraCI/Interfacing_TraCI_from_Python.md) or [sumolib](Tools/Sumolib.md) in my own python script?

In order to import these libraries the folder {{SUMO}}/tools must be in your
[python search path](https://docs.python.org/2/tutorial/modules.html#the-module-search-path).
This can be accomplished by modifying the python variable **sys.path**
as explained [here](TraCI/Interfacing_TraCI_from_Python.md) and
[here](Tools/Sumolib.md). Alternatively you can hard-code the
path to your SUMO-installation into your script:

```
 import sys
 sys.path.append('/your/path/to/sumo/tools')
 import traci
 import sumolib
```

### the python scripts do not accept command line arguments (windows only)

This is a [known issue](https://stackoverflow.com/questions/2640971/)
with some python installations. The usual workaround if you are not
allowed to do edit the registry, is to call the python interpreter
explicitly, e.g. instead of

```
script.py <argument>
```

you do

```
python script.py <argument>
```

### [osmWebWizard.py](Tools/Import/OSM.md#osmWebWizard.py) fails to generate Scenario on Windows 10

This can happen with an outdated version of python 2.7. Updating to
2.715 (64bit) has been reported as fixing this problem.

## (Communication) Network Simulators

### How do I use *.tcl* files with NS2?

  Questions regarding NS2 should be put to [the NS2 mailing list](http://www.isi.edu/nsnam/ns/ns-lists.html).

### How do I combine SUMO with a network simulator?

  Check out [veins](http://veins.car2x.org/).
  
## Validity
### What are publications on SUMO's validity?
- https://elib.dlr.de/120856/
- https://elib.dlr.de/112657/
- https://elib.dlr.de/6657/
- https://elib.dlr.de/121278/
- https://elib.dlr.de/97240/
- https://elib.dlr.de/115720/
- https://elib.dlr.de/116607/
- https://ieeexplore.ieee.org/abstract/document/7906642

## Outdated (Questions for very old versions of SUMO)

### Vehicles do not drive on the last edge of their route

  Up to (and including) version 0.13.1 a vehicle finished its route
  (per default) on position 0 of the last edge. You can change this by
  setting the attribute **departPos** in the vehicle definition (see
  [Definition_of_Vehicles,_Vehicle_Types,_and_Routes](Definition_of_Vehicles,_Vehicle_Types,_and_Routes.md)).
  Negative values count backwards from the end of the edge, thus
  setting **departPos**="-1" will make the vehicle drive (almost) to
  the end of the last edge. The new default in more recent versions is
  the end of the edge, which should disable this behavior.

### Vehicle jumps backward

  When I try to evaluate my vehicle dump, sometimes vehicle seem to
  jump backward (their position is smaller than in the previous step)

  To avoid jams due to an inappropriate lane changing behavior,
  vehicles may swap their lanes. In this case each vehicle obtains the
  position and speed of the other vehicle. As the position of the
  vehicles may differ a bit, one of the vehicles may seem to jump
  backwards.

  This should not happen from Version0.8 on.
