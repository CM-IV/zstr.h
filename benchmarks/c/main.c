
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "zstr.h"

#define ITER_SSO   10000000 // 10 Million.
#define ITER_LARGE 10000000 // 10 Million bytes.

static double now() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}


void bench_sso_zstr() 
{
    double start = now();
    volatile size_t len_sum = 0;
    
    for (int i = 0; i < ITER_SSO; i++) 
    {
        zstr s = zstr_from("Small");
        len_sum += zstr_len(&s);
        zstr_free(&s);
    }
    
    printf("[zstr SSO]   Time: %.4fs (Sum: %zu)\n", now() - start, len_sum);
}

void bench_sso_malloc() 
{
    double start = now();
    volatile size_t len_sum = 0;
    
    for (int i = 0; i < ITER_SSO; i++) 
    {
        char *s = malloc(6);
        if (s) 
        {
            memcpy(s, "Small", 6);
            len_sum += 5;
            free(s);
        }
    }
    
    printf("[Raw Malloc] Time: %.4fs (Sum: %zu)\n", now() - start, len_sum);
}


void bench_append_zstr() 
{
    double start = now();
    zstr s = zstr_init();
    
    for (int i = 0; i < ITER_LARGE; i++) 
    {
        zstr_push(&s, 'A');
    }
    
    printf("[zstr Append] Time: %.4fs (Cap: %zu)\n", now() - start, s.is_long ? s.l.cap : 23);
    zstr_free(&s);
}

void bench_append_naive() 
{
    printf("[Naive Realloc] Skipped (Too slow - would take minutes)\n");
}

void bench_append_optimized() 
{
    double start = now();
    size_t len = 0;
    size_t cap = 0;
    char *data = NULL;
    
    for (int i = 0; i < ITER_LARGE; i++) 
    {
        if (len + 1 > cap) 
        {
            cap = (cap == 0) ? 32 : cap * 2;
            data = realloc(data, cap);
        }
        data[len++] = 'A';
    }
    data[len] = '\0';
    
    printf("[Manual Opt]  Time: %.4fs (Cap: %zu)\n", now() - start, cap);
    free(data);
}

int main(void) 
{
    printf("=> Benchmark C: SSO (%d iterations)\n", ITER_SSO);
    bench_sso_zstr();
    bench_sso_malloc();
    
    printf("\n=> Benchmark C: Append (%d bytes)\n", ITER_LARGE);
    bench_append_zstr();
    bench_append_optimized();
    bench_append_naive();
    
    return 0;
}
