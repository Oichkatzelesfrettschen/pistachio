#include <l4/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

struct DIR {
    size_t pos;
};

static const char *root_entries[] = {
    "README",
    "docs",
    "kernel",
    nullptr
};

DIR *opendir(const char *name)
{
    if (!name || strcmp(name, "/") != 0)
        return nullptr;

    DIR *d = static_cast<DIR*>(malloc(sizeof(DIR)));
    if (!d)
        return nullptr;
    d->pos = 0;
    return d;
}

struct dirent *readdir(DIR *dirp)
{
    static struct dirent ent;
    if (!dirp)
        return nullptr;

    const char *name = root_entries[dirp->pos];
    if (!name)
        return nullptr;

    strncpy(ent.d_name, name, sizeof(ent.d_name) - 1);
    ent.d_name[sizeof(ent.d_name) - 1] = '\0';
    dirp->pos++;
    return &ent;
}

void rewinddir(DIR *dirp)
{
    if (dirp)
        dirp->pos = 0;
}

int closedir(DIR *dirp)
{
    free(dirp);
    return 0;
}

int main()
{
    printf("VFS server started\n");
    while (1) {
        L4_ThreadId_t from = L4_nilthread;
        L4_Receive(from, &from);
        L4_Reply(from);
    }
    return 0;
}
