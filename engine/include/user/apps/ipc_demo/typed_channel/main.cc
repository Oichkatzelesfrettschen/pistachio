#include <typed_channel.h>
#include <stdio.h>

int main() {
    typed_channel_t ch;
    typed_channel_init(&ch, L4_Myself());
    L4_Word_t words[1] = {42};
    exo_ipc_status st = typed_channel_send(&ch, words, 1);
    printf("typed_channel_send returned %d\n", (int)st);
    return 0;
}
