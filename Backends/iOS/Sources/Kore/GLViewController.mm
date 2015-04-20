#import "GLViewController.h"
#import "GLView.h"
#include "pch.h"
#import <Foundation/Foundation.h>
#include <Kore/Math/Core.h>

static GLView* glView;

void beginGL() {
	[glView begin];
}

void endGL() {
	[glView end];
}

void showKeyboard() {
	[glView showKeyboard];
}

void hideKeyboard() {
	[glView hideKeyboard];
}

@implementation GLViewController

- (void)loadView {
	self.view = glView = [[GLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
}

@end
