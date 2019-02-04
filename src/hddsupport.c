#include "sys/fcntl.h"
#include "include/opl.h"
#include "include/lang.h"
#include "include/gui.h"
#include "include/supportbase.h"
#include "include/hddsupport.h"
#include "include/util.h"
#include "include/themes.h"
#include "include/textures.h"
#include "include/ioman.h"
#include "include/system.h"
#ifdef CHEAT
#include "include/cheatman.h"
#endif
#include "modules/iopcore/common/cdvd_config.h"

#include <hdd-ioctl.h>

#define OPL_HDD_MODE_PS2LOGO_OFFSET 0x17F8

extern void *hdd_cdvdman_irx;
extern int size_hdd_cdvdman_irx;

extern void *hdd_hdpro_cdvdman_irx;
extern int size_hdd_hdpro_cdvdman_irx;

extern void *ps2atad_irx;
extern int size_ps2atad_irx;

extern void *hdpro_atad_irx;
extern int size_hdpro_atad_irx;

extern void *ps2hdd_irx;
extern int size_ps2hdd_irx;

extern void *ps2fs_irx;
extern int size_ps2fs_irx;

#ifdef VMC
extern void *hdd_mcemu_irx;
extern int size_hdd_mcemu_irx;
#endif

static unsigned char hddForceUpdate = 1;
static unsigned char hddHDProKitDetected = 0;
static unsigned char hddModulesLoaded = 0;

static char *hddPrefix = "pfs0:";
static hdl_games_list_t hddGames;

const char *oplPart = "hdd0:+OPL";

// forward declaration
static item_list_t hddGameList;

static void hddInitModules(void)
{

    hddLoadModules();

    // update Themes
    char path[256];
    sprintf(path, "%sTHM", hddPrefix);
    thmAddElements(path, "/", hddGameList.mode);

    sbCreateFolders(hddPrefix, 0);
}

// HD Pro Kit is mapping the 1st word in ROM0 seg as a main ATA controller,
// The pseudo ATA controller registers are accessed (input/ouput) by writing
// an id to the main ATA controller
#define HDPROreg_IO8 (*(volatile unsigned char *)0xBFC00000)
#define CDVDreg_STATUS (*(volatile unsigned char *)0xBF40200A)

static int hddCheckHDProKit(void)
{
    int ret = 0;

    DIntr();
    ee_kmode_enter();
    // HD Pro IO start commands sequence
    HDPROreg_IO8 = 0x72;
    CDVDreg_STATUS = 0;
    HDPROreg_IO8 = 0x34;
    CDVDreg_STATUS = 0;
    HDPROreg_IO8 = 0x61;
    CDVDreg_STATUS = 0;
    u32 res = HDPROreg_IO8;
    CDVDreg_STATUS = 0;

    // check result
    if ((res & 0xff) == 0xe7) {
        HDPROreg_IO8 = 0xe3;
        CDVDreg_STATUS = 0;
        // HD Pro IO finish commands sequence
        HDPROreg_IO8 = 0xf3;
        CDVDreg_STATUS = 0;
        ret = 1;
    }
    ee_kmode_exit();
    EIntr();

    if (ret)
        LOG("HDDSUPPORT HD Pro Kit detected!\n");

    return ret;
}

//Taken from libhdd:
#define PFS_ZONE_SIZE 8192
#define PFS_FRAGMENT 0x00000000

static int CreateOPLPartition(const char *oplPart, const char *mountpoint)
{
    int formatArg[3] = {PFS_ZONE_SIZE, 0x2d66, PFS_FRAGMENT};
    int fd, result;
    char cmd[43];

    sprintf(cmd, "%s,128M", oplPart);
    if ((fd = fileXioOpen(cmd, O_CREAT | O_TRUNC | O_RDWR, 0644)) >= 0) {
        fileXioClose(fd);
        result = fileXioFormat(mountpoint, oplPart, (const char *)&formatArg, sizeof(formatArg));
    } else {
        result = fd;
    }

    return result;
}

