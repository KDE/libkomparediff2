cmake_minimum_required(VERSION 3.16)

project(LibKompareDiff2 VERSION "5.3.0")

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.91.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMGenerateExportHeader)
include(ECMDeprecationSettings)

include(CMakePackageConfigHelpers)
include(FeatureSummary)

set(ADDITIONAL_REQUIRED_QT_COMPONENTS)
if(BUILD_TESTING)
    list(APPEND ADDITIONAL_REQUIRED_QT_COMPONENTS Test)
endif()
if (QT_MAJOR_VERSION STREQUAL "6")
    list(APPEND ADDITIONAL_REQUIRED_QT_COMPONENTS Core5Compat) # QTextCodec
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
        Config
        XmlGui
        I18n
        KIO
)

ecm_set_disabled_deprecation_versions(
    QT 5.15.2
    KF 5.91
)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
