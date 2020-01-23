@Library('xmos_jenkins_shared_library@develop') _

getApproval()

pipeline {
  agent {
    label 'x86_64&&macOS'
  }
  environment {
    REPO = 'lib_dfu'
    VIEW = "${env.JOB_NAME.contains('PR-') ? REPO+'_'+env.CHANGE_TARGET : REPO+'_'+env.BRANCH_NAME}"
  }
  options {
    skipDefaultCheckout()
  }
  stages {
    stage('Get view') {
      steps {
        xcorePrepareSandbox("${VIEW}", "${REPO}")
      }
    }
    stage('Library checks') {
      steps {
        xcoreLibraryChecks("${REPO}")
      }
    }
    stage('xCORE builds') {
      steps {
        dir("${REPO}") {
          dir("${REPO}") {
            runXdoc('doc')
          }
        }
      }
    }
    stage('Device simulation tests') {
      steps {
        dir("${REPO}/tests/device_simulation") {
          runWaf('.')
          viewEnv() {
            runPytest()
          }
        }
      }
    }
    stage('Build of hardware system tests') {
      steps {
        dir("${REPO}/tests/system_hardware") {
          runWaf('.')
        }
      }
    }
    stage('Host tests') {
      steps {
        dir("${REPO}/tests/host") {
          sh 'make'
          viewEnv() {
            runPytest()
          }
        }
      }
    }
  }
  post {
    success {
      updateViewfiles()
    }
    cleanup {
      xcoreCleanSandbox()
    }
  }
}
