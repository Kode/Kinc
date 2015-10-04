#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/glu.h>
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/CGLContext.h>

@interface BasicOpenGLView : NSOpenGLView {
	
}

+ (NSOpenGLPixelFormat*) basicPixelFormat;

- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;

- (void) mouseDown:(NSEvent*)theEvent;
- (void) mouseUp:(NSEvent*)theEvent;
- (void) mouseMoved:(NSEvent*)theEvent;
- (void) mouseDragged:(NSEvent*)theEvent;
- (void) rightMouseDown:(NSEvent*)theEvent;
- (void) rightMouseUp:(NSEvent*)theEvent;
- (void) rightMouseDragged:(NSEvent*)theEvent;
- (void) scrollWheel:(NSEvent*)theEvent;

- (void) prepareOpenGL;
- (void) update;		// moved or resized

- (BOOL) acceptsFirstResponder;
- (BOOL) becomeFirstResponder;
- (BOOL) resignFirstResponder;

- (id) initWithFrame: (NSRect)frameRect;
- (void) switchBuffers;

@end