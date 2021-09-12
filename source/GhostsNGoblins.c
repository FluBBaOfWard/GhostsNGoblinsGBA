#include <gba.h>

#include "GhostsNGoblins.h"
#include "Gfx.h"
#include "ARM6809/ARM6809.h"
#include "GnGVideo/GnGVideo.h"


int packState(void *statePtr) {
	int size = gngSaveState(statePtr, &gngVideo_0);
	size += m6809SaveState(statePtr+size, &m6809OpTable);
	return size;
}

void unpackState(const void *statePtr) {
	int size = gngLoadState(&gngVideo_0, statePtr);
	m6809LoadState(&m6809OpTable, statePtr+size);
}

int getStateSize() {
	int size = gngGetStateSize();
	size += m6809GetStateSize();
	return size;
}
