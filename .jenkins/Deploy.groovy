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
                                    sh "rm -rf build"
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
                                                  -DJRM_ENABLE_UAT=ON \
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
                                                -DJRM_DISABLE_TESTS=ON ^
                                                -DJRM_ENABLE_UAT=ON

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

        /*stage('Deploy (Prepare Release Bundle)') {
            agent { label 'Linux' }

            steps {
                script {
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
                    #!/bin/bash

                    cd release
                    ls -lhat
                    tar -xf *.tar.gz
                    unzip *.zip
                    ls -lhat

                    rm -rf *.zip
                    rm -rf *.tar.gz
                    ls -lhat

                    cat <<EOF > notes.md
# Release ${./jrm --version | grep "jrm (JustReflectMe)" | sed -E "s/jrm \(\w+\)\s+//"}

# Tested Env:
- Win11 | MSVC 2022
- Debian Linux | GCC 15.2 | Clang 21.1.6

## Stability:
- [![MSVC Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FWinBuild_MSVC_Debug%2F&label=MSVC%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/WinBuild_MSVC_Debug/replace_it_with_your_number/) [![MSVC Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FWinBuild_MSVC_Release%2F&label=MSVC%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/WinBuild_MSVC_Release/replace_it_with_your_number/)
- [![GCC Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_GCC_Debug%2F&label=GCC%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_GCC_Debug/replace_it_with_your_number/) [![GCC Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_GCC_Release%2F&label=GCC%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_GCC_Release/replace_it_with_your_number/)
- [![Clang Debug](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_Clang_Debug%2F&label=Clang%20Debug)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_Clang_Debug/replace_it_with_your_number/) [![Clang Release](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_Clang_Release%2F&label=Clang%20Release)](https://jenkins.vakon.space/job/JustReflectMe/job/LinuxBuild_Clang_Release/replace_it_with_your_number/)
- [![StaticCodeAnalysis](https://img.shields.io/jenkins/build?jobUrl=https%3A%2F%2Fjenkins.vakon.space%2Fjob%2FJustReflectMe%2Fjob%2FLinuxBuild_Clang_Debug%2F&label=Static%20Code%20Analysis)](https://jenkins.vakon.space/job/JustReflectMe/job/StaticCodeAnalysis/replace_it_with_your_number/)
Built at: \$(date -u)
                    EOF
                    """


                    sh "ls -R release"
                }

                archiveArtifacts artifacts: "release/**", fingerprint: true
            }
        }*/

    }
}
