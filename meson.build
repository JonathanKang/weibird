project('gnome-weibo', 'c', version : '0.0')

gw_name = meson.project_name()
gw_version = meson.project_version()

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

#dependencies
gl_deps = [
    dependency('gtk+-3.0')
]

subdir('src')
