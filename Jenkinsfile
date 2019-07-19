pipeline {
    agent any

    environment {
        UE4_ROOT = '/var/lib/jenkins/UnrealEngine_4.22'
    }

    options {
        buildDiscarder(logRotator(numToKeepStr: '3', artifactNumToKeepStr: '3'))
    }

    stages {

        stage('Docs') {
            steps {
                sh 'make docs'
                sh 'mv Doxygen/ ~/carla-simulator.github.io'
                sh 'cd ~/carla-simulator.github.io'
                sh 'git add .'
                sh 'git commit -m "Updated c++ docs"'
                sh 'git push'
                sh 'cd ~/carla'
            }
        }

        stage('Deploy') {
            when { anyOf { branch "master"; buildingTag() } }
            steps {
                sh 'make deploy ARGS="--replace-latest --docker-push"'
            }
        }
    }

    post {
        always {
            deleteDir()
        }
    }
}
