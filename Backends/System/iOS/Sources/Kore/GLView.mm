#include "pch.h"

#import "GLView.h"

#include <kinc/input/acceleration.h>
#include <kinc/input/keyboard.h>
#include <kinc/input/mouse.h>
#include <kinc/input/rotation.h>
#include <kinc/input/surface.h>
#include <kinc/system.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/rendertarget.h>

#ifdef KORE_OPENGL
#include <Kore/OpenGLWindow.h>
#endif

namespace {
	const int touchmaxcount = 20;
	void* touches[touchmaxcount];

	void initTouches() {
		for (int i = 0; i < touchmaxcount; ++i) {
			touches[i] = nullptr;
		}
	}

	int getTouchIndex(void* touch) {
		for (int i = 0; i < touchmaxcount; ++i) {
			if (touches[i] == touch) return i;
		}
		return -1;
	}

	int addTouch(void* touch) {
		for (int i = 0; i < touchmaxcount; ++i) {
			if (touches[i] == nullptr) {
				touches[i] = touch;
				return i;
			}
		}
		return -1;
	}

	int removeTouch(void* touch) {
		for (int i = 0; i < touchmaxcount; ++i) {
			if (touches[i] == touch) {
				touches[i] = nullptr;
				return i;
			}
		}
		return -1;
	}

	GLint backingWidth, backingHeight;
}

extern "C" int kinc_window_width(int window) {
	return backingWidth;
}

extern "C" int kinc_window_height(int window) {
	return backingHeight;
}

@implementation GLView

#ifdef KORE_METAL
+ (Class)layerClass {
	return [CAMetalLayer class];
}
#else
+ (Class)layerClass {
	return [CAEAGLLayer class];
}
#endif

#ifdef KORE_METAL
void initMetalCompute(id<MTLDevice> device, id<MTLCommandQueue> commandQueue);

- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:(CGRect)frame];
	self.contentScaleFactor = [UIScreen mainScreen].scale;

	backingWidth = frame.size.width * self.contentScaleFactor;
	backingHeight = frame.size.height * self.contentScaleFactor;
	
	initTouches();

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
#else
- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:(CGRect)frame];
	self.contentScaleFactor = [UIScreen mainScreen].scale;
	
	backingWidth = frame.size.width * self.contentScaleFactor;
	backingHeight = frame.size.height * self.contentScaleFactor;

	initTouches();

	CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;

	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
	                                                                          kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

	if (!context || ![EAGLContext setCurrentContext:context]) {
		//[self release];
		return nil;
	}

	glGenFramebuffersOES(1, &defaultFramebuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	Kinc_Internal_windows[0].framebuffer = defaultFramebuffer;
	
	glGenRenderbuffersOES(1, &colorRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);

	glGenRenderbuffersOES(1, &depthStencilRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthStencilRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthStencilRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_STENCIL_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthStencilRenderbuffer);

	// Start acceletometer
	hasAccelerometer = false;
#ifndef KORE_TVOS
	motionManager = [[CMMotionManager alloc] init];
	if ([motionManager isAccelerometerAvailable]) {
		motionManager.accelerometerUpdateInterval = 0.033;
		[motionManager startAccelerometerUpdates];
		hasAccelerometer = true;
	}
#endif

#ifndef KORE_TVOS
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(onKeyboardHide:) name:UIKeyboardWillHideNotification object:nil];
#endif

	return self;
}
#endif

#ifdef KORE_METAL
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

		backingWidth = (int)[texture width];
		backingHeight = (int)[texture height];

		if (renderPassDescriptor == nil) {
			renderPassDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
		}
		renderPassDescriptor.colorAttachments[0].texture = texture;
		renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
		renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
		renderPassDescriptor.depthAttachment.clearDepth = 1;
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
#else
- (void)begin {
	[EAGLContext setCurrentContext:context];
	//glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	//glViewport(0, 0, backingWidth, backingHeight);

#ifndef KORE_TVOS
	// Accelerometer updates
	if (hasAccelerometer) {

		CMAcceleration acc = motionManager.accelerometerData.acceleration;

		if (acc.x != lastAccelerometerX || acc.y != lastAccelerometerY || acc.z != lastAccelerometerZ) {

			(*kinc_acceleration_callback)(acc.x, acc.y, acc.z);

			lastAccelerometerX = acc.x;
			lastAccelerometerY = acc.y;
			lastAccelerometerZ = acc.z;
		}
	}
#endif
}
#endif

