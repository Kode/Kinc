#include "pch.h"

#include "filewriter.h"

#include <kinc/error.h>
#include <kinc/log.h>
#include <kinc/system.h>

#include <stdio.h>
#include <string.h>

#if defined(KORE_PS4) || defined(KORE_SWITCH)
#define MOUNT_SAVES
bool mountSaveData(bool);
void unmountSaveData();
#endif

bool Kinc_FileWriter_Open(Kinc_FileWriter *writer, const char *filepath) {
#ifdef MOUNT_SAVES
	if (!mountSaveData(true)) {
		return false;
	}
#endif
	char path[1001];
	strcpy(path, Kinc_Internal_SavePath());
	strcat(path, filepath);
	writer->file = fopen(path, "wb");
	if (writer->file == NULL) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Could not open file %s.", filepath);
		return false;
	}
	return true;
}

void Kinc_FileWriter_Close(Kinc_FileWriter *writer) {
	if (writer->file == NULL) {
		return;
	}
	fclose((FILE*)writer->file);
	writer->file = NULL;
#ifdef MOUNT_SAVES
	unmountSaveData();
#endif
}

void Kinc_FileWriter_Write(Kinc_FileWriter *writer, void *data, int size) {
	fwrite(data, 1, size, (FILE*)writer->file);
}
