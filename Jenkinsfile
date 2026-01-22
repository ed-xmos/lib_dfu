@Library('xmos_jenkins_shared_library@v0.43.3') _

getApproval()

pipeline {

    agent none

    parameters {
        string(
            name: 'TOOLS_VERSION',
            defaultValue: '15.3.1',
            description: 'XTC tools version'
        )
        string(
            name: 'XMOSDOC_VERSION',
            defaultValue: 'v8.0.1',
            description: 'xmosdoc version'
        )
        string(
            name: 'INFR_APPS_VERSION',
            defaultValue: 'v3.3.0',
            description: 'The infr_apps version'
        )
        choice(
            name: 'TEST_LEVEL', choices: ['smoke', 'default', 'extended'],
            description: 'The level of test coverage to run'
        )
    }

    options {
        skipDefaultCheckout()
        timestamps()
        buildDiscarder(xmosDiscardBuildSettings(onlyArtifacts = false))
    }

    stages {
        // This is a prompt of what testing needs porting from old Jenkinsfile
        //     stage('Tests') {
        //       parallel {
        //         stage('Device simulation tests') {
        //           steps {
        //             dir("${REPO}/tests/device_simulation") {
        //               runWaf('.')
        //               viewEnv() {
        //                 runPytest()
        //               }
        //             }
        //           }
        //         }
        //         stage('Build of hardware system tests') {
        //           steps {
        //             dir("${REPO}/tests/system_hardware") {
        //               runWaf('.')
        //             }
        //           }
        //         }
        //         stage('Host tests') {
        //           steps {
        //             dir("${REPO}/tests/host") {
        //               sh 'make'
        //               viewEnv() {
        //                 runPytest()
        //               }
        //             }
        //           }
        //         }
        //       }
        //     }
        //   }

        stage('🏗️ Build and test') {
            agent {
                label 'x86_64 && linux && documentation'
            }

            stages {
                stage('Checkout') {
                    steps {

                        println "Stage running on ${env.NODE_NAME}"

                        script {
                            def (server, user, repo) = extractFromScmUrl()
                            env.REPO_NAME = repo
                        }

                        dir(REPO_NAME){
                            checkoutScmShallow()
                        }
                    }
                }

                // stage('Examples build') {
                //     steps {
                //         dir("${REPO_NAME}/examples") {
                //             xcoreBuild()
                //         }
                //     }
                // }
                  
                stage('Build host app') {
                    steps {
                        dir(REPO_NAME) {
                            dir("host") {
                                sh "cmake -B build"
                                sh "cmake --build build"
                            }
                            archiveArtifacts artifacts: "host/suffix_generator/bin/dfu_suffix_generator", fingerprint: true
                            archiveArtifacts artifacts: "host/libsuffix_verifier/lib/libsuffix_verifier.a", fingerprint: true
                        }
                    }
                }
                
                stage('Repo checks') {
                    steps {
                        warnError("Repo checks failed")
                        {
                            runRepoChecks("${WORKSPACE}/${REPO_NAME}")
                        }
                    }
                }

                stage('Doc build') {
                    steps {
                        dir(REPO_NAME) {
                            buildDocs()
                        }
                    }
                }

                stage('Tests') {
                    steps {
                        dir("${REPO_NAME}/tests") {
                            withTools(params.TOOLS_VERSION) {
                                createVenv(reqFile: "requirements.txt")
                                withVenv {
                                    // Host tests
                                    dir("host") {
                                        sh "cmake -B build"
                                        sh "cmake --build build"
                                    }
                                    // xcoreBuild(archiveBins: false)
                                    // Use the TEST_LEVEL parameter to control the test coverage
                                    runPytest("--level=${params.TEST_LEVEL} -k host")
                                }
                            }
                        }
                    }
                }
            }
            post {
                cleanup {
                    xcoreCleanSandbox()
                }
            }
        } // stage 'Build and test'
        
        stage('🚀 Release') {
            when {
                expression { triggerRelease.isReleasable() }
            }
            steps {
                triggerRelease()
            }
        } // stage "build and test"
    }
}