void hddLoadModules(void)
{
    int ret;
    static char hddarg[] = "-o"
                           "\0"
                           "4"
                           "\0"
                           "-n"
                           "\0"
                           "20";

    LOG("HDDSUPPORT LoadModules\n");

    if (!hddModulesLoaded) {
        hddModulesLoaded = 1;

        // try to detect HD Pro Kit (not the connected HDD),
        // if detected it loads the specific ATAD module
        hddHDProKitDetected = hddCheckHDProKit();
        if (hddHDProKitDetected)
            ret = sysLoadModuleBuffer(&hdpro_atad_irx, size_hdpro_atad_irx, 0, NULL);
        else {
            sysInitDev9();
            ret = sysLoadModuleBuffer(&ps2atad_irx, size_ps2atad_irx, 0, NULL);
        }
        if (ret < 0) {
            LOG("HDD: No HardDisk Drive detected.\n");
            setErrorMessageWithCode(_STR_HDD_NOT_CONNECTED_ERROR, ERROR_HDD_IF_NOT_DETECTED);
            return;
        }

        ret = sysLoadModuleBuffer(&ps2hdd_irx, size_ps2hdd_irx, sizeof(hddarg), hddarg);
        if (ret < 0) {
            LOG("HDD: HardDisk Drive not formatted (HDD).\n");
            setErrorMessageWithCode(_STR_HDD_NOT_FORMATTED_ERROR, ERROR_HDD_MODULE_HDD_FAILURE);
            return;
        }

        //Check if a HDD unit is connected
        ret = fileXioDevctl("hdd0:", APA_DEVCTL_STATUS, NULL, 0, NULL, 0);
        if ((ret >= 3) || (ret < 0)) {
            LOG("HDD: No HardDisk Drive detected.\n");
            setErrorMessageWithCode(_STR_HDD_NOT_CONNECTED_ERROR, ERROR_HDD_NOT_DETECTED);
            return;
        }

        ret = sysLoadModuleBuffer(&ps2fs_irx, size_ps2fs_irx, 0, NULL);
        if (ret < 0) {
            LOG("HDD: HardDisk Drive not formatted (PFS).\n");
            setErrorMessageWithCode(_STR_HDD_NOT_FORMATTED_ERROR, ERROR_HDD_MODULE_PFS_FAILURE);
            return;
        }

        LOG("HDDSUPPORT modules loaded\n");

        hddSetIdleTimeout(gHDDSpindown * 12); // gHDDSpindown [0..20] -> spindown [0..240] -> seconds [0..1200]

        ret = fileXioMount(hddPrefix, oplPart, FIO_MT_RDWR);
        if (ret == -ENOENT) {
            //Attempt to create the partition.
            if ((CreateOPLPartition(oplPart, hddPrefix)) >= 0)
                fileXioMount(hddPrefix, oplPart, FIO_MT_RDWR);
        }
    }
}

void hddInit(void)
{
    LOG("HDDSUPPORT Init\n");
    hddForceUpdate = 1;
    configGetInt(configGetByType(CONFIG_OPL), "hdd_frames_delay", &hddGameList.delay);
    ioPutRequest(IO_CUSTOM_SIMPLEACTION, &hddInitModules);
    hddGameList.enabled = 1;
}

item_list_t *hddGetObject(int initOnly)
{
    if (initOnly && !hddGameList.enabled)
        return NULL;
    return &hddGameList;
}

static int hddNeedsUpdate(void)
{
    if (hddForceUpdate) {
        hddForceUpdate = 0;
        return 1;
    }

    return 0;
}

static int hddUpdateGameList(void)
{
    return (hddGetHDLGamelist(&hddGames) == 0 ? hddGames.count : 0);
}

static int hddGetGameCount(void)
{
    return hddGames.count;
}

static void *hddGetGame(int id)
{
    return (void *)&hddGames.games[id];
}

static char *hddGetGameName(int id)
{
    return hddGames.games[id].name;
}

static int hddGetGameNameLength(int id)
{
    return HDL_GAME_NAME_MAX + 1;
}

