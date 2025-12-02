#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "zstr.h"
#include "sds.h"

#define ITER_SSO   10000000 // 10 Million.
#define ITER_LARGE 10000000 // 10 Million bytes.

static double now() 
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

double test_sso_zstr() 
{
    double start = now();
    volatile size_t len_sum = 0;
    
    for (int i = 0; i < ITER_SSO; i++) 
    {
        zstr s = zstr_from("Small"); 
        len_sum += zstr_len(&s);
        zstr_free(&s);
    }
    double duration = now() - start;
    printf("[zstr SSO]   Time: %.4fs (Sum: %zu)\n", duration, len_sum);
    return duration;
}

double test_sso_sds() 
{
    double start = now();
    volatile size_t len_sum = 0;
    
    for (int i = 0; i < ITER_SSO; i++) 
    {
        sds s = sdsnew("Small");
        len_sum += sdslen(s);
        sdsfree(s);
    }
    double duration = now() - start;
    printf("[SDS Heap]   Time: %.4fs (Sum: %zu)\n", duration, len_sum);
    return duration;
}

double test_append_zstr() 
{
    double start = now();
    zstr s = zstr_init();
    
    // zstr_reserve(&s, ITER_LARGE); // Optional: Pre-alloc
    for (int i = 0; i < ITER_LARGE; i++) 
    {
        zstr_push(&s, 'A');
    }
    
    double duration = now() - start;
    printf("[zstr Append] Time: %.4fs\n", duration);
    zstr_free(&s);
    return duration;
}

double test_append_sds() 
{
    double start = now();
    sds s = sdsempty();
    
    for (int i = 0; i < ITER_LARGE; i++) 
    {
        s = sdscatlen(s, "A", 1);
    }
    
    double duration = now() - start;
    printf("[SDS Append]  Time: %.4fs\n", duration);
    sdsfree(s);
    return duration;
}

int main(void) 
{
    printf("=> Benchmark C: Small Strings (%d iter)\n", ITER_SSO);
    double t_zstr_sso = test_sso_zstr();
    double t_sds_sso  = test_sso_sds();
    
    if (t_zstr_sso < t_sds_sso) 
    {
        printf("   SUMMARY: zstr was %.2fx FASTER than SDS on small strings.\n", t_sds_sso / t_zstr_sso);
    } 
    else 
    {
        printf("   SUMMARY: zstr was %.2fx SLOWER than SDS on small strings.\n", t_zstr_sso / t_sds_sso);
    }

    printf("\n=> Benchmark C: Append (%d bytes)\n", ITER_LARGE);
    double t_zstr_app = test_append_zstr();
    double t_sds_app  = test_append_sds();

    if (t_zstr_app < t_sds_app) 
    {
        printf("   SUMMARY: zstr was %.2fx FASTER than SDS on large appends.\n", t_sds_app / t_zstr_app);
    } 
    else 
    {
        printf("   SUMMARY: zstr was %.2fx SLOWER than SDS on large appends.\n", t_zstr_app / t_sds_app);
    }
    
    return 0;
}
