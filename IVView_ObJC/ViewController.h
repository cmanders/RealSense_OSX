//
//  ViewController.h
//  IVView_ObJC
//
//  Created by Corey Manders on 9/12/15.

//

#import <Cocoa/Cocoa.h>
#import "Intel_Depth_Camera.hpp"

// now build lookup table


@interface ViewController : NSViewController



@property (weak) IBOutlet NSImageView *cameraImage;
@property StreamType currentStream;
@end

ViewController * myself;
NSBitmapImageRep *colorRep = nullptr;
NSBitmapImageRep *singleChannelRep = nullptr;
NSImage *   myRGB = nullptr;
