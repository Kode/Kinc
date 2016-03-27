#import "BasicOpenGLView.h"
#include <Kore/pch.h>
#include <Kore/System.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>

@implementation BasicOpenGLView

namespace {
    bool shift = false;
}

+ (NSOpenGLPixelFormat*) basicPixelFormat {
    // TODO (DK) pass via argument in
    int aa = 1; //Kore::Application::the()->antialiasing();
	if (aa > 0) {
		NSOpenGLPixelFormatAttribute attributes[] = {
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
			NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
			NSOpenGLPFASupersample,
			NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)1,
			NSOpenGLPFASamples, (NSOpenGLPixelFormatAttribute)aa,
            NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)8,
			(NSOpenGLPixelFormatAttribute)0
		};
		return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	}
	else {
		NSOpenGLPixelFormatAttribute attributes[] = {
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFADepthSize, (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
			NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
            NSOpenGLPFAStencilSize, (NSOpenGLPixelFormatAttribute)8,
			(NSOpenGLPixelFormatAttribute)0
		};
		return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	}
}

- (void)switchBuffers {
	[[self openGLContext] flushBuffer];
}

- (void)keyDown:(NSEvent*)theEvent {
	if ([theEvent isARepeat]) return;
	NSString* characters = [theEvent characters];
	if ([characters length]) {
		unichar ch = [characters characterAtIndex:0];
		int keycode = ch;
		if (ch >= L'a' && ch <= L'z') keycode = keycode - L'a' + L'A';
		if (ch >= L'A' && ch <= L'Z') {
			switch (ch) {
			default:
                if ([theEvent modifierFlags] & NSShiftKeyMask) {
					if (!shift) Kore::Keyboard::the()->_keydown(Kore::Key_Shift, 0);
					Kore::Keyboard::the()->_keydown((Kore::KeyCode)keycode, ch);
                    shift = true;
                }
                else {
					if (shift) Kore::Keyboard::the()->_keyup(Kore::Key_Shift, 0);
					Kore::Keyboard::the()->_keydown((Kore::KeyCode)keycode, ch);
                    shift = false;
				}
                break;
			}
		}
		else {
			switch (ch) {
			case NSRightArrowFunctionKey:
				Kore::Keyboard::the()->_keydown(Kore::Key_Right, 0);
				break;
			case NSLeftArrowFunctionKey:
				Kore::Keyboard::the()->_keydown(Kore::Key_Left, 0);
				break;
			case NSUpArrowFunctionKey:
				Kore::Keyboard::the()->_keydown(Kore::Key_Up, 0);
				break;
			case NSDownArrowFunctionKey:
				Kore::Keyboard::the()->_keydown(Kore::Key_Down, 0);
				break;
            case 27:
                Kore::Keyboard::the()->_keydown(Kore::Key_Escape, 0);
                break;
            case NSEnterCharacter:
            case NSNewlineCharacter:
            case NSCarriageReturnCharacter:
				Kore::Keyboard::the()->_keydown(Kore::Key_Enter, 0);
				break;
            case 0x7f:
                Kore::Keyboard::the()->_keydown(Kore::Key_Backspace, 0);
                break;
			default:
				Kore::Keyboard::the()->_keydown((Kore::KeyCode)keycode, ch);
				break;	
			}
		}
	}
}

- (void)keyUp:(NSEvent*)theEvent {
	NSString* characters = [theEvent characters];
    if ([characters length]) {
        unichar ch = [characters characterAtIndex:0];
		int keycode = ch;
		if (ch >= L'a' && ch <= L'z') keycode = keycode - L'a' + L'A';
		switch (ch) {
		case NSRightArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::Key_Right, 0);
			break;
		case NSLeftArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::Key_Left, 0);
			break;
		case NSUpArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::Key_Up, 0);
			break;
		case NSDownArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::Key_Down, 0);
			break;
        case 27:
            Kore::Keyboard::the()->_keyup(Kore::Key_Escape, 0);
            break;
        case NSEnterCharacter:
        case NSNewlineCharacter:
        case NSCarriageReturnCharacter:
            Kore::Keyboard::the()->_keyup(Kore::Key_Enter, 0);
            break;
		case 0x7f:
			Kore::Keyboard::the()->_keyup(Kore::Key_Backspace, 0);
			break;
		default:
			Kore::Keyboard::the()->_keyup((Kore::KeyCode)keycode, ch);
			break;
		}
	}
}

namespace {
	int getMouseX(NSEvent* event) {
        // TODO (DK) map [theEvent window] to window id instead of 0
		return static_cast<int>([event locationInWindow].x);
	}
	
	int getMouseY(NSEvent* event) {
        // TODO (DK) map [theEvent window] to window id instead of 0
        return static_cast<int>(Kore::System::windowHeight(0) - [event locationInWindow].y);
	}
	
	bool controlKeyMouseButton = false;
}

- (void)mouseDown:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	if ([theEvent modifierFlags] & NSControlKeyMask) {
		controlKeyMouseButton = true;
		Kore::Mouse::the()->_press(0, 1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		controlKeyMouseButton = false;
		Kore::Mouse::the()->_press(0, 0, getMouseX(theEvent), getMouseY(theEvent));
	}
}

- (void)mouseUp:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	if (controlKeyMouseButton) {
		Kore::Mouse::the()->_release(0, 1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		Kore::Mouse::the()->_release(0, 0, getMouseX(theEvent), getMouseY(theEvent));
	}
	controlKeyMouseButton = false;
}

- (void)mouseMoved:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	Kore::Mouse::the()->_move(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)mouseDragged:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	Kore::Mouse::the()->_move(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseDown:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	Kore::Mouse::the()->_press(0, 1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseUp:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	Kore::Mouse::the()->_release(0, 1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseDragged:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	Kore::Mouse::the()->_move(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)scrollWheel:(NSEvent*)theEvent {
    // TODO (DK) map [theEvent window] to window id instead of 0
	int delta = [theEvent deltaY];
	Kore::Mouse::the()->_scroll(0, delta);
}

- (void)prepareOpenGL {
	const GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (void)update { // window resizes, moves and display changes (resize, depth and display config change)
	[super update];
}

- (id)initWithFrame:(NSRect)frameRect {
	NSOpenGLPixelFormat* pf = [BasicOpenGLView basicPixelFormat];
	self = [super initWithFrame: frameRect pixelFormat: pf];
	
	[self prepareOpenGL];
    //[[self openGLContext] makeCurrentContext];
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
