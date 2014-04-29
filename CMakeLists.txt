cmake_minimum_required(VERSION 2.8.12)

project(LibKompareDiff2)

# search packages used by KDE
include(FeatureSummary)
find_package(Qt5Core NO_MODULE)
set_package_properties(Qt5Core PROPERTIES TYPE REQUIRED)
find_package(Qt5Widgets NO_MODULE)
set_package_properties(Qt5Widgets PROPERTIES TYPE REQUIRED)
find_package(Qt5Test NO_MODULE)
find_package(ECM 0.0.9 NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMSetupVersion)

find_package(KF5 CONFIG REQUIRED
	CoreAddons Codecs Config XmlGui I18n KIO KDELibs4Support
)

set(KF5_VERSION "5.0")
ecm_setup_version(${KF5_VERSION}
                  VARIABLE_PREFIX LIBKOMPAREDIFF2
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2ConfigVersion.cmake"
                  SOVERSION 5)

set(komparediff2_SRCS
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
)


add_library(komparediff2 ${komparediff2_SRCS})

target_link_libraries(komparediff2
    Qt5::Core
    Qt5::Widgets
    KF5::Codecs
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::XmlGui
    KF5::KDELibs4Support
)

set_target_properties(komparediff2 PROPERTIES VERSION ${LIBKOMPAREDIFF2_VERSION_STRING}
SOVERSION ${LIBKOMPAREDIFF2_VERSION_STRING} )

install(TARGETS komparediff2 EXPORT LibKompareDiff2Targets ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)

install(FILES
    diff2export.h
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
configure_file( "${CMAKE_SOURCE_DIR}/LibKompareDiff2Config.cmake.in" "${CMAKE_BINARY_DIR}/LibKompareDiff2Config.cmake" @ONLY )

set(_LibKompareDiff2_CONFIG_DEST "${LIB_INSTALL_DIR}/cmake/LibKompareDiff2")

install( FILES
        "${CMAKE_CURRENT_BINARY_DIR}/LibKompareDiff2ConfigVersion.cmake"
        "${CMAKE_BINARY_DIR}/LibKompareDiff2Config.cmake"
        DESTINATION "${_LibKompareDiff2_CONFIG_DEST}" )
install( EXPORT LibKompareDiff2Targets
         DESTINATION "${_LibKompareDiff2_CONFIG_DEST}"
         NAMESPACE LibKompareDiff2Import__
         FILE LibKompareDiff2Targets.cmake )
