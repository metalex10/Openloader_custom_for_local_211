/*
  Copyright 2006-2008 Polo
  Copyright 2009-2010, ifcaro, jimmikaelkael & Polo
  Copyright 2016 doctorxyz
  Licenced under Academic Free License version 3.0
  Review Open-Ps2-Loader README & LICENSE files for further details.
  
  Some parts of the code have been taken from Polo's HD Project and doctorxyz's GSM
*/

#include "ee_core.h"
#include "util.h"

#define ALL_MODE -1

typedef struct
{
    u32 addr;
    u32 val;
    u32 check;
} game_patch_t;

typedef struct
{
    char *game;
    int mode;
    game_patch_t patch;
} patchlist_t;

//Keep patch codes unique!
#define PATCH_GENERIC_NIS 0xDEADBEE0
#define PATCH_GENERIC_CAPCOM 0xBABECAFE
#define PATCH_GENERIC_AC9B 0xDEADBEE1
#define PATCH_GENERIC_SLOW_READS 0xDEADBEE2
#define PATCH_SDF_MACROSS 0x00065405
#define PATCH_SRW_IMPACT 0x0021e808
#define PATCH_RNC_UYA 0x00398498
#define PATCH_ZOMBIE_ZONE 0xEEE62525

static const patchlist_t patch_list[] = {
    {"SLES_524.58", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Disgaea Hour of Darkness PAL - disable cdvd timeout stuff
    {"SLUS_206.66", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Disgaea Hour of Darkness NTSC U - disable cdvd timeout stuff
    {"SLPS_202.51", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Makai Senki Disgaea NTSC J - disable cdvd timeout stuff
    {"SLPS_202.50", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Makai Senki Disgaea (limited edition) NTSC J - disable cdvd timeout stuff
    {"SLPS_731.03", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Makai Senki Disgaea (PlayStation2 the Best) NTSC J - disable cdvd timeout stuff
    {"SLES_529.51", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Phantom Brave PAL - disable cdvd timeout stuff
    {"SLUS_209.55", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Phantom Brave NTSC U - disable cdvd timeout stuff
    {"SLPS_203.45", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Phantom Brave NTSC J - disable cdvd timeout stuff
    {"SLPS_203.44", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Phantom Brave (limited edition) NTSC J - disable cdvd timeout stuff
    {"SLPS_731.08", USB_MODE, {PATCH_GENERIC_NIS, 0x00000000, 0x00000000}},        // Phantom Brave: 2-shuume Hajime Mashita (PlayStation 2 the Best) NTSC J - disable cdvd timeout stuff
    {"SLUS_213.17", ALL_MODE, {PATCH_GENERIC_CAPCOM, 0x00149210, 0x00000000}},     // SFA anthology US
    {"SLES_540.85", ALL_MODE, {PATCH_GENERIC_CAPCOM, 0x00148db0, 0x00000000}},     // SFA anthology EUR
    {"SLPM_664.09", ALL_MODE, {PATCH_GENERIC_CAPCOM, 0x00149210, 0x00000000}},     // SFZ Generation JP
    {"SLPM_659.98", ALL_MODE, {PATCH_GENERIC_CAPCOM, 0x00146fd0, 0x00000000}},     // Vampire: Darkstakers collection JP
    {"SLUS_212.00", USB_MODE, {PATCH_GENERIC_AC9B, 0x00000000, 0x00000000}},       // Armored Core Nine Breaker NTSC U - skip failing case on binding a RPC server
    {"SLES_538.19", USB_MODE, {PATCH_GENERIC_AC9B, 0x00000000, 0x00000000}},       // Armored Core Nine Breaker PAL - skip failing case on binding a RPC server
    {"SLPS_254.08", USB_MODE, {PATCH_GENERIC_AC9B, 0x00000000, 0x00000000}},       // Armored Core Nine Breaker NTSC J - skip failing case on binding a RPC server
    {"SLUS_210.05", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00100000, 0x001ac514}}, // Kingdom Hearts 2 US - [Gummi mission freezing fix (check addr is where to patch,
    {"SLES_541.14", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00100000, 0x001ac60c}}, // Kingdom Hearts 2 UK - val is the amount of delay cycles)]
    {"SLES_542.32", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00100000, 0x001ac60c}}, // Kingdom Hearts 2 FR
    {"SLES_542.33", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00100000, 0x001ac60c}}, // Kingdom Hearts 2 DE
    {"SLES_542.34", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00100000, 0x001ac60c}}, // Kingdom Hearts 2 IT
    {"SLES_542.35", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00100000, 0x001ac60c}}, // Kingdom Hearts 2 ES
    {"SLPM_662.33", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00100000, 0x001ac44c}}, // Kingdom Hearts 2 JPN
    {"SLPM_666.75", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00149210, 0x001adf64}}, // Kingdom Hearts 2 Final Mix JPN
    {"SLUS_212.87", ETH_MODE, {PATCH_GENERIC_SLOW_READS, 0x000c0000, 0x006cd15c}}, // Prince of Persia: The Two Thrones NTSC U - slow down cdvd reads
    {"SLUS_212.87", HDD_MODE, {PATCH_GENERIC_SLOW_READS, 0x00040000, 0x006cd15c}}, // Prince of Persia: The Two Thrones NTSC U - slow down cdvd reads
    {"SLES_537.77", ETH_MODE, {PATCH_GENERIC_SLOW_READS, 0x000c0000, 0x006cd6dc}}, // Prince of Persia: The Two Thrones PAL - slow down cdvd reads
    {"SLES_537.77", HDD_MODE, {PATCH_GENERIC_SLOW_READS, 0x00040000, 0x006cd6dc}}, // Prince of Persia: The Two Thrones PAL - slow down cdvd reads
    {"SLUS_210.22", ETH_MODE, {PATCH_GENERIC_SLOW_READS, 0x000c0000, 0x0060f42c}}, // Prince of Persia: Warrior Within NTSC U - slow down cdvd reads
    {"SLUS_210.22", HDD_MODE, {PATCH_GENERIC_SLOW_READS, 0x00040000, 0x0060f42c}}, // Prince of Persia: Warrior Within NTSC U - slow down cdvd reads
    {"SLES_528.22", ETH_MODE, {PATCH_GENERIC_SLOW_READS, 0x000c0000, 0x0060f4dc}}, // Prince of Persia: Warrior Within PAL - slow down cdvd reads
    {"SLES_528.22", HDD_MODE, {PATCH_GENERIC_SLOW_READS, 0x00040000, 0x0060f4dc}}, // Prince of Persia: Warrior Within PAL - slow down cdvd reads
    {"SLUS_214.32", ALL_MODE, {PATCH_GENERIC_SLOW_READS, 0x00080000, 0x002baf34}}, // NRA Gun Club NTSC U
    {"SLPM_654.05", HDD_MODE, {PATCH_SDF_MACROSS, 0x00200000, 0x00249b84}},        // Super Dimensional Fortress Macross JPN
    {"SLUS_202.30", ALL_MODE, {0x00132d14, 0x10000018, 0x0c046744}},               // Max Payne NTSC U - skip IOP reset before to exec demo elfs
    {"SLES_503.25", ALL_MODE, {0x00132ce4, 0x10000018, 0x0c046744}},               // Max Payne PAL - skip IOP reset before to exec demo elfs
    {"SLUS_204.40", ALL_MODE, {0x0021bb00, 0x03e00008, 0x27bdff90}},               // Kya: Dark Lineage NTSC U - disable game debug prints
    {"SLES_514.73", ALL_MODE, {0x0021bd10, 0x03e00008, 0x27bdff90}},               // Kya: Dark Lineage PAL - disable game debug prints
    {"SLUS_204.96", ALL_MODE, {0x00104900, 0x03e00008, 0x27bdff90}},               // V-Rally 3 NTSC U - disable game debug prints
    {"SLES_507.25", ALL_MODE, {0x00104518, 0x03e00008, 0x27bdff70}},               // V-Rally 3 PAL - disable game debug prints
    {"SLUS_201.99", ALL_MODE, {0x0012a6d0, 0x24020001, 0x0c045e0a}},               // Shaun Palmer's Pro Snowboarder NTSC U
    {"SLUS_201.99", ALL_MODE, {0x0013c55c, 0x10000012, 0x04400012}},               // Shaun Palmer's Pro Snowboarder NTSC U
    {"SLES_553.46", ALL_MODE, {0x0035414C, 0x2402FFFF, 0x0C0EE74E}},               // Rugby League 2: World Cup Edition PAL
    {"SLPS_251.03", ALL_MODE, {PATCH_SRW_IMPACT, 0x00000000, 0x00000000}},         // Super Robot Wars IMPACT Limited Edition
    {"SLPS_251.04", ALL_MODE, {PATCH_SRW_IMPACT, 0x00000000, 0x00000000}},         // Super Robot Wars IMPACT
    {"SCUS_973.53", ALL_MODE, {PATCH_RNC_UYA, 0x0084c645, 0x00000000}},            // Ratchet and Clank: Up Your Arsenal
    {"SCES_524.56", ALL_MODE, {PATCH_RNC_UYA, 0x0084c726, 0x00000000}},            // Ratchet and Clank: Up Your Arsenal
    {"SLES_533.98", ALL_MODE, {PATCH_ZOMBIE_ZONE, 0x001b2c08, 0x00000000}},        // Zombie Zone
    {"SLES_544.61", ALL_MODE, {PATCH_ZOMBIE_ZONE, 0x001b3e20, 0x00000000}},        // Zombie Hunters
    {"SLPM_625.25", ALL_MODE, {PATCH_ZOMBIE_ZONE, 0x001b1dc0, 0x00000000}},        // Simple 2000 Series Vol. 61: The Oneechanbara
    {"SLPM_626.38", ALL_MODE, {PATCH_ZOMBIE_ZONE, 0x001b355c, 0x00000000}},        // Simple 2000 Series Vol. 80: The Oneechanpuruu
    {NULL, 0, {0x00000000, 0x00000000, 0x00000000}}                                // terminater
};

#define JAL(addr) (0x0c000000 | (((addr)&0x03ffffff) >> 2))
#define JMP(addr) (0x08000000 | (0x3ffffff & ((addr) >> 2)))
#define FNADDR(jal) (((jal)&0x03ffffff) << 2)

static int (*cdRead)(u32 lsn, u32 nsectors, void *buf, int *mode);
static unsigned int g_delay_cycles;

// Nippon Ichi Software games generic patch to disable cdvd timeout
static void NIS_generic_patches(void)
{
    static const u32 NIScdtimeoutpattern[] = {
        0x3c010000,
        0x8c230000,
        0x24630001,
        0x3c010000,
        0xac230000,
        0x3c010000,
        0x8c230000,
        0x2861037b,
        0x14200000,
        0x00000000};
    static const u32 NIScdtimeoutpattern_mask[] = {
        0xffff0000,
        0xffff0000,
        0xffffffff,
        0xffff0000,
        0xffff0000,
        0xffff0000,
        0xffff0000,
        0xffffffff,
        0xffff0000,
        0xffffffff};
    u32 *ptr;

    ptr = find_pattern_with_mask((u32 *)0x00100000, 0x01e00000, NIScdtimeoutpattern, NIScdtimeoutpattern_mask, 0x28);
    if (ptr) {
        u16 jmp = _lw((u32)ptr + 32) & 0xffff;
        _sw(0x10000000 | jmp, (u32)ptr + 32);
    }
}

// Armored Core 9 Breaker generic USB patch
static void AC9B_generic_patches(void)
{
    static u32 AC9Bpattern[] = {
        0x8e450000,
        0x0220202d,
        0x0c000000,
        0x0000302d,
        0x04410003,
        0x00000000,
        0x10000005,
        0x2402ffff,
        0x8e020000,
        0x1040fff6};
    static const u32 AC9Bpattern_mask[] = {
        0xffffffff,
        0xffffffff,
        0xfc000000,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff,
        0xffffffff};
    u32 *ptr;

    ptr = find_pattern_with_mask((u32 *)0x00100000, 0x01e00000, AC9Bpattern, AC9Bpattern_mask, 0x28);
    if (ptr)
        _sw(0, (u32)ptr + 36);
}

static int delayed_cdRead(u32 lsn, u32 nsectors, void *buf, int *mode)
{
    int r;
    unsigned int count;

    r = cdRead(lsn, nsectors, buf, mode);
    count = g_delay_cycles;
    while (count--)
        asm("nop\nnop\nnop\nnop");

    return r;
}

static void generic_delayed_cdRead_patches(u32 patch_addr, u32 delay_cycles)
{
    // set configureable delay cycles
    g_delay_cycles = delay_cycles;

    // get original cdRead() pointer
    cdRead = (void *)FNADDR(_lw(patch_addr));

    // overwrite with a JAL to our delayed_cdRead function
    _sw(JAL((u32)delayed_cdRead), patch_addr);
}

static int (*capcom_lmb)(void *modpack_addr, int mod_index, int mod_argc, char **mod_argv);

static void apply_capcom_protection_patch(void *modpack_addr, int mod_index, int mod_argc, char **mod_argv)
{
    u32 iop_addr = _lw((u32)modpack_addr + (mod_index << 3) + 8);
    u32 opcode = 0x10000025;
    SyncDCache((void *)opcode, (void *)((unsigned int)&opcode + sizeof(opcode)));
    smem_write((void *)(iop_addr + 0x270), (void *)&opcode, sizeof(opcode));

    capcom_lmb(modpack_addr, mod_index, mod_argc, mod_argv);
}

static void generic_capcom_protection_patches(u32 patch_addr)
{
    capcom_lmb = (void *)FNADDR(_lw(patch_addr));
    _sw(JAL((u32)apply_capcom_protection_patch), patch_addr);
}

static void Invoke_CRSGUI_Start(void)
{
    int (*pCRSGUI_Start)(int arg1, int arg2) = (void *)0x003054b0;

    pCRSGUI_Start(*(int *)0x0078f79c, 0);
}

static void SDF_Macross_patch(void)
{
    /*	Choujikuu Yousai Macross appears to have a rather large problem with it: it appears to use its GUI before initialization is completed.
		I did not attempt to figure out whether it's really a timing problem (whereby this happens before initialization is completed by another thread... if there is one),
		or if its normal functionality was the result of pure luck that SEGA had.

		The problems that it has are quite evident when this game is run within PCSX2. I still do not know why DECI2 does not detect the TLB exception when it
		dereferences NULL pointers, but it might have something to do with the game accessing the debug registers (PCSX2's logs indicate that). */

    _sw(JMP((unsigned int)&Invoke_CRSGUI_Start), 0x001f8520);
}

extern void SRWI_IncrementCntrlFlag(void);

static void SRWI_IMPACT_patches(void)
{
    //Phase 1	- Replace all segments of code that increment cntrl_flag with a multithread-safe implementation.
    //In cdvCallBack()
    _sw(JAL((unsigned int)&SRWI_IncrementCntrlFlag), 0x0021e840);
    _sw(0x00000000, 0x0021e84c);
    _sw(0x00000000, 0x0021e854);
    //In cdvMain()
    _sw(0x00000000, 0x00220ac8);
    _sw(JAL((unsigned int)&SRWI_IncrementCntrlFlag), 0x00220ad0);
    _sw(0x00000000, 0x00220ad8);
    _sw(JAL((unsigned int)&SRWI_IncrementCntrlFlag), 0x00220b20);
    _sw(0x00000000, 0x00220b28);
    _sw(0x00000000, 0x00220b30);
    _sw(JAL((unsigned int)&SRWI_IncrementCntrlFlag), 0x00220ba0);
    _sw(0x00000000, 0x00220ba8);

    /* Phase 2
		sceCdError() will be polled continuously until it succeeds in retrieving the CD/DVD drive status.
		However, the callback thread has a higher priority than the main thread
		and this might result in a freeze because the main thread wouldn't ever release the libcdvd semaphore, and so calls to sceCdError() by the callback thread wouldn't succeed.
		This problem occurs more frequently than the one addressed above.

		Since the PlayStation 2 EE uses non-preemptive multitasking, we can solve this problem by lowering the callback thread's priority th below the main thread.
		The problem is solved because the main thread can then interrupt the callback thread until it has completed its tasks.	*/
    //In cdvCallBack()
    _sw(0x24040060, 0x0021e944); //addiu $a0, $zero, 0x60 (Set the CD/DVD callback thread's priority to 0x60)
}

void RnC3_AlwaysAllocMem(void);

static void RnC3_UYA_patches(void *address)
{
    unsigned int word1, word2;

    /*	Preserve the pointer to the allocated IOP RAM.
		This game's main executable is obfuscated and/or compressed in some way,
		but thankfully the segment that needs to be patched is just offset by 1 byte.

		It contains an IOP module that seems to load other modules (iop_stash_daemon),
		which unfortunately seems to be the heart of its anti-emulator protection system.
		It (and the EE-side code) appears to be playing around with a pointer to IOP RAM,
		based on the modules that are loaded.

		Right before this IOP module is loaded with a custom LoadModuleBuffer function, the game will allocate a large buffer on the IOP.
		This buffer is then used for loading iop_stash_daemon, which also uses it to load other modules before freeing it.
		Unfortunately, the developers appear to have hardcoded the pointer, rather than using the return value of sceAllocSysMemory().

		This module will also check for the presence of bit 29 in the pointer. If it's absent, then the code will not allocate memory and the game will freeze after the first cutscene in Veldin.
		Like with crazyc's original patch, this branch here will have to be adjusted:
			beqz $s7, 0x13
		... to be:
			beqz $s7, 0x01

		iop_stash_daemon will play with the pointer in the following ways, based on each module it finds:
			1. if it's a module with no name (first 4 characters are 0s), left-shift once.
			2. if it's a module beginning with "Deci", left-shift once.
			3. if it's a module beginning with "cdvd", right-shift once.

		For us, it's about preserving the pointer to the allocated buffer and to adjust it accordingly:
			For TOOL units, there are 6 DECI2 modules and 2 libcdvd modules. Therefore the pointer should be right-shifted by 4.
			For retail units, there are 2 libcdvd modules. Therefore the pointer should be left-shifted by 2.	*/

    word1 = JAL((unsigned int)&RnC3_AlwaysAllocMem);
    switch (GameMode) {
        case HDD_MODE:
//For HDD mode, the CDVDMAN module has its name as "dev9", so adjust the shifting accordingly.
#ifdef _DTL_T10000
            word2 = 0x00021943; //sra $v1, $v0, 5	For DTL-T10000.
#else
            word2 = 0x00021840; //sll $v1, $v0, 1	For retail sets.
#endif
            break;
        default:
#ifdef _DTL_T10000
            word2 = 0x00021903; //sra $v1, $v0, 4	For DTL-T10000.
#else
            word2 = 0x00021880; //sll $v1, $v0, 2	For retail sets.
#endif
    }

    memcpy(address, &word1, 4);
    memcpy((u8 *)address + 8, &word2, 4);
}

static void (*pZZscePadEnd)(void);
static void (*pZZInitIOP)(void);

static void ZombieZone_preIOPInit(void)
{
    pZZscePadEnd();
    pZZInitIOP();
}

static void ZombieZone_patches(unsigned int address)
{
    static const unsigned int ZZpattern[] = {
        0x2403000f, //addiu v1, zero, $000f
        0x24500000, //addiu s0, v0, xxxx
        0x3c040000, //lui a0, xxxx
        0xffbf0020, //sd ra, $0020(sp)
    };
    static const unsigned int ZZpattern_mask[] = {
        0xffffffff,
        0xffff0000,
        0xffff0000,
        0xffffffff};
    u32 *ptr;

    //Locate scePadEnd().
    ptr = find_pattern_with_mask((u32 *)0x001c0000, 0x01f00000, ZZpattern, ZZpattern_mask, sizeof(ZZpattern));
    if (ptr) {
        pZZInitIOP = (void *)FNADDR(_lw(address));
        pZZscePadEnd = (void *)(ptr - 3);

        _sw(JAL((unsigned int)&ZombieZone_preIOPInit), address);
    }
}

// Skip Bink (.BIK) Videos
// This patch is expected to work with all games using ChoosePlayMovie statements, for instance:
// SLUS_215.41(Ratatouille), SLES_541.72 (Garfield 2), SLES_555.22 (UP), SLUS_217.36(Wall-E), SLUS_219.31 (Toy Story 3)
int Skip_BIK_Videos(void)
{
    static const char ChoosePlayMoviePattern[] = {"ChoosePlayMovie"}; // We are looking for an array of 15+1=16 elements: "ChoosePlayMovie" + '\0'(NULL Character).
                                                                      // That's fine (find_pattern_with_mask works with multiples of 4 bytes since it expects a u32 pattern buffer).
    static const unsigned int ChoosePlayMoviePattern_mask[] = {       // We want an exact match, so let's fill it with 255 (0xFF) values.
                                                               0xffffffff,
                                                               0xffffffff,
                                                               0xffffffff,
                                                               0xffffffff};

    u32 *ptr;
    ptr = find_pattern_with_mask((u32 *)0x00100000, 0x01ec0000, (u32 *)ChoosePlayMoviePattern, ChoosePlayMoviePattern_mask, sizeof(ChoosePlayMoviePattern_mask));
    if (ptr) {
        _sw(0x41414141, (u32)ptr); // The built-in game engine script language never will interprete ChoosePlayMovie statements as valid,
                                   // since here we are replacing the command "ChoosePlayMovie" by "AAAAePlayMovie"
                                   // 0x41414141 = "AAAAAA" ;-)
        return 1;
    } else
        return 0;
}

// Skip Videos (sceMpegIsEnd) Code - nachbrenner's basic method, based on CMX/bongsan's original idea
// Source: http://replay.waybackmachine.org/20040419134616/http://nachbrenner.pcsx2.net/chapter1.html
// This patch is expected to work with all games using sceMpegIsEnd, for example:
// SCUS_973.99(God of War I), SLUS_212.42 (Burnout Revenge) and SLES-50613 (Woody Woodpecker: Escape from Buzz Buzzard Park)
int Skip_Videos_sceMpegIsEnd(void)
{
    static const unsigned int sceMpegIsEndPattern[] = {
        0x8c830040, // lw	$v1, $0040($a0)
        0x03e00008, // jr	$ra
        0x8c620000, // lw	$v0, 0($v1)
    };
    static const unsigned int sceMpegIsEndPattern_mask[] = {
        0xffffffff,
        0xffffffff,
        0xffffffff,
    };

    u32 *ptr;
    ptr = find_pattern_with_mask((u32 *)0x00100000, 0x01ec0000, sceMpegIsEndPattern, sceMpegIsEndPattern_mask, sizeof(sceMpegIsEndPattern));
    if (ptr) {
        _sw(0x24020001, (u32)ptr + 8); // addiu 	$v0, $zero, 1 <- HERE!!!
        return 1;
    } else
        return 0;
}

void apply_patches(void)
{
    const patchlist_t *p;

    // if there are patches matching game name/mode then fill the patch table
    for (p = patch_list; p->game; p++) {
        if ((!_strcmp(GameID, p->game)) && ((p->mode == ALL_MODE) || (GameMode == p->mode))) {
            switch (p->patch.addr) {
                case PATCH_GENERIC_NIS:
                    NIS_generic_patches();
                    break;
                case PATCH_GENERIC_AC9B:
                    AC9B_generic_patches();
                    break;
                case PATCH_GENERIC_SLOW_READS:
                    generic_delayed_cdRead_patches(p->patch.check, p->patch.val); // slow reads generic patch
                    break;
                case PATCH_SDF_MACROSS:
                    SDF_Macross_patch();
                    break;
                case PATCH_GENERIC_CAPCOM:
                    generic_capcom_protection_patches(p->patch.val); // Capcom anti cdvd emulator protection patch
                    break;
                case PATCH_SRW_IMPACT:
                    SRWI_IMPACT_patches();
                    break;
                case PATCH_RNC_UYA:
                    RnC3_UYA_patches((unsigned int *)p->patch.val);
                    break;
                case PATCH_ZOMBIE_ZONE:
                    ZombieZone_patches(p->patch.val);
                    break;
                default: // Single-value patches
                    if (_lw(p->patch.addr) == p->patch.check)
                        _sw(p->patch.val, p->patch.addr);
            }
        }
    }
    if (g_compat_mask & COMPAT_MODE_4)
        if (!Skip_BIK_Videos())         // First try to Skip Bink (.BIK) Videos method...
            Skip_Videos_sceMpegIsEnd(); // If - and only if the previous approach didn't work, so try to Skip Videos (sceMpegIsEnd) method.
}
