#import "GLView.h"
#include "pch.h"
#include <Kore/Application.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Sensor.h>
#include <Kore/Input/Surface.h>
#include <Kore/System.h>

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

int Kore::System::screenWidth() {
	return backingWidth;
}

int Kore::System::screenHeight() {
	return backingHeight;
}

@implementation GLView

#ifdef SYS_METAL
+ (Class)layerClass {
	return [CAMetalLayer class];
}
#else
+ (Class)layerClass {
    return [CAEAGLLayer class];
}
#endif

#ifdef SYS_METAL
- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:(CGRect)frame];
	self.contentScaleFactor = [UIScreen mainScreen].scale;
	
	initTouches();
	
	device = MTLCreateSystemDefaultDevice();
	commandQueue = [device newCommandQueue];
	library = [device newDefaultLibrary];
	
	CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
	
	metalLayer.device = device;
	metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	metalLayer.framebufferOnly = YES;
	//metalLayer.presentsWithTransaction = YES;
	
	metalLayer.opaque = YES;
	metalLayer.backgroundColor = nil;
	
	return self;
}
#else
- (id)initWithFrame:(CGRect)frame {
	self = [super initWithFrame:(CGRect)frame];
	self.contentScaleFactor = [UIScreen mainScreen].scale;
	
	initTouches();
	
	CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
	
	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];

	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	
	if (!context || ![EAGLContext setCurrentContext:context]) {
		//[self release];
		return nil;
	}
	
	glGenFramebuffersOES(1, &defaultFramebuffer);
	glGenRenderbuffersOES(1, &colorRenderbuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
	
	glGenRenderbuffersOES(1, &depthRenderbuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);

    // Start acceletometer
	hasAccelerometer = false;

	motionManager = [[CMMotionManager alloc]init];
	if ([motionManager isAccelerometerAvailable]) {
		motionManager.accelerometerUpdateInterval = 0.033;
		[motionManager startAccelerometerUpdates];
		hasAccelerometer = true;
	}
	
	[[NSNotificationCenter defaultCenter]addObserver:self selector:@selector(onKeyboardHide:) name:UIKeyboardWillHideNotification object:nil];

	return self;
}
#endif

#ifdef SYS_METAL
- (void)begin {
	@autoreleasepool {
		CAMetalLayer* metalLayer = (CAMetalLayer*)self.layer;
	
		drawable = [metalLayer nextDrawable];
		
		//printf("It's %i\n", drawable == nil ? 0 : 1);
		//if (drawable == nil) return;
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
	
		//id <MTLCommandQueue> commandQueue = [device newCommandQueue];
		commandBuffer = [commandQueue commandBuffer];
		//if (drawable != nil) {
		commandEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
		//}
		
	}
}
#else
- (void)begin {
	[EAGLContext setCurrentContext:context];
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
    glViewport(0, 0, backingWidth, backingHeight);

    // Accelerometer updates
    if (hasAccelerometer) {

    	CMAcceleration acc = motionManager.accelerometerData.acceleration;

    	if (acc.x != lastAccelerometerX || acc.y != lastAccelerometerY || acc.z != lastAccelerometerZ) {
 			
    		Kore::Sensor::_changed(Kore::SensorAccelerometer, acc.x, acc.y, acc.z);

 			lastAccelerometerX = acc.x;
 			lastAccelerometerY = acc.y;
 			lastAccelerometerZ = acc.z;
 		}
    }
}
#endif

#ifdef SYS_METAL
static float red = 0.0f;

- (void)end {
	@autoreleasepool {
		[commandEncoder endEncoding];
		[commandBuffer presentDrawable:drawable];
		[commandBuffer commit];
		commandBuffer = nil;
	
		//if (drawable != nil) {
		//	[commandBuffer waitUntilScheduled];
		//	[drawable present];
		//}
	}
}
#else
- (void)end {
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER_OES]; //crash at end
}
#endif

#ifdef SYS_METAL
-(void)layoutSubviews {
	
}
#else
- (void)layoutSubviews {
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
	
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
	printf("backingWitdh/Height: %i, %i\n", backingWidth, backingHeight);
	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT24_OES, backingWidth, backingHeight);
	
	if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
	}
}
#endif

#ifdef SYS_METAL
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
			CGPoint point = [touch locationInView: self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				Kore::Mouse::the()->_press(0, x, y);
			}
			Kore::Surface::the()->_touchStart(index, x, y);
		}
	}
}

- (void)touchesMoved:(NSSet*)touches withEvent:(UIEvent*)event {
	for (UITouch* touch in touches) {
		int index = getTouchIndex((__bridge void*)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView: self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				Kore::Mouse::the()->_move(x, y);
			}
			Kore::Surface::the()->_move(index, x, y);
		}
	}
}

- (void)touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
	for (UITouch* touch in touches) {
		int index = removeTouch((__bridge void*)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView: self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				Kore::Mouse::the()->_release(0, x, y);
			}
			Kore::Surface::the()->_touchEnd(index, x, y);
		}
	}
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(UIEvent*)event {
	for (UITouch* touch in touches) {
		int index = removeTouch((__bridge void*)touch);
		if (index >= 0) {
			CGPoint point = [touch locationInView: self];
			float x = point.x * self.contentScaleFactor;
			float y = point.y * self.contentScaleFactor;
			if (index == 0) {
				Kore::Mouse::the()->_release(0, x, y);
			}
			Kore::Surface::the()->_touchEnd(index, x, y);
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
		unichar ch = [text characterAtIndex: [text length] - 1];
		if (ch == 8212) ch = '_';
		if (ch == L'\n') {
			Kore::Keyboard::the()->_keydown(Kore::Key_Return, '\n');
			Kore::Keyboard::the()->_keyup(Kore::Key_Return, '\n');
		}
		else if (ch >= L'a' && ch <= L'z') {
			if (shiftDown) {
				Kore::Keyboard::the()->_keyup(Kore::Key_Shift, 0);
				shiftDown = false;
			}
			Kore::Keyboard::the()->_keydown((Kore::KeyCode)(ch + L'A' - L'a'), ch);
			Kore::Keyboard::the()->_keyup((Kore::KeyCode)(ch + L'A' - L'a'), ch);
		}
		else {
			if (!shiftDown) {
				Kore::Keyboard::the()->_keydown(Kore::Key_Shift, 0);
				shiftDown = true;
			}
			Kore::Keyboard::the()->_keydown((Kore::KeyCode)ch, ch);
			Kore::Keyboard::the()->_keyup((Kore::KeyCode)ch, ch);
		}
	}
}

- (void)deleteBackward {
	Kore::Keyboard::the()->_keydown(Kore::Key_Backspace, 0);
	Kore::Keyboard::the()->_keyup(Kore::Key_Backspace, 0);
}

- (BOOL)canBecomeFirstResponder {
	return YES;
}

- (void)onKeyboardHide:(NSNotification*)notification {
	Kore::System::hideKeyboard();
}

#ifdef SYS_METAL
- (id <MTLDevice>)metalDevice {
	return device;
}

- (id <MTLLibrary>)metalLibrary {
	return library;
}

- (id <MTLRenderCommandEncoder>)metalEncoder {
	return commandEncoder;
}
#endif

@end
