add_executable(spinlock_fairness spinlock_fairness.c)
target_link_libraries(spinlock_fairness PRIVATE pthread)

add_executable(posix_test_file posix/test_file.c)
add_executable(posix_test_process posix/test_process.c)
add_executable(posix_test_timer posix/test_timer.c)

add_custom_target(tests DEPENDS spinlock_fairness posix_test_file posix_test_process posix_test_timer)
