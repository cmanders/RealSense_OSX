//
//  ViewController.m
//  IVView_ObJC
//
//  Created by Corey Manders on 9/12/15.

//

#import "ViewController.h"

/* an example of using the Intel_Depth_Cam class (C++) from an objective-c
   cocoa app. 
 
   Note that is slightly more tricky than just calling the library from a c++ program.
 
 */



@interface ViewController()

//various cocoa outlets for the interface
@property (weak) IBOutlet NSSliderCell *motion_trade_off_slider;
@property (weak) IBOutlet NSSlider *laser_strength_slider;
@property (weak) IBOutlet NSSlider *confidence_slider;
@property (weak) IBOutlet NSSliderCell *accuracy_slider;
@property (weak) IBOutlet NSSliderCell *filter_option;
@property int rgb_width;
@property int rgb_height;
@property int rgb_fps;
@property (weak) IBOutlet NSMenu *streamTypeMenu;
@property (weak) IBOutlet NSTextField *laserStrengthLabel;
@property (weak) IBOutlet NSTextField *confidenceLabel;

@property (weak) IBOutlet NSTextField *filterOptionLabel;

@property (weak) IBOutlet NSTextField *MRLabel;
@property (weak) IBOutlet NSTextField *accuracyLabel;
@property (weak) IBOutlet NSButton *autoExposureCheck;
@property (weak) IBOutlet NSSliderCell *expTimeSlider;

@property (weak) IBOutlet NSSlider *brightnessSlider;


@end
unsigned short *holding_pen;

@implementation ViewController

Intel_Depth_Camera * camera;

- (void)viewDidLoad {
    [super viewDidLoad];
      myself = self; //get a reference to self for the C++ functions
    // initialize the Intel_Depth_Camera class
    camera = new Intel_Depth_Camera();
    camera->set_up_camera();
    self.currentStream = DEPTH;
    self.rgb_height = 480;
    self.rgb_width = 640;
    self.rgb_fps = 60;
    
    for (int i = 0;i< 65;i++){
        NSString * newItem;
        if (camera->capabilities[i][0]==RGB){
            newItem = [NSString stringWithFormat:@"RGB %dx%d %dfps",camera->capabilities[i][1],
                              camera->capabilities[i][2],
                              camera->capabilities[i][3]
                              ];
        }
        else if(camera->capabilities[i][0]==DEPTH){
            newItem = [NSString stringWithFormat:@"Depth %dx%d %dfps",camera->capabilities[i][1],
                       camera->capabilities[i][2],
                       camera->capabilities[i][3]
                       ];
        }
        else if(camera->capabilities[i][0]==IR){
            newItem = [NSString stringWithFormat:@"IR %dx%d %dfps",camera->capabilities[i][1],
                       camera->capabilities[i][2],
                       camera->capabilities[i][3]
                       ];
        }
        NSMenuItem * newEntry = [[NSMenuItem alloc] initWithTitle:newItem action:nil keyEquivalent:@""];
        [self.streamTypeMenu addItem:newEntry];
    }
    
    holding_pen = (unsigned short*)malloc(640*480*sizeof(unsigned short));
    
    //start both an RGB Stream and Depth Stream
    [self setUpRGBStream];
    [self setUpDepthStream];
    
    //set the inital values of the slider using
    self.laser_strength_slider.intValue = camera->get_laser_strength();
    self.laserStrengthLabel.intValue = self.laser_strength_slider.intValue;
    NSLog(@"current laser strens = %d",camera->get_laser_strength());

    self.confidence_slider.intValue = camera->get_confidence_thresh();
    
    self.confidenceLabel.intValue =self.confidence_slider.intValue;
    
    self.motion_trade_off_slider.intValue = camera->get_motion_tradeoff();
    
    self.MRLabel.intValue =self.motion_trade_off_slider.intValue;
    self.accuracy_slider.intValue = camera->get_accuracy();
    self.accuracyLabel.intValue =self.accuracy_slider.intValue;
    int state = camera->get_auto_exposure();
    if (state == 1){
        self.autoExposureCheck.state = 0;
        self.expTimeSlider.enabled = YES;
    }
    else {
        self.autoExposureCheck.state = 1;
        self.expTimeSlider.enabled = NO;
    }
    int br = camera->get_brightness();
    self.brightnessSlider.intValue = br;

    
}


