project('weibird', 'c', version : '0.1.0')

wb_name = meson.project_name()
wb_version = meson.project_version()

wb_prefix = get_option('prefix')
wb_bindir = join_paths(wb_prefix, get_option('bindir'))
wb_datadir = join_paths(wb_prefix, get_option('datadir'))
wb_pkgdatadir = join_paths(wb_datadir, wb_name)

# warning cflags
warning_cflags = [
    '-Wall',
    '-Wstrict-prototypes',
    '-Wnested-externs',
    '-Werror=missing-prototypes',
    '-Werror=implicit-function-declaration',
    '-Werror=pointer-arith',
    '-Werror=init-self',
    '-Werror=format-security',
    '-Werror=format=2',
    '-Werror=missing-include-dirs',
    '-Werror=return-type'
]
c_compiler = meson.get_compiler('c')
supported_warning_cflags = c_compiler.get_supported_arguments(warning_cflags)
add_global_arguments(supported_warning_cflags, language : 'c')

gnome = import('gnome')

#dependencies
wb_deps = [
    dependency('glib-2.0'),
    dependency('gtk+-3.0'),
    dependency('json-glib-1.0'),
    dependency('libsoup-2.4'),
    dependency('rest-0.7'),
    dependency('webkit2gtk-4.0')
]

data_dir = join_paths(meson.source_root(), 'data')

subdir('data')
subdir('src')

meson.add_install_script('meson_post_install.py')
