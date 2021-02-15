---
title: Basics/Using the Command Line Applications
permalink: /Basics/Using_the_Command_Line_Applications/
---

Most of the applications from the SUMO package are command line tools.
Currently, only [sumo-gui](../sumo-gui.md) and [netedit](../netedit.md) are not. If you do not
know what a "command line" is, we refer you to the page about [basic computer skills](../Basics/Basic_Computer_Skills.md#running_programs_from_the_command_line).

The following presents some peculiarities of the SUMO-suite
applications.

# Using SUMO Applications from the Command Line

SUMO applications are plain executables. You just start them by typing
their name from the command line; for example
[netgenerate](../netgenerate.md) is called by

```
netgenerate.exe
```

under Windows and by

```
netgenerate
```

under Linux.

This simply starts the application
([netgenerate](../netgenerate.md) in this case). As no parameter
has been given, the application does not know what to do and prints only
an information about itself:

```
Eclipse SUMO netgenerate Version {{Version}}
 Build features: Linux-4.1.39-56-default Proj GDAL GUI
 Copyright (C) 2001-2020 German Aerospace Center (DLR) and others; https://sumo.dlr.de
 License EPL-2.0: Eclipse Public License Version 2 <https://eclipse.org/legal/epl-v20.html>
 Use --help to get the list of options.
```

# Options

Each application has a set of options which define which files shall be
processed or generated, or which define the application's behaviour.
Normally, an application needs at least two parameter - an input file
and an output file - but almost always more parameter are used for a
fine-grained control. Each application's options are described within
the application's description. In the following, it is described how
options are set.

## Setting Options on the Command Line

There are two kinds of options: boolean options which do not require an
argument and are set to true if the option is present (but accept usual
boolean values like "true" and "false" as argument) and options which
require an argument. Setting an option with an argument on the command
line consists of two parts - the option name and the option's value. For
example, if one wants the simulation to load a certain road network,
"mynet.net.xml", the following must be written:

```
--net mynet.net.xml
```

The '--' in front indicates that the option's long name is following
("net") in this case. After a whitespace the option's value must be
given. It is also possible to use a '=' instead of the whitespace:

```
--net=mynet.net.xml
```

Some often used options can be abbreviated. The abbreviation for the
**--net**-option is **-n**. The following has the same effect as the two examples
above:

```
-n mynet.net.xml
```

Please note that an abbreviation is indicated using a single '-'.

!!! note
    Not all abbreviations have the same meaning across the applications from the SUMO-suite.

## Option Value Types

The SUMO applications know what kind of a value they expect to be set.
For example, [netgenerate](../netgenerate.md) allows you to set the
default number of lanes, which of course must be an integer value. In
the case, a string or something else is given, this is recognized and
the application answers with an error message on startup. Please note
that the decimal point in a float is encoded using a dot (".").

A special case of value types are lists, for example the list of
additional files to load into a simulation. When giving more than a
single file, the files must be divided using ','. This also counts for
lists of other value types, as integers or floating point numbers.

# Configuration Files

Because the list of options may get very long, configuration files were
introduced. You can set up a configuration file which contains all the
parameter you want to start the application with. Moreover, you have to
start the application with only this configuration file given.

A configuration file is an XML-file that has a root element named
`configuration`. The options are written as
element names, with the wanted value being stored in the attribute
`value` (or `v`);
the option **--net-file *test.net.xml*** given on the command line would become
`<net-file value="test.net.xml"/>` within the
configuration file. For the boolean options the value should be either
"true", "on", "yes", "1", or "x" for the activation and "false", "off",
"no", or "0" for deactivating the option (case does not matter here).

For the example above, the configuration file (let's save it under
"test.sumocfg", see below) would look like:

```
<configuration>
    <input>
        <net-file value="test.net.xml"/>
        <route-files value="test.rou.xml"/>
        <additional-files value="test.add.xml"/>
    </input>
</configuration>
```

The section `input` given above has only
documentation purposes and no functional meaning.

A less verbose but equivalent version would look like:

```
<configuration>
    <n v="test.net.xml"/>
    <r v="test.rou.xml"/>
    <a v="test.add.xml"/>
</configuration>
```

The according [sumo](../sumo.md) execution call - working with both
configuration versions - would be:

```
sumo.exe -c test.sumocfg
```

This means that instead of the parameters, we only give the name of the
configuration file using the option **--configuration-file** {{DT_FILE}} or **-c** {{DT_FILE}}. If you want to give no further
options on the command line it is possible to leave out the "-c" as
well:

```
sumo.exe test.sumocfg
```

## Naming Conventions for Configuration Files

Depending on the targeted application, the configuration files have
different extensions. It is highly recommended to follow this
convention. For using simulation configurations with
[sumo-gui](../sumo-gui.md) this is even required -
[sumo-gui](../sumo-gui.md) can only read simulation configurations
named "\*.sumocfg".

All conventions for configuration extensions can be found on the page on
[used file extensions](../Other/File_Extensions.md).

## Configuration Files vs. Command Line Parameter

In addition to a configuration file, further command line parameter can
also be given on the command line. If a parameter is set within the
named configuration file as well as given on the command line, the value
given on the command line is used (overwrites the one within the
configuration file). If you want to disable a boolean option which was
enabled in the configuration file, you need to give the "false" value on
the command line explicitly, like **--verbose false**

## Generating Configuration Files, Templates and Schemata

The applications of the SUMO package allow you to generate configuration
file templates. It is possible to save an empty configuration - a
configuration template. This can be done by using the **--save-template** {{DT_FILE}}. In this case,
the configuration will only contain the parameters filled with their
default values.

Also it is possible to save a configuration file which contains the
currently set values. An application can be forced to do this using the
option **--save-configuration** {{DT_FILE}}.

Last but not least one can generate an [XML schema](https://en.wikipedia.org/wiki/XML_Schema_%28W3C%29) (using the
option **--save-schema** {{DT_FILE}}) to validate configuration files against. For the SUMO
application this schema should be equivalent to the one found at
<https://sumo.dlr.de/xsd/sumoConfiguration.xsd> (respectively for the
other executables). Please note that the schema is more strict than the
SUMO options parser since it only validates the verbose version given
above.

