sources = files(
    'gw-application.c',
    'gw-headerbar.c',
    'gw-main.c',
    'gw-multi-media-widget.c',
    'gw-image.c',
    'gw-timeline-list.c',
    'gw-timeline-row.c',
    'gw-util.c',
    'gw-window.c'
)

sources += gnome.compile_resources(
    'gw-resources',
    gresource,
    c_name: 'gw',
    source_dir: data_dir,
    dependencies: resource_data,
    export: true
)

executable(
    gw_name,
    sources,
    dependencies : gw_deps,
    install : true,
    install_dir: gw_bindir
)
