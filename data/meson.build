resource_data = files(
    'gw-timeline-list.ui',
    'gw-window.ui'
)

gresource = files('org.gnome.Weibo.gresource.xml')

# GSettings schemas
gnome.compile_schemas(
    build_by_default: true,
    depend_files: 'org.gnome.Weibo.gschema.xml'
)
install_data(
    'org.gnome.Weibo.gschema.xml',
    install_dir: join_paths(gw_datadir, 'glib-2.0', 'schemas')
)
