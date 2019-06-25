sources = files(
    'wb-application.c',
    'wb-avatar-widget.c',
    'wb-comment.c',
    'wb-comment-list.c',
    'wb-comment-row.c',
    'wb-compose-window.c',
    'wb-headerbar.c',
    'wb-image-button.c',
    'wb-main.c',
    'wb-main-widget.c',
    'wb-media-dialog.c',
    'wb-multi-media-widget.c',
    'wb-name-button.c',
    'wb-timeline-list.c',
    'wb-tweet-detail-page.c',
    'wb-tweet-item.c',
    'wb-tweet-row.c',
    'wb-user.c',
    'wb-util.c',
    'wb-window.c'
)

enum_headers = files(
    'wb-headerbar.h',
    'wb-image-button.h',
    'wb-main-widget.h'
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