In either case, if further information on the parameters is wanted, one
can also pass the option **--save-commented**. Then, some further comments on each parameter
are generated.

## Environment variables in Configuration Files

It is possible to refer to environment variables in configuration files. The syntax to refer to an environment variable is **${VARNAME}**. For example, your configuration file may reference a variable called **NETFILENAME**, containing the name of a network file, with the following configuration settings.

```
<configuration >
    <input>
        <net-file value="${NETFILENAME}.net.xml"/>
    </input>
</configuration>
```

# Common Options

The applications from the SUMO suite share several options. They are
given in the following.

## Reporting Options

| Option  | Description  |
|---------|--------------|
| **-v** {{DT_BOOL}}<br>**--verbose** {{DT_BOOL}}  | Switches to verbose output; *default: **false*** |
| **--print-options** {{DT_BOOL}} | Prints option values before processing; *default: **false***  |
|  **-?** {{DT_BOOL}}<br>**--help** {{DT_BOOL}} | Prints this screen; *default: **false***  |
| **-V** {{DT_BOOL}}<br>**--version** {{DT_BOOL}}  | Prints the current version; *default: **false***  |
| **-X** {{DT_STR}}<br>**--xml-validation** {{DT_STR}}  | Set schema validation scheme of XML inputs ("never", "auto" or "always"); *default: **auto***  |
| **--xml-validation.net** {{DT_STR}}  | Set schema validation scheme of SUMO network inputs ("never", "auto" or "always"); *default: **never*** |
| **-W** {{DT_BOOL}}<br>**--no-warnings** {{DT_BOOL}}  | Disables output of warnings; *default: **false***  |
| **-l** {{DT_FILE}}<br>**--log** {{DT_FILE}}  | Writes all messages to FILE (implies verbose)  |
| **--message-log** {{DT_FILE}}  | Writes all non-error messages to FILE (implies verbose)  |
| **--error-log** {{DT_FILE}}  | Writes all warnings and errors to FILE  |

The logging options **--log** and **--message-log** also enable the verbose output but only into
the given file (unless **--verbose** was given as well). Errors get always printed to
the console (in addition to a possible log-file).

