#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#ifndef SYS_TVOS
#import <CoreMotion/CMMotionManager.h>
#endif

@interface GLViewController : UIViewController {
@private
}

- (void)loadView;

@end