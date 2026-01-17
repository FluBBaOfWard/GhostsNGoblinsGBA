#include <gba.h>
#include <string.h>

#include "FileHandling.h"
#include "Shared/EmuMenu.h"
#include "Shared/EmuSettings.h"
#include "Shared/AsmExtra.h"
#include "Emubase.h"
#include "Main.h"
#include "Gui.h"
#include "Cart.h"
#include "Gfx.h"
#include "io.h"
#include "GhostsNGoblins.h"

static const char *const folderName = "acds";
static const char *const settingName = "settings.cfg";

EWRAM_BSS int selectedGame = 0;
EWRAM_BSS ConfigData cfg;

static bool loadRoms(int gameNr, bool doLoad);

#define GAMECOUNT (3)
static const int gameCount = GAME_COUNT;
static const char *const gameNames[GAME_COUNT] = {"gng","makai","gngb"};
static const char *const gameZipNames[GAME_COUNT] = {"gng.zip","makai.zip","gngb.zip"};
static const int fileCount[GAMECOUNT] = {25,25,25};
static const char *const romFilenames[GAME_COUNT][25] = {
	{"na-5.ic52","nb-5.ic51",     "n0-5.ic13", "nc-5.bin",    "n1-5.ic1","n2-5.ic14","n6-5.ic28","n7-5.ic27","n8-5.ic26","n9-5.ic25",           "nh-5.bin","nn-5.bin","ni-5.bin","no-5.bin","nd-5.bin","nj-5.bin","ne-5.bin","nk-5.bin","nf-5.bin","nl-5.bin","ng-5.bin","nm-5.bin",                                     "n5-5.ic31","n4-5.ic32","n3-5.ic33"},
	{"ta18-11.bin","nb-01.bin",   "n0-5.bin",  "ta18-25.bin", "ta18-01.bin","ta18-06.bin","n7-5.bin","ta18-02.bin","ta18-04.bin","ta18-03.bin", "ta18-20.bin","ta18-14.bin","ta18-19.bin","ta18-13.bin","ta18-24.bin","ta18-18.bin","ta18-23.bin","ta18-17.bin","ta18-22.bin","ta18-16.bin","ta18-21.bin","ta18-15.bin", "ta18-07.bin","ta18-08.bin","ta18-09.bin"},
	{"ta18-11.bin","ta18-10.bin", "n0-5.bin",  "ta18-25.bin", "ta18-01.bin","ta18-06.bin","n7-5.bin","ta18-02.bin","ta18-04.bin","ta18-03.bin", "ta18-20.bin","ta18-14.bin","ta18-19.bin","ta18-13.bin","ta18-24.bin","ta18-18.bin","ta18-23.bin","ta18-17.bin","ta18-22.bin","ta18-16.bin","ta18-21.bin","ta18-15.bin", "ta18-07.bin","ta18-08.bin","ta18-09.bin"}
};
static const int romFilesizes[GAME_COUNT][25] = {
	{0x8000,0x8000, 0x8000, 0x8000, 0x8000,0x8000,0x8000,0x8000,0x8000,0x8000, 0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000, 0x8000,0x8000,0x8000},
	{0x8000,0x8000, 0x8000, 0x8000, 0x8000,0x8000,0x8000,0x8000,0x8000,0x8000, 0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000, 0x8000,0x8000,0x8000},
	{0x8000,0x8000, 0x8000, 0x8000, 0x8000,0x8000,0x8000,0x8000,0x8000,0x8000, 0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000,0x8000, 0x8000,0x8000,0x8000}
};

//---------------------------------------------------------------------------------
void applyConfigData(void) {
	emuSettings  = cfg.emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	gScaling     = cfg.scaling & 1;
	gFlicker     = cfg.flicker & 1;
	gGammaValue  = cfg.gammaValue;
	sleepTime    = cfg.sleepTime;
	joyCfg       = (joyCfg & ~0x400) | ((cfg.controller & 1) << 10);
	gDipSwitch0  = cfg.dipSwitch0;
	gDipSwitch1  = cfg.dipSwitch1;
	gDipSwitch2  = cfg.dipSwitch2;
}

void updateConfigData(void) {
	strcpy(cfg.magic, "cfg");
	cfg.emuSettings = emuSettings & ~EMUSPEED_MASK; // Clear speed setting.
	cfg.scaling     = gScaling & 1;
	cfg.flicker     = gFlicker & 1;
	cfg.gammaValue  = gGammaValue;
	cfg.sleepTime   = sleepTime;
	cfg.controller  = (joyCfg >> 10) & 1;
	cfg.dipSwitch0  = gDipSwitch0;
	cfg.dipSwitch1  = gDipSwitch1;
	cfg.dipSwitch2  = gDipSwitch2;
}

