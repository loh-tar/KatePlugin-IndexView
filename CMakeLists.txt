# Because the only official example CMake config I found was pretty outdated
# from 2004, https://kate-editor.org/2004/01/06/writing-a-kate-plugin/
# I like to thank Tatsh, https://github.com/Tatsh/kate-wakatime, where I could
# crib what to remove from the orig Kate main CMake file
# March 2024: While porting to Qt6/KF6 I found this "new" example, sadly
# not up to date  https://develop.kde.org/docs/apps/kate/plugin/ showing Qt5/KF5

# FIXME KDE has a lot of changes in there CMake stuff, no idea how important are
# these and if it may stop working here at some sudden point
# Here some links
# https://develop.kde.org/docs/apps/kate/plugin/   # looks also a little outdated with KF5 var-names
# https://invent.kde.org/utilities/kate/-/blob/master/CMakeLists.txt
# https://invent.kde.org/utilities/kate/-/blob/master/addons/CMakeLists.txt
# https://invent.kde.org/utilities/kate/-/blob/master/addons/symbolviewer/CMakeLists.txt

cmake_minimum_required(VERSION 3.16.0 FATAL_ERROR)

project(kate-plugin-index-view)

set(QT_MIN_VERSION "6.6.0")
set(KF6_DEP_VERSION "6.0.0")

# Prepare for Qt6 https://doc.qt.io/qt-6/portingguide.html
add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x060600)

set(KDE_SKIP_RPATH_SETTINGS TRUE)
find_package(ECM 1.1.0 REQUIRED NO_MODULE)
# find_package(ECM ${KF6_DEP_VERSION} QUIET REQUIRED NO_MODULE) # used upstream

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)
# find_package(KF6TextEditor NO_MODULE)
find_package(KF6 "${KF6_DEP_VERSION}" REQUIRED COMPONENTS
# Not sure if something is missed or could be removed
    Config
    I18n
    TextEditor
    XmlGui
)

# tell about our features (and what is missing)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

include(ECMOptionalAddSubdirectory)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)

# Sane flags from Kate project
add_definitions(
  -DQT_USE_QSTRINGBUILDER
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_STRICT_ITERATORS
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_NO_SIGNALS_SLOTS_KEYWORDS
#   -DQT_USE_FAST_OPERATOR_PLUS
)

# Needed to find the moc file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

ecm_optional_add_subdirectory(index-view)


# kate: space-indent on; indent-width 4; replace-tabs on;