static char *hddGetGameStartup(int id)
{
    return hddGames.games[id].startup;
}

#ifndef __CHILDPROOF
static void hddDeleteGame(int id)
{
    hddDeleteHDLGame(&hddGames.games[id]);
    hddForceUpdate = 1;
}

static void hddRenameGame(int id, char *newName)
{
    hdl_game_info_t *game = &hddGames.games[id];
    strcpy(game->name, newName);
    hddSetHDLGameInfo(&hddGames.games[id]);
    hddForceUpdate = 1;
}
#endif

static void hddLaunchGame(int id, config_set_t *configSet)
{
    int i, size_irx = 0;
    int EnablePS2Logo = 0;
#ifdef CHEAT
    int result;
#endif
    void **irx = NULL;
    char filename[32];
    hdl_game_info_t *game = &hddGames.games[id];
    struct cdvdman_settings_hdd *settings;

#ifdef VMC
    apa_header_t part_hdr;
    char vmc_name[2][32];
    int fd, part_valid = 0, size_mcemu_irx = 0;
    hdd_vmc_infos_t hdd_vmc_infos;
    memset(&hdd_vmc_infos, 0, sizeof(hdd_vmc_infos_t));

    configGetVMC(configSet, vmc_name[0], sizeof(vmc_name[0]), 0);
    configGetVMC(configSet, vmc_name[1], sizeof(vmc_name[1]), 1);

    if (vmc_name[0][0] || vmc_name[1][0]) {
        fileXioUmount(hddPrefix);
        fd = fileXioOpen(oplPart, O_RDONLY, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH);
        if (fd >= 0) {
            if (fileXioIoctl2(fd, APA_IOCTL2_GETHEADER, NULL, 0, (void *)&part_hdr, sizeof(apa_header_t)) == sizeof(apa_header_t)) {
                if (part_hdr.nsub <= 4) {
                    hdd_vmc_infos.parts[0].start = part_hdr.start;
                    hdd_vmc_infos.parts[0].length = part_hdr.length;
                    LOG("HDDSUPPORT hdd_vmc_infos.parts[0].start : 0x%X\n", hdd_vmc_infos.parts[0].start);
                    LOG("HDDSUPPORT hdd_vmc_infos.parts[0].length : 0x%X\n", hdd_vmc_infos.parts[0].length);
                    for (i = 0; i < part_hdr.nsub; i++) {
                        hdd_vmc_infos.parts[i + 1].start = part_hdr.subs[i].start;
                        hdd_vmc_infos.parts[i + 1].length = part_hdr.subs[i].length;
                        LOG("HDDSUPPORT hdd_vmc_infos.parts[%d].start : 0x%X\n", i + 1, hdd_vmc_infos.parts[i + 1].start);
                        LOG("HDDSUPPORT hdd_vmc_infos.parts[%d].length : 0x%X\n", i + 1, hdd_vmc_infos.parts[i + 1].length);
                    }
                    part_valid = 1;
                }
            }
            fileXioClose(fd);
        }
        fileXioMount(hddPrefix, oplPart, FIO_MT_RDWR); // if this fails, something is really screwed up
    }

    if (part_valid) {
        char vmc_path[256];
        int vmc_id, have_error = 0;
        vmc_superblock_t vmc_superblock;
        pfs_inode_t pfs_inode;

        for (vmc_id = 0; vmc_id < 2; vmc_id++) {
            if (vmc_name[vmc_id][0]) {
                have_error = 1;
                hdd_vmc_infos.active = 0;
                if (sysCheckVMC(hddPrefix, "/", vmc_name[vmc_id], 0, &vmc_superblock) > 0) {
                    hdd_vmc_infos.flags = vmc_superblock.mc_flag & 0xFF;
                    hdd_vmc_infos.flags |= 0x100;
                    hdd_vmc_infos.specs.page_size = vmc_superblock.page_size;
                    hdd_vmc_infos.specs.block_size = vmc_superblock.pages_per_block;
                    hdd_vmc_infos.specs.card_size = vmc_superblock.pages_per_cluster * vmc_superblock.clusters_per_card;

                    // Check vmc inode block chain (write operation can cause damage)
                    snprintf(vmc_path, sizeof(vmc_path), "%sVMC/%s.bin", hddPrefix, vmc_name[vmc_id]);
                    fd = fileXioOpen(vmc_path, O_RDWR, FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IXUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IXGRP | FIO_S_IROTH | FIO_S_IWOTH | FIO_S_IXOTH);
                    if (fileXioIoctl2(fd, PFS_IOCTL2_GET_INODE, NULL, 0, (void *)&pfs_inode, sizeof(pfs_inode_t)) == sizeof(pfs_inode_t)) {
                        if (pfs_inode.number_data <= 11) {
                            have_error = 0;
                            hdd_vmc_infos.active = 1;
                            for (i = 0; i < pfs_inode.number_data - 1; i++) {
                                hdd_vmc_infos.blocks[i].number = pfs_inode.data[i + 1].number;
                                hdd_vmc_infos.blocks[i].subpart = pfs_inode.data[i + 1].subpart;
                                hdd_vmc_infos.blocks[i].count = pfs_inode.data[i + 1].count;
                                LOG("HDDSUPPORT hdd_vmc_infos.blocks[%d].number     : 0x%X\n", i, hdd_vmc_infos.blocks[i].number);
                                LOG("HDDSUPPORT hdd_vmc_infos.blocks[%d].subpart    : 0x%X\n", i, hdd_vmc_infos.blocks[i].subpart);
                                LOG("HDDSUPPORT hdd_vmc_infos.blocks[%d].count      : 0x%X\n", i, hdd_vmc_infos.blocks[i].count);
                            }
                        } // else Vmc file too much fragmented
                    }
                    fileXioClose(fd);
                }

                if (have_error) {
                    char error[256];
                    snprintf(error, sizeof(error), _l(_STR_ERR_VMC_CONTINUE), vmc_name[vmc_id], (vmc_id + 1));
                    if (!guiMsgBox(error, 1, NULL))
                        return;
                }

                for (i = 0; i < size_hdd_mcemu_irx; i++) {
                    if (((u32 *)&hdd_mcemu_irx)[i] == (0xC0DEFAC0 + vmc_id)) {
                        if (hdd_vmc_infos.active)
                            size_mcemu_irx = size_hdd_mcemu_irx;
                        memcpy(&((u32 *)&hdd_mcemu_irx)[i], &hdd_vmc_infos, sizeof(hdd_vmc_infos_t));
                        break;
                    }
                }
            }
        }
    }
#endif

    if (gRememberLastPlayed) {
        configSetStr(configGetByType(CONFIG_LAST), "last_played", game->startup);
        saveConfig(CONFIG_LAST, 0);
    }

    char gid[5];
    configGetDiscIDBinary(configSet, gid);

    int dmaType = 0, dmaMode = 7, compatMode = 0;
    configGetInt(configSet, CONFIG_ITEM_COMPAT, &compatMode);
    configGetInt(configSet, CONFIG_ITEM_DMA, &dmaMode);
    if (dmaMode < 3)
        dmaType = 0x20;
    else {
        dmaType = 0x40;
        dmaMode -= 3;
    }
    hddSetTransferMode(dmaType, dmaMode);
    hddSetIdleTimeout(gHDDSpindown * 12);

    if (hddHDProKitDetected) {
        size_irx = size_hdd_hdpro_cdvdman_irx;
        irx = &hdd_hdpro_cdvdman_irx;
    } else {
        size_irx = size_hdd_cdvdman_irx;
        irx = &hdd_cdvdman_irx;
    }

    sbPrepare(NULL, configSet, size_irx, irx, &i);

#ifdef CHEAT
    if ((result = sbLoadCheats(hddPrefix, game->startup)) < 0) {
        switch (result) {
            case -ENOENT:
                guiWarning(_l(_STR_NO_CHEATS_FOUND), 10);
                break;
            default:
                guiWarning(_l(_STR_ERR_CHEATS_LOAD_FAILED), 10);
        }
    }
#endif

    settings = (struct cdvdman_settings_hdd *)((u8 *)irx + i);

    // patch 48bit flag
    settings->common.media = hddIs48bit() & 0xff;

    // patch start_sector
    settings->lba_start = game->start_sector;

    if (configGetStrCopy(configSet, CONFIG_ITEM_ALTSTARTUP, filename, sizeof(filename)) == 0)
        strcpy(filename, game->startup);

    if (gPS2Logo)
        EnablePS2Logo = CheckPS2Logo(0, game->start_sector + OPL_HDD_MODE_PS2LOGO_OFFSET);

    deinit(NO_EXCEPTION); // CAREFUL: deinit will call hddCleanUp, so hddGames/game will be freed

#ifdef VMC
#define HDD_MCEMU size_mcemu_irx, &hdd_mcemu_irx,
#else
#define HDD_MCEMU
#endif

    sysLaunchLoaderElf(filename, "HDD_MODE", size_irx, irx, HDD_MCEMU EnablePS2Logo, compatMode);
}

