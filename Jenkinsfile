pipeline {
    agent none

    environment
    {
        UE4_ROOT = '/home/jenkins/UnrealEngine_4.22'
    }

    options
    {
        buildDiscarder(logRotator(numToKeepStr: '3', artifactNumToKeepStr: '3'))
    }

    stages
    {

        stage('Setup')
        {
            agent { label 'build' }
            steps
            {
                sh 'make setup'
            }
        }

        stage('Build')
        {
            agent { label 'build' }
            steps
            {
                sh 'make LibCarla'
                sh 'make PythonAPI'
                sh 'make CarlaUE4Editor'
                sh 'make examples'
            }
            post
            {
                always
                {
                    archiveArtifacts 'PythonAPI/carla/dist/*.egg'
                    stash includes: 'PythonAPI/carla/dist/*.egg', name: 'eggs'
                }
            }
        }

        stage('Unit Tests')
        {
            agent { label 'build' }
            steps
            {
                sh 'make check ARGS="--all --xml"'
            }
            post
            {
                always
                {
                    junit 'Build/test-results/*.xml'
                    archiveArtifacts 'profiler.csv'
                }
            }
        }

        stage('Retrieve Content')
        {
            agent { label 'build' }
            steps
            {
                sh './Update.sh'
            }
        }

        stage('Package')
        {
            agent { label 'build' }
            steps
            {
                sh 'make package'
                sh 'make package ARGS="--packages=AdditionalMaps --clean-intermediate"'
                sh 'make examples ARGS="localhost 3654"'
            }
            post {
                always {
                    archiveArtifacts 'Dist/*.tar.gz'
                    stash includes: 'Dist/CARLA*.tar.gz', name: 'carla_package'
                    stash includes: 'Examples/', name: 'examples'
                }
            }
        }

        stage('Smoke Tests')
        {
            agent { label 'gpu' }
            steps
            {
                unstash name: 'eggs'
                unstash name: 'carla_package'
                sh 'tar -xvzf Dist/CARLA*.tar.gz -C Dist/'
                sh 'DISPLAY= ./Dist/CarlaUE4.sh -opengl --carla-rpc-port=3654 --carla-streaming-port=0 -nosound > CarlaUE4.log &'
                sh 'make smoke_tests ARGS="--xml"'
                unstash name: 'examples'
                sh 'make run-examples ARGS="localhost 3654"'
            }
            post
            {
                always
                {
                    archiveArtifacts 'CarlaUE4.log'
                    junit 'Build/test-results/smoke-tests-*.xml'
                }
            }
        }

        stage('Deploy')
        {
            agent { label 'build' }
            when { anyOf { branch "master"; buildingTag() } }
            steps
            {
                sh 'git checkout .'
                sh 'make deploy ARGS="--replace-latest --docker-push"'
            }
        }

        stage('Doxygen')
        {
            agent { label 'build' }
            when { anyOf { branch "master"; buildingTag() } }
            steps
            {
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

    // post {
    //     always {
    //         deleteDir()
    //     }
    // }

}
