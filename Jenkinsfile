pipeline
{
    stages
    {
        stage('Ubuntu')
        {
            agent { label "ubuntu_22_04" }
            stages {
                stage('Configure')
                {
                    steps {
                        sh "cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/LinuxToolchain.cmake -DLAUNCH_ARGS=-prefernvidia -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON -DBUILD_CARLA_UNREAL=ON -DCARLA_UNREAL_ENGINE_PATH=$CARLA_UNREAL_ENGINE_PATH"
                    }
                }
                stage('Build Python API')
                {
                    steps {
                        sh "cmake --build Build --target package"
                    }
                }
                stage('Build CARLA')
                {
                    steps {
                        sh "cmake --build Build --target carla-unreal"
                    }
                }
                stage('Package')
                {
                    steps {
                        sh "cmake --build Build --target package"
                    }
                }
                // TODO: move these stages to carla-studio as its own Jenkinsfile
                stage('Prepare CarlaStudio')
                {
                    steps {
                        sh """
                            # Clone carla-studio if not already present
                            if [ ! -d "carla-studio" ]; then
                                echo "Cloning carla-studio repository..."
                                git clone https://github.com/carla-simulator/carla-studio.git carla-studio
                            else
                                echo "carla-studio already present, updating..."
                                cd carla-studio
                                git fetch origin
                                git checkout master
                                cd ..
                            fi
                        """
                    }
                }
                stage('Build CarlaStudio')
                {
                    steps {
                        sh """
                            if [ -d "carla-studio" ] && [ -f "carla-studio/Makefile" ]; then
                                echo "Building CarlaStudio..."
                                cd carla-studio
                                make CARLA_DIR="${WORKSPACE}" CARLA_BUILD_DIR="${WORKSPACE}/Build"
                                cd ..
                                echo "CarlaStudio build completed successfully"
                            else
                                echo "ERROR: carla-studio checkout failed or Makefile not found"
                                exit 1
                            fi
                        """
                    }
                }
            }
        }
    }
}