static config_set_t *hddGetConfig(int id)
{
    char path[256];
    hdl_game_info_t *game = &hddGames.games[id];

#ifdef OPL_IS_DEV_BUILD
    snprintf(path, sizeof(path), "%sCFG-DEV/%s.cfg", hddPrefix, game->startup);
#else
    snprintf(path, sizeof(path), "%sCFG/%s.cfg", hddPrefix, game->startup);
#endif
    config_set_t *config = configAlloc(0, NULL, path);
    configRead(config);

    configSetStr(config, CONFIG_ITEM_NAME, game->name);
    configSetInt(config, CONFIG_ITEM_SIZE, game->total_size_in_kb >> 10);
    configSetStr(config, CONFIG_ITEM_FORMAT, "HDL");
    configSetStr(config, CONFIG_ITEM_MEDIA, game->disctype == 0x12 ? "CD" : "DVD");
    configSetStr(config, CONFIG_ITEM_STARTUP, game->startup);

    return config;
}

static int hddGetImage(char *folder, int isRelative, char *value, char *suffix, GSTEXTURE *resultTex, short psm)
{
    char path[256];
    if (isRelative)
        sprintf(path, "%s%s/%s_%s", hddPrefix, folder, value, suffix);
    else
        sprintf(path, "%s%s_%s", folder, value, suffix);
    return texDiscoverLoad(resultTex, path, -1, psm);
}

