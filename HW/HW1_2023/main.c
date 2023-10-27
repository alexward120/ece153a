#include "cache_model.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define EVENTS (double)100000

void problem_a() {
    printf("Problem a ============\n");

    int total_cycles_enabled = 0;
    int total_cycles_disabled = 0;

    cm_enable_cache(); // enable cache
    for (int i = 0; i < EVENTS; i++) {
        cm_do_access(rand_int(CM_ADDRESS_SPACE_SIZE));
        total_cycles_enabled += cm_get_last_access_cycles();
    }
    printf("%d total cycles with cache enabled\n", total_cycles_enabled);
    printf("Expected value %.2f\n", (double)total_cycles_enabled / EVENTS);

    cm_disable_cache(); // disable cache
    for (int i = 0; i < EVENTS; i++) {
        cm_do_access(rand_int(CM_ADDRESS_SPACE_SIZE));
        total_cycles_disabled += cm_get_last_access_cycles();
    }
    printf("%d total cycles with cache disabled\n", total_cycles_disabled);
    printf("Expected value %.2f\n", (double)total_cycles_disabled / EVENTS);
}

int problem_1_b_helper(int prev_address) {
    int RANGE = 40;
    float s = 0.6;
    float p = 0.35;

    int next_address;

    double seed = (rand() % 1000) / 1000.0;
    if (seed < s) { // sequential
        next_address = (prev_address + 1) % CM_ADDRESS_SPACE_SIZE;
    } 
    else if (seed < s + p) { // nearby within 40 words
        int dist = (rand() % (2 * RANGE)) - RANGE;
        next_address = (prev_address + dist) % CM_ADDRESS_SPACE_SIZE;
        if (next_address < 0) {
            next_address += CM_ADDRESS_SPACE_SIZE;
        }
    } 
    else { // not nearby outside 40 words
        int dist = (rand() % (CM_ADDRESS_SPACE_SIZE - 2 * RANGE)) + (RANGE + 1);
        next_address = (prev_address + dist) % CM_ADDRESS_SPACE_SIZE;
    }

    return next_address;
}

void problem_b() {
        printf("Problem 1b ============\n");
        int random_address = rand() % CM_ADDRESS_SPACE_SIZE;

        cm_enable_cache();
        float cached_sum = 0;
        for(int i = 0; i < EVENTS; i++) {
            random_address = problem_1_b_helper(random_address);
            cm_do_access(random_address);
            cached_sum += cm_get_last_access_cycles();
        }
        printf("Cached Average: %.3f\n", cached_sum / EVENTS);

        cm_disable_cache();
        float uncached_sum = 0;
        for(int i = 0; i < EVENTS; i++) {
            random_address = problem_1_b_helper(random_address);
            cm_do_access(random_address);
            uncached_sum += cm_get_last_access_cycles();
        }
        printf("Uncached Average: %.3f\n", uncached_sum / EVENTS);
}

void problem_c() { //gather access times and output into a text file
        printf("Problem 1c ============\n");
        int random_address = rand() % CM_ADDRESS_SPACE_SIZE;
        cm_enable_cache();
        FILE *fp = fopen("cached1.txt", "w"); //problem 1a cached data
        for(int i = 0; i < EVENTS; i++) {
            cm_do_access(rand_int(CM_ADDRESS_SPACE_SIZE));
            fprintf(fp, "%d\n", cm_get_last_access_cycles());
        }
        fclose(fp);
        cm_disable_cache();
        fp = fopen("uncached1.txt", "w"); //problem 1a uncached data
        for(int i = 0; i < EVENTS; i++) {
            cm_do_access(rand_int(CM_ADDRESS_SPACE_SIZE));
            fprintf(fp, "%d\n", cm_get_last_access_cycles());
        }
        fclose(fp);

        cm_enable_cache();
        fp = fopen("cached2.txt", "w"); //problem 1b cached data
        for(int i = 0; i < EVENTS; i++) { 
                random_address = problem_1_b_helper(random_address);
                cm_do_access(random_address);
                fprintf(fp, "%d\n", cm_get_last_access_cycles());
        }
        fclose(fp);

        cm_disable_cache();
        fp = fopen("uncached2.txt", "w"); //problem 1b uncached data
        for(int i = 0; i < EVENTS; i++) {
                random_address = problem_1_b_helper(random_address);
                cm_do_access(random_address);
                fprintf(fp, "%d\n", cm_get_last_access_cycles());
        }
        fclose(fp);
}

int main() {
    srand(time(NULL)); // seed random number generator
    cm_init(); // initialize cache model
    problem_a(); // run problem a
    problem_b(); // run problem b
    problem_c(); // run problem c
    return 0;
}