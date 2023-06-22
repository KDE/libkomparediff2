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
    VERSION ${KOMPAREDIFF2_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.4
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
    ${_generate_export_header_version_args}
)

target_link_libraries(komparediff2
    PUBLIC
        KF${KF_MAJOR_VERSION}::XmlGui
        KF${KF_MAJOR_VERSION}::ConfigCore
        Qt::Widgets
        Qt::Core
    PRIVATE
        KF${KF_MAJOR_VERSION}::KIOCore
        KF${KF_MAJOR_VERSION}::I18n
        KF${KF_MAJOR_VERSION}::CoreAddons
)

if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(komparediff2 PRIVATE Qt6::Core5Compat)
endif()

target_include_directories(komparediff2 INTERFACE "$<INSTALL_INTERFACE:${KOMPAREDIFF2_INSTALL_INCLUDEDIR}>")
if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.4.0)
    target_include_directories(komparediff2 PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/compat>")
endif()

install(TARGETS komparediff2 EXPORT LibKompareDiff2Targets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(komparediff2_CamelCase_HEADERS
    HEADER_NAMES
        Difference,DifferenceString
        DiffHunk
        DiffModel
        DiffModelList
        DiffSettings
        Kompare
        KompareModelList
        Marker
        SettingsBase
    PREFIX KompareDiff2
    REQUIRED_HEADERS komparediff2_HEADERS
)

# provide compat headers for old include path prefix libkomparediff2/
if(NOT EXCLUDE_DEPRECATED_BEFORE_AND_AT STREQUAL "CURRENT" AND EXCLUDE_DEPRECATED_BEFORE_AND_AT VERSION_LESS 5.4.0)
    set(compat_headers
        compat/diff2_export.h
        compat/diff2_export_p.h
    )

    function(generate_compat_headers)
        foreach(classname ${ARGV})
            string(TOLOWER ${classname} classname_lc)
            set(header_name "${classname_lc}.h")
            set(compat_header "${CMAKE_CURRENT_BINARY_DIR}/compat/${header_name}")
            configure_file(compat/header.h.in ${compat_header} @ONLY)
            list(APPEND compat_headers ${compat_header})
        endforeach()
        set(compat_headers ${compat_headers} PARENT_SCOPE)
    endfunction()

    generate_compat_headers(
        Difference
        DiffHunk
        DiffModel
        DiffModelList
        DiffSettings
        Kompare
        KompareModelList
        Marker
        SettingsBase
    )

    install(
        FILES ${compat_headers}
        DESTINATION ${KOMPAREDIFF2_INSTALL_INCLUDEDIR}/libkomparediff2
        COMPONENT Devel
    )
endif()

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
