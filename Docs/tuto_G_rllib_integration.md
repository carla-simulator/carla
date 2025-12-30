# RLlib Integration

The RLlib integration brings support between the [Ray/RLlib](https://github.com/ray-project/ray) library and CARLA, allowing the easy use of the CARLA environment for training and inference purposes. Ray is an open source framework that provides a simple, universal API for building distributed applications. Ray is packaged with RLlib, a scalable reinforcement learning library, and Tune, a scalable hyperparameter tuning library. 

The RLlib integration allows users to create and use CARLA as an environment of Ray and use that environment for training and inference purposes. The integration is ready to use both locally and in the cloud using AWS. 

In this guide we will outline the requirements needed for running the RLlib integration both locally and on AWS, the structure of the integration repository, an overview of how to use the library and then an example of how to set up a Ray experiment using CARLA as an environment.

- [__Before you begin__](#before-you-begin)
    - [Requirements for running locally](#requirements-for-running-locally)
    - [Requirements for running on AWS Cloud](#requirements-for-running-on-aws-cloud)
- [__RLlib repository structure__](#rllib-repository-structure)
- [__Creating your own experiment__](#creating-your-own-experiment)
    - [The experiment class](#1-the-experiment-class)
    - [The environment configuration](#2-the-environment-configuration)
    - [The training and inference scripts](#3-the-training-and-inference-scripts)
- [__DQN example__](#dqn-example)
- [__Running on AWS__](#running-on-aws)
    - [Configure AWS](#configure-aws)
    - [Create the training AMI](#create-the-training-ami)
    - [Configure the cluster](#configure-the-cluster)
    - [Run the training](#run-the-training)
    - [Running the DQN example on AWS](#running-the-dqn-example-on-aws)

---

## Before you begin

- Download the RLlib integration from [GitHub](https://github.com/carla-simulator/rllib-integration/tree/main) or clone the repository directly:

```sh
    git clone https://github.com/carla-simulator/rllib-integration.git
```

- Requirements vary depending on if you are running locally or on AWS:

>###### Requirements for running locally

>>- [Install a package version of CARLA](https://github.com/carla-simulator/carla/releases) and import the [additional assets](https://carla.readthedocs.io/en/latest/start_quickstart/#import-additional-assets). __The recommended version is CARLA 0.9.11__ as the integration was designed and tested with this version. Other versions may be compatible but have not been fully tested, so use these at your own discretion. 
>>- Navigate into the root folder of the RLlib integration repository and install the Python requirements:

                python3 -m pip install -r requirements.txt

>>- Set an environment variable to locate the CARLA package by running the command below or add `CARLA_ROOT=path/to/carla` to your `.bashrc` file:

                export CARLA_ROOT=path/to/carla

>###### Requirements for running on AWS Cloud

>>- The requirements for running on AWS are taken care of automatically in an install script found in the RLlib integration repository. Find more details in the section ["Running on AWS"](#running-on-aws).

---

## RLlib repository structure

The repository is divided into three directories:

- `rllib_integration` contains all the infrastructure related to CARLA and how to set up the CARLA server, clients and actors. This provides the basic structure that all training and testing experiments must follow.
- `aws` has the files needed to run in an AWS instance. `aws_helper.py` provides several functionalities that ease the management of EC2 instances, including instance creation and sending and receiving data.
- `dqn_example` and the `dqn_*` files in the root directory provide an easy-to-understand example on how to set up a Ray experiment using CARLA as its environment.

---

## Creating your own experiment

This section provides a general overview on how to create your own experiment. For a more specific example, see the next section ["DQN example"](#dqn-example).

You will need to create at least four files:

- The experiment class
- The environment configuration
- The training and inference scripts


#### 1. The experiment class

To use the CARLA environment you need to define a training experiment. Ray requires environments to return a series of specific information. You can see details on the CARLA environment in [`rllib-integration/rllib_integration/carla_env.py`][carlaEnv]. 

The information required by Ray is dependent on your specific experiment so all experiments should inherit from [`BaseExperiment`][baseExperiment]. This class contains all the functions that need to be overwritten for your own experiment. These are all functions related to the actions, observations and rewards of the training.

[carlaEnv]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/carla_env.py
[baseExperiment]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/base_experiment.py#L41

#### 2. The environment configuration

The experiment should be configured through a `.yaml` file. Any settings passed through the configuration file will override the default settings. The locations of the different default settings are explained below.

The configuration file has three main uses:

1. Sets up most of the CARLA server and client settings, such as timeout or map quality. See the default values [here][defaultCarlaSettings]. 
2. Sets up variables specific to your experiment as well as specifying town conditions and the spawning of the ego vehicle and its sensors. The default settings are found [here][defaultExperimentSettings] and provide an example of how to set up sensors.
3. Configures settings specific to [Ray's training][raySettings]. These settings are related to the specific trainer used. If you are using a built-in model, you can apply settings for it here. 

[defaultCarlaSettings]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/carla_core.py#L23
[defaultExperimentSettings]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/base_experiment.py#L12
[raySettings]: https://github.com/ray-project/ray/blob/master/rllib/agents/trainer.py

#### 3. The training and inference scripts

The last step is to create your own training and inference scripts. This part is completely up to you and is dependent on the Ray API. If you want to create your own specific model, check out [Ray's custom model documentation][rayCustomModel].

[rayCustomModel]: https://docs.ray.io/en/master/rllib-models.html#custom-models-implementing-your-own-forward-logic

---

## DQN example

This section builds upon the previous section to show a specific example on how to work with the RLlib integration using the [BirdView pseudosensor][birdview] and Ray's [DQNTrainer][dqntrainer].

[birdview]: https://github.com/carla-simulator/rllib-integration/blob/main/rllib_integration/sensors/bird_view_manager.py
[dqntrainer]: https://github.com/ray-project/ray/blob/master/rllib/agents/dqn/dqn.py#L285

The structure of the DQN example is as follows:

- __The experiment class__: [`DQNExperiment`][dqnExperiment], which overwrites the methods of the `BaseExperiment` class.
- __The configuration file__: [`dqn_example/dqn_config.yaml`][dqnConfig]
- __The training file__: [`dqn_train.py`][dqnTrain]
- __The inference file__:
    - __With Ray__: [`dqn_inference_ray.py`][dqnInferenceRay] 
    - __Without Ray__: [`dqn_inference.py`][dqnInference]

[dqnExperiment]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_example/dqn_experiment.py#L19
[dqnConfig]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_example/dqn_config.yaml
[dqnTrain]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_train.py
[dqnInferenceRay]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_inference_ray.py
[dqnInference]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_inference.py

To run the example locally:

1. Install pytorch:

        python3 -m pip install -r dqn_example/dqn_requirements.txt

2. Run the training file:

        python3 dqn_train.py dqn_example/dqn_config.yaml --name dqn        

!!! Note
    The default configuration uses 1 GPU and 12 CPUs, so if your local machine doesn't have that capacity, lower the numbers in the [configuration file][dqnConfig]. 
    
    If you experience out of memory problems, consider reducing the `buffer_size` parameter. 

---

## Running on AWS

This section explains how to use the RLlib integration to automatically run training and inference on AWS EC2 instances. To handle the scaling of instances we use the [Ray autoscaler API][rayAutoscaler].

[rayAutoscaler]: https://docs.ray.io/en/latest/cluster/index.html

#### Configure AWS

You will need to configure your boto3 environment correctly. Check [here][awsBoto3] for more information.

[awsBoto3]: https://boto3.amazonaws.com/v1/documentation/api/latest/guide/configuration.html

#### Create the training AMI

Use the provided [`aws_helper.py`][awsHelper] script to automatically create the image needed for training by running the command below, passing in the name of the base image and the installation script `install.sh` found in [`rllib-integration/aws/install`][installsh]:

        python3 aws_helper.py create-image --name <AMI-name> --installation-scripts <installation-scripts> --instance-type <instance-type> --volume-size <volume-size>

[awsHelper]: https://github.com/carla-simulator/rllib-integration/blob/main/aws/aws_helper.py
[installsh]: https://github.com/carla-simulator/rllib-integration/blob/main/aws/install/install.sh

#### Configure the cluster

Once the image is created, there will be an output with image information. To use the Ray autoscaler, update the `<ImageId>` and `<SecurityGroupIds>` settings in your [autoscaler configuration file][autoscalerSettings] with the information from the output.

[autoscalerSettings]: https://docs.ray.io/en/latest/cluster/config.html

#### Run the training

With the image created, you can use Ray's API to run the training on the cluster:

1. Initialize the cluster:

        ray up <autoscaler_configuration_file>

2. (Optional) If the local code has been modified after the cluster initialization, run this command to update it:

        ray rsync-up <autoscaler_configuration_file> <path_to_local_folder> <path_to_remote_folder>

3. Run the training:

        ray submit <autoscaler_configuration_file> <training_file>

4. (Optional) Monitor the cluster status:

        ray attach <autoscaler_configuration_file>
        watch -n 1 ray status

5. Shutdown the cluster:

        ray down <autoscaler_configuration_file>


#### Running the DQN example on AWS

To run the DQN example on AWS:

1. Create the image by passing the [`dqn_example/dqn_autoscaler.yaml`][dqnAutoscaler] configuration to the following command:

        python3 aws_helper.py create-image --name <AMI-name> --installation-scripts install/install.sh --instance-type <instance-type> --volume-size <volume-size>

[dqnAutoscaler]: https://github.com/carla-simulator/rllib-integration/blob/main/dqn_example/dqn_autoscaler.yaml

2. Update the `<ImageId>` and `<SecurityGroupIds>` settings in [`dqn_autoscaler.yaml`][dqnAutoscaler] with the information provided by the previous command.

3. Initialize the cluster:

        ray up dqn_example/dqn_autoscaler.yaml

4. (Optional) Update remote files with local changes:

        ray rsync-up dqn_example/dqn_autoscaler.yaml dqn_example .
        ray rsync-up dqn_example/dqn_autoscaler.yaml rllib_integration .

5. Run the training:

        ray submit dqn_example/dqn_autoscaler.yaml dqn_train.py -- dqn_example/dqn_config.yaml --auto

6. (Optional) Monitor the cluster status:

        ray attach dqn_example/dqn_autoscaler.yaml
        watch -n 1 ray status

7. Shutdown the cluster:

        ray down dqn_example/dqn_autoscaler.yaml

---

This guide has outlined how to install and run the RLlib integration on AWS and on a local machine. If you have any questions or ran into any issues working through the guide, feel free to post in the [forum](https://github.com/carla-simulator/carla/discussions/) or raise an issue on [GitHub](https://github.com/carla-simulator/rllib-integration).
