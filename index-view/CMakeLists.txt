add_definitions(-DTRANSLATION_DOMAIN=\"kateindexview\")
########### next target ###############
set(kateindexviewplugin_PART_SRCS
    kate_plugin_index_view.cpp
    index_view.cpp
    filter_box.cpp
    icon_collection.cpp
    # Parser master classes, logical ordered
    parser.cpp
    document_parser.cpp
    program_parser.cpp
    # ...and derivated classes, alphabetical ordered
    bash_parser.cpp
    cpp_parser.cpp
    ecma_parser.cpp
    fortran_parser.cpp
    markdown_parser.cpp
    perl_parser.cpp
    php_parser.cpp
    plaintext_parser.cpp
    python_parser.cpp
    ruby_parser.cpp
    tcl_parser.cpp
)

set(kateindexviewplugin_PART_UI
    kate_plugin_index_view_config_page.ui
)
ki18n_wrap_ui(kateindexviewplugin_PART_SRCS ${kateindexviewplugin_PART_UI} )

# resource for ui file and stuff
qt5_add_resources(kateindexviewplugin_PART_SRCS plugin.qrc)

add_library(kateindexviewplugin MODULE ${kateindexviewplugin_PART_SRCS})

kcoreaddons_desktop_to_json (kateindexviewplugin kateindexviewplugin.desktop)

target_link_libraries(kateindexviewplugin
    KF5::TextEditor
    KF5::I18n
)

install(TARGETS kateindexviewplugin DESTINATION ${PLUGIN_INSTALL_DIR}/ktexteditor)


# kate: space-indent on; indent-width 4; replace-tabs on;
