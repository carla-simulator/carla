pipeline {
    agent any

    environment {
        UE4_ROOT = '/var/lib/jenkins/UnrealEngine_4.21'
    }

    options {
        buildDiscarder(logRotator(numToKeepStr: '3', artifactNumToKeepStr: '3'))
    }

    stages {

        stage('Setup') {
            steps {
                sh 'make setup'
            }
        }

        stage('Build') {
            steps {
                sh 'make LibCarla'
                sh 'make PythonAPI'
                sh 'make CarlaUE4Editor'
            }
            post {
                always {
                    archiveArtifacts 'PythonAPI/dist/*.egg'
                }
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

        stage('Retrieve Content') {
            steps {
                sh './Update.sh'
            }
        }

        stage('Package') {
            steps {
                sh 'make package'
                sh 'make export-maps ARGS="--map=/Game/Carla/Maps/Town06 --file=Town06"'
            }
            post {
                always {
                    archiveArtifacts 'Dist/*.tar.gz'
                    archiveArtifacts 'ExportedMaps/*.tar.gz'
                }
            }
        }

        stage('Smoke Tests') {
            steps {
                sh 'DISPLAY= ./Dist/*/LinuxNoEditor/CarlaUE4.sh --carla-rpc-port=3654 --carla-streaming-port=0 > CarlaUE4.log &'
                sh 'make smoke_tests ARGS="--xml"'
            }
            post {
                always {
                    archiveArtifacts 'CarlaUE4.log'
                    junit 'Build/test-results/smoke-tests-*.xml'
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
