#import "GLView.h"
#import "GLViewController.h"

#import <Foundation/Foundation.h>

#include <kinc/graphics5/rendertarget.h>
#include <kinc/math/core.h>

#include <objc/runtime.h>

static GLView *glView;

static bool visible;

void beginGL() {
#ifdef KORE_METAL
	if (!visible) {
		return;
	}
#endif
	[glView begin];
}

void endGL() {
#ifdef KORE_METAL
	if (!visible) {
		return;
	}
#endif
	[glView end];
}

void showKeyboard() {
	[glView showKeyboard];
}

void hideKeyboard() {
	[glView hideKeyboard];
}

#ifdef KORE_METAL

CAMetalLayer *getMetalLayer() {
	return [glView metalLayer];
}

id getMetalDevice() {
	return [glView metalDevice];
}

id getMetalLibrary() {
	return [glView metalLibrary];
}

id getMetalQueue() {
	return [glView metalQueue];
}

#endif

@implementation GLViewController

- (void)loadView {
	visible = true;
	self.view = glView = [[GLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
}

- (void)setVisible:(BOOL)value {
	visible = value;
}

@end
