project('clash',
        'c', 
        default_options: ['prefix=/usr/local',
                          'buildtype=debug',
                          'debug=true',
                          'strip=false',
                          'warning_level=3',
                          'werror=false',
                          'b_ndebug=false',
                          'c_std=gnu18',],
        license: 'AGPL-3.0',
        meson_version: '>=0.62.2',
        version: 'r@0@.@1@'.format(run_command('git', 'rev-list', '--count', 'HEAD', capture: true, check: true).stdout().strip(),
                                   run_command('git', 'rev-parse', '--short', 'HEAD', capture: true, check: true).stdout().strip()))

# https://mesonbuild.com/Reference-manual_builtin_meson.html#mesonget_compiler
CC=meson.get_compiler('c')
assert('gcc'==CC.get_id())
assert('ld.bfd'==CC.get_linker_id())
# message(CC.get_id())
# message(CC.get_linker_id())

foreach h: ['arpa/inet.h', 'fcntl.h', 'netdb.h', 'netinet/in.h', 'stdint.h', 'strings.h', 'sys/ioctl.h', 'sys/socket.h', 'unistd.h']
  CC.has_header(h, required: true)
endforeach
foreach f: ['bzero', 'dup2', 'inet_ntoa', 'memset', 'regcomp', 'setlocale', 'socket', 'strdup', 'strerror', 'strstr']
  assert(CC.has_function(f))
endforeach
CC.has_header_symbol('netinet/in.h', 'INET_ADDRSTRLEN', required: true)
CC.has_header_symbol('arpa/inet.h', 'INET_ADDRSTRLEN', required: true)

# https://mesonbuild.com/Reference-manual_functions.html#get_option
# https://mesonbuild.com/Adding-arguments.html
# https://mesonbuild.com/Syntax.html#string-formatting
# https://clash.gitbook.io/doc/restful-api
# https://github.com/haishanh/yacd/blob/master/src/index.template.ejs
# <div id="app" data-base-url="http://127.0.0.1:9090"></div>
RESTFUL_PORT = get_option('RESTFUL_PORT')
add_global_arguments(f'-DRESTFUL_PORT=@RESTFUL_PORT@', language: 'c')
add_global_arguments('-D_GNU_SOURCE',
                     '-Wno-unused-parameter',
                     '-Winline',
                     '-Wshadow',
                     '-Wno-pedantic',
                     language: 'c')

# mask out /usr/local/include/libclash.h
# https://mesonbuild.com/Reference-manual_elementary_str.html#strformat
# gcc -xc -E -v /dev/null
# gcc -x c -E -Wp,-v /dev/null
add_global_arguments('-I@0@'.format(meson.current_source_dir()/'include'), language: 'c')

# https://mesonbuild.com/Reference-manual_functions.html#dependency
THREAD = dependency('threads')
BSD = dependency('libbsd', version: '>=0.10.0')
CRYPTO = dependency('libcrypto', version: '>=1.1.1k')
CURL = dependency('libcurl', version: '>=7.76.0')
GLIB = dependency('glib-2.0', version: '>=2.68.1')
JSONC = dependency('json-c', version: '>=0.15')
PROCPS = dependency('libprocps', version: '3.3.17')
QRENCODE = dependency('libqrencode', version: '4.1.1')
READLINE = dependency('readline', version: '>=8.1')
SSLIBEV = dependency('shadowsocks-libev', version: '3.3.5')
SYSTEMD = dependency('libsystemd', version: '>=248')
YAML = dependency('yaml-0.1', version: '0.2.5')

subdir('include')
subdir('lib')
subdir('bin.convert')
subdir('bin.script')
subdir('bin.sip002')
subdir('bin.tun')

# subdir('doc')
