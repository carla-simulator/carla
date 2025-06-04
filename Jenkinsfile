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
            }
        }
    }
}