void initSettings() {
	memset(&cfg, 0, sizeof(ConfigData));
	cfg.emuSettings = AUTOPAUSE_EMULATION | AUTOLOAD_NVRAM;
	cfg.scaling     = SCALED;
	cfg.flicker     = 1;
	cfg.sleepTime   = 60*60*5;

	applyConfigData();
}

int loadSettings() {
	bytecopy_((u8 *)&cfg, (u8 *)SRAM+0x10000-sizeof(ConfigData), sizeof(ConfigData));
	if (strstr(cfg.magic, "cfg")) {
		applyConfigData();
		infoOutput("Settings loaded.");
		return 0;
	}
	else {
		updateConfigData();
		infoOutput("Error in settings file.");
	}
	return 1;
}
void saveSettings() {
	updateConfigData();

	bytecopy_((u8 *)SRAM+0x10000-sizeof(ConfigData), (u8 *)&cfg, sizeof(ConfigData));
	infoOutput("Settings saved.");
}

int loadNVRAM() {
	return 0;
}

void saveNVRAM() {
}

void loadState(void) {
	unpackState(testState);
	infoOutput("Loaded state.");
}
void saveState(void) {
	packState(testState);
	infoOutput("Saved state.");
}
/*
void loadState(void) {
	u32 *statePtr;
//	FILE *file;
	char stateName[32];

	if (findFolder(folderName)) {
		return;
	}
	strlcpy(stateName, gameNames[selectedGame], sizeof(stateName));
	strlcat(stateName, ".sta", sizeof(stateName));
	int stateSize = getStateSize();
	if ((file = fopen(stateName, "r"))) {
		if ((statePtr = malloc(stateSize))) {
			fread(statePtr, 1, stateSize, file);
			unpackState(statePtr);
			free(statePtr);
			infoOutput("Loaded state.");
		}
		else {
			infoOutput("Couldn't alloc mem for state.");
		}
		fclose(file);
	}
}

void saveState(void) {
	u32 *statePtr;
//	FILE *file;
	char stateName[32];

	if (findFolder(folderName)) {
		return;
	}
	strlcpy(stateName, gameNames[selectedGame], sizeof(stateName));
	strlcat(stateName, ".sta", sizeof(stateName));
	int stateSize = getStateSize();
	if ((file = fopen(stateName, "w"))) {
		if ( (statePtr = malloc(stateSize))) {
			packState(statePtr);
			fwrite(statePtr, 1, stateSize, file);
			free(statePtr);
			infoOutput("Saved state.");
		}
		else {
			infoOutput("Couldn't alloc mem for state.");
		}
		fclose(file);
	}
}
*/
//---------------------------------------------------------------------------------
bool loadGame() {
	if (loadRoms(selected, false)) {
		return true;
	}
	selectedGame = selected;
	loadRoms(selectedGame, true);
	setEmuSpeed(0);
	loadCart(selectedGame,0);
	if (emuSettings & 4) {
		loadState();
	}
	return false;
}

bool loadRoms(int game, bool doLoad) {
//	int i, j, count;
//	bool found;
//	u8 *romArea = ROM_Space;
//	FILE *file;

//	count = fileCount[game];
/*
	chdir("/");			// Stupid workaround.
	if (chdir(currentDir) == -1) {
		return true;
	}

	for (i=0; i<count; i++) {
		found = false;
		if ((file = fopen(romFilenames[game][i], "r"))) {
			if (doLoad) {
				fread(romArea, 1, romFilesizes[game][i], file);
				romArea += romFilesizes[game][i];
			}
			fclose(file);
			found = true;
		}
		else {
			for (j=0; j<GAMECOUNT; j++) {
				if (!(findFileInZip(gameZipNames[j], romFilenames[game][i]))) {
					if (doLoad) {
						loadFileInZip(romArea, gameZipNames[j], romFilenames[game][i], romFilesizes[game][i]);
						romArea += romFilesizes[game][i];
					}
					found = true;
					break;
				}
			}
		}
		if (!found) {
			infoOutput("Couldn't open file:");
			infoOutput(romFilenames[game][i]);
			return true;
		}
	}
*/
	return false;
}
