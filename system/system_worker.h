#ifndef SYSTEM_WORKER_H
#define SYSTEM_WORKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "system_environment_internal.h"

namespace lmice
{

class SystemWorker
{
public:
    SystemWorker(lmice_environment_t* e);
    int processSignal();
private:
    lmice_environment_t* env;
};

}

//inline __attribute__((always_inline))
int lmice_worker_init(lmice_environment_t *env);

inline __attribute__((always_inline))
int lmice_worker_release(lmice_environment_t *env);

//inline __attribute__((always_inline))
int lmice_master_init(lmice_environment_t *env);

void lmice_master_loop(lmice_environment_t* env);

int lmice_force_clean(lmice_environment_t *);
#ifdef __cplusplus
}
#endif

#endif /** SYSTEM_WORKER_H */
