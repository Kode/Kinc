#pragma once

struct HMONITOR__;

struct DisplayData {
	HMONITOR__* id;
	char name[32];
	bool primary, available;
	int x, y, width, height, ppi, frequency;
	DisplayData();
};
