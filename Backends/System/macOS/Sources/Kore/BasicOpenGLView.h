#ifdef KORE_METAL
#import <MetalKit/MTKView.h>
#else
#import <Cocoa/Cocoa.h>
#import <OpenGL/CGLContext.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#endif

#ifdef KORE_METAL

@interface BasicOpenGLView : MTKView {
@private
	id<MTLDevice> device;
	id<MTLCommandQueue> commandQueue;
	id<MTLCommandBuffer> commandBuffer;
	id<MTLRenderCommandEncoder> commandEncoder;
	id<CAMetalDrawable> drawable;
	id<MTLLibrary> library;
	MTLRenderPassDescriptor* renderPassDescriptor;
}

#else

// (DK) context sharing
// www.cocoabuilder.com/archive/cocoa/29573-sharing-opengl-context.html
// basically:
//  -don't use NSOpenGLView, but implement all that by hand
//  -use -initWithFormat:shareContext: (NSOpenGLContext) to setup the shared contexts
@interface BasicOpenGLView : NSOpenGLView {
}

#endif

#ifdef KORE_METAL
- (id<MTLDevice>)metalDevice;
- (id<MTLLibrary>)metalLibrary;
- (id<MTLRenderCommandEncoder>)metalEncoder;

- (void)begin;
- (void)end;
#endif

+ (NSOpenGLPixelFormat*)basicPixelFormat;

- (void)keyDown:(NSEvent*)theEvent;
- (void)keyUp:(NSEvent*)theEvent;

- (void)mouseDown:(NSEvent*)theEvent;
- (void)mouseUp:(NSEvent*)theEvent;
- (void)mouseMoved:(NSEvent*)theEvent;
- (void)mouseDragged:(NSEvent*)theEvent;
- (void)rightMouseDown:(NSEvent*)theEvent;
- (void)rightMouseUp:(NSEvent*)theEvent;
- (void)rightMouseDragged:(NSEvent*)theEvent;
- (void)scrollWheel:(NSEvent*)theEvent;
- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender;

- (void)prepareOpenGL;
- (void)update; // moved or resized

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;

- (id)initWithFrame:(NSRect)frameRect;
- (void)switchBuffers;

@end
