#!/usr/bin/env groovy

pipeline
{
    agent { label "gpu" }
    stages
    {
        stage('Configure')
        {
            steps {
              sh "cmake --version"
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
            cmake --build Build --target package
          }
        }
    }
}
