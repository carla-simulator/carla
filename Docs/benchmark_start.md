Driving Benchmark
===============

The  *driving benchmark* module is made
to evaluate a driving controller (agent) and obtain 
metrics about its performance. 

This module is mainly designed for:

* Users that work with autonomous driving agents and want
to see how they perform in carla.

On this section you will learn.

* How to quickly get started and benchmark a trivial agent right away.
* Learn about the general implementation architecture of the driving 
benchmark module.
* Learn how to set up your agent to be benchmarked and create your
how set of experiments to challenge your agents. 
* Learn about the performance metrics used.




Getting Started
----------------

As a way to familiarize yourself with the system we
provide a trivial agent performing in an small
set of experiments (Basic). To execute it, simply
run:

    $ ./driving_benchmark_example.py


Keep in mind that to run the command above, you need a CARLA simulator
server running, at localhost and on port 2000.
   

We already provide the same benchmark used in the CoRL
2017 paper. 
The CoRL 2017 experiment suite can be run in a trivial agent by
running:

    $ ./driving_benchmark_example.py --corl-2017

This benchmark example can be further configured.
Run the help command to see options available.

    $ ./driving_benchmark_example.py --help


When running the driving benchmark for the basic configuration
you should [expect the following results](benchmark_creating.md/#expected-results).



