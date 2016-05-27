#ifndef SAND
#define SAND

#include <stdint.h>
#include "globalVars.h"

void gravity(int32_t angle);
int32_t absolut(int32_t in);
int32_t probability(int32_t gravity_angle, int32_t neighbor_angle);
int32_t pseudoCos(int32_t dir);

#endif
