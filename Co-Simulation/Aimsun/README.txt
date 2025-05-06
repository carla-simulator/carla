This README describes how to start the co-simulation between Carla and Aimsun Next

1-Open the Aimsun file "example_network.ang".
2-Start the CARLA simulator.
3-Start the traffic simulation on the Aimsun environment by running replication 442.
4-Immediatly after run the script named "aimsun_eai_controller.py" from a command window or from your IDE.
5-The co-simulation should start

Notes:

a) It is NECESSARY that all the python scripts are located inside a CARLA folder like, for example, the "examples" folder inside CARLA's PythonAPI folder. The Aimsun network does not need to be in that folder.

b) The location of the CARLA map needs to be set in line 117 of the aimsun_eai_controller.py

c) Explore the script to see all settings that you can define for this experiment. Contact me if there are any questions on alkis.papadoulis@aimsun.com