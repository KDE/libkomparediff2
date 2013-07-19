cmake_minimum_required(VERSION 2.6)
project(LibKompareDiff2)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libdialogpages ${CMAKE_CURRENT_SOURCE_DIR}/../komparepart ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces  )

set(diff2_PART_SRCS
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
	stringlistpair.cpp)


kde4_add_library(komparediff2 SHARED ${diff2_PART_SRCS})

target_link_libraries(komparediff2 ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS})

set_target_properties(komparediff2 PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

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

if(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
    set(_LibKompareDiff2_CONFIG_DEST "${LIB_INSTALL_DIR}/cmake/libkomparediff2")
else(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)
    set(_LibKompareDiff2_CONFIG_DEST "${LIB_INSTALL_DIR}/libkomparediff2")
endif(KDE4_USE_COMMON_CMAKE_PACKAGE_CONFIG_DIR)

install( FILES
        "${CMAKE_BINARY_DIR}/LibKompareDiff2Config.cmake"
        DESTINATION "${_LibKompareDiff2_CONFIG_DEST}" )
install( EXPORT LibKompareDiff2Targets
         DESTINATION "${_LibKompareDiff2_CONFIG_DEST}"
         NAMESPACE LibKompareDiff2Import__
         FILE LibKompareDiff2Targets.cmake )
