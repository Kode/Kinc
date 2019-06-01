#include <Kore/pch.h>

#import "BasicOpenGLView.h"

#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/system.h>

#ifdef KORE_METAL
#include <kinc/graphics5/graphics.h>
#endif

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
		return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
	}
	else {
		NSOpenGLPixelFormatAttribute attributes[] = {
		    NSOpenGLPFADoubleBuffer,         NSOpenGLPFADepthSize,           (NSOpenGLPixelFormatAttribute)24, // 16 bit depth buffer
		    NSOpenGLPFAOpenGLProfile,        NSOpenGLProfileVersion3_2Core,  NSOpenGLPFAStencilSize,
		    (NSOpenGLPixelFormatAttribute)8, (NSOpenGLPixelFormatAttribute)0};
		return [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
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
					if (!shift) Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_SHIFT);
					shift = true;
				}
				else {
					if (shift) Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_SHIFT);
					shift = false;
				}
				break;
			}
		}
		switch (ch) {
		case NSRightArrowFunctionKey:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_RIGHT);
			break;
		case NSLeftArrowFunctionKey:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_LEFT);
			break;
		case NSUpArrowFunctionKey:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_UP);
			break;
		case NSDownArrowFunctionKey:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_DOWN);
			break;
		case 27:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_ESCAPE);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_RETURN);
			break;
		case 0x7f:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_BACKSPACE);
			break;
		case 32:
			Kinc_Internal_Keyboard_TriggerKeyDown(KINC_KEY_SPACE);
			break;
		default:
			if (ch == 'x' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char* text = Kinc_Internal_CutCallback();
				if (text != nullptr) {
					NSPasteboard* board = [NSPasteboard generalPasteboard];
					[board clearContents];
					[board setString:[NSString stringWithUTF8String:text] forType:NSStringPboardType];
				}
				break;
			}
			if (ch == 'c' && [theEvent modifierFlags] & NSCommandKeyMask) {
				char* text = Kinc_Internal_CopyCallback();
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
					Kinc_Internal_PasteCallback(charData);
				}
				break;
			}
			if (ch >= L'a' && ch <= L'z') {
				Kinc_Internal_Keyboard_TriggerKeyDown(ch - L'a' + KINC_KEY_A);
			}
			else if (ch >= L'A' && ch <= L'Z') {
				Kinc_Internal_Keyboard_TriggerKeyDown(ch - L'A' + KINC_KEY_A);
			}
			else if (ch >= L'0' && ch <= L'9') {
				Kinc_Internal_Keyboard_TriggerKeyDown(ch - L'0' + KINC_KEY_0);
			}
			Kinc_Internal_Keyboard_TriggerKeyPress(ch);
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
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_RIGHT);
			break;
		case NSLeftArrowFunctionKey:
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_LEFT);
			break;
		case NSUpArrowFunctionKey:
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_UP);
			break;
		case NSDownArrowFunctionKey:
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_DOWN);
			break;
		case 27:
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_ESCAPE);
			break;
		case NSEnterCharacter:
		case NSNewlineCharacter:
		case NSCarriageReturnCharacter:
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_RETURN);
			break;
		case 0x7f:
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_BACKSPACE);
			break;
		case 32:
			Kinc_Internal_Keyboard_TriggerKeyUp(KINC_KEY_SPACE);
			break;
		default:
			if (ch >= L'a' && ch <= L'z') {
				Kinc_Internal_Keyboard_TriggerKeyUp(ch - L'a' + KINC_KEY_A);
			}
			else if (ch >= L'A' && ch <= L'Z') {
				Kinc_Internal_Keyboard_TriggerKeyUp(ch - L'A' + KINC_KEY_A);
			}
			else if (ch >= L'0' && ch <= L'9') {
				Kinc_Internal_Keyboard_TriggerKeyUp(ch - L'0' + KINC_KEY_0);
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
		return static_cast<int>(kinc_height() - [event locationInWindow].y);
	}

	bool controlKeyMouseButton = false;
}

- (void)mouseDown:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	if ([theEvent modifierFlags] & NSControlKeyMask) {
		controlKeyMouseButton = true;
		Kinc_Internal_Mouse_TriggerPress(0, 1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		controlKeyMouseButton = false;
		Kinc_Internal_Mouse_TriggerPress(0, 0, getMouseX(theEvent), getMouseY(theEvent));
	}
}

- (void)mouseUp:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	if (controlKeyMouseButton) {
		Kinc_Internal_Mouse_TriggerRelease(0, 1, getMouseX(theEvent), getMouseY(theEvent));
	}
	else {
		Kinc_Internal_Mouse_TriggerRelease(0, 0, getMouseX(theEvent), getMouseY(theEvent));
	}
	controlKeyMouseButton = false;
}

