#ifndef _METROTRK_MW_CRITICAL_SECTION_GC_H
#define _METROTRK_MW_CRITICAL_SECTION_GC_H

void MWInitializeCriticalSection(unsigned int* section);
void MWEnterCriticalSection(unsigned int* section);
void MWExitCriticalSection(unsigned int* section);
void MWTerminateCriticalSection(unsigned int* section);

#endif
