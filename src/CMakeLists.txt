add_definitions(-DTRANSLATION_DOMAIN=\"libkomparediff2\")

set(LibKompareDiff2_SOVERSION 5)

add_library(komparediff2 SHARED)

target_sources(komparediff2 PRIVATE
    kompareprocess.cpp
    komparemodellist.cpp
    diffmodellist.cpp
    diffmodel.cpp
    difference.cpp
    diffhunk.cpp
    diffsettings.cpp
    settingsbase.cpp
    parser.cpp
    parserbase.cpp
    cvsdiffparser.cpp
    diffparser.cpp
    perforceparser.cpp
    stringlistpair.cpp
    kompare.cpp
)

ecm_qt_declare_logging_category(komparediff2
    HEADER komparediffdebug.h
    IDENTIFIER LIBKOMPAREDIFF2
    CATEGORY_NAME "libkomparediff"
    DESCRIPTION "libkomparediff"
    EXPORT komparediff2
)

generate_export_header(komparediff2 BASE_NAME diff2)

target_link_libraries(komparediff2
    PUBLIC
        KF5::XmlGui
        KF5::ConfigCore
        Qt::Widgets
        Qt::Core
    PRIVATE
        KF5::Codecs
        KF5::KIOCore
        KF5::I18n
        KF5::CoreAddons
)

if (TARGET Qt6::Core5Compat)
    target_link_libraries(komparediff2 PRIVATE Qt6::Core5Compat)
endif()

target_include_directories(komparediff2 INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>")

set_target_properties(komparediff2 PROPERTIES
    VERSION ${LibKompareDiff2_VERSION}
    SOVERSION ${LibKompareDiff2_SOVERSION}
    EXPORT_NAME "KompareDiff2"
)

install(TARGETS komparediff2 EXPORT LibKompareDiff2Targets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/diff2_export.h
        settingsbase.h
        diffsettings.h
        komparemodellist.h
        difference.h
        diffmodel.h
        diffmodellist.h
        marker.h
        kompare.h
        diffhunk.h
    DESTINATION
        ${KDE_INSTALL_INCLUDEDIR}/libkomparediff2
    COMPONENT
        Devel
)

ecm_qt_install_logging_categories(
    EXPORT komparediff2
    FILE libkomparediff2.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)


# CMake config files
set(_LibKompareDiff2_CONFIG_DEST "${KDE_INSTALL_CMAKEPACKAGEDIR}/LibKompareDiff2")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/LibKompareDiff2Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2Config.cmake"
    INSTALL_DESTINATION  "${_LibKompareDiff2_CONFIG_DEST}"
)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

install( FILES
        "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2ConfigVersion.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2Config.cmake"
        DESTINATION "${_LibKompareDiff2_CONFIG_DEST}" )
install( EXPORT LibKompareDiff2Targets
         DESTINATION "${_LibKompareDiff2_CONFIG_DEST}"
         FILE LibKompareDiff2Targets.cmake )
