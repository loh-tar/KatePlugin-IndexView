option(REPORT "Setting of 1 enables parsers Status Report generation" 0)
if(REPORT)
    add_definitions(-DGENERATE_REPORT)
endif(REPORT)

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
    xml_type_parser.cpp
    # ...and derivated classes, alphabetical ordered
    bash_parser.cpp
    cpp_parser.cpp
    ecma_parser.cpp
    fortran_parser.cpp
    go_parser.cpp
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
    kate_plugin_index_view_config_page_about_parser.ui
)
ki18n_wrap_ui(kateindexviewplugin_PART_SRCS ${kateindexviewplugin_PART_UI} )

# resource for ui file and stuff
qt6_add_resources(kateindexviewplugin_PART_SRCS plugin.qrc)

add_library(kateindexviewplugin MODULE ${kateindexviewplugin_PART_SRCS})

target_link_libraries(kateindexviewplugin
    KF6::TextEditor
    KF6::I18n
)

install(TARGETS kateindexviewplugin DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/ktexteditor)

# kate: space-indent on; indent-width 4; replace-tabs on;
