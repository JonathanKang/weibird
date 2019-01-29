sources = files(
    'wb-application.c',
    'wb-headerbar.c',
    'wb-main.c',
    'wb-multi-media-widget.c',
    'wb-image-button.c',
    'wb-timeline-list.c',
    'wb-timeline-row.c',
    'wb-util.c',
    'wb-window.c'
)

enum_headers = files(
    'wb-image-button.h'
)

sources += gnome.compile_resources(
    'wb-resources',
    gresource,
    c_name: 'wb',
    source_dir: data_dir,
    dependencies: resource_data,
    export: true
)

enum = 'wb-enums'
sources += gnome.mkenums_simple(
    enum,
    sources : enum_headers
)

executable(
    wb_name,
    sources,
    include_directories : [top_inc, src_inc],
    dependencies : wb_deps,
    install : true,
    install_dir: wb_bindir
)
