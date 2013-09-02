#import "BasicOpenGLView.h"
#include <Kore/pch.h>
#include <Kore/Application.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/KeyEvent.h>
#include <Kore/Input/Mouse.h>

@implementation BasicOpenGLView

+ (NSOpenGLPixelFormat*) basicPixelFormat {
	NSOpenGLPixelFormatAttribute attributes[] = {
		//NSOpenGLPFAWindow,
		NSOpenGLPFADoubleBuffer,	// double buffered
		NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
		NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
		(NSOpenGLPixelFormatAttribute)nil
	};
	return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
}

- (void)switchBuffers {
	[[self openGLContext] flushBuffer];
}

- (void)keyDown:(NSEvent*)theEvent {
	NSString* characters = [theEvent characters];
	if ([characters length]) {
		unichar ch = [characters characterAtIndex:0];
		if (ch >= L'a' && ch <= L'z') {
			switch (ch) {
			case L'a':
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Left));
				break;
			case L'w':
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Up));
				break;
			case L'd':
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Right));
				break;
			default:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(ch + L'A' - L'a'));
				break;
			}
		}
		else {
			switch (ch) {
			case NSRightArrowFunctionKey:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Right));
				break;
			case NSLeftArrowFunctionKey:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Left));
				break;
			case NSUpArrowFunctionKey:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Up));
				break;
			case NSDownArrowFunctionKey:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(Kore::Key_Down));
				break;
			default:
				Kore::Keyboard::the()->keydown(Kore::KeyEvent(ch));
				break;	
			}
		}
	}
}

- (void)keyUp:(NSEvent*)theEvent {
	NSString* characters = [theEvent characters];
    if ([characters length]) {
        unichar ch = [characters characterAtIndex:0];
		if (ch >= L'a' && ch <= L'z') {
			switch (ch) {
			case L'a':
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Left));
				break;
			case L'w':
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Up));
				break;
			case L'd':
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Right));
				break;
			default:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(ch + L'A' - L'a'));
				break;
			}
		}
		else {
			switch (ch) {
			case NSRightArrowFunctionKey:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Right));
				break;
			case NSLeftArrowFunctionKey:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Left));
				break;
			case NSUpArrowFunctionKey:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Up));
				break;
			case NSDownArrowFunctionKey:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(Kore::Key_Down));
				break;
			default:
				Kore::Keyboard::the()->keyup(Kore::KeyEvent(ch));
				break;	
			}
		}
	}
}

namespace {
	int getMouseX(NSEvent* event) {
		return static_cast<int>([event locationInWindow].x);
	}
	
	int getMouseY(NSEvent* event) {
		return static_cast<int>(Kore::Application::the()->height() - [event locationInWindow].y);
	}
}

- (void)mouseDown:(NSEvent*)theEvent {
	Kore::Mouse::the()->_pressLeft(Kore::MouseEvent(getMouseX(theEvent), getMouseY(theEvent)));
}

- (void)mouseUp:(NSEvent*)theEvent {
	Kore::Mouse::the()->_releaseLeft(Kore::MouseEvent(getMouseX(theEvent), getMouseY(theEvent)));
}

- (void)mouseMoved:(NSEvent*)theEvent {
	Kore::Mouse::the()->_move(Kore::MouseEvent(getMouseX(theEvent), getMouseY(theEvent)));
}

- (void)mouseDragged:(NSEvent*)theEvent {
	Kore::Mouse::the()->_move(Kore::MouseEvent(getMouseX(theEvent), getMouseY(theEvent)));
}

- (void)rightMouseDown:(NSEvent*)theEvent {
	Kore::Mouse::the()->_pressRight(Kore::MouseEvent(getMouseX(theEvent), getMouseY(theEvent)));
}

- (void)rightMouseUp:(NSEvent*)theEvent {
	Kore::Mouse::the()->_releaseRight(Kore::MouseEvent(getMouseX(theEvent), getMouseY(theEvent)));
}

- (void)rightMouseDragged:(NSEvent*)theEvent {
	Kore::Mouse::the()->_move(Kore::MouseEvent(getMouseX(theEvent), getMouseY(theEvent)));
}


- (void) prepareOpenGL {
    const GLint swapInt = 1;

    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval]; // set to vbl sync

	glEnable(GL_DEPTH_TEST);
	glPolygonOffset (1.0f, 1.0f);	
	glClearColor(0.0f, 1.0f, 0.0f, 0.0f);
}

- (void) update { // window resizes, moves and display changes (resize, depth and display config change)
	[super update];
}

- (id) initWithFrame: (NSRect) frameRect {
	NSOpenGLPixelFormat * pf = [BasicOpenGLView basicPixelFormat];
	self = [super initWithFrame: frameRect pixelFormat: pf];
	
	[self prepareOpenGL];
	
    return self;
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (BOOL)becomeFirstResponder {
  return  YES;
}

- (BOOL)resignFirstResponder {
  return YES;
}

@end
