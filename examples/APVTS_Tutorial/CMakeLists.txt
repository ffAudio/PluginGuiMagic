cmake_minimum_required(VERSION 3.13.0)

set(project_name "apvtsTutorial")

set(${project_name}_sources
    Source/PluginProcessor.cpp)

project(${project_name} VERSION ${version})

# add the plugin targets
juce_add_plugin(${project_name}
                VERSION "${version}"
                COMPANY_NAME "Foleys Finest Audio"
                PLUGIN_MANUFACTURER_CODE "FFAU"
                PLUGIN_CODE "PgmV"
                FORMATS ${FORMATS}
                VST3_CATEGORIES "Fx"
                AAX_CATEGORY "AAX_ePlugInCategory_EQ"
                AU_MAIN_TYPE "kAudioUnitType_Effect"
                COMPANY_WEBSITE "https://foleysfinest.com"
                COMPANY_EMAIL "info@foleysfinest.com"
                BUNDLE_ID "com.foleysfinest.APVTS-Tutorial"
                PLUGIN_NAME "PGM-APVTS-Tutorial"
                PRODUCT_NAME "PGM-APVTS-Tutorial")


juce_generate_juce_header (${project_name})

target_sources(${project_name}
    PRIVATE
    ${${project_name}_sources})

# add required flags
target_link_libraries(${project_name} 
    PRIVATE
    ${project_sources}
    foleys_gui_magic
    juce::juce_audio_basics
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_cryptography
    juce::juce_gui_extra
    juce::juce_opengl
    juce::juce_recommended_warning_flags 
    juce::juce_recommended_config_flags 
    juce::juce_recommended_lto_flags)

target_compile_definitions(${project_name}
    PUBLIC 
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0)

foreach(FORMAT ${FORMATS})
    get_target_property(ARTEFACTS_DIR ${project_name}_${FORMAT} LIBRARY_OUTPUT_DIRECTORY)
    add_custom_command(TARGET ${project_name}_${FORMAT} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${ARTEFACTS_DIR} ${COPY_FOLDER})
endforeach()