static void hddCleanUp(int exception)
{
    LOG("HDDSUPPORT CleanUp\n");

    if (hddGameList.enabled) {
        hddFreeHDLGamelist(&hddGames);

        if ((exception & UNMOUNT_EXCEPTION) == 0)
            fileXioUmount(hddPrefix);
    }

    hddModulesLoaded = 0;
}

#ifdef VMC
static int hddCheckVMC(char *name, int createSize)
{
    return sysCheckVMC(hddPrefix, "/", name, createSize, NULL);
}
#endif

static item_list_t hddGameList = {
    HDD_MODE, 0, MODE_FLAG_COMPAT_DMA, MENU_MIN_INACTIVE_FRAMES, HDD_MODE_UPDATE_DELAY, "HDD Games", _STR_HDD_GAMES, &hddInit, &hddNeedsUpdate, &hddUpdateGameList,
#ifdef __CHILDPROOF
    &hddGetGameCount, &hddGetGame, &hddGetGameName, &hddGetGameNameLength, &hddGetGameStartup, NULL, NULL,
#else
    &hddGetGameCount, &hddGetGame, &hddGetGameName, &hddGetGameNameLength, &hddGetGameStartup, &hddDeleteGame, &hddRenameGame,
#endif
#ifdef VMC
    &hddLaunchGame, &hddGetConfig, &hddGetImage, &hddCleanUp, &hddCheckVMC, HDD_ICON
#else
    &hddLaunchGame, &hddGetConfig, &hddGetImage, &hddCleanUp, HDD_ICON
#endif
};
