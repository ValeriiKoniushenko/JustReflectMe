pipeline {
    agent none

    environment {
        BUILD_TYPE = 'Release'
    }

    stages {

        stage('Build Matrix') {
            parallel {

                stage('Linux') {
                    agent { label 'Linux' }

                    environment {
                        C_COMPILER  = 'clang'
                        CPP_COMPILER = 'clang++'
                    }

                    stages {

                        stage('Prepare') {
                            steps {
                                script {
                                    def isTriggeredByCron =
                                        currentBuild.getBuildCauses(
                                            'hudson.triggers.TimerTrigger$TimerTriggerCause'
                                        )

                                    if (isTriggeredByCron) {
                                        sh "rm -rf build"
                                    }
                                }
                            }
                        }

                        stage('Build') {
                            steps {
                                script {
                                    def buildDir = "build/${C_COMPILER}/${BUILD_TYPE}"
                                    retry(2) {
                                        sh """
                                            rm -rf ${buildDir}

                                            cmake -S . -B ${buildDir} \
                                                  -DCMAKE_C_COMPILER=${C_COMPILER} \
                                                  -DCMAKE_CXX_COMPILER=${CPP_COMPILER} \
                                                  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
                                                  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
                                                  -DJRM_DISABLE_TESTS=ON

                                            cmake --build ${buildDir} -- -j4
                                        """
                                    }
                                }
                            }
                        }

                        stage('Package') {
                            steps {
                                script {
                                    def buildPath = "build/${C_COMPILER}/${BUILD_TYPE}"
                                    def archiveName = "Linux-${BUILD_TYPE}.tar.gz"

                                    sh """
                                        rm -f ${archiveName}
                                        tar czf ${archiveName} ${buildPath}/bin/*
                                    """

                                    archiveArtifacts artifacts: archiveName, fingerprint: true
                                }
                            }
                        }
                    }
                }

                stage('Windows') {
                    agent { label 'Windows' }

                    stages {

                        stage('Prepare') {
                            steps {
                                script {
                                    def isTriggeredByCron =
                                        currentBuild.getBuildCauses(
                                            'hudson.triggers.TimerTrigger$TimerTriggerCause'
                                        )

                                    if (isTriggeredByCron) {
                                        bat "IF EXIST build rmdir /S /Q build"
                                    }
                                }
                            }
                        }

                        stage('Build') {
                            steps {
                                script {
                                    retry(2) {
                                        bat """
                                            IF EXIST build rmdir /S /Q build

                                            cmake -S . -B build ^
                                                -G "Visual Studio 17 2022" ^
                                                -A x64 ^
                                                -DJRM_DISABLE_TESTS=ON

                                            cmake --build build --config %BUILD_TYPE% -- /m:2
                                        """
                                    }
                                }
                            }
                        }

                        stage('Package') {
                            steps {
                                script {
                                    def archiveName = "${BUILD_TYPE}-Win64.zip"

                                    bat """
                                        if exist ${archiveName} del /Q ${archiveName}
                                        powershell Compress-Archive ^
                                            -Path build\\bin\\%BUILD_TYPE%\\* ^
                                            -DestinationPath ${archiveName}
                                    """

                                    archiveArtifacts artifacts: archiveName, fingerprint: true
                                }
                            }
                        }
                    }
                }
            }
        }

        stage('Deploy (Prepare Release Bundle)') {
            agent { label 'Linux' }

            // when {
            //     buildingTag()
            // }

            steps {
                script {
                    echo "Preparing release bundle for tag: ${env.TAG_NAME}"

                    sh "rm -rf release"
                }

                copyArtifacts(
                    projectName: env.JOB_NAME,
                    selector: specific("${env.BUILD_NUMBER}"),
                    filter: "*.tar.gz, *.zip",
                    target: "release/"
                )

                script {
                    // Generate build metadata
                    sh """
                        cat <<EOF > release/BUILD_INFO.txt
                        Tag: ${env.TAG_NAME}
                        Built at: $(date -u)
                        EOF
                    """

                    sh "ls -R release"
                }

                archiveArtifacts artifacts: "release/**", fingerprint: true
            }
        }

    }
}
