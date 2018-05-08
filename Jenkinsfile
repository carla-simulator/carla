pipeline {
    agent any

    environment {
        UE4_ROOT = '/var/lib/jenkins/UnrealEngine_4.18'
    }

    options {
        buildDiscarder(logRotator(numToKeepStr: '6', artifactNumToKeepStr: '6'))
    }

    stages {

        stage('Setup') {
            steps {
                sh './Setup.sh --jobs=12'
            }
        }

        stage('Build') {
            steps {
                sh './Rebuild.sh --no-editor'
            }
        }

        stage('Package') {
            steps {
                sh './Package.sh --clean-intermediate'
            }
        }

    }

    post {

        always {
            archiveArtifacts 'Dist/*.tar.gz'
            deleteDir()
        }

    }
}
