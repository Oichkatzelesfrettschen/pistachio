project('pistachio_demos', ['cpp', 'c'],
    default_options: ['cpp_std=c++23', 'c_std=c23'])

add_project_arguments('-Werror', language: ['c', 'cpp'])

# Global include directories used by demos and libraries
pistachio_inc = include_directories(
    'include',
    'user/include',
    'user/contrib/elf-loader/include',
    'third_party/eigen'
)

subdir('user/lib')
subdir('user/serv')
subdir('user/apps')
