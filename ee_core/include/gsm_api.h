/*
#
# Graphics Synthesizer Mode Selector (a.k.a. GSM) - Force (set and keep) a GS Mode, then load & exec a PS2 ELF
#-------------------------------------------------------------------------------------------------------------
# Copyright 2009, 2010, 2011 doctorxyz & dlanor
# Copyright 2011, 2012 doctorxyz, SP193 & reprep
# Copyright 2013 Bat Rastard
# Copyright 2014, 2015, 2016 doctorxyz
# Licenced under Academic Free License version 2.0
# Review LICENSE file for further details.
#
*/

void UpdateGSMParams(u32 interlace, u32 mode, u32 ffmd, u64 display, u64 syncv, u64 smode2, u32 dx_offset, u32 dy_offset);
void EnableGSM(void);
void DisableGSM(void);
