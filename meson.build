project('clash',
        'c', 
        # https://mesonbuild.com/Builtin-options.html
        default_options: ['prefix=/usr/local',
                          'buildtype=debug',
                          'debug=true',
                          'strip=false',
                          'warning_level=2',  # level 3 introduces -Wpedantic
                          'werror=false',
                          'b_ndebug=false',
                          'c_std=gnu18',],
        license: 'AGPL-3.0',
        meson_version: '>=0.62.2',
        # https://mesonbuild.com/Reference-manual_functions.html#run_command
        # printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
        version: 'r'
                 +run_command('git', 'rev-list', '--count', 'HEAD', capture: true, check: true).stdout().strip()
                 +'.'
                 +run_command('git', 'rev-parse', '--short', 'HEAD', capture: true, check: true).stdout().strip())


# https://mesonbuild.com/Reference-manual_builtin_meson.html#mesonget_compiler
assert('gcc'==meson.get_compiler('c').get_id())

# https://mesonbuild.com/Reference-manual_functions.html#get_option
# # https://mesonbuild.com/Adding-arguments.html
# https://mesonbuild.com/Syntax.html#string-formatting
RESTFUL_PORT = get_option('RESTFUL_PORT')
add_global_arguments(f'-DRESTFUL_PORT=@RESTFUL_PORT@', language: 'c')
add_global_arguments(f'-D_GNU_SOURCE', language: 'c')
add_global_arguments('-Wno-unused-parameter -Winline -Wshadow', language: 'c')

# https://mesonbuild.com/Reference-manual_functions.html#dependency
JSONC = dependency('json-c', version: '>=0.15')
CURL = dependency('libcurl', version: '>=7.76.0')
YAML = dependency('yaml-0.1', version: '0.2.5')
GLIB = dependency('glib-2.0', version: '>=2.68.1')
CRYPTO = dependency('libcrypto', version: '>=1.1.1k')
QRENCODE = dependency('libqrencode', version: '4.1.1')

# subdir('doc')
subdir('include')
subdir('lib')
# subdir('bin.tun')
subdir('bin.sip002')
subdir('bin.convert')
