//
//  Intel_Depth_Camera.cpp
//  IVView_ObJC
//
//  Created by Corey Manders on 5/1/16.
//

#include "Intel_Depth_Camera.hpp"

/*  cb is callback function for then the uvc camera has RGB data ready
    the data is transformed from YUY2 to RGB
    rgb_func is the external function that was passed-in when streaming was started
 
 */
void cb(uvc_frame_t *frame, void *ptr) {
    
    uvc_frame_t *bgr ;
    uvc_error_t ret;
    

    /* We'll convert the image from YUV/JPEG to BGR, so allocate space */
    bgr = uvc_allocate_frame(frame->width * frame->height * 3);
    if (!bgr) {
        printf("unable to allocate bgr frame!");
        return;
    }
    
    /* Do the BGR conversion */
    ret = uvc_any2rgb(frame, bgr);
    if (ret) {
        uvc_perror(ret, "uvc_any2bgr");
        uvc_free_frame(bgr);
        return;
    }
    rgb_func((unsigned char*)bgr->data);
    uvc_free_frame(bgr);
  
}

/*  callback function for then the uvc camera has Depth data ready
    func is the external function that was passed-in when streaming was started

 */

void cb_gray16(uvc_frame_t *frame, void *ptr) {
    func((unsigned short*)frame->data);
}

void cb_gray8(uvc_frame_t *frame, void *ptr) {

    singleChannelFunc((unsigned char*)frame->data);
}

/* set_upCamera simply initialized the modified UVC Library and looks for the RealSense Camera */
int Intel_Depth_Camera::set_up_camera(){
    

    res = uvc_init(&ctx, NULL);
    if (res < 0) {
        uvc_perror(res, "uvc_init");
        return -1;
    }
    else{
        printf("UVC Initialized!\n");
    }
    
    res = uvc_find_device(
                          ctx, &dev,
                          0x8086, 0x0a66, NULL); /* filter devices: vendor_id, product_id, "serial_num" */
    if (res < 0) {
        uvc_perror(res, "uvc_find_device"); /* no devices found */
        return -2;
    }
    return 0;
}

// no longer being used
int Intel_Depth_Camera::stop_streaming(){
     uvc_stop_streaming(devh_d);
    uvc_stop_streaming(devh_rgb);
    return 0;
}

/*  what follows are the getters and setters for the Depth Parameters,
    such as Laser Strength, Confidence Threshold, Accuracy, Range/Motion Tradeoff,
    and the RealSense filter option. All of the functions access the UVC Extension Register
 */

// Laser Strength can be between 0 and 16
int Intel_Depth_Camera::set_laser_strength(int strength)
{
    uvc_error_t res;
    if (strength > 16 || strength < 0)return -1;
    unsigned short t = (unsigned short)strength;
    res= (uvc_error_t)uvc_set_ctrl(devh_d, 5, 1,&t,2);
    return (int)res;
}

int Intel_Depth_Camera::get_laser_strength()
{
    unsigned short data;
    uvc_get_ctrl(devh_d, 5, 1,&data,2,UVC_GET_CUR);
    return data;
}

//accuracy can be between 0 and 3
int Intel_Depth_Camera::set_accuracy(int accuracy)
{
    int res;
    if (accuracy > 3 || accuracy < 0) return -1;
    unsigned short t = (unsigned short)accuracy;
    res = uvc_set_ctrl(devh_d, 5, 2,&t,2);
    return (int)res;
}

int Intel_Depth_Camera::get_accuracy()
{
    unsigned short t;
    uvc_get_ctrl(devh_d, 5, 2,&t,2,UVC_GET_CUR);
    return (int)t;
}

bool Intel_Depth_Camera::is_depth_stream_open()
{
    return depth_stream_is_open;
}

bool Intel_Depth_Camera::is_ir_stream_open()
{
    return ir_stream_is_open;
}

