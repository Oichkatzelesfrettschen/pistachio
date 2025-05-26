#include <stdio.h>
#include <dirent.h>

int main(void) {
    DIR *d = opendir("/");
    if (!d) {
        perror("opendir");
        return 1;
    }
    struct dirent *de;
    while ((de = readdir(d)) != NULL) {
        printf("%s\n", de->d_name);
    }
    closedir(d);
    return 0;
}
