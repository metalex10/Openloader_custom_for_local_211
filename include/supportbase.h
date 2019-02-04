#ifndef __SUPPORT_BASE_H
#define __SUPPORT_BASE_H

#define UL_GAME_NAME_MAX 32
#define ISO_GAME_NAME_MAX 64
#define GAME_STARTUP_MAX 12

enum GAME_FORMAT {
    GAME_FORMAT_USBLD = 0,
    GAME_FORMAT_OLD_ISO,
    GAME_FORMAT_ISO,
};

typedef struct
{
    char name[ISO_GAME_NAME_MAX + 1]; // MUST be the higher value from UL / ISO
    char startup[GAME_STARTUP_MAX + 1];
    char extension[5];
    unsigned char parts;
    unsigned char media;
    unsigned short format;
    int sizeMB;
} base_game_info_t;

typedef struct
{
    unsigned char name[UL_GAME_NAME_MAX];
    unsigned char startup[15];
    unsigned char parts;
    unsigned char media; //Disc type
    unsigned char unknown[4];
    unsigned char Byte08; //Always 0x08
    unsigned char unknown2[10];
} USBExtreme_game_entry_t;

int sbIsSameSize(const char *prefix, int prevSize);
int sbCreateSemaphore(void);
int sbReadList(base_game_info_t **list, const char *prefix, int *fsize, int *gamecount);
int sbPrepare(base_game_info_t *game, config_set_t *configSet, int size_cdvdman, void **cdvdman_irx, int *patchindex);
void sbUnprepare(void *pCommon);
void sbDelete(base_game_info_t **list, const char *prefix, const char *sep, int gamecount, int id);
void sbRename(base_game_info_t **list, const char *prefix, const char *sep, int gamecount, int id, char *newname);
config_set_t *sbPopulateConfig(base_game_info_t *game, const char *prefix, const char *sep);
void sbCreateFolders(const char *path, int createDiscImgFolders);

//ISO9660 filesystem management functions.
u32 sbGetISO9660MaxLBA(const char *path);
int sbProbeISO9660(const char *path, base_game_info_t *game, u32 layer1_offset);
int sbProbeISO9660_64(const char *path, base_game_info_t *game, u32 layer1_offset);

#ifdef CHEAT
int sbLoadCheats(const char *path, const char *file);
#endif

#endif
