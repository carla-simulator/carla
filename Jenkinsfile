#!/usr/bin/env groovy

pipeline
{
    agent { label "gpu" }
    stages
    {
      stage("Platforms")
      {
        parallel
        {
          stage('Linux')
          {
            stages {
              stage('Configure')
              {
                  steps {
                    sh "nvidia-smi"
                  }
              }
              stage('Build')
              {
                steps {
                  sh "vulkaninfo"
                }
              }
              stage('Package')
              {
                steps {
                  sh "cmake --version"
                }
              }
            }
          }
          stage('Windows')
          {
            stages {
              stage('Configure')
              {
                  steps {
                    sh "cmake --version"
                  }
              }
              stage('Build')
              {
                steps {
                  sh "cmake --version"
                }
              }
              stage('Package')
              {
                steps {
                  sh "cmake --version"
                }
              }
            }
          }
        }
      }
    }
}
