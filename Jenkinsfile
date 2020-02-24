pipeline {
    agent any

    environment {
        UE4_ROOT = '/var/lib/jenkins/UnrealEngine_4.22'
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
                sh 'make examples'
            }
            post {
                always {
                    archiveArtifacts 'PythonAPI/carla/dist/*.egg'
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
                sh 'make package ARGS="--packages=AdditionalMaps --clean-intermediate"'
            }
            post {
                always {
                    archiveArtifacts 'Dist/*.tar.gz'
                }
            }
        }

        stage('Smoke Tests') {
            steps {
                sh 'DISPLAY= ./Dist/*/LinuxNoEditor/CarlaUE4.sh -opengl --carla-rpc-port=3654 --carla-streaming-port=0 -nosound > CarlaUE4.log &'
                sh 'make smoke_tests ARGS="--xml"'
                sh 'make run-examples ARGS="localhost 3654"'
            }
            post {
                always {
                    archiveArtifacts 'CarlaUE4.log'
                    junit 'Build/test-results/smoke-tests-*.xml'
                }
            }
        }

        stage('Deploy') {
            when { anyOf { branch "master"; buildingTag() } }
            steps {
                sh 'git checkout .'
                sh 'make deploy ARGS="--replace-latest --docker-push"'
            }
        }

        stage('Doxygen') {
            when { anyOf { branch "master"; buildingTag() } }
            steps {
                sh 'make docs'
                sh 'rm -rf ~/carla-simulator.github.io/Doxygen'
                sh 'cp -rf ./Doxygen ~/carla-simulator.github.io/'
                sh 'cd ~/carla-simulator.github.io && \
                    git pull && \
                    git add Doxygen && \
                    git commit -m "Updated c++ docs" || true && \
                    git push'
            }
        }
    }

    post {
        always {
            deleteDir()
        }
    }
}
