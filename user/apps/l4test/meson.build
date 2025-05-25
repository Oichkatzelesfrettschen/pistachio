arch = 'amd64'
arch_dir = join_paths(meson.current_source_dir(), arch)

sources = files(
    'main.cc', 'assert.cc', 'menu.cc', 'threads.cc', 'string.cc',
    'ipc.cc', 'ipc-string.cc', 'ipc-pf.cc', 'ipc-smp.cc',
    'kip.cc', 'mem.cc', 'sig0.cc', 'exreg.cc', 'tcontrol.cc',
    'schedule.cc',
    join_paths(arch, 'crt0.S'),
    join_paths(arch, 'help.cc'),
    join_paths(arch, 'tests.cc')
)

executable('l4test',
    sources,
    include_directories: [pistachio_inc, include_directories(arch)],
    dependencies: [l4_dep, io_dep])
