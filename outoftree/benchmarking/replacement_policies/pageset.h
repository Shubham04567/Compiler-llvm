#ifndef PAGESET_H
#define PAGESET_H

#include "PolicyCheck.h"

hitspCaches** p_plotpoints;

hitspCaches* init_plot_points();

void free_hitspCaches(hitspCaches* hpc);

// allocates array for plots
void allocate_plotarr();

#endif