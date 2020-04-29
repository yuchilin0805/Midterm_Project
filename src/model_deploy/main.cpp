#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"
#include "mbed.h"

#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "uLCD_4DGL.h"


Serial pc(USBTX, USBRX);
InterruptIn button(SW2);
DigitalIn confirmbutton(SW3);
DigitalOut redled(LED1); 
DigitalOut greenled(LED2);
Thread t1;
uLCD_4DGL uLCD(D1, D0, D2);
Thread t2;
Thread t3;

Timer timers;
Timer timer2;

int interruptcall=0;

int PredictGesture(float* output);
EventQueue queue1(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
EventQueue queue3(32 * EVENTS_EVENT_SIZE);
int gesture_result();
constexpr int kTensorArenaSize = 60 * 1024;
uint8_t tensor_arena[kTensorArenaSize];


int player;
int endplaying=0;
int nowplaying=0;
int nextsong;
int numberofsongs=2;
int ready_to_load=2;
int song_select_mode=0;
int load_or_unload_mode=0;
int unload_mode=0;
void playmusic();
void loadsong();
void load_or_unload();
// Set up logging.
static tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;

// Map the model into a usable data structure. This doesn't involve any
// copying or parsing, it's a very lightweight operation.
const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
 TfLiteTensor* model_input;
int input_length;
tflite::MicroInterpreter* interpreter;
class songs{
  public:
    void loadinfo(int*);
    string name;
    int length;
    void printinfo();
    void printname();
    void unload();
  private:
    int* info;
};
void songs::loadinfo(int* x){
  info =new int[length];
  for(int i=0;i<length;i++){
    info[i]=x[i];
  }
}
void songs::printinfo(){
  for(int i=0;i<length;i++){
    uLCD.printf("%d ",info[i]);
  }
}
void songs::printname(){
  int j=0;
  while(name[j]!='\0'){
    uLCD.printf("%c",name[j]);
    j++;
  }
}
void songs::unload(){
  name="0";
  length=0;
  delete []info;
  //info=NULL;
}
songs songlist[8];
void showlist(){
  uLCD.cls();
  uLCD.textbackground_color(BLACK);
  song_select_mode=1;
  for(int i=0;i<numberofsongs;i++){
    if(i==nowplaying)
      uLCD.textbackground_color(BLUE);
    else
      uLCD.textbackground_color(BLACK);
    uLCD.printf("%d:",i);
    songlist[i].printname();
    uLCD.printf("\n");
  }
  uLCD.textbackground_color(BLACK);
  uLCD.printf("\n");
  uLCD.printf("forward :ring\n");
  uLCD.printf("backward :slope\n");
  uLCD.printf("load or unload:one\n");
  int result=gesture_result();
  if(result==-2)
    load_or_unload();
  else if(result>=0){
    nextsong=result;
    player=queue2.call_in(500,playmusic);
  }
  else
    uLCD.printf("errors");
  song_select_mode=0;
}
void load_or_unload(){
  uLCD.cls();
  load_or_unload_mode=1;
  uLCD.printf("load: ring\n");
  uLCD.printf("unload: slope\n");
  int result=gesture_result();
  load_or_unload_mode=0;
  if(result==0)
    loadsong();
  else if(result==1){
    uLCD.cls();
    uLCD.textbackground_color(BLACK);
    for(int i=0;i<numberofsongs;i++){
      if(i==0)
        uLCD.textbackground_color(BLUE);
      else
        uLCD.textbackground_color(BLACK);
      uLCD.printf("%d:",i);
      songlist[i].printname();
      uLCD.printf("\n");
    }
    uLCD.printf("which song to unload\n");
    uLCD.printf("forward :ring\n");
    uLCD.printf("backward :slope\n");
    unload_mode=1;
    int unloadnumber=gesture_result();
    unload_mode=0;
    songlist[unloadnumber].unload();
    int i=unloadnumber;
    while(i<numberofsongs-1){
      songlist[i]=songlist[i+1];
      //songlist[i].length=songlist[i+1].length;
      //songlist[i].name=songlist[i+1].name;
      i++;
    }
    numberofsongs--;
    ready_to_load--;
  }

  

}
void loadsong(){
  int i=0; 
  char a[10];
  while(true){
    if(pc.readable()){
      a[i]=pc.getc();
      if(a[i]=='\n')
        break;
      i++;
    }
  }
  char c[i];
  for(int j=0;j<i;j++)
    c[j]=a[j];
  int16_t signalLength=atoi(c);
  songlist[ready_to_load].length=signalLength;
  
  i=0;
  char read[20];
  while(true){
    if(pc.readable()){
      read[i]=pc.getc();
      if(read[i]=='$'){
        read[i]='\0';
        break;
      }
      i++;
    }
  }
  songlist[ready_to_load].name=string(read);
  int signal[signalLength];
  int serialCount = 0;
  char serialInBuffer[32];
  i=0;  
  while(i < signalLength){
    if(pc.readable()){
      serialInBuffer[serialCount] = pc.getc();
      serialCount++;
      if(serialCount == 3){
        serialInBuffer[serialCount] = '\0';
        signal[i] = (int) atof(serialInBuffer);
        serialCount = 0;
        i++;
      }
    }
  }
  uLCD.cls();
  uLCD.printf("%d",songlist[ready_to_load].length);  
  int k=0;
  while(songlist[ready_to_load].name[k]!='\0'){
    uLCD.printf("%c",songlist[ready_to_load].name[k]);
    k++;
  }
  songlist[ready_to_load].loadinfo(signal);
  songlist[ready_to_load].printinfo();
  ready_to_load++;
  numberofsongs++;
}
void mode_selection(){
  if(timers.read_ms()>1000){
    queue2.cancel(player);
    greenled=1;
    redled=0;
    uLCD.cls();
    uLCD.printf("choose mode\n");
    uLCD.printf("0:forward :ring\n");
    uLCD.printf("1:backward :slope\n");
    uLCD.printf("2:song selection :one\n");
    int a=gesture_result();
    if(a==0){
      nextsong=nowplaying-1; 
      player=queue2.call_in(500,playmusic);
    }
    else if(a==1){
      nextsong=nowplaying+1;
      player=queue2.call_in(500,playmusic);
    }
    else if(a==2){
      showlist();        
    }
    else{
      uLCD.printf("error\n");
    }
    //uLCD.printf("a: %d",a);
    timers.reset();
  }
  uLCD.printf("ee");
}
void playmusic(){
  redled=0;
  greenled=1;
  endplaying=0;
  nowplaying=nextsong;
  uLCD.cls();
  uLCD.printf("now playing : %d\n",nowplaying);
  
  
  songlist[nowplaying].printname();
  uLCD.printf("\n");
  wait(1);
  /*timer2.start();
  while(1){
    if(timer2.read()>10){
      nowplaying++;
      timer2.reset();
      endplaying=1;
      break;
    }
  }*/
}
// Return the result of the last prediction
int PredictGesture(float* output) {
  // How many times the most recent gesture has been matched in a row
  static int continuous_count = 0;
  // The result of the last prediction
  static int last_predict = -1;

  // Find whichever output has a probability > 0.8 (they sum to 1)
  int this_predict = -1;
  for (int i = 0; i < label_num; i++) {
    if (output[i] > 0.8) this_predict = i;
  }

  // No gesture was detected above the threshold
  if (this_predict == -1) {
    continuous_count = 0;
    last_predict = label_num;
    return label_num;
  }

  if (last_predict == this_predict) {
    continuous_count += 1;
  } else {
    continuous_count = 0;
  }
  last_predict = this_predict;

  // If we haven't yet had enough consecutive matches for this gesture,
  // report a negative result
  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
    return label_num;
  }
  // Otherwise, we've seen a positive result, so clear all our variables
  // and report it
  continuous_count = 0;
  last_predict = -1;

  return this_predict;
}