//range/motion tradeoff can be between 0 and 100
int Intel_Depth_Camera::set_motion_tradeoff(int tradeoff){
    int res;
    if (tradeoff > 100 || tradeoff < 0) return -1;
    unsigned short t = (unsigned short)tradeoff;
    res = uvc_set_ctrl(devh_d, 5, 3,&t,2);
    return (int)res;
}
int Intel_Depth_Camera::get_motion_tradeoff(){
    unsigned short t;
    uvc_get_ctrl(devh_d, 5, 3,&t,2,UVC_GET_CUR);
    return (int)t;
}

//filter option can be between 0 and 8
int Intel_Depth_Camera::set_filter_option(int option){
    int res;
    if (option > 8 || option < 0) return -1;
    unsigned short t = (unsigned short)option;
    res = uvc_set_ctrl(devh_d, 5, 5,&t,2);
    return (int)res;
}
int Intel_Depth_Camera::get_filter_option(){
    unsigned short t;
    uvc_get_ctrl(devh_d, 5, 5,&t,2,UVC_GET_CUR);
    return (int)t;
}


//confidence threshold can be between 0 and 16
int Intel_Depth_Camera::set_confidence_thresh(int thresh){
    int res;
    if (thresh > 16 || thresh < 0) return -1;
    unsigned short t = (unsigned short)thresh;
    res = uvc_set_ctrl(devh_d, 5, 6,&t,2);
    return (int)res;
}
int Intel_Depth_Camera::get_confidence_thresh(){
    unsigned short t;
    uvc_get_ctrl(devh_d, 5, 6,&t,2,UVC_GET_CUR);
    return (int)t;
}

int Intel_Depth_Camera::get_auto_exposure(){
    
    uint8_t mode;
    uvc_get_ae_mode(devh_rgb, &mode,  UVC_GET_CUR);
    printf("mode -= %d\n",mode);
    return (int)mode;
    
}
void Intel_Depth_Camera::set_auto_exposure(int state){
    uvc_error_t err;
    uint8_t mode = (int)state;

    err =uvc_set_ae_mode(devh_rgb, mode);


}

int Intel_Depth_Camera::get_exposure_abs(){
    uint32_t time;
    uvc_error_t err;
    err= uvc_get_exposure_abs(devh_rgb, &time, UVC_GET_CUR);
    return  time;
}

void Intel_Depth_Camera::set_exposure_abs(int time){
    uint32_t utime = (uint32_t)time;
    uvc_error_t err;
    err= uvc_set_exposure_abs(devh_rgb, utime);
}

int  Intel_Depth_Camera::get_brightness(){
    uvc_error_t err;
    int16_t brightness;
    err= uvc_get_brightness(devh_rgb, &brightness, UVC_GET_CUR);
    return brightness;
}
void Intel_Depth_Camera::set_brightness(int brightness){
    if (brightness <1 || brightness > 8) return;
    uint16_t gain;
    uvc_error_t err;
    
    err= uvc_get_gain(devh_rgb, &gain, UVC_GET_MIN);
    printf("b min = %d\n",gain);
    
    err= uvc_get_gain(devh_rgb, &gain, UVC_GET_MAX);
    printf("b max = %d\n",gain);
    
    err= uvc_get_gain(devh_rgb, &gain, UVC_GET_CUR);
    printf("b cur = %d\n",gain);
    
    uvc_set_brightness(devh_rgb, brightness);
}

int Intel_Depth_Camera::get_gain(){
    uint16_t gain;
    uvc_error_t err;
    err= uvc_get_gain(devh_rgb, &gain, UVC_GET_CUR);
    return gain;
}
void Intel_Depth_Camera::set_gain(int gain){
    uvc_set_gain(devh_rgb, gain);
    
}

/* start_rgb_stream will start RGB streaming. A callback function pointer needs to be passed
   in, which will be called when the UVC callback function within this library is called.
    The prototype for the function is:
    void callback(unsigned char * data)
 */
