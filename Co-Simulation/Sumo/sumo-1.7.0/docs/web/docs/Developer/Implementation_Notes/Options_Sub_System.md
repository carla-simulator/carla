---
title: Developer/Implementation Notes/Options Sub System
permalink: /Developer/Implementation_Notes/Options_Sub_System/
---

## Introduction

All major SUMO applications use the same classes for parsing command
line options and configuration files. These classes encapsulate parsing
options from the command line and XML-configuration files and validating
whether they are of a correct type. The options sub system also supports
writing configuration-templates, saving current (parsed/loaded) settings
to files, and printing the help screen.

## Initialising Options

### First Steps

Each application has exactly one instance of the **OptionsCont**
(options container) - class. A reference to this class may be obtained
using

```
static OptionsCont &OptionsCont::getOptions();
```

Before an application starts registering its options, it must let
OptionsCont know itself. It is needed due to the fact that OptionsCont
is responsible for printing a basic information about the application
and because it also generates the help screen which includes the
application's name, possibly example calls, etc. Due to this, the first
touch of the options sub system for each application looks similar to
the following which is borrowed from netconvert_main.cpp:

```
   OptionsCont &oc = OptionsCont::getOptions();
   // give some application descriptions
   oc.setApplicationDescription("Road network importer / builder for the road traffic simulation SUMO.");
#ifdef WIN32
   oc.setApplicationName("netconvert.exe", "SUMO netconvert Version " + (string)VERSION_STRING);
#else
   oc.setApplicationName("sumo-netconvert", "SUMO netconvert Version " + (string)VERSION_STRING);
#endif
```

The ifdef-block is used to differ between the supported OSes
(Windows/Linux) where the SUMO-applications are named slightly
different.

Please note that the description set by setApplicationDescription is a
brief description and should fit into one line (be not longer than 80
characters).

### Registering Options

Before the command line options are parsed, their names, allowed types
and their default values (if existing) must be inserted into this
container. This mostly done within a dedicated method or function.

To insert an application option into the container, use one of the
following polymorphs:

```
void doRegister(const std::string &name1, Option *v);
```

or

```
void doRegister(const std::string &name1, char abbr, Option *v);
```

The parameter "name1" is the name (or one of the possible names, better
to say) of the inserted option. The character "abbr" within the second
call is the possible one-char abbreviation of the name.

```
Caution: The behaviour after adding the same abbreviation for two different options is not defined.
```

The third parameter is an option that shall be assigned to the name(s).
Remark that the **OptionsCont** takes the responsibility for this option
- it will be deleted when the **OptionsCont** is deleted. The option
itself must be an instance of one of the following classes, which all
are derived from the class **Option**:

- Option_Integer
- Option_Bool
- Option_Float
- Option_String
- Option_FileName
- Option_IntVector
- Option_StringVector

The names of the classes reflect the types which are awaited as
parameter for the option. Do also remark, that later access for reading
should be done in dependence to the type - it means that you should not
try to get a string from an **Option_Float**. This results in an
exception.

!!! caution
    All subclasses of Option are stored within the files ''{{SUMO}}*/src/utils/options/Option.h* and ''{{SUMO}}*/src/utils/options/Option.cpp*. This is surely not wished as when following the coding styleguide, each class should be stored in a separate pair of files. Also, one could imagine to use templates, here.

Every one of the option's subclasses has two constructors: one
that is parametrised with the appropriate type and a parameter-less one.
The parametrised constructor is used to supply default values for
options, in the other case, no value is known to the system at first. An
example: settings the "verbose"-switch as following, lets the
application run quiet as default:

```
oc.doRegister("verbose", 'v', new Option_Bool(false));
```

#### Adding Synonyms

One can also add synonyms for options. If you want to implement the
option "**name-of-vehicle-that-shall-halt**", but you wish to have a
shorter name, too, use the following to make the same option accessible
using both "**name-of-vehicle-that-shall-halt**" and "**v2h**":

```
oc.doRegister("name-of-vehicle-that-shall-halt", new Option_String());
oc.addSynonyme("name-of-vehicle-that-shall-halt", "v2h");
```

## Retrieving Options

If no default value was supplied for an option, you should ask the
**OptionsCont** whether it has been set. Access to values of unset
options causes exceptions. This may sound hard, but as it's only the
developer who accesses the options container in fact, it's the best way
to assure security of the code. One can determine whether an option is
set using

```
bool OptionsCont::isSet(const std::string &name) const;
```

If one has to know whether an option still stores the default value or
a value was supplied by the user, he/she may call

```
bool OptionsCont::isDefault(const std::string &name) const;
```

Dependent on the type of an option, one can retrieve its value using
the following methods from OptionsCont:

```
int OptionsCont::getInt(const std::string &name) const;
long OptionsCont::getLong(const std::string &name) const;
std::string OptionsCont::getString(const std::string &name) const;
float OptionsCont::getFloat(const std::string &name) const;
const IntVector &OptionsCont::getIntVector(const std::string &name) const;
const StringVector &OptionsCont::getStringVector(const std::string &name) const;
```

Using an inproper method yields an exception.

You may have noticed, that there is one method to retrieve a certain
value less than types available. The type **Option_FileName** returns a
string-vector, too. The only difference is their type which allows a different
processing of the values to set when parsing.

Please note, that it is good style to use the long (longest in fact)
name for retrieving an option's value and that the same name (synonyme)
should be used at all places. This makes it easier to locate this
option's usage.

## Implementation

All classes needed to store and process options can be found within
{{SUMO}}/src/utils/options/