#include "HIDManager.h"
#include "../pch.h"

using namespace Kore;

HIDManager::HIDManager() {
	gamepads = 12;
	gamepadList = new std::vector<HIDGamepad*>();
	for (int i = 0; i < gamepads; i++) {
		gamepadList->push_back(new HIDGamepad(i));
	}
}

HIDManager::~HIDManager() {
	for (int i = 0; i < gamepads; i++) {
		delete gamepadList->at(i);
	}
	gamepadList->clear();
	delete gamepadList;
	gamepadList = NULL;
}

void HIDManager::Update() {
	for (int i = 0; i < gamepads; i++) {
		gamepadList->at(i)->Update();
	}
}
