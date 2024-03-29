cmake_minimum_required(VERSION 3.13.0)

set (version 1.3.5)

project(PluginGuiMagic VERSION ${version})

# Configure JUCE sources
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)
set_property(GLOBAL PROPERTY USE_FOLDERS YES)

set (subprojectPath ${CMAKE_CURRENT_LIST_DIR}/JUCE)
add_subdirectory (${subprojectPath} ${CMAKE_CURRENT_LIST_DIR}/build/juce)

# add foleys_gui_magic to juce. You need to list it in `target_link_libraries` too
juce_add_module(${CMAKE_CURRENT_LIST_DIR}/modules/foleys_gui_magic)

set_property(GLOBAL PROPERTY JUCE_COPY_PLUGIN_AFTER_BUILD TRUE)
set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE INTERNAL "")

# check which formats we want to build
set(FORMATS "VST3" "Standalone")
if (AAX_PATH)
    list(APPEND FORMATS "AAX")
    juce_set_aax_sdk_path(${AAX_PATH})
endif()

if (VST2_PATH)
    list(APPEND FORMATS "VST2")
    juce_set_vst2_sdk_path(${VST2_PATH})
endif()

if (APPLE)
    list(APPEND FORMATS "AU")
endif()

# setup the copying to the output folder
if (APPLE)
    set(COPY_FOLDER ${CMAKE_SOURCE_DIR}/Builds/MacOSX)
elseif(WIN32)
    set(COPY_FOLDER ${CMAKE_SOURCE_DIR}/Builds/VisualStudio2019)
endif()

add_subdirectory(examples/APVTS_Tutorial)
add_subdirectory(examples/EqualizerExample)
add_subdirectory(examples/ExtendingExample)
add_subdirectory(examples/FoleysSynth)
add_subdirectory(examples/SignalGenerator)