// change the type of image shown when the user changes the popUp Button value
- (IBAction)imageTypeButton:(NSPopUpButtonCell *)sender {
    int line = (int)sender.indexOfSelectedItem;
    NSLog(@"%d: %d %d %d",camera->capabilities[line][0],camera->capabilities[line][1],camera->capabilities[line][2],camera->capabilities[line][3]);
  
    self.rgb_width =camera->capabilities[line][1];
    self.rgb_height =camera->capabilities[line][2];
    self.rgb_fps =camera->capabilities[line][3];
    
    if (camera->capabilities[line][0] == RGB){
        self.currentStream = NONE;
        [self setUpRGBStream];
        self.currentStream = RGB;
    }
    else if (camera->capabilities[line][0] == DEPTH){
          self.currentStream = NONE;
        [self setUpDepthStream];
        self.currentStream = DEPTH;
    }
    else if (camera->capabilities[line][0] == IR){
          self.currentStream = NONE;
        [self setUpIRStream];
        self.currentStream = IR;
    }
    
/*
    if (sender.indexOfSelectedItem == 1){
       
        self.currentStream = RGB;
    }
    if (sender.indexOfSelectedItem == 0){
         if (camera->is_ir_stream_open())[self setUpDepthStream];
        self.currentStream = DEPTH;
    }
    if (sender.indexOfSelectedItem == 2){
        if (camera->is_depth_stream_open()) [self setUpIRStream];
        self.currentStream = IR;
    }
    */
}

// use the camera class to change the camera parameters when the sliders are changed
- (IBAction)confidenceThreshChange:(NSSliderCell *)sender {
    camera->set_confidence_thresh(sender.intValue);
    self.confidenceLabel.intValue = sender.intValue;
}
- (IBAction)filterOptionChange:(NSSliderCell *)sender {
    camera->set_filter_option(sender.intValue);
    self.filterOptionLabel.intValue = sender.intValue;
}

- (IBAction)motionTradeoffChange:(NSSliderCell *)sender {
    camera->set_motion_tradeoff(sender.intValue);
    self.MRLabel.intValue = sender.intValue;
}

- (IBAction)accuracyChange:(NSSliderCell *)sender {
    camera->set_accuracy(sender.intValue);
    self.accuracyLabel.intValue = sender.intValue;
}
- (IBAction)setLaser:(NSSliderCell *)sender {
    self.laserStrengthLabel.intValue = sender.intValue;
    camera->set_laser_strength(sender.intValue);
}

void stopStreaming(uvc_device_handle_t *devh)
{
    camera->stop_streaming();
}

//an example of querying the laser strength using the Intel_Depth_Camera class
int getLaserStrength(uvc_device_handle_t *devh)
{
    return camera->get_laser_strength();
}

// callback_rgb is the callback function who's pointer is passed to the Intel_Depth_Camera
// when RGB streaming is started

void callback_rgb(unsigned char * data){
    if (myself.currentStream != RGB) return ;
    @autoreleasepool {
        
        
            colorRep =
            [[NSBitmapImageRep alloc]
             initWithBitmapDataPlanes: nil  // allocate the pixel buffer for us
             pixelsWide: myself.rgb_width
             pixelsHigh: myself.rgb_height
             bitsPerSample: 8
             samplesPerPixel:3
             hasAlpha: NO
             isPlanar: NO
             colorSpaceName: NSCalibratedRGBColorSpace
             bytesPerRow: 0
             bitsPerPixel: 24];
            
        
        myRGB = [[NSImage alloc] initWithSize:NSMakeSize(myself.rgb_width,myself.rgb_height)];
        
        
        unsigned char* pix = [colorRep bitmapData];
        memcpy(pix,data, [colorRep pixelsWide]*[colorRep pixelsHigh] *[colorRep samplesPerPixel]);
        [myRGB addRepresentation:colorRep];
        
        myself.cameraImage.image = myRGB;
        myRGB = nil;
    }
}

