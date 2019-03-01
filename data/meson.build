subdir('icons')

resource_data = files(
    'wb-headerbar.ui',
    'wb-main-widget.ui',
    'wb-media-dialog.ui',
    'wb-timeline-list.ui',
    'wb-window.ui'
)

gresource = files('com.jonathankang.Weibird.gresource.xml')

# GSettings schemas
gnome.compile_schemas(
    build_by_default: true,
    depend_files: 'com.jonathankang.Weibird.gschema.xml'
)

install_data(
    'com.jonathankang.Weibird.gschema.xml',
    install_dir: join_paths(wb_datadir, 'glib-2.0', 'schemas')
)

css_files = files(
    'wb-style.css'
)

install_data(
    css_files,
    install_dir : wb_pkgdatadir
)

install_data(
    'com.jonathankang.Weibird.desktop',
    install_dir : join_paths(wb_datadir, 'applications')
)

install_data(
    'com.jonathankang.Weibird.appdata.xml',
    install_dir : join_paths(wb_datadir, 'metainfo')
)
