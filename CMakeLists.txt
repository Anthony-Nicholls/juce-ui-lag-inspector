cmake_minimum_required(VERSION 3.24)

option(COPY_PLUGIN_AFTER_BUILD "Copy plugin after build" ON)
option(WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
option(JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS "Render with multiple paint calls" OFF)
set(JUCE_GIT_TAG "develop" CACHE STRING "The JUCE git tag/branch/commit to fetch")
set(JUCE_PATH "" CACHE PATH "Path to a local JUCE version (if empty, JUCE will be fetched from GitHub)")

# To disable treating warnings as errors invoke CMake with --compile-no-warning-as-error
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Use the date for the version of the project
string(TIMESTAMP TODAY "%Y.%m.%d")
project(UILagInspector VERSION ${TODAY})

# Add the JUCE module source files to the project
set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

if(JUCE_PATH)
    add_subdirectory(${JUCE_PATH} juce)
else()
    include(FetchContent)

    FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG ${JUCE_GIT_TAG}
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(juce)
endif()

# Use the static runtime libraries on Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

juce_add_plugin(UILagInspector
    COMPANY_NAME "Anthony Nicholls"
    PLUGIN_MANUFACTURER_CODE Acea
    PLUGIN_CODE Ulag
    FORMATS AAX AU AUv3 LV2 VST3 Standalone
    PRODUCT_NAME "UI Lag Inspector"
    COPY_PLUGIN_AFTER_BUILD ${COPY_PLUGIN_AFTER_BUILD}
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.anthony-nicholls.ui-lag-inspector"
    LV2URI "https://github.com/Anthony-Nicholls/juce-ui-lag-inspector"
)

# Without this Xcode will warn that stripping has been avoided while copying the
# AUv3 plugin into the Standalone App bundle.
set_target_properties(UILagInspector_Standalone PROPERTIES
    XCODE_ATTRIBUTE_COPY_PHASE_STRIP NO
)

target_sources(UILagInspector
    PRIVATE
        source/Graph.h
        source/Graph.cpp

        source/PluginEditor.h
        source/PluginEditor.cpp

        source/PluginProcessor.h
        source/PluginProcessor.cpp
)

target_include_directories(UILagInspector
    PRIVATE
        source
)

target_compile_definitions(UILagInspector
    PRIVATE
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS=$<BOOL:${JUCE_COREGRAPHICS_RENDER_WITH_MULTIPLE_PAINT_CALLS}>
)

target_link_libraries(UILagInspector
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_basics
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra

        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_compile_features(UILagInspector PRIVATE cxx_std_17)

add_custom_target(generate_version
    COMMAND ${CMAKE_COMMAND} -E echo "${PROJECT_VERSION}" > "version.txt"
)
