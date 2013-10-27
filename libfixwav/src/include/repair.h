#ifndef REPAIR_H_INCLUDED
#define REPAIR_H_INCLUDED

#include "fixwav_manager.h"

int launch_repair(WaveData *info, WaveHeader *header, uint8_t* p);
int write_header(uint8_t *newheader, FILE* outfile, WaveData *info);

#endif
// REPAIR_H_INCLUDED