#ifdef KORE_METAL
- (void)end {
	@autoreleasepool {
    if (commandEncoder != nil && commandBuffer != nil) {
      [commandEncoder endEncoding];
      [commandBuffer presentDrawable:drawable];
      [commandBuffer commit];
    }
		
		commandBuffer = nil;
    commandEncoder = nil;

		// if (drawable != nil) {
		//	[commandBuffer waitUntilScheduled];
		//	[drawable present];
		//}
	}
}
#else
- (void)end {
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER_OES]; // crash at end
}
#endif

#ifdef KORE_METAL
- (void)layoutSubviews {
}
#else
- (void)layoutSubviews {
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];

	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);

	printf("backingWitdh/Height: %i, %i\n", backingWidth, backingHeight);

	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthStencilRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH24_STENCIL8_OES, backingWidth, backingHeight);

	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
	}
}
#endif

#ifdef KORE_METAL
- (void)dealloc {
}
#else
- (void)dealloc {
	if (defaultFramebuffer) {
		glDeleteFramebuffersOES(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}

	if (colorRenderbuffer) {
		glDeleteRenderbuffersOES(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}

	if (depthStencilRenderbuffer) {
		glDeleteRenderbuffersOES(1, &depthStencilRenderbuffer);
		depthStencilRenderbuffer = 0;
	}

	if ([EAGLContext currentContext] == context) [EAGLContext setCurrentContext:nil];

	//[context release];
	context = nil;

	//[super dealloc];
}
#endif

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
	for (UITouch* touch in touches) {
		int index = getTouchIndex((__bridge void*)touch);
		if (index == -1) index = addTouch((__bridge void*)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_press(0, 0, x, y);
			}
			kinc_internal_surface_trigger_touch_start(index, x, y);
		}
	}
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
	for (UITouch* touch in touches) {
		int index = getTouchIndex((__bridge void*)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_move(0, x, y);
			}
			kinc_internal_surface_trigger_move(index, x, y);
		}
	}
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
	for (UITouch* touch in touches) {
		int index = removeTouch((__bridge void*)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_release(0, 0, x, y);
			}
			kinc_internal_surface_trigger_touch_end(index, x, y);
		}
	}
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
	for (UITouch* touch in touches) {
		int index = removeTouch((__bridge void*)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView:self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				kinc_internal_mouse_trigger_release(0, 0, x, y);
			}
			kinc_internal_surface_trigger_touch_end(index, x, y);
		}
	}
}

namespace {
	NSString* keyboardstring;
	UITextField* myTextField = nullptr;
	bool shiftDown = false;
}

- (void)showKeyboard {
	[self becomeFirstResponder];
}

- (void)hideKeyboard {
	[self resignFirstResponder];
}

- (BOOL)hasText {
	return YES;
}

- (void)insertText:(NSString*)text {
	if ([text length] == 1) {
		unichar ch = [text characterAtIndex:[text length] - 1];
		if (ch == 8212) ch = '_';
		if (ch == L'\n') {
			kinc_internal_keyboard_trigger_key_down(KINC_KEY_RETURN);
			kinc_internal_keyboard_trigger_key_up(KINC_KEY_RETURN);
		}
		else if (ch >= L'a' && ch <= L'z') {
			if (shiftDown) {
				kinc_internal_keyboard_trigger_key_up(KINC_KEY_SHIFT);
				shiftDown = false;
			}
			kinc_internal_keyboard_trigger_key_down(ch + KINC_KEY_A - L'a');
			kinc_internal_keyboard_trigger_key_up(ch + KINC_KEY_A - L'a');
			kinc_internal_keyboard_trigger_key_press(ch);
		}
		else {
			if (!shiftDown) {
				kinc_internal_keyboard_trigger_key_down(KINC_KEY_SHIFT);
				shiftDown = true;
			}
			kinc_internal_keyboard_trigger_key_down(ch + KINC_KEY_A - L'A');
			kinc_internal_keyboard_trigger_key_up(ch + KINC_KEY_A - L'A');
			kinc_internal_keyboard_trigger_key_press(ch);
		}
	}
}

- (void)deleteBackward {
	kinc_internal_keyboard_trigger_key_down(KINC_KEY_BACKSPACE);
	kinc_internal_keyboard_trigger_key_up(KINC_KEY_BACKSPACE);
}

- (BOOL)canBecomeFirstResponder {
	return YES;
}

- (void)onKeyboardHide:(NSNotification*)notification {
	kinc_keyboard_hide();
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
		renderPassDescriptor.depthAttachment.clearDepth = 1;
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
