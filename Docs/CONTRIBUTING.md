Contributing to CARLA
=====================

> _This document is a work in progress and might be incomplete._

We are more than happy to accept contributions!

How can I contribute?

  * Reporting bugs
  * Feature requests
  * Code contributions

Reporting bugs
--------------

Use our [issue section](issueslink) on GitHub. Please check before that the
issue is not already reported.

[issueslink]: https://github.com/carla-simulator/carla/issues

Feature requests
----------------

Please check first the list of [feature requests][frlink]. If it is not there
and you think is a feature that might be interesting for users, please submit
your request as a new issue.

[frlink]: https://github.com/carla-simulator/carla/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+request%22

Code contributions
------------------

So you are considering making a code contribution, great! we love to have
contributions from the community.

Before starting hands-on on coding, please check out the
[projects page][projectslink] to see if we are already working on that, it would
be a pity putting an effort into something just to discover that someone else
was already working on that. In case of doubt or to discuss how to proceed,
please contact one of us (or send an email to carla.simulator@gmail.com).

[projectslink]: https://github.com/carla-simulator/carla/projects/1

#### What should I know before I get started?

Check out the ["CARLA Design"](carla_design.md) document to get an idea on the
different modules that compose CARLA, and chose the most appropriate one to hold
the new feature. We are aware the developers documentation is still scarce,
please ask us in case of doubt, and of course don't hesitate to improve the
current documentation if you feel confident enough.

#### Coding standard

Please follow the current coding style when submitting new code.

###### General

  * Use spaces, not tabs.
  * Avoid adding trailing whitespace as it creates noise in the diffs.
  * Comments should not exceed 80 columns, code may exceed this limit a bit in rare occasions if it results in clearer code.

###### Python

  * All code must be compatible with Python 2.7, 3.5, and 3.6.
  * [Pylint](https://www.pylint.org/) should not give any error or warning (few exceptions apply with external classes like `numpy`, see our `.pylintrc`).
  * Python code follows [PEP8 style guide](https://www.python.org/dev/peps/pep-0008/) (use `autopep8` whenever possible).

###### C++

  * Compilation should not give any error or warning (`clang++ -Wall -Wextra -std=C++14`).
  * Unreal C++ code (CarlaUE4 and Carla plugin) follow the [Unreal Engine's Coding Standard](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/) with the exception of using spaces instead of tabs.
  * CarlaServer uses [Google's style guide](https://google.github.io/styleguide/cppguide.html).

#### Pull-requests

Once you think your contribution is ready to be added to CARLA, please submit a
pull-request to the `dev` branch.

Try to be as descriptive as possible when filling the pull-request description.

Please note that there are some checks that the new code is required to pass
before we can do the merge. The checks are automatically run by the continuous
integration system, you will see a green tick mark if all the checks succeeded.
If you see a red mark, please correct your code accordingly.
