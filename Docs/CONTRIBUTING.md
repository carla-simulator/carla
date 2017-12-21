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
issue was not already added.

[issueslink]: https://github.com/carla-simulator/carla/issues

Feature requests
----------------

Please check first the list of [feature requests][frlink]. If it is not there
and you think is a feature that might be interesting for users, please submit
your request as a new issue.

[frlink]: https://github.com/carla-simulator/carla/issues?q=is%3Aissue+is%3Aopen+label%3A%22feature+request%22

Code contributions
------------------

Before starting hands-on on coding, please check out the
[projects page][projectslink] to see if we are already working on that. In case
of doubt or to discuss how to proceed, please contact one of us (or send an
email to carla.simulator@gmail.com).

[projectslink]: https://github.com/carla-simulator/carla/projects/1

#### What should I know before I get started?

Check out the ["CARLA Design"](carla_design.md) document to get an idea on the
different modules that compose CARLA, and chose the most appropriate one to hold
the new feature.

#### Coding style

Please follow the current coding style when submitting new code.

  * Use spaces, not tabs.
  * Comments should not exceed 80 columns, code may exceed this limit a bit in rare
occasions if it results in clearer code.
  * Python code follows [PEP8 style guide](https://www.python.org/dev/peps/pep-0008/) (use `autopep8` whenever possible).
  * Unreal C++ code, CarlaUE4 and Carla plugin, follow the [Unreal Engine's Coding Standard](https://docs.unrealengine.com/latest/INT/Programming/Development/CodingStandard/) with the exception of using spaces instead of tabs.
  * CarlaServer uses [Google's style guide](https://google.github.io/styleguide/cppguide.html).

#### Pull request

Once you think your contribution is ready to be added to CARLA, please submit a
pull request and one of our team members will take a look at it.

Try to be as descriptive as possible when filling the pull-request description.
