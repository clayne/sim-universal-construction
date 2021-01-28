#ifndef _OSCISTACK_H_
#define _OSCISTACK_H_

#include <config.h>
#include <primitives.h>
#include <fastrand.h>
#include <uthreads.h>
#include <types.h>
#include <osci.h>
#include <pool.h>
#include <uthreads.h>
#include <queue-stack.h>

typedef struct OsciStackStruct {
    OsciStruct object_struct CACHE_ALIGN;
    volatile Node *top CACHE_ALIGN;
    PoolStruct *pool_node CACHE_ALIGN;
} OsciStackStruct;

typedef struct OsciStackThreadState {
    OsciThreadState th_state;
} OsciStackThreadState;

void OsciStackInit(OsciStackStruct *stack_object_struct, uint32_t nthreads, uint32_t fibers_per_thread);
void OsciStackThreadStateInit(OsciStackStruct *object_struct, OsciStackThreadState *lobject_struct, int pid);
void OsciStackApplyPush(OsciStackStruct *object_struct, OsciStackThreadState *lobject_struct, ArgVal arg, int pid);
RetVal OsciStackApplyPop(OsciStackStruct *object_struct, OsciStackThreadState *lobject_struct, int pid);

#endif
