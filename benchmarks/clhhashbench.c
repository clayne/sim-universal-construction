#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>

#include <config.h>
#include <primitives.h>
#include <fastrand.h>
#include <threadtools.h>
#include <clhhash.h>
#include <barrier.h>
#include <bench_args.h>

#define N_BUCKETS        64
#define LOAD_FACTOR      1
#define INITIAL_CAPACITY (LOAD_FACTOR * N_BUCKETS)

#define RANDOM_RANGE (INITIAL_CAPACITY * log(INITIAL_CAPACITY))

CLHHash object_struct CACHE_ALIGN;
int64_t d1 CACHE_ALIGN, d2;
Barrier bar CACHE_ALIGN;
BenchArgs bench_args CACHE_ALIGN;

inline static void *Execute(void *Arg) {
    int64_t key, value;
    CLHHashThreadState *th_state;
    long i, rnum;
    volatile int j;
    long id = (long)Arg;

    fastRandomSetSeed(id + 1);
    th_state = getAlignedMemory(CACHE_LINE_SIZE, sizeof(CLHHashThreadState));
    CLHHashThreadStateInit(&object_struct, th_state, N_BUCKETS, (int)id);
    if (id == 0) {
        for (i = 0; i < INITIAL_CAPACITY; i++) {
            key = fastRandomRange32(1, RANDOM_RANGE);
            value = id;
            CLHHashInsert(&object_struct, th_state, key, value, id);
        }
    }
    BarrierWait(&bar);
    if (id == 0)
        d1 = getTimeMillis();

    for (i = 0; i < bench_args.runs; i++) {
        int imode = i % 10;

        rnum = fastRandomRange(1, bench_args.max_work);
        for (j = 0; j < rnum; j++)
            ;
        key = fastRandomRange32(1, RANDOM_RANGE);
        value = id;
        if (imode < 2) {
            CLHHashDelete(&object_struct, th_state, key, id);
        } else if (imode < 4) {
            CLHHashInsert(&object_struct, th_state, key, value, id);
        } else {
            CLHHashSearch(&object_struct, th_state, key, id);
        }
    }
    BarrierWait(&bar);
    if (id == 0) d2 = getTimeMillis();

    return NULL;
}

int main(int argc, char *argv[]) {
    parseArguments(&bench_args, argc, argv);
    CLHHashInit(&object_struct, N_BUCKETS, bench_args.nthreads);

    BarrierSet(&bar, bench_args.nthreads);
    StartThreadsN(bench_args.nthreads, Execute, bench_args.fibers_per_thread);
    JoinThreadsN(bench_args.nthreads - 1);

    printf("time: %d (ms)\tthroughput: %.2f (millions ops/sec)\t", (int)(d2 - d1), bench_args.runs * bench_args.nthreads / (1000.0 * (d2 - d1)));
    printStats(bench_args.nthreads, bench_args.total_runs);

    return 0;
}
