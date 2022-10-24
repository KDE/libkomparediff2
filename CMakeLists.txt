cmake_minimum_required(VERSION 3.16)

project(LibKompareDiff2 VERSION "5.2")

set(QT_MIN_VERSION "5.15.0")
set(KF_MIN_VERSION "5.90.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(GenerateExportHeader)

set(ADDITIONAL_REQUIRED_QT_COMPONENTS)
if(BUILD_TESTING)
    list(APPEND ADDITIONAL_REQUIRED_QT_COMPONENTS Test)
endif()

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} CONFIG REQUIRED
    COMPONENTS
        Core
        Widgets
        ${ADDITIONAL_REQUIRED_QT_COMPONENTS}
)

find_package(KF5 ${KF_MIN_VERSION} REQUIRED
    COMPONENTS
        CoreAddons
        Codecs
        Config
        XmlGui
        I18n
        KIO
)

if (QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6Core5Compat)
endif()

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055400
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