int main(int argc, char* argv[]) {

  
  t1.start(callback(&queue1, &EventQueue::dispatch_forever));
  t2.start(callback(&queue2, &EventQueue::dispatch_forever));
  t3.start(callback(&queue3, &EventQueue::dispatch_forever));
  timers.start();
  button.rise(queue1.event(mode_selection));
  songlist[0].name="little star";
  songlist[1].name="little bee";
  nextsong=0;
  // wait(1);
  
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
        "Model provided is schema version %d not equal "
        "to supported version %d.",
        model->version(), TFLITE_SCHEMA_VERSION);
    return -1;
  }

  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  static tflite::MicroOpResolver<6> micro_op_resolver;
  micro_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                               tflite::ops::micro::Register_MAX_POOL_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                             tflite::ops::micro::Register_RESHAPE(), 1);
  // Build an interpreter to run the model with
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
 interpreter = &static_interpreter;

  // Allocate memory from the tensor_arena for the model's tensors
  interpreter->AllocateTensors();

  // Obtain pointer to the model's input tensor
  model_input = interpreter->input(0);
  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != config.seq_length) ||
      (model_input->dims->data[2] != kChannelNumber) ||
      (model_input->type != kTfLiteFloat32)) {
    error_reporter->Report("Bad input tensor parameters in model");
    return -1;
  }

 input_length = model_input->bytes / sizeof(float);

  TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
  if (setup_status != kTfLiteOk) {
    error_reporter->Report("Set up failed\n");
    return -1;
  }

  error_reporter->Report("Set up successful...\n");
  
  
  
  player=queue2.call(playmusic);
  while(1){
    /*if(endplaying)
      player=queue2.call(playmusic);*/
    redled=1;
    greenled=0;
    
    
  }

  
  
  
  
  
}
int gesture_result(){
  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation.
  redled=1;
  greenled=0;
  // Whether we should clear the buffer next time we fetch data
  bool should_clear_buffer = false;
  bool got_data = false;

  // The gesture index of the prediction
  int gesture_index;

  
  int getanswer=-1;
  int nowselect=nowplaying;
  int nowunload=0;
  while(true){
    redled=0;

    // Attempt to read new data from the accelerometer
    got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                 input_length, should_clear_buffer);
    
    // If there was no new data,
    // don't try to clear the buffer again and wait until next time
    if (!got_data) {
      should_clear_buffer = false;
      continue;
    }

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      error_reporter->Report("Invoke failed on index: %d\n", begin_index);
      continue;
    }

    // Analyze the results to obtain a prediction
    gesture_index = PredictGesture(interpreter->output(0)->data.f);

    // Clear the buffer next time we read data
    should_clear_buffer = gesture_index < label_num;

    // Produce an output
    if (gesture_index < label_num) {
      error_reporter->Report(config.output_message[gesture_index]);
      getanswer=gesture_index;
      /////////////////////////unload
      if(unload_mode){
        if(getanswer==0){
          if(nowunload!=0)
            nowunload--;
          else{
            nowunload=numberofsongs-1;
          }
        }
        else if(getanswer==1){
          if(nowunload==numberofsongs-1)
            nowunload=0;
          else
            nowunload++;
        }        
        else
          uLCD.printf("choose again\n");
       
        uLCD.cls();
        uLCD.locate(0,0);
        for(int i=0;i<numberofsongs;i++){
          if(i==nowunload)
             uLCD.textbackground_color(BLUE);
          else{
            uLCD.textbackground_color(BLACK);
          }
          uLCD.printf("%d:",i);
          songlist[i].printname();
          uLCD.printf("\n");
        }
        uLCD.textbackground_color(BLACK);
        uLCD.printf("\n");
        uLCD.printf("forward :ring\n");
        uLCD.printf("backward :slope\n");
        uLCD.printf("confirm with SW3\n"); 
      }
  ///////////////////songselect
      else if(song_select_mode && !load_or_unload_mode&& !unload_mode){
        if(getanswer==0){
          if(nowselect!=0)
            nowselect--;
          else{
            nowselect=numberofsongs-1;
          }
        }
        else if(getanswer==1){
          if(nowselect==numberofsongs-1)
            nowselect=0;
          else
            nowselect++;
        }
        uLCD.cls();
        uLCD.locate(0,0);
        for(int i=0;i<numberofsongs;i++){
          
          if(i==nowselect)
             uLCD.textbackground_color(BLUE);
          else{
            uLCD.textbackground_color(BLACK);
          }
          
          uLCD.printf("%d:",i);
          songlist[i].printname();
          uLCD.printf("\n");
        }
        uLCD.textbackground_color(BLACK);
        uLCD.printf("\n");
        uLCD.printf("forward :ring\n");
        uLCD.printf("backward :slope\n");
        uLCD.printf("song selection :one\n");
        if(getanswer==2)
          uLCD.printf("load or unload?\n");
        uLCD.printf("confirm with SW3\n"); 
      }
      else if(song_select_mode && load_or_unload_mode){
        uLCD.textbackground_color(BLACK);
        uLCD.cls();
        uLCD.printf("load: ring\n");
        uLCD.printf("unload: slope\n");
        if(getanswer==2)
          uLCD.printf("choose again\n");
        else if(getanswer==0)
          uLCD.printf("load?\nconfirm with SW3\n");
        else if(getanswer==1)
          uLCD.printf("unload?\nconfirm with SW3\n"); 
      }
      else{
        uLCD.textbackground_color(BLACK);
        uLCD.printf("you choose %d\n",getanswer);
        uLCD.printf("confirm with SW3\n"); 
        
      }
      
    }
    if(confirmbutton==0){
       uLCD.printf("got your confirm\n");
       wait(0.3);
       break;
    }
    
  }
  if(unload_mode){
    return nowunload;
  }
  if(song_select_mode&&!load_or_unload_mode){
    if(getanswer==2)
      return -2;
    else
      return nowselect;
  }
  else if(song_select_mode&&load_or_unload_mode)
    return getanswer;    
  else
    return getanswer;
}