The [XML validation](../XMLValidation.md) options enable [XML
schema processing](https://xerces.apache.org/xerces-c/schema-3.html) in
the XML parser. This performs a basic validation of the input and is
highly recommended especially for beginners because it easily finds
spelling mistakes in the input which otherwise might be silently
ignored. Validation is only performed if the [XML-schema is declared within the input file](../XMLValidation.md).

## Random Number Options

These options configure how the seed of the random number generator is
determined. The same seed leads to the same sequence of generated random
numbers.

By default the seed is a hard-coded fixed value. So, as long as all
configuration settings are kept the same, the outputs of repeated
simulation runs will be the same. To change this, use one of the
following options.

| Option  | Description |
|---|---|
| **--seed** {{DT_INT}}  | Set a particular seed for the random number generator. By using different values you can have different but still reproducible simulation runs.  |
| **--random**  | Make SUMO choose a seed. If available the seed will be based on output of /dev/urandom otherwise the seed will be derived from current system time. This option has **precedence** over option **--seed** {{DT_INT}}.   |

**Attention**: The precedence of **--random** over **--seed** {{DT_INT}} means that it is **impossible**
to set **--random** in a configuration file and overwrite it by **--seed** {{DT_INT}} in the command
line. There might be a solution in future.

# Generating and Reading Files

Almost every file the tools from the SUMO package read or generate are
written in XML. You should get familiar with
[XML](https://en.wikipedia.org/wiki/XML) before starting to work with
SUMO. SUMO allows you to import files from different sources, but
"native" SUMO files - road network descriptions, route and/or demand
descriptions, infrastructure descriptions, etc. - are SUMO-specific, not
following any standard. XML files can be read and written using a simple
text editor and we usually do this. Nonetheless, several other tools
exist and may be reused.

For some of the file types used by SUMO, a xsd (XML Schema Definition)
exists. Please consult the page on [file extensions](../Other/File_Extensions.md) for a list of described
file formats.

## Hints on using XML

- The default viewer for XML files on Windows is the Internet
  Explorer. It may be used for validating your XML files. The syntax
  of an XML file should be valid if the Internet Explorer can
  completely load it.
- [Eclipse](https://www.eclipse.org/) allows to write XML documents
  following a given [xsd (XML Schema Definition)](https://en.wikipedia.org/wiki/XML_Schema_%28W3C%29).

# Writing files

Several options expect a file name ({{DT_FILE}}) to write into as parameter. When
being given at the command line, the given path is assumed to be
relative to the current working directory. When being given in a
configuration file, the file path is assumed to be relative to the
configuration file's path. Absolute paths are of course also allowed.

Usually, existing files with the same name are overwritten without a
warning. The directory must exist, where the output file shall be
written.

Besides writing to a file, further special notations allow to:

- write into the null-device (no output at all): use "NUL" or
  "/dev/null" for file name (both notations work platform independent)
- write into a socket: use "<HOST\>:<PORT\>" for file name
- write to stdout (print on command line): use "stdout" or "-" for
  file name
- write to stderr: use "stderr" for file name
- The special string 'TIME' within a filename will be replaced with
  the application start time

At the moment it is not possible to read input from sockets or from
stdin.

As a simple way of modifying output file names, the option **--output-prefix** {{DT_STR}} is provided.
The given string will be prepended to all files written by an
application.

!!! note
    Multiple date sources (i.e. detector definitions) are permitted to write to the same output file.

# Using Python tools from the Command Line

Many [tools](../Tools.md) that are distributed by SUMO (in the
{{SUMO}}/tools folder are written in the [python](https://www.python.org/)
programming language. To use them, **python 2.7** must be [installed on your computer](https://www.python.org/downloads/).

Then you need to make sure that the [environment variable *SUMO_HOME*](#additional_environment_variables) is set. The
easiest way is to open the command-line window using
[start-command-line.bat](#sumo_0123_and_later).

Furthermore you need to make sure that your computer knows where to find
the python tools. The easiest (but somewhat cumbersome) method is to run
the tool using its full path:

```
C:\Users\yourname>D:\path_to_sumo\tools\randomTrips.py  ... arguments ...
```

Alternatively, you can add the directory in which the tool lies to your
[PATH-Variable](#configuring_path_settings).