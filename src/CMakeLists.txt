set(KOMPAREDIFF2_INSTALL_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR}/KompareDiff2")
set(KOMPAREDIFF2_CMAKECONFIG_NAME "LibKompareDiff2")

add_definitions(-DTRANSLATION_DOMAIN=\"libkomparediff2\")

ecm_setup_version(PROJECT
    VARIABLE_PREFIX      KOMPAREDIFF2
    VERSION_HEADER       "komparediff2_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${KOMPAREDIFF2_CMAKECONFIG_NAME}ConfigVersion.cmake"
    COMPATIBILITY        SameMajorVersion
    SOVERSION            5
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

if (QT_MAJOR_VERSION STREQUAL "5")
    set(_generate_export_header_version_args)
else()
    # For Qt6/KF6 world transitively include the version header
    if(ECM_VERSION VERSION_LESS "5.106")
        set(include_version_header_code "#include <komparediff2_version.h>\n")
        set(_generate_export_header_version_args CUSTOM_CONTENT_FROM_VARIABLE include_version_header_code)
    else()
        set(_generate_export_header_version_args USE_VERSION_HEADER VERSION_BASE_NAME KOMPAREDIFF2)
    endif()
endif()

ecm_generate_export_header(komparediff2
    BASE_NAME diff2
    VERSION ${KOMPAREDIFF2_VERSION}
    DEPRECATED_BASE_VERSION 0
    ${_generate_export_header_version_args}
)

target_link_libraries(komparediff2
    PUBLIC
        KF5::XmlGui
        KF5::ConfigCore
        Qt::Widgets
        Qt::Core
    PRIVATE
        KF5::KIOCore
        KF5::I18n
        KF5::CoreAddons
)

target_include_directories(komparediff2 INTERFACE "$<INSTALL_INTERFACE:${KOMPAREDIFF2_INSTALL_INCLUDEDIR}>")

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
        ${KOMPAREDIFF2_INSTALL_INCLUDEDIR}/libkomparediff2
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
    "${CMAKE_CURRENT_SOURCE_DIR}/LibKompareDiff2Config.cmake.in"
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
