include_directories(.. ${CMAKE_BINARY_DIR})

include(ECMMarkAsTest)

set( EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

set(interactiveDiffTest_SRCS
    interactivedifftest.cpp
    ../perforceparser.cpp
    ../cvsdiffparser.cpp
    ../difference.cpp
    ../diffhunk.cpp
    ../diffmodel.cpp
    ../diffmodellist.cpp
    ../diffparser.cpp
    ../komparemodellist.cpp
    ../kompareprocess.cpp
    ../parser.cpp
    ../parserbase.cpp
    ../stringlistpair.cpp
    ../kompare.cpp
)

add_executable(interactiveDiffTest ${interactiveDiffTest_SRCS})
add_test(libkomparediff2-interactiveDiffTest interactiveDiffTest)
ecm_mark_as_test(interactiveDiffTest)
set_target_properties(interactiveDiffTest PROPERTIES COMPILE_FLAGS "-DDIFF2_EXPORT=")
target_link_libraries(interactiveDiffTest
    Qt5::Core
    Qt5::Test
    KF5::CoreAddons
    KF5::Parts
    KF5::I18n
    KF5::XmlGui
    KF5::KIOCore
)

set(levenshteinTest_SRCS
    levenshteintest.cpp
    ../difference.cpp
    ../stringlistpair.cpp
)

add_executable(levenshteintest ${levenshteinTest_SRCS})
add_test(libkomparediff2-levenshteintest levenshteintest)
ecm_mark_as_test(levenshteintest)
set_target_properties(levenshteintest PROPERTIES COMPILE_FLAGS "-DDIFF2_EXPORT=")
target_link_libraries(levenshteintest
    Qt5::Test
    Qt5::Core
)
