/*
 * Foleys Finest Jenkinsfile
 *
 * Build server setup:
 *
 *  Pipeline Project
 *   - Pipeline script from SCM
 *   - Advanced clone Behaviours
 *     - Fetch tags
 *     - Shallow clone (depth 1)
 *   - Advanced sub-modules behaviours
 *     - Recursively update submodules
 *     - Use credentials from default remote of parent repository
 *   - Clean before checkout
 *
 *  Environment Variables
 *   - BUILD_SERVER_PLATFORM (mac|windows)
 *   - KEYCHAIN_PASSWORD (backquote $)
 *
 *  Mac only
 *   - APP_SIGNATURE  (8FRDEFHGAV)
 *
 *  Windows only
 *   - MSBUILD        (C:\Program Files (x86)\MSBuild\14.0\Bin\MSBuild.exe)
 *   - ZIP_EXE        (C:\Program Files\7-Zip\7z.exe)
 *   - INNO_SETUP_EXE (C:\Program Files (x86)\Inno Setup 5\ISCC.exe)
 */ 


def MSBUILD = env.MSBUILD
def ZIP_EXE = env.ZIP_EXE

node 
{
    stage('Checkout')
    {
        checkout scm
    }

    stage('Projucer')
    {
        buildProjucer()
    }

    stage('APVTS_Tutorial')
    {
        buildPlugin ("APVTS_Tutorial", "")
    }

    stage('SignalGenerator')
    {
        buildPlugin ("SignalGenerator", "")
    }

    stage('ExtendingExample')
    {
        buildPlugin ("ExtendingExample", "")
    }

    stage('EqualizerExample')
    {
        buildPlugin ("EqualizerExample", "")
    }

    stage('FoleysSynth')
    {
        buildPlugin ("FoleysSynth", "")
    }
}


void makeOutputDirectory(String platform)
{
    if (platform == 'mac')
        sh "mkdir output"
    else if (platform == 'windows')
        bat "md output"
}

void buildProjucer()
{
    if (env.BUILD_SERVER_PLATFORM == 'mac')
    {
        sh """
        cd JUCE/extras/Projucer
        xcodebuild -configuration Release -project Builds/MacOSX/Projucer.xcodeproj
        """
    }
    else if (env.BUILD_SERVER_PLATFORM == 'windows')
    {               
        bat """
        cd "JUCE\\extras\\Projucer"
        "${env.MSBUILD2017}" /p:Configuration=Release Builds\\VisualStudio2017\\Projucer.sln
        """
    }
}

void createProject(String project)
{
    def PROJUCER_BIN = "JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer"
    def PROJUCER_EXE = "JUCE\\extras\\Projucer\\Builds\\VisualStudio2017\\x64\\Release\\App\\Projucer.exe"

    if (env.BUILD_SERVER_PLATFORM == 'mac')
        sh "${PROJUCER_BIN} --resave ${project}.jucer"
    else if (env.BUILD_SERVER_PLATFORM == 'windows')
        bat "${PROJUCER_EXE} --resave ${project}.jucer"
}

void buildProject(String folder, String project, String configuration)
{
    if (env.BUILD_SERVER_PLATFORM == 'mac')
    {
        sh """
        cd ${folder}
        xcodebuild -configuration ${configuration} -project Builds/MacOSX/${project}.xcodeproj
        """
    }
    else if (env.BUILD_SERVER_PLATFORM == 'windows')
    {
        bat """
        cd "${folder}"
        "${env.MSBUILD2019}" /p:Configuration=${configuration} Builds\\VisualStudio2019\\${project}.sln
        """
    }
}

def buildPlugin (String pluginName, String iLokID)
{
    def PROJUCER_BIN = "JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer"
    def PROJUCER_EXE = "JUCE\\extras\\Projucer\\Builds\\VisualStudio2017\\x64\\Release\\App\\Projucer.exe"

    try
    {
        if (env.BUILD_SERVER_PLATFORM == 'mac')
        {
            sh "${PROJUCER_BIN} --resave examples/${pluginName}/${pluginName}.jucer"
            def VERSION=sh script: "${PROJUCER_BIN} --get-version examples/${pluginName}/${pluginName}.jucer", returnStdout: true
            sh "xcodebuild -configuration Release -project examples/${pluginName}/Builds/MacOSX/${pluginName}.xcodeproj"

        }
        else if (env.BUILD_SERVER_PLATFORM == 'windows')
        {
            bat "${PROJUCER_EXE} --resave examples\\${pluginName}\\${pluginName}.jucer"

            bat """
            "${env.MSBUILD2019}" /p:Configuration=Release examples\\${pluginName}\\Builds\\VisualStudio2019\\${pluginName}.sln
            """
        }
    }
    catch (error)
    {
        throw error
    }
}


