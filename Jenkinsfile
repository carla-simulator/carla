#!/usr/bin/env groovy

pipeline
{
    agent { label "gpu" }
    stages
    {
      parallel
      {
        stage('Linux')
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
        stage('Windows')
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
