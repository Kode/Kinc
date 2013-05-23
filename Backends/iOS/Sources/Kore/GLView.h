#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>

@interface GLView : UIView <UITextFieldDelegate> {    
@private
	EAGLContext *context;
	GLint backingWidth, backingHeight;
	GLuint defaultFramebuffer, colorRenderbuffer, depthRenderbuffer;
}

- (void)begin;
- (void)end;
- (void)showKeyboard;
- (void)hideKeyboard;

@end