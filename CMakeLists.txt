cmake_minimum_required(VERSION 3.22)

string(TIMESTAMP TODAY "%Y.%m.%d")
project(UILagInspector VERSION ${TODAY})

include(FetchContent)

FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG develop
    GIT_SHALLOW TRUE
)

set_property(GLOBAL PROPERTY USE_FOLDERS YES)
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Show all module sources in IDE projects" ON)

FetchContent_MakeAvailable(juce)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


juce_add_plugin(UILagInspector
    COMPANY_NAME "Anthony Nicholls"
    PLUGIN_MANUFACTURER_CODE ANic
    PLUGIN_CODE UILg
    FORMATS AAX AU AUv3 VST3 Standalone
    PRODUCT_NAME "UI Lag Inspector"
    COPY_PLUGIN_AFTER_BUILD TRUE
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.anthony-nicholls.ui-lag-inspector"
)

target_sources(UILagInspector
    PRIVATE
        Source/Graph.h
        Source/Graph.cpp

        Source/PluginEditor.h
        Source/PluginEditor.cpp

        Source/PluginProcessor.h
        Source/PluginProcessor.cpp
)

target_include_directories(UILagInspector
    PRIVATE
        Source
)

target_compile_definitions(UILagInspector
    PRIVATE
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(UILagInspector
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_basics
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_processors_headless
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
