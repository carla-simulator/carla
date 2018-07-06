pipeline {
    agent any

    environment {
        UE4_ROOT = '/var/lib/jenkins/UnrealEngine_4.19'
    }

    options {
        buildDiscarder(logRotator(numToKeepStr: '6', artifactNumToKeepStr: '6'))
    }

    stages {

        stage('Setup') {
            steps {
                sh 'make setup'
            }
        }

        stage('Download Content') {
            steps {
                sh './Update.sh'
            }
        }

        stage('LibCarla') {
            steps {
                sh 'make LibCarla'
            }
        }

        stage('PythonAPI') {
            steps {
                sh 'make PythonAPI'
            }
        }

        stage('Unit Tests') {
            steps {
                sh 'make check'
            }
        }

        stage('CarlaUE4Editor') {
            steps {
                sh 'make CarlaUE4Editor'
            }
        }

        stage('Package') {
            steps {
                sh 'make package'
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
