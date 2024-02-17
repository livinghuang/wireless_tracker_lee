#ifndef _PSENSOR_H
#define _PSENSOR_H

#include "Dps310.h"
#include "DpsClass.h"

extern Dps310 Dps310PressureSensor;

void dsp310_init(void);

void dsp310_fetch(void);
#endif