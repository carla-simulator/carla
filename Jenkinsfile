pipeline {
    agent any

    environment {
        UE4_ROOT = '/var/lib/jenkins/UnrealEngine_4.18'
    }

    stages {

        stage('Setup') {
            steps {
                sh './Setup.sh'
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
        success {
            archiveArtifacts 'Dist/*.tar.gz'
        }
    }
}
