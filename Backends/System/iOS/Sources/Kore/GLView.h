#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#ifdef KORE_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#else
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#endif
#ifndef KORE_TVOS
#import <CoreMotion/CMMotionManager.h>
#endif

namespace Kore {
	namespace Graphics5 {
		class RenderTarget;
	}
}

@interface GLView : UIView <UIKeyInput> {
@private
#ifdef KORE_METAL
	id<MTLDevice> device;
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> commandEncoder;
	id<CAMetalDrawable> drawable;
	id<MTLLibrary> library;
	MTLRenderPassDescriptor* renderPassDescriptor;
	id<MTLTexture> depthTexture;
#else
	EAGLContext* context;
	GLuint defaultFramebuffer, colorRenderbuffer, depthStencilRenderbuffer;
#endif

#ifndef KORE_TVOS
	CMMotionManager* motionManager;
#endif
	bool hasAccelerometer;
	float lastAccelerometerX, lastAccelerometerY, lastAccelerometerZ;
}

- (void)begin;
- (void)end;
- (void)showKeyboard;
- (void)hideKeyboard;
#ifdef KORE_METAL
- (id<MTLDevice>)metalDevice;
- (id<MTLLibrary>)metalLibrary;
- (id<MTLRenderCommandEncoder>)metalEncoder;
- (void)newRenderPass:(Kore::Graphics5::RenderTarget*)renderTarget wait: (bool)wait;
#endif

@end
