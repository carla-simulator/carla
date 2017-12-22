CARLA Benchmark
===============

Running the Benchmark
---------------------

The "carla" api provides a basic benchmarking system, that allows making several
tests on a certain agent. We already provide the same benchmark used in the CoRL
2017 paper. By running this benchmark you can compare the results of your agent
to the results obtained by the agents show in the paper.

!!! important
    Currently not tested on python 3

Besides the requirements of the CARLA client, the benchmark package also needs
the future package

    $ sudo pip install future

By running the benchmark a default agent that just go straight will be tested.
To run the benchmark you need a server running. For a default localhost server
on port 2000, to run the benchmark you just need to run

    $ ./run_benchmark.py

or

    $ python run_benchmark.py

Run the help command to see options available

    $ ./run_benchmark.py --help
