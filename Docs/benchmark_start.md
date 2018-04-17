Driving Benchmark
===============

The  *driving benchmark* module is made
to evaluate a driving controller (agent) and obtain 
metrics about its performance. 

This module is mainly designed for:

* Users that work developing autonomous driving agents and want
to see how they perform in CARLA.

On this section you will learn.

* How to quickly get started and benchmark a trivial agent right away.
* Learn about the general implementation [architecture of the driving 
benchmark module](benchmark_structure.md).
* Learn [how to set up your agent  and create your
own set of experiments](benchmark_creating.md).
* Learn about the [performance metrics used](benchmark_metrics.md).




Getting Started
----------------

As a way to familiarize yourself with the system we
provide a trivial agent performing in an small
set of experiments (Basic). To execute it, simply
run:


    $ ./driving_benchmark_example.py


Keep in mind that, to run the command above, you need a CARLA simulator
 running at localhost and on port 2000.
   

We already provide the same benchmark used in the [CoRL
2017 paper](http://proceedings.mlr.press/v78/dosovitskiy17a/dosovitskiy17a.pdf).
The CoRL 2017 experiment suite can be run in a trivial agent by
running:

    $ ./driving_benchmark_example.py --corl-2017

This benchmark example can be further configured.
Run the help command to see options available.

    $ ./driving_benchmark_example.py --help

One of the options available is to be able to continue
from a previous benchmark execution. For example,
to continue a experiment in CoRL2017  with a log name of "driving_benchmark_test", run:

    $ ./driving_benchmark_example.py --continue-experiment -n driving_benchmark_test --corl-2017


!!! note
    if the log name already exists and you don't set it to continue, it
    will create another log under a different name.

When running the driving benchmark for the basic configuration
you should [expect these results](benchmark_basic_results_town01)
to be printed on the terminal in more or less 5 minutes,
depending on your machine configuration.



