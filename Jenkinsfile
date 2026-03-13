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
            defaultValue: 'v3.2.1',
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
        stage('Init') {
            agent { label 'x86_64 && linux' }
            steps {
                script {
                    def (server, user, repo) = extractFromScmUrl()
                    env.REPO_NAME = repo
                }
            }
        }
        stage('Parallel Stages') {
            parallel {
                stage('🏗️ Build and docs') {
                    agent {
                        label 'x86_64 && linux && documentation'
                    }

                    stages {
                        stage('Checkout') {
                            steps {

                                println "Stage running on ${env.NODE_NAME}"

                                dir(REPO_NAME){
                                    checkoutScmShallow()
                                }
                            }
                        }

                        stage('Examples build') {
                            steps {
                                dir("${REPO_NAME}/examples") {
                                    dir("i2c/device") {
                                        xcoreBuild()
                                    }
                                    dir("i2c/host_xcore") {
                                        xcoreBuild()
                                    }
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

                        stage("Archive sandbox") {
                            steps {
                                archiveSandbox(REPO_NAME)
                            }
                        }
                    }
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                } // stage 'Build and docs'

                stage('🔧 Sim tests') {
                    agent {
                        label 'x86_64 && linux'
                    }
                    stages {
                        stage('Checkout') {
                            steps {
                                dir(REPO_NAME) {
                                    checkoutScmShallow()
                                }
                            }
                        }

                        stage('Build Linux host apps') {
                            steps {
                                dir(REPO_NAME) {
                                    dir("host") {
                                        sh "cmake -B build"
                                        sh "cmake --build build"

                                    }
                                    archiveArtifacts artifacts: "host/suffix_generator/bin/dfu_suffix_generator", fingerprint: true
                                    archiveArtifacts artifacts: "host/libsuffix_verifier/lib/libsuffix_verifier.a", fingerprint: true
                                    archiveArtifacts artifacts: "host/xmosdfu/bin/xmosdfu", fingerprint: true
                                }
                            }
                        }

                        stage('Run sim tests') {
                            steps {
                                dir("${REPO_NAME}/tests") {
                                    withTools(params.TOOLS_VERSION) {
                                        createVenv(reqFile: "requirements.txt")
                                        withVenv {
                                            dir("dummy") {
                                                xcoreBuild(archiveBins: false)
                                            }
                                            dir("host") {
                                                sh "cmake -B build"
                                                sh "cmake --build build"
                                                runPytest("--level=${params.TEST_LEVEL}")
                                            }
                                            dir("device_simulation/dfu") {
                                                runPytest("--level=${params.TEST_LEVEL}")
                                            }
                                            dir("device_simulation/modules") {
                                                runPytest("--level=${params.TEST_LEVEL}")
                                            }
                                            dir("device_simulation/dfu_flash") {
                                                runPytest("--level=${params.TEST_LEVEL}")
                                            }
                                            dir("device_simulation/dfu_dnload") {
                                                runPytest("--level=${params.TEST_LEVEL}")
                                            }
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
                }  // Sim tests

                stage('Build Mac x86 host app') {
                    agent {
                        label 'x86_64 && macOS'
                    }
                    steps {
                        println "Stage running on ${env.NODE_NAME}"

                        dir(REPO_NAME) {
                            checkoutScmShallow()
                            dir("host") {
                                    sh "cmake -B build"
                                    sh "cmake --build build"

                                }
                                archiveArtifacts artifacts: "host/suffix_generator/bin/dfu_suffix_generator", fingerprint: true
                                archiveArtifacts artifacts: "host/libsuffix_verifier/lib/libsuffix_verifier.a", fingerprint: true
                                archiveArtifacts artifacts: "host/xmosdfu/bin/xmosdfu", fingerprint: true
                        }
                    }
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                }  // Build Mac x86 host app

                stage('Build Mac arm host app') {
                    agent {
                        label 'arm64 && macOS'
                    }
                    steps {
                        println "Stage running on ${env.NODE_NAME}"

                        dir(REPO_NAME) {
                            checkoutScmShallow()
                            dir("host") {
                                sh "cmake -B build"
                                sh "cmake --build build"

                            }
                            archiveArtifacts artifacts: "host/suffix_generator/bin/dfu_suffix_generator", fingerprint: true
                            archiveArtifacts artifacts: "host/libsuffix_verifier/lib/libsuffix_verifier.a", fingerprint: true
                            archiveArtifacts artifacts: "host/xmosdfu/bin/xmosdfu", fingerprint: true
                        }
                    }
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                }  // Build Mac arm host app

                stage('Build Pi host app') {
                    agent {
                        label 'pi'
                    }
                    steps {
                        println "Stage running on ${env.NODE_NAME}"

                        // Bring in device control code to test the I2C host app on RPi
                        // sh 'git clone --depth 1 git@github.com:xmos/lib_device_control.git'
                        // TODO - return to the above...
                        sh 'git clone --depth 1 -b feature/dfu-testing git@github.com:humphrey-xmos/lib_device_control.git'

                        dir(REPO_NAME) {
                            checkoutScmShallow()
                            dir("host") {
                                sh "cmake -B build"
                                sh "cmake --build build"

                            }
                            archiveArtifacts artifacts: "host/suffix_generator/bin/dfu_suffix_generator", fingerprint: true
                            archiveArtifacts artifacts: "host/libsuffix_verifier/lib/libsuffix_verifier.a", fingerprint: true
                            archiveArtifacts artifacts: "host/xmosdfu/bin/xmosdfu", fingerprint: true
                        }
                    }
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                }  // Build Pi host app

                stage('Build Windows host app') {
                    agent {
                        label 'x86_64 && windows && usb_audio'
                    }
                    steps {
                        println "Stage running on ${env.NODE_NAME}"

                        dir(REPO_NAME) {
                            checkoutScmShallow()
                            withVS() {
                                dir("host") {
                                    sh "cmake -B build"
                                    sh "cmake --build build"

                                }
                                archiveArtifacts artifacts: "host/suffix_generator/bin/dfu_suffix_generator", fingerprint: true
                                archiveArtifacts artifacts: "host/libsuffix_verifier/lib/libsuffix_verifier.a", fingerprint: true
                                archiveArtifacts artifacts: "host/xmosdfu/bin/xmosdfu", fingerprint: true
                            } // withVS()
                        }
                    }
                    post {
                        cleanup {
                            xcoreCleanSandbox()
                        }
                    }
                }  // Build Windows host app

                stage('🔧 I2C HW Tests') {
                    agent {
                        label 'xvf3610_int'
                    }

                    stages {
                        stage('Checkout') {
                            steps {
                                println "Stage running on ${env.NODE_NAME}"

                                dir(REPO_NAME){
                                    checkoutScmShallow()
                                    // Get dependencies (lib_device_control) for the I2C host tests on RPi and build example for test
                                    dir("examples/i2c/device") {
                                        withTools(params.TOOLS_VERSION) {
                                            xcoreBuild()
                                        }
                                    }
                                }
                            }
                        }
                        stage('I2C DFU tests') {
                            steps {
                                dir ("${REPO_NAME}/tests") {
                                    withTools(params.TOOLS_VERSION) {
                                        createVenv(reqFile: "requirements.txt")
                                        withVenv {
                                            dir("i2c_rpi_hardware") {
                                                withXTAG(["XVF3610_INT"]) {  xtagIds ->
                                                    sh "echo ${xtagIds}"
                                                    runPytest("-n=1 --adapter-id ${xtagIds[0]}")
                                                }
                                            }
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
                } // stage "🔧 I2C HW Tests"

                stage('🔧 Hardware Tests') {
                    agent {
                        label 'sw-hw-xcai-exp0 || sw-hw-xcai-exp1 || sw-hw-xcai-exp2 || sw-hw-xcai-exp3'
                    }
                    stages {
                        stage('Checkout') {
                            steps {
                                println "Stage running on ${env.NODE_NAME}"
                                dir(REPO_NAME){
                                    checkoutScmShallow()
                                }
                            }
                        }
                        stage('System Hardware Test') {
                            steps {
                                dir ("${REPO_NAME}/tests") {
                                    withTools(params.TOOLS_VERSION) {
                                        createVenv(reqFile: "requirements.txt")
                                        withVenv {
                                            dir("dummy") {
                                                xcoreBuild(archiveBins: false)
                                            }
                                            dir("flash_hardware/prepare_upgrade_slot") {
                                                withXTAG(["XCORE-AI-EXPLORER"]) {
                                                    xtagIds -> runPytest("-n=1 --adapter-id ${xtagIds[0]}")
                                                }
                                            }
                                            dir("system_hardware/write_upgrade_boot_only") {
                                                withXTAG(["XCORE-AI-EXPLORER"]) {
                                                    xtagIds -> runPytest("-n=1 --adapter-id ${xtagIds[0]}")
                                                }
                                            }
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
                } // stage "🔧 Hardware Tests"
            } // parallel
        } // stage Parallel Stages

        stage(' Release') {
            when {
                expression { triggerRelease.isReleasable() }
            }
            steps {
                triggerRelease()
            }
        } // stage "build and test"
    }
}