void callback_single_channel(unsigned char * data){
    if (myself.currentStream != IR ) return;
    
    @autoreleasepool {
        
        if (singleChannelRep == nullptr){
            singleChannelRep  =
            [[NSBitmapImageRep alloc]
             initWithBitmapDataPlanes: nil  // allocate the pixel buffer for us
             pixelsWide: myself.rgb_width
             pixelsHigh: myself.rgb_height
             bitsPerSample: 8
             samplesPerPixel:1
             hasAlpha: NO
             isPlanar: NO
             colorSpaceName: NSCalibratedWhiteColorSpace
             bytesPerRow: 0
             bitsPerPixel: 8];
            
        }
        myRGB = [[NSImage alloc] initWithSize:NSMakeSize(myself.rgb_width,myself.rgb_height)];
        
        unsigned char* pix = [singleChannelRep bitmapData];
        memcpy(pix,data, myself.rgb_width*myself.rgb_height );
        [myRGB addRepresentation:singleChannelRep];
        
        myself.cameraImage.image = myRGB;
        myRGB = nil;
    }

    
}

-(void)processRangeData
{


    
}

// callback is the callback function who's pointer is passed to the Intel_Depth_Camera
// when Depth streaming is started
void callback(unsigned short * data){
    
    
    if (myself.currentStream != DEPTH)return;
    @autoreleasepool {
        
        // generate a greyscale image representation //
        
        colorRep =
        [[NSBitmapImageRep alloc]
         initWithBitmapDataPlanes: nil  // allocate the pixel buffer for us
         pixelsWide: myself.rgb_width
         pixelsHigh: myself.rgb_height
         bitsPerSample: 8
         samplesPerPixel:3
         hasAlpha: NO
         isPlanar: NO
         colorSpaceName: NSCalibratedRGBColorSpace // 0 = black, 1 = white in this color space
         bytesPerRow: 0     // passing 0 means "you figure it out"
         bitsPerPixel: 24];   // this must agree with bitsPerSample and samplesPerPixel
        
        
        // unsigned short * mdata = data;
        
        
        NSInteger rowBytes = [colorRep bytesPerRow];
        unsigned char* pix = [colorRep bitmapData];
        for ( int i = 0; i < myself.rgb_height; ++i )
        {
            for ( int j = 0; j <myself.rgb_width;++j )
            {
                unsigned char oVal =(data[i * myself.rgb_width + j]/255);
                if (oVal == 0){
                    pix[i * rowBytes + j*3] = 0;
                    pix[i * rowBytes + j*3+1] =0;
                    pix[i * rowBytes + j*3+2] =0;
                }
                else {
                    unsigned char rVal = 255-oVal;
                    pix[i * rowBytes + j*3] = r[rVal]*255;
                    pix[i * rowBytes + j*3+1] =g[rVal]*255;
                    pix[i * rowBytes + j*3+2] =b[rVal]*255;
                }
            }
        }
        
        
        if (myRGB == nullptr)
            myRGB = [[NSImage alloc] initWithSize:NSMakeSize(myself.rgb_width,myself.rgb_height)];
        [myRGB addRepresentation:colorRep];
        myself.cameraImage.image = myRGB;
        myRGB = nil;
    }
    
}

// examples of starting the RGB and Depth streams and passing in the needed callback functions
-(void)setUpRGBStream
{
    camera->start_rgb_stream(callback_rgb,self.rgb_width,self.rgb_height,self.rgb_fps);
}

-(void)setUpDepthStream
{
    camera->start_depth_stream(callback, self.rgb_width,self.rgb_height,self.rgb_fps);
}

-(void)setUpIRStream
{
    camera->start_ir_stream(callback_single_channel,self.rgb_width,self.rgb_height,self.rgb_fps);
}
- (IBAction)autoExposureChange:(NSButtonCell*)sender {

    if (sender.state == 0){
        camera->set_auto_exposure(1);
        self.expTimeSlider.enabled = true;
    }
    else{
        camera->set_auto_exposure(2);
        self.expTimeSlider.enabled= false;
    }
}

- (IBAction)setExposure:(NSSliderCell *)sender {
    camera->set_exposure_abs(sender.intValue);
}
- (IBAction)setBrightness:(NSSlider *)sender {
    camera->set_brightness(sender.intValue);
}
- (IBAction)setGain:(NSSliderCell *)sender {
    camera->set_gain(sender.intValue);
}


- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

@end
