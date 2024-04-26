#!/usr/bin/env groovy

pipeline
{
    agent { label "gpu" }
    stages
    {
        stage('Configure')
        {
            steps {
              cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/LinuxToolchain.cmake -DLAUNCH_ARGS="-prefernvidia" -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH
            }
        }
        stage('Build')
        {
          steps {
            cmake --build Build --target carla-python-api-install
          }
        }
        stage('Package')
        {
          steps {
            sh "cmake --build Build --target package"
          }
        }
    }
}
