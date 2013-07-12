cmake_minimum_required(VERSION 2.6)
project(kompare)

# search packages used by KDE
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../libdialogpages ${CMAKE_CURRENT_SOURCE_DIR}/../komparepart ${CMAKE_CURRENT_SOURCE_DIR}/../interfaces  )


########### next target ###############

set(diff2_PART_SRCS
	kompareprocess.cpp 
	komparemodellist.cpp 
	diffmodellist.cpp 
	diffmodel.cpp 
	difference.cpp 
	diffhunk.cpp 
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

install(TARGETS komparediff2 ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
