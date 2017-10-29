#include <Kore/pch.h>

#import "BasicOpenGLView.h"

#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/System.h>

@implementation BasicOpenGLView

namespace {
	bool shift = false;
}

#ifndef KORE_METAL
+ (NSOpenGLPixelFormat*)basicPixelFormat {
	// TODO (DK) pass via argument in
	int aa = 1; // Kore::Application::the()->antialiasing();
	if (aa > 0) {
		NSOpenGLPixelFormatAttribute attributes[] = {NSOpenGLPFADoubleBuffer,          NSOpenGLPFADepthSize,
													 (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
													 NSOpenGLPFAOpenGLProfile,         NSOpenGLProfileVersion3_2Core,
													 NSOpenGLPFASupersample,           NSOpenGLPFASampleBuffers,
													 (NSOpenGLPixelFormatAttribute)1,  NSOpenGLPFASamples,
													 (NSOpenGLPixelFormatAttribute)aa, NSOpenGLPFAStencilSize,
													 (NSOpenGLPixelFormatAttribute)8,  (NSOpenGLPixelFormatAttribute)0};
		return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	}
	else {
		NSOpenGLPixelFormatAttribute attributes[] = {
			NSOpenGLPFADoubleBuffer,         NSOpenGLPFADepthSize,           (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
			NSOpenGLPFAOpenGLProfile,        NSOpenGLProfileVersion3_2Core,  NSOpenGLPFAStencilSize,
			(NSOpenGLPixelFormatAttribute)8, (NSOpenGLPixelFormatAttribute)0};
		return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	}
}

- (void)switchBuffers {
	[[self openGLContext] flushBuffer];
}
#endif

- (void)keyDown:(NSEvent*)theEvent {
	if ([theEvent isARepeat]) return;
	NSString* characters = [theEvent characters];
	if ([characters length]) {
		unichar ch = [characters characterAtIndex:0];
		if (ch >= L'A' && ch <= L'Z') {
			switch (ch) {
			default:
				if ([theEvent modifierFlags] & NSShiftKeyMask) {
					if (!shift) Kore::Keyboard::the()->_keydown(Kore::KeyShift);
					shift = true;
				}
				else {
					if (shift) Kore::Keyboard::the()->_keyup(Kore::KeyShift);
					shift = false;
				}
				break;
			}
		}
		switch (ch) {
		case NSRightArrowFunctionKey:
			Kore::Keyboard::the()->_keydown(Kore::KeyRight);
			break;
		case NSLeftArrowFunctionKey:
			Kore::Keyboard::the()->_keydown(Kore::KeyLeft);
			break;
		case NSUpArrowFunctionKey:
			Kore::Keyboard::the()->_keydown(Kore::KeyUp);
			break;
		case NSDownArrowFunctionKey:
			Kore::Keyboard::the()->_keydown(Kore::KeyDown);
			break;
		case 27:
			Kore::Keyboard::the()->_keydown(Kore::KeyEscape);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			Kore::Keyboard::the()->_keydown(Kore::KeyReturn);
			break;
		case 0x7f:
			Kore::Keyboard::the()->_keydown(Kore::KeyBackspace);
			break;
		case 32:
			Kore::Keyboard::the()->_keydown(Kore::KeySpace);
			break;
		default:
			if (ch == 'x' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char* text = Kore::System::cutCallback();
				if (text != nullptr) {
					NSPasteboard* board = [NSPasteboard generalPasteboard];
					[board clearContents];
					[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
				}
				break;
			}
			if (ch == 'c' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char* text = Kore::System::copyCallback();
				if (text != nullptr) {
					NSPasteboard* board = [NSPasteboard generalPasteboard];
					[board clearContents];
					[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
				}
				break;
			}
			if (ch == 'v' && [theEvent modifierFlags] & NSCommandKeyMask) {
				NSPasteboard* board = [NSPasteboard generalPasteboard];
				NSString* data = [board stringForType:NSStringPboardType];
				if (data != nil) {
					char charData[4096];
					strcpy(charData, [data UTF8String]);
					Kore::System::pasteCallback(charData);
				}
				break;
			}
			if (ch >= L'a' && ch <= L'z') {
				Kore::Keyboard::the()->_keydown((Kore::KeyCode)(ch - L'a' + Kore::KeyA));
			}
			else if (ch >= L'A' && ch <= L'Z') {
				Kore::Keyboard::the()->_keydown((Kore::KeyCode)(ch - L'A' + Kore::KeyA));
			}
			else if (ch >= L'0' && ch <= L'9') {
				Kore::Keyboard::the()->_keydown((Kore::KeyCode)(ch - L'0' + Kore::Key0));
			}
			Kore::Keyboard::the()->_keypress(ch);
			break;
		}
	}
}

- (void)keyUp:(NSEvent*)theEvent {
	NSString* characters = [theEvent characters];
	if ([characters length]) {
		unichar ch = [characters characterAtIndex:0];
		switch (ch) {
		case NSRightArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::KeyRight);
			break;
		case NSLeftArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::KeyLeft);
			break;
		case NSUpArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::KeyUp);
			break;
		case NSDownArrowFunctionKey:
			Kore::Keyboard::the()->_keyup(Kore::KeyDown);
			break;
		case 27:
			Kore::Keyboard::the()->_keyup(Kore::KeyEscape);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			Kore::Keyboard::the()->_keyup(Kore::KeyReturn);
			break;
		case 0x7f:
			Kore::Keyboard::the()->_keyup(Kore::KeyBackspace);
			break;
		case 32:
			Kore::Keyboard::the()->_keyup(Kore::KeySpace);
			break;
		default:
			if (ch >= L'a' && ch <= L'z') {
				Kore::Keyboard::the()->_keyup((Kore::KeyCode)(ch - L'a' + Kore::KeyA));
			}
			else if (ch >= L'A' && ch <= L'Z') {
				Kore::Keyboard::the()->_keyup((Kore::KeyCode)(ch - L'A' + Kore::KeyA));
			}
			else if (ch >= L'0' && ch <= L'9') {
				Kore::Keyboard::the()->_keyup((Kore::KeyCode)(ch - L'0' + Kore::Key0));
			}
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
	Kore::Mouse::the()->_scroll(0, -delta);
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender {
	NSPasteboard *pboard = [sender draggingPasteboard];
	NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
	if ([[pboard types] containsObject:NSURLPboardType]) {
		if (sourceDragMask & NSDragOperationLink) {
			return NSDragOperationLink;
		}
	}
	return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender {
	NSPasteboard *pboard = [sender draggingPasteboard];
	NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
	if ([[pboard types] containsObject:NSURLPboardType]) {
		NSURL *fileURL = [NSURL URLFromPasteboard:pboard];
		wchar_t *filePath = (wchar_t *)[fileURL.path cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
		Kore::System::dropFilesCallback(filePath);
	}
	return YES;
}

#ifndef KORE_METAL
- (void)prepareOpenGL {
	const GLint swapInt = 1;
	[[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}
#endif

- (void)update { // window resizes, moves and display changes (resize, depth and display config change)
	[super update];
}

#ifndef KORE_METAL
- (id)initWithFrame:(NSRect)frameRect {
	NSOpenGLPixelFormat* pf = [BasicOpenGLView basicPixelFormat];
	self = [super initWithFrame:frameRect pixelFormat:pf];

	[self prepareOpenGL];
	//[[self openGLContext] makeCurrentContext];
	return self;
}
#else
- (id)initWithFrame:(NSRect)frameRect {
	self = [super initWithFrame:frameRect];
	
	device = MTLCreateSystemDefaultDevice();
	commandQueue = [device newCommandQueue];
	library = [device newDefaultLibrary];
	
	CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
	
	metalLayer.device = device;
	metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	metalLayer.framebufferOnly = YES;
	// metalLayer.presentsWithTransaction = YES;
	
	metalLayer.opaque = YES;
	metalLayer.backgroundColor = nil;
	
	return self;
}
#endif

- (BOOL)acceptsFirstResponder {
	return YES;
}

- (BOOL)becomeFirstResponder {
	return YES;
}

- (BOOL)resignFirstResponder {
	return YES;
}

#ifdef KORE_METAL
- (id<MTLDevice>)metalDevice {
	return device;
}

- (id<MTLLibrary>)metalLibrary {
	return library;
}

- (id<MTLRenderCommandEncoder>)metalEncoder {
	return commandEncoder;
}

- (void)begin {
	@autoreleasepool {
		CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;

		drawable = [metalLayer nextDrawable];

		// printf("It's %i\n", drawable == nil ? 0 : 1);
		// if (drawable == nil) return;
		id<MTLTexture> texture = drawable.texture;

		// backingWidth = (int)[texture width];
		// backingHeight = (int)[texture height];
		
		renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = texture;
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);

		// id <MTLCommandQueue> commandQueue = [device newCommandQueue];
		commandBuffer = [commandQueue commandBuffer];
		// if (drawable != nil) {
		commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		//}
		
		[commandEncoder retain];
		[drawable retain];
		[commandBuffer retain];
	}
}

- (void)end {
	@autoreleasepool {
		[commandEncoder endEncoding];
		[commandBuffer presentDrawable:drawable];
		[commandBuffer commit];
		commandBuffer = nil;

		// if (drawable != nil) {
		//	[commandBuffer waitUntilScheduled];
		//	[drawable present];
		//}
	}
	
	[commandEncoder release];
	[drawable release];
	[commandBuffer release];
}
#endif

@end
