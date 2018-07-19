pipeline {
    agent any

    environment {
        UE4_ROOT = '/var/lib/jenkins/UnrealEngine_4.20'
    }

    options {
        buildDiscarder(logRotator(numToKeepStr: '3', artifactNumToKeepStr: '3'))
    }

    stages {

        stage('Setup') {
            steps {
                sh 'make setup'
                sh './Update.sh'
            }
        }

        stage('Build') {
            steps {
                sh 'make LibCarla'
                sh 'make PythonAPI'
                sh 'make CarlaUE4Editor'
            }
        }

        stage('Unit Tests') {
            steps {
                sh 'make check ARGS="--all --xml"'
            }
            post {
                always {
                    junit 'Build/test-results/*.xml'
                    archiveArtifacts 'profiler.csv'
                }
            }
        }

        stage('Package') {
            steps {
                sh 'make package'
            }
            post {
                always {
                    archiveArtifacts 'Dist/*.tar.gz'
                }
            }
        }

    }

    post {
        always {
            deleteDir()
        }
    }
}
