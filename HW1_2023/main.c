#include "cache_model.h"
#include <stdio.h>

int main() {
    cm_init();
    cm_enable_cache();
    int total_cycles_enabled = 0;
    int total_cycles_disabled = 0;
    for (int i = 0; i < 100000; i++) {
        cm_do_access(rand_int(CM_ADDRESS_SPACE_SIZE));
        total_cycles_enabled += cm_get_last_access_cycles();
    }
    printf("%d total cycles with cache enabled\n", total_cycles_enabled);
    cm_disable_cache();
    for (int i = 0; i < 100000; i++) {
        cm_do_access(rand_int(CM_ADDRESS_SPACE_SIZE));
        total_cycles_disabled += cm_get_last_access_cycles();
    }
    printf("%d total cycles with cache disabled\n", total_cycles_disabled);
    return 0;
}
