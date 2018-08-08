# Kate project
project (kate)

# minimal requirements
cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.4.0")
set(KF5_DEP_VERSION "5.13.0") # we need KMultiTabBar::appendButton(const QIcon &..., ...)

# KDE Application Version, managed by release script
set (KDE_APPLICATIONS_VERSION_MAJOR "18")
set (KDE_APPLICATIONS_VERSION_MINOR "11")
set (KDE_APPLICATIONS_VERSION_MICRO "70")
set (KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

# we need some parts of the ECM CMake helpers
find_package (ECM 1.1.0 REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(ECMAddAppIcon)
include(GenerateExportHeader)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include (CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core DBus Widgets Sql)

if(BUILD_TESTING)
  find_package(Qt5Test ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()

# Load the frameworks we need
find_package(KF5 "${KF5_DEP_VERSION}" REQUIRED COMPONENTS
  Config
  Crash
  I18n
  JobWidgets
  KIO
  Parts
  TextEditor
  WindowSystem
  XmlGui
  IconThemes
)

# some optional deps, to make compiling on some OSes easier
# # find_package(KF5 "${KF5_DEP_VERSION}" COMPONENTS
# #   Activities
# #   DocTools
# # )

if (KF5Activities_FOUND)
    add_definitions(-DKActivities_FOUND)
endif ()

# use sane compile flags
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)

# config.h
check_function_exists (ctermid HAVE_CTERMID)
configure_file (config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# let our config.h be found first in any case
include_directories (BEFORE ${CMAKE_CURRENT_BINARY_DIR})

# kwrite
ecm_optional_add_subdirectory (kwrite)

# kate application
ecm_optional_add_subdirectory (kate)

# addons, e.g. kate plugins, plasma applets, ...
ecm_optional_add_subdirectory (addons)

# docs, if doc tools around
if (KF5DocTools_FOUND)
    ecm_optional_add_subdirectory (doc)
endif ()

# tell about our features (and what is missing)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)








# detect additional frameworks
find_package(KF5 "${KF5_DEP_VERSION}" OPTIONAL_COMPONENTS Wallet Plasma Service ItemModels ThreadWeaver NewStuff IconThemes GuiAddons)

set_package_properties(KF5Wallet PROPERTIES PURPOSE "Required to build the katesql addon")
set_package_properties(KF5Plasma PROPERTIES PURPOSE "Required to build the sessionapplet addon")
set_package_properties(KF5Service PROPERTIES PURPOSE "Required to build the sessionapplet addon")
set_package_properties(KF5ItemModels PROPERTIES PURPOSE "Required to build the project, konsole addon")
set_package_properties(KF5ThreadWeaver PROPERTIES PURPOSE "Required to build the project addon")
set_package_properties(KF5NewStuff PROPERTIES PURPOSE "Required to build the snippets and project addons")

# document switcher
ecm_optional_add_subdirectory (filetree)

# search in open documents and files
ecm_optional_add_subdirectory (search)

# ALT+Tab like tab switcher
ecm_optional_add_subdirectory (tabswitcher)

# ctags
ecm_optional_add_subdirectory (kate-ctags)

# backtrace
ecm_optional_add_subdirectory (backtracebrowser)

# file browser
ecm_optional_add_subdirectory (filebrowser)

# xml completion
ecm_optional_add_subdirectory (xmltools)

# XML Validation plugin
ecm_optional_add_subdirectory (xmlcheck)

# open header matching to current file
ecm_optional_add_subdirectory (openheader)

# debugger plugin, needs windows love, guarded until ported to win32
if (NOT WIN32)
    ecm_optional_add_subdirectory (gdbplugin)
endif ()

# list symbols and functions in a file
ecm_optional_add_subdirectory (symbolviewer)

# replicode integration
ecm_optional_add_subdirectory (replicode)

# pipe text through some external command
ecm_optional_add_subdirectory (textfilter)

# Rust complection plugin
ecm_optional_add_subdirectory (rustcompletion)

# D completion plugin
ecm_optional_add_subdirectory (lumen)

# build plugin
ecm_optional_add_subdirectory (katebuild-plugin)

# close document except this one (or similar)
ecm_optional_add_subdirectory (close-except-like)

if(KF5Wallet_FOUND)
    # kate sql
    ecm_optional_add_subdirectory (katesql)
endif()

if(KF5NewStuff_FOUND)
    # snippets
    ecm_optional_add_subdirectory (snippets)
endif()

# live preview of sources in target format
ecm_optional_add_subdirectory (preview)

# terminal tool view
if(KF5Service_FOUND AND NOT WIN32)
    ecm_optional_add_subdirectory (konsole)
endif()

if(KF5ItemModels_FOUND AND KF5ThreadWeaver_FOUND AND KF5NewStuff_FOUND)
    # small & smart project manager
    ecm_optional_add_subdirectory (project)
endif()

if (KF5Plasma_FOUND AND KF5Service_FOUND)
    ecm_optional_add_subdirectory (sessionapplet)
endif()
