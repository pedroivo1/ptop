#include <stdlib.h>

typedef struct
{
    char model[16];
    float temp;
    int freq; 
} CPU_mon;

CPU_mon* new_cpumon()
{
    CPU_mon* cpumon = (CPU_mon*) malloc(sizeof(CPU_mon*));

    if (cpumon != NULL)
    {

    }
    return cpumon;
}
