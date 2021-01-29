---
title: Developer/Implementation Notes/Error Handling
permalink: /Developer/Implementation_Notes/Error_Handling/
---

<center>

<font color="red">**Please note that this page describes what is
currently in work.**</font>

</center>

## Failure types (not academic)

One can distinguish the following types of problems:

- Startup input data is not correct (a mandatory value is not set or
  is not numeric when it should be etc.)
- Dynamic input data is not correct
- Some things within our design or our implementation are not correct
  (we try to get an integer from a string option etc., NULL-pointer
  exceptions, trying to access deleted classes etc.)
- System failures (out of memory etc.)

The distinction between *startup* input data and *dynamic* input data is
not as easy as one might think. Basically *startup* refers to everything
referenced directly on the command line or via a configuration file
while dynamic refers to interactive input via different interfaces such
as GUI or socket connections. The problem is that some data given at
startup is processed with quite a delay by the simulation (notably
routes) and probably cannot even be checked (e.g. for being well formed
XML) at startup due to performance problems. This kind of data will be
considered *dynamic*.

Thus the term *startup input data* refers to every data (file) which is
processed before the application starts doing something useful (whatever
that means).

## Command line applications

### Philosophy for command line applications

We assume that all the input data should be valid. We will not try to
solve any bugs made by the user. We just inform him what went wrong and
let him patch the problem. For *startup data* this means we finish the
application with an error message if erroneous data is encountered. As
the input data may be broken on several places, the application should
try to parse all data, first, reporting all encountered errors, and quit
then. For *dynamic data* this means, if we have no possibility to give
direct feedback (for instance on one way socket connections) we ignore
the data (with something like a log message), otherwise we try to give
feedback.

*Programming bugs* should be catched in a way that allows to retrack
them. The user shall not be confronted with any segfaults-popups or
things like that. Rather, an error message about an improper behavior
shall be printed (currently: "Quitting (on unknown error).") and the
application should quit correctly.

*System errors* should be catched and printed to the user. The
application should then quit correctly.

### Implementation in command line applications

#### Incorrect startup data

As said in [\#Philosophy for command line
applications](#philosophy_for_command_line_applications) we
just report the problem and quit then (with an information that we could
not accomplish what was to do).

For this, the main function of each of our command line applications has
a try/catch-block and all of the processing done by this application is
done within this block. What is always catched is the **ProcessError** -
exception, defined in src/utils/common/UtilExceptions. This class
contains a message which is reported as soon as the try/catch-block
located in the main function is reached. As in earlier versions of SUMO,
a ProcessError could be called with no message (in this case the message
is set to "Process Error"), we only print the message if the catched
ProcessError has really one set.

```
int
main(int argc, char **argv)
{
    // initialise return code
    int ret = 0;
    ... make uncritical initialisation ...
    try {
       ... do some critical stuff ...
    } catch (ProcessError &e) {
        if(string(e.what())!=string("Process Error") && string(e.what())!=string("")) {
            MsgHandler::getErrorInstance()->inform(e.what());
        }
        MsgHandler::getErrorInstance()->inform("Quitting (on error).", false);
        // set return code to "failure"
        ret = 1;
#ifndef _DEBUG
    } catch (...) {
        MsgHandler::getErrorInstance()->inform("Quitting (on unknown error).", false);
        // set return code to "failure"
        ret = 1;
#endif
    }
    ... clean up ...
    // return the return code
    return ret;
}
```

The second catch-block will be described later.

So basically the only thing we have to do is to generate a
**ProcessError** as soon as something is not as we would have expect it
and give it a proper message. Still, this is only half of the truth.
Because we do not want the application to quit immediately when an error
occurs, but print all errors and quit then, we have to insert a further
error handling layer. In the current implementation, handler which parse
the input data are reporting the errors to the error instance of the
MsgHandler. As soon as reading a file ends, it is checked whether an
error occured. In this case, a ProcessError is thrown. If the handler is
not parsing the data himself, but using other structures, these
structures may throw an **InvalidArgument** which then has to be catched
and reported to the error instance of the MsgHandler.

Be aware that some things already allocated have to be deleted before
throwing a ProcessError.

#### Implementation bugs

The second catch block within the above snipplet is currently meant to
catch everything else. In fact, these may be both system failures and
programming faults. So, we only catch this in the release-version so
that an end-user will be informed about an error (though with no verbose
error message). In the case we are debugging the code, the
exception/fault is not catched - our debugger should stay at the
position the exception was generated.

#### System failures

Are currently catched by the later catch-block. Probably, the easiest
method to handle those would be to give a list of possible failures and
try to handle them in catch-blocks as it is done with **ProcessErrors**.

## GUI applications

### Philosophy for gui applications

Now hat is lethal for a command line application (false input parameter)
is not for a gui application which shall report about the error but
shall still allow the user to patch his inputs and to try to reload the
settings.

## Open issues

### SUMO

- Network errors
  - There is no check for link consistency; to be exact, it is
    possible that some combinations of junction/incoming <-\> cedge
    <-\> succedge do not match but still are loaded with no errors
  - There is no check for unused values
- WAUT errors
  - How comes that a tls-switch - output is build though errors
    occured? (see
    tests/sumo/errors/wauts/cross3ltl_error_unknown_wautid)
  - The programs are checked in a lazy way, the error is reported
    too late (see
    tests/sumo/errors/wauts/cross3ltl_error_unknown_to_prog)
  - Undescribed behavior for
    cross3ltl_error_missing_junctiondef,
    cross3ltl_error_missing_reftime