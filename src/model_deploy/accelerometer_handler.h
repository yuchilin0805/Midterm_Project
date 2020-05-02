#ifndef ACCELEROMETER_HANDLER_H_
#define ACCELEROMETER_HANDLER_H_

#define kChannelNumber 3

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
//#include "uLCD_4DGL.h"
extern int begin_index;
extern TfLiteStatus SetupAccelerometer(tflite::ErrorReporter* error_reporter);
extern bool ReadAccelerometer(tflite::ErrorReporter* error_reporter,
                              float* input, int length, bool reset_buffer);
extern void ReadAcc_taiko(int);
extern int* record;
extern void del_record();
extern void init_record(int);
extern int starttaiko;
extern int hit;

 //uLCD_4DGL uLCD(D1, D0, D2);
#endif  // ACCELEROMETER_HANDLER_H_