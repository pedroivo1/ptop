#ifndef DISPLAY_H
#define DISPLAY_H

#include "monitor.h"

void setup_terminal();
void restore_terminal();
void render_interface(CpuMonitor* cpumon);

#endif
