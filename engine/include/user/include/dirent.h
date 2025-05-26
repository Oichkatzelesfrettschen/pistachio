#ifndef USER_DIRENT_H
#define USER_DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif

struct dirent {
    char d_name[256];
};

typedef struct DIR DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);
int closedir(DIR *dirp);

#ifdef __cplusplus
}
#endif

#endif /* USER_DIRENT_H */
