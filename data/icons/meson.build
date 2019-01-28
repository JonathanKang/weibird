app_icons = [
    ['scalable', 'com.jonathankang.Weibird.svg'],
    ['symbolic', 'com.jonathankang.Weibird-symbolic.svg']
]

foreach icon : app_icons
    install_data(
        join_paths(icon[0], icon[1]),
        install_dir : join_paths(wb_datadir, 'icons', 'hicolor', icon[0], 'apps')
    )
endforeach
