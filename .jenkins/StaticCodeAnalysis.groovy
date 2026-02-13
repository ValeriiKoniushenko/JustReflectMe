pipeline {
    agent { label 'Linux' }

    stages {
        stage('Static Code Analysis') {
            steps {
                script {
                    sh '''
                        cppcheck --enable=all \
                            --check-level=exhaustive \
                            --suppress=functionStatic \
                            --suppress=useStlAlgorithm \
                            --suppress=missingIncludeSystem \
                            --suppress=unusedFunction \
                            --suppress=missingInclude \
                            --suppress=noExplicitConstructor \
                            --xml --xml-version=2 sources/ 2> cppcheck.xml
                    '''
                }
            }
        }
    }
    post {
        always {
            recordIssues(
                enabledForFailure: true,
                tools: [
                    cppCheck(pattern: 'cppcheck.xml')
                ]
            )
        }
    }
}
