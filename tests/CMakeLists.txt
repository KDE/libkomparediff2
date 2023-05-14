include(ECMMarkAsTest)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

remove_definitions(
    -DQT_NO_CAST_FROM_ASCII
)

set(LIB_SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(LIB_BINARY_DIR "${CMAKE_BINARY_DIR}/src")
include_directories(
    ${LIB_SOURCE_DIR}
    ${LIB_BINARY_DIR}
)

ecm_qt_declare_logging_category(komparediff2_DBG_SRCS
    HEADER komparediffdebug.h
    IDENTIFIER LIBKOMPAREDIFF2
    CATEGORY_NAME "libkomparediff"
)

add_executable(interactiveDiffTest)

target_sources(interactiveDiffTest PRIVATE
    interactivedifftest.cpp
    ${LIB_SOURCE_DIR}/perforceparser.cpp
    ${LIB_SOURCE_DIR}/cvsdiffparser.cpp
    ${LIB_SOURCE_DIR}/difference.cpp
    ${LIB_SOURCE_DIR}/diffhunk.cpp
    ${LIB_SOURCE_DIR}/diffmodel.cpp
    ${LIB_SOURCE_DIR}/diffmodellist.cpp
    ${LIB_SOURCE_DIR}/diffparser.cpp
    ${LIB_SOURCE_DIR}/komparemodellist.cpp
    ${LIB_SOURCE_DIR}/kompareprocess.cpp
    ${LIB_SOURCE_DIR}/parser.cpp
    ${LIB_SOURCE_DIR}/parserbase.cpp
    ${LIB_SOURCE_DIR}/stringlistpair.cpp
    ${LIB_SOURCE_DIR}/kompare.cpp
    ${komparediff2_DBG_SRCS}
)

add_test(NAME libkomparediff2-interactiveDiffTest COMMAND interactiveDiffTest)
ecm_mark_as_test(interactiveDiffTest)
set_target_properties(interactiveDiffTest PROPERTIES COMPILE_FLAGS "-DDIFF2_EXPORT=")
target_link_libraries(interactiveDiffTest
    Qt::Core
    Qt::Test
    KF${KF_MAJOR_VERSION}::CoreAddons
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::XmlGui
    KF${KF_MAJOR_VERSION}::KIOCore
)
if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(interactiveDiffTest Qt6::Core5Compat)
endif()

add_executable(levenshteintest)
target_sources(levenshteintest PRIVATE
    levenshteintest.cpp
    ${LIB_SOURCE_DIR}/difference.cpp
    ${LIB_SOURCE_DIR}/stringlistpair.cpp
    ${komparediff2_DBG_SRCS}
)

add_test(NAME libkomparediff2-levenshteintest COMMAND levenshteintest)
ecm_mark_as_test(levenshteintest)
set_target_properties(levenshteintest PROPERTIES COMPILE_FLAGS "-DDIFF2_EXPORT=")
target_link_libraries(levenshteintest
    Qt::Test
    Qt::Core
)
