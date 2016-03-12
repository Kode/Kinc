#include "../pch.h"
#include <Kore/Input/Mouse.h>
#include <Kore/Log.h>
#include <Kore/System.h>



using namespace Kore;

void Mouse::_lock(int windowId, bool truth){
    show(!truth);
    if (truth) {
        int width = System::windowWidth(windowId);
        int height = System::windowHeight(windowId);

        int x, y;
        getPosition(windowId, x, y);

        // Guess the new position of X and Y
        int newX = x;
        int newY = y;

        // Correct the position of the X coordinate
        // if the mouse is out the window
        if (x < 0) {
            newX -= x;
        }
        else if (x > width) {
            newX -= x - width;
        }

        // Correct the position of the Y coordinate
        // if the mouse is out the window
        if (y < 0) {
            newY -= y;
        }
        else if (y > height){
            newY -= y - height;
        }

        // Force the mouse to stay inside the window
        setPosition(windowId, newX, newY);
    }
}

bool Mouse::canLock(int windowId){
	return true;
}

void Mouse::show(bool truth){
	//TODO
}

void Mouse::setPosition(int windowId, int x, int y){

}

void Mouse::getPosition(int windowId, int& x, int& y){

}
