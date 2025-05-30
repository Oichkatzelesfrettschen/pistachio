#pragma once
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Spawn a child process running PATH with argv and envp. */
pid_t posix_spawn(const char *path, char *const argv[], char *const envp[]);

/* execve variant that handles a NULL envp by inheriting the current environment. */
int posix_execve(const char *path, char *const argv[], char *const envp[]);

/* waitpid variant that retries if interrupted by signals. */
pid_t posix_waitpid(pid_t pid, int *status, int options);

#ifdef __cplusplus
}
#endif
