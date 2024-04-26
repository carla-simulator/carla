#!/usr/bin/env groovy

pipeline
{
    agent { label "gpu" }
    stages
    {
        stage('Configure')
        {
            steps {
              sh "ls"
            }
        }
        stage('Launch')
        {
          parallel {
            stage('Test On Windows'){
              steps {
                sh "ls"
              }
            }
            stage('Test On MacOS'){
              steps {
                sh "ls"
              }
            }
            stage('Test On Linux'){
              steps {
                sh "ls"
              }
            }
          }
        }
    }
}
