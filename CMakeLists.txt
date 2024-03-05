cmake_minimum_required(VERSION 3.16)

project(LibKompareDiff2 VERSION "6.0.0")

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMGenerateExportHeader)
include(ECMGenerateHeaders)
include(ECMDeprecationSettings)

include(CMakePackageConfigHelpers)
include(FeatureSummary)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

set(ADDITIONAL_REQUIRED_QT_COMPONENTS)
if(BUILD_TESTING)
    list(APPEND ADDITIONAL_REQUIRED_QT_COMPONENTS Test)
endif()

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED
    COMPONENTS
        Core
        Widgets
        ${ADDITIONAL_REQUIRED_QT_COMPONENTS}
)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED
    COMPONENTS
        CoreAddons
        Config
        XmlGui
        I18n
        KIO
)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 6.0
)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
