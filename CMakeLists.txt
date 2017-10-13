cmake_minimum_required(VERSION 2.8.12)

project(LibKompareDiff2)

find_package(ECM 5.16.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMPackageConfigHelpers)
include(ECMQtDeclareLoggingCategory)

include(FeatureSummary)
include(GenerateExportHeader)

set(QT_MIN_VERSION "5.4.0")
find_package(Qt5Core ${QT_MIN_VERSION} NO_MODULE)
set_package_properties(Qt5Core PROPERTIES TYPE REQUIRED)
find_package(Qt5Widgets ${QT_MIN_VERSION} NO_MODULE)
set_package_properties(Qt5Widgets PROPERTIES TYPE REQUIRED)
find_package(Qt5Test ${QT_MIN_VERSION} NO_MODULE)

set(KF5_DEP_VERSION "5.28.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
	CoreAddons Codecs Config XmlGui I18n KIO
)

set(KOMPAREDIFF2_VERSION "5.2")
ecm_setup_version(${KOMPAREDIFF2_VERSION}
                  VARIABLE_PREFIX LIBKOMPAREDIFF2
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2ConfigVersion.cmake"
                  SOVERSION 5)

add_definitions(
    -DQT_DEPRECATED_WARNINGS
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050400
    -DQT_USE_QSTRINGBUILDER
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_CAST_FROM_ASCII
    -DQT_NO_CAST_FROM_BYTEARRAY
    -DQT_NO_URL_CAST_FROM_STRING
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
    -DQT_STRICT_ITERATORS
)

add_definitions(-DTRANSLATION_DOMAIN=\"libkomparediff2\")

ecm_qt_declare_logging_category(komparediff2_DBG_SRCS
    HEADER komparediffdebug.h
    IDENTIFIER LIBKOMPAREDIFF2
    CATEGORY_NAME "libkomparediff"
)

set(komparediff2_SRCS
	${komparediff2_DBG_SRCS}
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

add_library(komparediff2 ${komparediff2_SRCS})
generate_export_header(komparediff2 BASE_NAME diff2)

target_link_libraries(komparediff2
    Qt5::Core
    Qt5::Widgets
    KF5::Codecs
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::XmlGui
)

target_include_directories(komparediff2 INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}>")

set_target_properties(komparediff2 PROPERTIES
    VERSION ${LIBKOMPAREDIFF2_VERSION_STRING}
    SOVERSION ${LIBKOMPAREDIFF2_SOVERSION}
    EXPORT_NAME "KompareDiff2"
)

install(TARGETS komparediff2 EXPORT LibKompareDiff2Targets ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)

install(FILES
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
    DESTINATION ${INCLUDE_INSTALL_DIR}/libkomparediff2  COMPONENT Devel
)

# Config.cmake file.
set(_LibKompareDiff2_CONFIG_DEST "${LIB_INSTALL_DIR}/cmake/LibKompareDiff2")

ecm_configure_package_config_file(
    "${CMAKE_SOURCE_DIR}/LibKompareDiff2Config.cmake.in"
    "${CMAKE_BINARY_DIR}/LibKompareDiff2Config.cmake"
    INSTALL_DESTINATION  "${_LibKompareDiff2_CONFIG_DEST}"
)

install( FILES
        "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2ConfigVersion.cmake"
        "${CMAKE_BINARY_DIR}/LibKompareDiff2Config.cmake"
        DESTINATION "${_LibKompareDiff2_CONFIG_DEST}" )
install( EXPORT LibKompareDiff2Targets
         DESTINATION "${_LibKompareDiff2_CONFIG_DEST}"
         FILE LibKompareDiff2Targets.cmake )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
