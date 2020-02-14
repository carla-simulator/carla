pipeline
{
    agent none

    options
    {
        buildDiscarder(logRotator(numToKeepStr: '3', artifactNumToKeepStr: '3'))
    }

    stages
    {
        stage('windows and ubuntu in parallel')
        {
            parallel
            {
                stage('ubuntu')
                {
                    environment
                    {
                        UE4_ROOT = '/home/jenkins/UnrealEngine_4.22'
                    }
                    stages
                    {
                        stage('ubuntu setup')
                        {
                            agent { label '2ubuntu && build' }
                            steps
                            {
                                sh 'make setup'
                            }
                        }
                        stage('ubuntu build')
                        {
                            agent { label 'ubuntu && build' }
                            steps
                            {
                                sh 'make LibCarla'
                                sh 'make PythonAPI'
                                lock('ubuntu_build')
                                {
                                    sh 'make CarlaUE4Editor'
                                }
                                sh 'make examples'
                            }
                            post
                            {
                                always
                                {
                                    archiveArtifacts 'PythonAPI/carla/dist/*.egg'
                                    stash includes: 'PythonAPI/carla/dist/*.egg', name: 'ubuntu_eggs'
                                }
                            }
                        }
                        stage('ubuntu unit tests')
                        {
                            agent { label 'ubuntu && build' }
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
                        stage('ubuntu retrieve content')
                        {
                            agent { label 'ubuntu && build' }
                            steps
                            {
                                sh './Update.sh'
                            }
                        }
                        stage('ubuntu package')
                        {
                            agent { label 'ubuntu && build' }
                            steps
                            {
                                sh 'make package'
                                sh 'make package ARGS="--packages=AdditionalMaps --clean-intermediate"'
                                sh 'make examples ARGS="localhost 3654"'
                            }
                            post {
                                always {
                                    archiveArtifacts 'Dist/*.tar.gz'
                                    stash includes: 'Dist/CARLA*.tar.gz', name: 'ubuntu_package'
                                    stash includes: 'Examples/', name: 'ubuntu_examples'
                                }
                            }
                        }
                        stage('ubuntu smoke tests')
                        {
                            agent { label 'ubuntu && gpu' }
                            steps
                            {
                                unstash name: 'ubuntu_eggs'
                                unstash name: 'ubuntu_package'
                                unstash name: 'ubuntu_examples'
                                sh 'tar -xvzf Dist/CARLA*.tar.gz -C Dist/'
                                lock('ubuntu_gpu')
                                {
                                    sh 'DISPLAY= ./Dist/CarlaUE4.sh -opengl --carla-rpc-port=3654 --carla-streaming-port=0 -nosound > CarlaUE4.log &'
                                    sh 'make smoke_tests ARGS="--xml"'
                                    sh 'make run-examples ARGS="localhost 3654"'
                                }
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
                        stage('ubuntu deploy')
                        {
                            agent { label 'ubuntu && build' }
                            when { anyOf { branch "master"; buildingTag() } }
                            steps
                            {
                                sh 'git checkout .'
                                sh 'make deploy ARGS="--replace-latest --docker-push"'
                            }
                        }
                        stage('ubuntu Doxygen')
                        {
                            agent { label 'ubuntu && build' }
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
                }
                stage('windows')
                {
                    environment
                    {
                        UE4_ROOT = 'C:\\Program Files\\Epic Games\\UE_4.22'
                    }
                    stages
                    {
                        stage('windows setup')
                        {
                            agent { label 'windows && build' }
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    make setup
                                """
                            }
                        }
                        stage('windows build')
                        {
                            agent { label 'windows && build' }
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    make setup
                                    make LibCarla
                                    make PythonAPI
                                """
                                lock('windows_build')
                                {
                                    bat """
                                        call ../setEnv64.bat
                                        make CarlaUE4Editor
                                    """
                                }
                                bat """
                                    call ../setEnv64.bat
                                    make examples
                                """
                            }
                            post
                            {
                                always
                                {
                                    archiveArtifacts 'PythonAPI/carla/dist/*.egg'
                                    stash includes: 'PythonAPI/carla/dist/*.egg', name: 'windows_eggs'
                                }
                            }
                        }
                        // stage('windows unit tests')
                        // {
                        //     agent { label 'windows && build' }
                        //     steps { bat 'rem Not Implemented'}
                        // }
                        stage('windows retrieve content')
                        {
                            agent { label 'windows && build' }
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    call Update.bat
                                """
                            }
                        }
                        stage('windows package')
                        {
                            agent { label 'windows && build' }
                            steps
                            {
                                bat """
                                    call ../setEnv64.bat
                                    make package
                                    rem make package ARGS="--packages=AdditionalMaps --clean-intermediate"
                                    rem make examples ARGS="localhost 3654"
                                """
                            }
                            post {
                                always {
                                    archiveArtifacts 'Dist/*.zip'
                                    stash includes: 'Dist/CARLA*.zip', name: 'windows_package'
                                    // stash includes: 'Examples/', name: 'windows_examples'
                                }
                            }
                        }
                        // stage('windows smoke test')
                        // {
                        //     agent { label 'windows && build' }
                        //     steps { bat 'rem Not Implemented'}
                        // }
                        // stage('windows deploy')
                        // {
                        //     agent { label 'windows && build' }
                        //     steps { bat 'rem Not Implemented'}
                        // }
                    }
                }
            }
        }
    }

    post
    {
        always
        {
            node('build')
            {
                deleteDir()
            }
            node('gpu')
            {
                deleteDir()
            }
        }
    }
}
