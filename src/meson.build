sources = files(
    'gw-application.c',
    'gw-headerbar.c',
    'gw-main.c',
    'gw-multi-media-widget.c',
    'gw-image-button.c',
    'gw-timeline-list.c',
    'gw-timeline-row.c',
    'gw-util.c',
    'gw-window.c'
)

enum_headers = files(
    'gw-image-button.h'
)

sources += gnome.compile_resources(
    'gw-resources',
    gresource,
    c_name: 'gw',
    source_dir: data_dir,
    dependencies: resource_data,
    export: true
)

enum = 'gw-enums'
sources += gnome.mkenums_simple(
    enum,
    sources : enum_headers
)

executable(
    gw_name,
    sources,
    dependencies : gw_deps,
    install : true,
    install_dir: gw_bindir
)