void Intel_Depth_Camera::start_rgb_stream(void (*f)(unsigned char *),int width,int height,int fps){
    if (!rgb_stream_is_open)
        res = uvc_open2(dev, &devh_rgb,0);
    else {
        uvc_stop_streaming(devh_rgb);
        rgb_stream_is_open = false;
    }
    if (res <0 )fprintf(stderr,"Error opening RGB Cam!!!");
    else fprintf(stderr,"RGB Camera Opened");
    uvc_print_diag(devh_rgb, stderr);
    
    res = uvc_get_stream_ctrl_format_size(devh_rgb, &ctrl_rgb,
                                          UVC_FRAME_FORMAT_YUYV,
                                          width, height, fps
                                          );

    ///  Print out the result
   // uvc_print_stream_ctrl(&ctrl_rgb, stderr);
    rgb_func = f;
    
    if (res < 0) {
        uvc_perror(res, "get_mode"); // device doesn't provide a matching stream
    } else {
        res = uvc_start_streaming(devh_rgb, &ctrl_rgb, cb, 0, 0);
        
        
        if (res < 0) {
            uvc_perror(res, "start_streaming"); // unable to start stream
        } else {
            puts("Streaming...");
            rgb_stream_is_open = true;
        }
    }
}
void Intel_Depth_Camera::start_ir_stream(void (*f)(unsigned char *), int width, int height, int fps){
    if (depth_stream_is_open){
         uvc_stop_streaming(devh_d);
        depth_stream_is_open = false;
        
    }
    if (!depth_stream_is_open && !ir_stream_is_open) res = uvc_open2(dev, &devh_d,1);
    if (res <0 )fprintf(stderr,"Error opening RGB Cam!!!");
    else fprintf(stderr,"RGB Camera Opened");
    // uvc_print_diag(devh_rgb, stderr);
    
    res = uvc_get_stream_ctrl_format_size(devh_d, &ctrl_d,
                                          UVC_FRAME_FORMAT_INVI,
                                          width, height, fps
                                          );
    
    ///  Print out the result
    uvc_print_stream_ctrl(&ctrl_d, stderr);
    singleChannelFunc = f;
    
    if (res < 0) {
        uvc_perror(res, "get_mode"); // device doesn't provide a matching stream
    } else {
        res = uvc_start_streaming(devh_d, &ctrl_d, cb_gray8, 0, 0);
        
        
        if (res < 0) {
            uvc_perror(res, "start_streaming"); // unable to start stream
        } else {
            puts("Streaming...");
            depth_stream_is_open = true;
        }
    }
}

/* start_depth_stream will start the Depth stream. A callback function pointer needs to be passed
 in, which will be called when the UVC callback function within this library is called.
 The prototype for the function is:
 void callback(unsigned short * data)
 
 Note that the data being received by the camera is 16-bit (Unsigned short), in the code example,
 this is converted to 8-bit and processed into RGB, however the unsigned short data could be processed
 directly and yield much greater accuracy than what is shown in the code sample. However, visualition 
 becomes an issue.
 */
void Intel_Depth_Camera::start_depth_stream(void (*f)(unsigned short*), int width, int height, int fps){
    if (depth_stream_is_open){
        uvc_stop_streaming(devh_d);
        depth_stream_is_open = false;
    }
    if (!ir_stream_is_open && !depth_stream_is_open) res = uvc_open2(dev, &devh_d,1);
    if (res <0 )fprintf(stderr,"Error opening Depth Cam!!!\n");
    else fprintf(stderr,"Depth Camera Opened");
    uvc_print_diag(devh_d, stderr);
    func = f;
    //  uvc_print_stream_ctrl(&ctrl_rgb, stderr);
    res = uvc_get_stream_ctrl_format_size(
                                          devh_d, &ctrl_d, /* result stored in ctrl */
                                          UVC_FRAME_FORMAT_INVZ, /*INVR is range info */
                                          // UVC_FRAME_FORMAT_YUYV,
                                          width, height, fps /* width, height, fps */
                                          );
    
    /* Print out the result */
    // uvc_print_stream_ctrl(&ctrl_d, stderr);
    //   uvc_print_diag(devh_d, NULL);
    
    if (res < 0) {
        uvc_perror(res, "get_mode"); /* device doesn't provide a matching stream */
    } else {

        res = uvc_start_streaming(devh_d, &ctrl_d, cb_gray16,0, 0);
        
        
        if (res < 0) {
            uvc_perror(res, "start_streaming"); /* unable to start stream */
        } else {
            puts("Streaming...");
            depth_stream_is_open = true;
        }
    }
}