- (void)mouseMoved:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	Kinc_Internal_Mouse_TriggerMove(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)mouseDragged:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	Kinc_Internal_Mouse_TriggerMove(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseDown:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	Kinc_Internal_Mouse_TriggerPress(0, 1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseUp:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	Kinc_Internal_Mouse_TriggerRelease(0, 1, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)rightMouseDragged:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	Kinc_Internal_Mouse_TriggerMove(0, getMouseX(theEvent), getMouseY(theEvent));
}

- (void)scrollWheel:(NSEvent*)theEvent {
	// TODO (DK) map [theEvent window] to window id instead of 0
	int delta = [theEvent deltaY];
	Kinc_Internal_Mouse_TriggerScroll(0, -delta);
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender {
	NSPasteboard* pboard = [sender draggingPasteboard];
	NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
	if ([[pboard types] containsObject:NSURLPboardType]) {
		if (sourceDragMask & NSDragOperationLink) {
			return NSDragOperationLink;
		}
	}
	return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
	NSPasteboard* pboard = [sender draggingPasteboard];
	//NSDragOperation sourceDragMask = [sender draggingSourceOperationMask];
	if ([[pboard types] containsObject:NSURLPboardType]) {
		NSURL* fileURL = [NSURL URLFromPasteboard:pboard];
		wchar_t* filePath = (wchar_t*)[fileURL.path cStringUsingEncoding:NSUTF32LittleEndianStringEncoding];
		Kinc_Internal_DropFilesCallback(filePath);
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
#ifdef KORE_OPENGL
	[super update];
#endif
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

void initMetalCompute(id<MTLDevice> device, id<MTLCommandQueue> commandBuffer);

- (id)initWithFrame:(NSRect)frameRect {
	self = [super initWithFrame:frameRect];

	device = MTLCreateSystemDefaultDevice();
	commandQueue = [device newCommandQueue];
	library = [device newDefaultLibrary];
	initMetalCompute(device, commandQueue);

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

- (void)resize:(NSSize)size {
	[self setFrameSize:size];
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
		
		if (depthTexture == nil || depthTexture.width != drawable.texture.width || depthTexture.height != drawable.texture.height) {
			MTLTextureDescriptor* descriptor = [MTLTextureDescriptor new];
			descriptor.textureType = MTLTextureType2D;
			descriptor.width = drawable.texture.width;
			descriptor.height = drawable.texture.height;
			descriptor.depth = 1;
			descriptor.pixelFormat = MTLPixelFormatDepth32Float_Stencil8;
			descriptor.arrayLength = 1;
			descriptor.mipmapLevelCount = 1;
			descriptor.resourceOptions = MTLResourceStorageModePrivate;
			descriptor.usage = MTLTextureUsageRenderTarget;
			depthTexture = [device newTextureWithDescriptor:descriptor];
		}

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
		renderPassDescriptor.depthAttachment.clearDepth = 99999;
		renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
		renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
		renderPassDescriptor.depthAttachment.texture = depthTexture;
		renderPassDescriptor.stencilAttachment.clearStencil = 0;
		renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
		renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
		renderPassDescriptor.stencilAttachment.texture = depthTexture;

		// id <MTLCommandQueue> commandQueue = [device newCommandQueue];
		commandBuffer = [commandQueue commandBuffer];
		// if (drawable != nil) {
		commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		//}
	}
}

- (void)end {
	@autoreleasepool {
		[commandEncoder endEncoding];
		[commandBuffer presentDrawable:drawable];
		[commandBuffer commit];

		// if (drawable != nil) {
		//	[commandBuffer waitUntilScheduled];
		//	[drawable present];
		//}
	}
}

- (void)newRenderPass:(struct kinc_g5_render_target*)renderTarget wait: (bool)wait {
	@autoreleasepool {
		[commandEncoder endEncoding];
		[commandBuffer commit];
		if (wait) {
			[commandBuffer waitUntilCompleted];
		}
		
		renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		renderPassDescriptor.colorAttachments[0].texture = renderTarget == nullptr ? drawable.texture : renderTarget->impl._tex;
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionLoad;
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
		renderPassDescriptor.depthAttachment.clearDepth = 99999;
		renderPassDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
		renderPassDescriptor.depthAttachment.storeAction = MTLStoreActionStore;
		renderPassDescriptor.depthAttachment.texture = renderTarget == nullptr ? depthTexture : renderTarget->impl._depthTex;
		renderPassDescriptor.stencilAttachment.clearStencil = 0;
		renderPassDescriptor.stencilAttachment.loadAction = MTLLoadActionDontCare;
		renderPassDescriptor.stencilAttachment.storeAction = MTLStoreActionDontCare;
		renderPassDescriptor.stencilAttachment.texture = renderTarget == nullptr ? depthTexture : renderTarget->impl._depthTex;
		
		commandBuffer = [commandQueue commandBuffer];
		commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
	}
}

#endif

@end
