#ifndef __SYSTEM_H
#define __SYSTEM_H

#ifdef VMC
#include "include/mcemu.h"
#endif

#define SYS_LOAD_MC_MODULES 0x01
#define SYS_LOAD_USB_MODULES 0x02
#define SYS_LOAD_ISOFS_MODULE 0x04

unsigned int USBA_crc32(char *string);
int sysGetDiscID(char *discID);
void sysInitDev9(void);
void sysShutdownDev9(void);
void sysReset(int modload_mask);
void sysExecExit();
void sysPowerOff(void);

#ifdef VMC
#define MCEMU int size_mcemu_irx, void **mcemu_irx,
#else
#define MCEMU
#endif

void sysLaunchLoaderElf(char *filename, char *mode_str, int size_cdvdman_irx, void **cdvdman_irx, MCEMU int EnablePS2Logo, unsigned int compatflags);

int sysExecElf(char *path);
int sysLoadModuleBuffer(void *buffer, int size, int argc, char *argv);
int sysCheckMC(void);
#ifdef VMC
int sysCheckVMC(const char *prefix, const char *sep, char *name, int createSize, vmc_superblock_t *vmc_superblock);
#endif

#endif
