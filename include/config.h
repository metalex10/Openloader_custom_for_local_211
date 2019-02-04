#ifndef __CONFIG_H
#define __CONFIG_H

//Enum for the different types of config files. Game-specific config files (<game ID>.cfg) will always have an ID of 0.
enum CONFIG_INDEX {
    CONFIG_INDEX_OPL = 0,
    CONFIG_INDEX_LAST,
    CONFIG_INDEX_APPS,
    CONFIG_INDEX_NETWORK,

    CONFIG_INDEX_COUNT
};

//Config type bits
#define CONFIG_OPL (1 << CONFIG_INDEX_OPL)
#define CONFIG_LAST (1 << CONFIG_INDEX_LAST)
#define CONFIG_APPS (1 << CONFIG_INDEX_APPS)
#define CONFIG_NETWORK (1 << CONFIG_INDEX_NETWORK)
#define CONFIG_ALL 0xFF

#define CONFIG_SOURCE_DEFAULT 0
#define CONFIG_SOURCE_USER 1
#define CONFIG_SOURCE_DLOAD 2 //Downloaded from the network

//Items for game config files.
#define CONFIG_ITEM_NAME "#Name"
#define CONFIG_ITEM_LONGNAME "#LongName"
#define CONFIG_ITEM_SIZE "#Size"
#define CONFIG_ITEM_FORMAT "#Format"
#define CONFIG_ITEM_MEDIA "#Media"
#define CONFIG_ITEM_STARTUP "#Startup"
#define CONFIG_ITEM_ALTSTARTUP "$AltStartup"
#define CONFIG_ITEM_VMC "$VMC"
#define CONFIG_ITEM_COMPAT "$Compatibility"
#define CONFIG_ITEM_DMA "$DMA"
#define CONFIG_ITEM_DNAS "$DNAS"
#define CONFIG_ITEM_CONFIGSOURCE "$ConfigSource"

//Per-Game GSM keys. -Bat-
#define CONFIG_ITEM_ENABLEGSM "$EnableGSM"
#define CONFIG_ITEM_GSMVMODE "$GSMVMode"
#define CONFIG_ITEM_GSMXOFFSET "$GSMXOffset"
#define CONFIG_ITEM_GSMYOFFSET "$GSMYOffset"

//Per-Game CHEAT keys. -Bat-
#define CONFIG_ITEM_ENABLECHEAT "$EnableCheat"
#define CONFIG_ITEM_CHEATMODE "$CheatMode"

//OPL config keys
#define CONFIG_OPL_THEME "theme"
#define CONFIG_OPL_LANGUAGE "language_text"
#define CONFIG_OPL_SCROLLING "scrolling"
#define CONFIG_OPL_BGCOLOR "bg_color"
#define CONFIG_OPL_TEXTCOLOR "text_color"
#define CONFIG_OPL_UI_TEXTCOLOR "ui_text_color"
#define CONFIG_OPL_SEL_TEXTCOLOR "sel_text_color"
#define CONFIG_OPL_USE_INFOSCREEN "use_info_screen"
#define CONFIG_OPL_ENABLE_COVERART "enable_coverart"
#define CONFIG_OPL_WIDESCREEN "wide_screen"
#define CONFIG_OPL_VMODE "vmode"
#define CONFIG_OPL_DISABLE_DEBUG "disable_debug"
#define CONFIG_OPL_PS2LOGO "ps2logo"
#define CONFIG_OPL_EXIT_PATH "exit_path"
#define CONFIG_OPL_AUTO_SORT "autosort"
#define CONFIG_OPL_AUTO_REFRESH "autorefresh"
#define CONFIG_OPL_DEFAULT_DEVICE "default_device"
#define CONFIG_OPL_ENABLE_WRITE "enable_delete_rename"
#define CONFIG_OPL_HDD_SPINDOWN "hdd_spindown"
#define CONFIG_OPL_USB_CHECK_FRAG "check_usb_frag"
#define CONFIG_OPL_USB_PREFIX "usb_prefix"
#define CONFIG_OPL_ETH_PREFIX "eth_prefix"
#define CONFIG_OPL_REMEMBER_LAST "remember_last"
#define CONFIG_OPL_AUTOSTART_LAST "autostart_last"
#define CONFIG_OPL_USB_MODE "usb_mode"
#define CONFIG_OPL_HDD_MODE "hdd_mode"
#define CONFIG_OPL_ETH_MODE "eth_mode"
#define CONFIG_OPL_APP_MODE "app_mode"
#define CONFIG_OPL_SWAP_SEL_BUTTON "swap_select_btn"

//Network config keys
#define CONFIG_NET_ETH_LINKM "eth_linkmode"
#define CONFIG_NET_PS2_DHCP "ps2_ip_use_dhcp"
#define CONFIG_NET_PS2_IP "ps2_ip_addr"
#define CONFIG_NET_PS2_NETM "ps2_netmask"
#define CONFIG_NET_PS2_GATEW "ps2_gateway"
#define CONFIG_NET_PS2_DNS "ps2_dns"
#define CONFIG_NET_SMB_NB_ADDR "smb_share_nb_addr"
#define CONFIG_NET_SMB_IP_ADDR "smb_ip"
#define CONFIG_NET_SMB_NBNS "smb_share_use_nbns"
#define CONFIG_NET_SMB_SHARE "smb_share"
#define CONFIG_NET_SMB_USER "smb_user"
#define CONFIG_NET_SMB_PASSW "smb_pass"
#define CONFIG_NET_SMB_PORT "smb_port"

#define CONFIG_KEY_NAME_LEN 32
#define CONFIG_KEY_VALUE_LEN 256

struct config_value_t
{
    //Including the NULL terminator
    char key[CONFIG_KEY_NAME_LEN];
    char val[CONFIG_KEY_VALUE_LEN];

    struct config_value_t *next;
};

typedef struct
{
    int type;
    struct config_value_t *head;
    struct config_value_t *tail;
    char *filename;
    int modified;
    u32 uid;
} config_set_t;

void configInit(char *prefix);
void configEnd();
config_set_t *configAlloc(int type, config_set_t *configSet, char *fileName);
void configFree(config_set_t *configSet);
config_set_t *configGetByType(int type);
int configSetStr(config_set_t *configSet, const char *key, const char *value);
int configGetStr(config_set_t *configSet, const char *key, const char **value);
int configGetStrCopy(config_set_t *configSet, const char *key, char *value, int length);
int configSetInt(config_set_t *configSet, const char *key, const int value);
int configGetInt(config_set_t *configSet, const char *key, int *value);
int configSetColor(config_set_t *configSet, const char *key, unsigned char *color);
int configGetColor(config_set_t *configSet, const char *key, unsigned char *color);
int configRemoveKey(config_set_t *configSet, const char *key);
void configMerge(config_set_t *dest, const config_set_t *source);

int configReadLegacyIP(void);
void configGetDiscIDBinary(config_set_t *configSet, void *dst);

int configRead(config_set_t *configSet);
int configReadBuffer(config_set_t *configSet, const void *buffer, int size);
int configReadMulti(int types);
int configWrite(config_set_t *configSet);
int configWriteMulti(int types);
int configGetStat(config_set_t *configSet, iox_stat_t *stat);
void configClear(config_set_t *configSet);

#ifdef VMC
void configGetVMC(config_set_t *configSet, char *vmc, int length, int slot);
void configSetVMC(config_set_t *configSet, const char *vmc, int slot);
void configRemoveVMC(config_set_t *configSet, int slot);
#endif

#endif
