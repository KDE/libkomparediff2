set(KOMPAREDIFF2_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR}/KompareDiff2")
set(KOMPAREDIFF2_CMAKECONFIG_NAME "KompareDiff2")

add_definitions(-DTRANSLATION_DOMAIN=\"libkomparediff2\")

ecm_setup_version(PROJECT
    VARIABLE_PREFIX      KOMPAREDIFF2
    VERSION_HEADER       "komparediff2_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${KOMPAREDIFF2_CMAKECONFIG_NAME}ConfigVersion.cmake"
    COMPATIBILITY        SameMajorVersion
    SOVERSION            6
)

add_library(komparediff2 SHARED)

set_target_properties(komparediff2 PROPERTIES
    VERSION     ${KOMPAREDIFF2_VERSION}
    SOVERSION   ${KOMPAREDIFF2_SOVERSION}
    EXPORT_NAME "KompareDiff2"
)

target_sources(komparediff2 PRIVATE
    kompareprocess.cpp
    komparemodellist.cpp
    diffmodellist.cpp
    diffmodel.cpp
    difference.cpp
    diffhunk.cpp
    diffsettings.cpp
    info.cpp
    settingsbase.cpp
    parser.cpp
    parserbase.cpp
    cvsdiffparser.cpp
    diffparser.cpp
    perforceparser.cpp
    stringlistpair.cpp
)

ecm_qt_declare_logging_category(komparediff2
    HEADER komparediffdebug.h
    IDENTIFIER LIBKOMPAREDIFF2
    CATEGORY_NAME "libkomparediff"
    DESCRIPTION "libkomparediff"
    EXPORT komparediff2
)

ecm_generate_export_header(komparediff2
    VERSION ${KOMPAREDIFF2_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    USE_VERSION_HEADER
)

target_link_libraries(komparediff2
    PUBLIC
        KF6::XmlGui
        KF6::ConfigCore
        Qt6::Widgets
        Qt6::Core
    PRIVATE
        KF6::KIOCore
        KF6::I18n
        KF6::CoreAddons
        Qt6::Core5Compat
)

target_include_directories(komparediff2 INTERFACE "$<INSTALL_INTERFACE:${KOMPAREDIFF2_INSTALL_INCLUDEDIR}>")

install(TARGETS komparediff2 EXPORT LibKompareDiff2Targets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(komparediff2_CamelCase_HEADERS
    HEADER_NAMES
        Difference
        DifferenceString
        DiffHunk
        DiffModel
        DiffModelList
        DiffSettings
        Info
        Global
        KompareModelList
        Marker
        SettingsBase
    PREFIX KompareDiff2
    REQUIRED_HEADERS komparediff2_HEADERS
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/komparediff2_export.h
        ${komparediff2_HEADERS}
    DESTINATION
        ${KOMPAREDIFF2_INSTALL_INCLUDEDIR}/komparediff2
    COMPONENT
        Devel
)
install(
    FILES
        ${komparediff2_CamelCase_HEADERS}
    DESTINATION
        ${KOMPAREDIFF2_INSTALL_INCLUDEDIR}/KompareDiff2
    COMPONENT
        Devel
)
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/komparediff2_version.h
    DESTINATION
        ${KOMPAREDIFF2_INSTALL_INCLUDEDIR}
    COMPONENT
        Devel
)

ecm_qt_install_logging_categories(
    EXPORT komparediff2
    FILE libkomparediff2.categories
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)


# CMake config files
set(_LibKompareDiff2_CONFIG_DEST "${KDE_INSTALL_CMAKEPACKAGEDIR}/${KOMPAREDIFF2_CMAKECONFIG_NAME}")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KompareDiff2Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${KOMPAREDIFF2_CMAKECONFIG_NAME}Config.cmake"
    INSTALL_DESTINATION  "${_LibKompareDiff2_CONFIG_DEST}"
)

install( FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${KOMPAREDIFF2_CMAKECONFIG_NAME}ConfigVersion.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${KOMPAREDIFF2_CMAKECONFIG_NAME}Config.cmake"
        DESTINATION "${_LibKompareDiff2_CONFIG_DEST}" )
install( EXPORT LibKompareDiff2Targets
         DESTINATION "${_LibKompareDiff2_CONFIG_DEST}"
         FILE ${KOMPAREDIFF2_CMAKECONFIG_NAME}Targets.cmake )
