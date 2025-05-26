userlib_inc = include_directories('.', '..', '../contrib/elf-loader/include')
eigen_inc = include_directories('../../third_party/eigen')

l4_lib = static_library('l4',
    'l4/debug.cc',
    'l4/amd64.cc',
    include_directories: [userlib_inc, pistachio_inc]
)
l4_dep = declare_dependency(link_with: l4_lib, include_directories: [userlib_inc, pistachio_inc])

io_lib = static_library('io',
    'io/get_hex.cc',
    'io/print.cc',
    'io/amd64.cc',
    include_directories: [userlib_inc, pistachio_inc]
)
io_dep = declare_dependency(link_with: io_lib, include_directories: [userlib_inc, pistachio_inc])

ipc_lib = static_library('ipc',
    'ipc/user_ipc.cc',
    'exo/exo_ipc.cc',
    include_directories: [userlib_inc, pistachio_inc]
)
ipc_dep = declare_dependency(link_with: ipc_lib, include_directories: [userlib_inc, pistachio_inc])

memory_lib = static_library('memory',
    'memory/memory.cc',
    include_directories: [userlib_inc, pistachio_inc]
)
memory_dep = declare_dependency(link_with: memory_lib, include_directories: [userlib_inc, pistachio_inc])

sched_lib = static_library('sched',
    'sched/sched_client.cc',
    include_directories: [userlib_inc, pistachio_inc]
)
sched_dep = declare_dependency(link_with: sched_lib, include_directories: [userlib_inc, pistachio_inc])

mlp_lib = static_library('mlp',
    'mlp/mlp.cc',
    include_directories: [userlib_inc, pistachio_inc, eigen_inc]
)
mlp_dep = declare_dependency(link_with: mlp_lib, include_directories: [userlib_inc, pistachio_inc, eigen_inc])
