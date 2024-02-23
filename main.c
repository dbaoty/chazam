
#include <stdio.h>
#include <portaudio.h>

typedef struct {
  float left_phase;
  float right_phase;
} paTestData;

static int pa_stream_callback(const void* input, void* output, unsigned long frame_count, const PaStreamCallbackTimeInfo* time_info, PaStreamCallbackFlags status_flags, void* user_data) {
  paTestData* data = (paTestData*)user_data;  
  float* out = (float*)output;
  unsigned int i;
  (void) input;

  for (i = 0; i < frame_count; ++i) {
    *(out++) = data->left_phase;
    *(out++) = data->right_phase;
    data->left_phase += 0.1f;
    if (data->left_phase >= 1.0f) data->left_phase -= 2.0f;
    data->right_phase += 0.03f;
    if (data->right_phase >= 1.0f) data->right_phase -= 2.0f;
  }

  return 0;
}

PaError pa_init() {
  return Pa_Initialize();
}

PaDeviceIndex pa_setup_input_device() {
  return Pa_GetDefaultInputDevice();
}

PaDeviceIndex pa_setup_output_device() {
  return Pa_GetDefaultOutputDevice();
}

PaError pa_shutdown() {
  return Pa_Terminate();
}

void pa_device_print(const PaDeviceIndex* pa_device) { 
  const PaDeviceInfo* pa_device_info = Pa_GetDeviceInfo(*pa_device);

  printf("\nInput device information:\n");
  printf("name -> %s\n", pa_device_info->name);
  printf("index -> %d\n", Pa_GetDefaultInputDevice());
  printf("max input channels -> %d\n", pa_device_info->maxInputChannels);
  printf("max output channels -> %d\n", pa_device_info->maxOutputChannels);
  printf("default low input latency -> %0.2f\n", pa_device_info->defaultLowInputLatency);
  printf("default low output latency -> %0.2f\n", pa_device_info->defaultLowOutputLatency);
  printf("default high input latency -> %0.2f\n", pa_device_info->defaultHighInputLatency);
  printf("default high output latency -> %0.2f\n", pa_device_info->defaultHighOutputLatency);
  printf("default sample rate -> %0.2f\n\n", pa_device_info->defaultSampleRate);
}

int main(void) {
  /* with the exception of PaNoError, all PaErrorCodes return negative values */
  PaError pa_initialize = pa_init();
  if (pa_initialize != paNoError) {
    printf("PortAudio initialization error: %s\n", Pa_GetErrorText(pa_initialize));
    return pa_initialize;
  }

  const char* pa_version = Pa_GetVersionText();
  printf("pa_version -> %s\n", pa_version);

  /* get default input device on system. The function call returns an index
   * corresponding to the captured input device. */
  PaDeviceIndex pa_input_device = pa_setup_input_device();
  if (pa_input_device == paNoDevice) {
    printf("PortAudio Input Device Setup failed!\n");
    pa_shutdown();
    return 0;
  }

  const PaDeviceInfo* pa_input_device_info = Pa_GetDeviceInfo(pa_input_device);
  pa_device_print(&pa_input_device);

  /* input stream parameters initialization */
  PaStreamParameters pa_input_parameters;
  pa_input_parameters.device= pa_input_device;
  pa_input_parameters.channelCount = pa_input_device_info->maxInputChannels;
  pa_input_parameters.sampleFormat = paFloat32;
  pa_input_parameters.suggestedLatency = pa_input_device_info->defaultLowInputLatency;
  pa_input_parameters.hostApiSpecificStreamInfo = NULL;

  
  /* get default output device on system. The function call returns an index
   * corresponding to the captured output device. */
  PaDeviceIndex pa_output_device = pa_setup_output_device();
  if (pa_output_device == paNoDevice) {
    printf("PortAudio Output Device Setup failed!\n");
    pa_shutdown();
    return 0;
  }

  const PaDeviceInfo* pa_output_device_info = Pa_GetDeviceInfo(pa_output_device);
  pa_device_print(&pa_output_device);

  PaStreamParameters pa_output_parameters;
  pa_output_parameters.device = pa_output_device;
  pa_output_parameters.channelCount = pa_output_device_info->maxOutputChannels;
  pa_output_parameters.sampleFormat = paFloat32;
  pa_output_parameters.suggestedLatency = pa_output_device_info->defaultLowOutputLatency;
  pa_output_parameters.hostApiSpecificStreamInfo = NULL;

  /* attempt to open a stream an input stream @ pa_input_device (?) */
  PaStream* pa_input_device_stream;
  PaError pa_open_stream = Pa_OpenStream(&pa_input_device_stream,
        &pa_input_parameters,
        &pa_output_parameters,
        pa_input_device_info->defaultSampleRate,
        0,
        paPrimeOutputBuffersUsingStreamCallback,
        NULL,
        NULL);

  if (pa_open_stream != paNoError) {
    printf("Stream open PaErrorText -> %s\n", Pa_GetErrorText(pa_open_stream));
    return 0;
  }

  Pa_Sleep(3 * 1000);

  PaError pa_start_stream = Pa_StartStream(pa_input_device_stream);
  if (pa_start_stream != paNoError) {
    printf("Stream start PaErrorText -> %s\n", Pa_GetErrorText(pa_start_stream));
  }

  Pa_Sleep(3 * 1000);

  Pa_StopStream(pa_input_device_stream);

  Pa_Sleep(3 * 1000);

  long int stream_info = Pa_GetStreamReadAvailable(&pa_input_device_stream);
  printf("STREAM INFO -> %s\n", Pa_GetErrorText(stream_info));

  Pa_Sleep(3 * 1000);

  PaError pa_close_stream = Pa_CloseStream(pa_input_device_stream);

  if (pa_close_stream != paNoError) {
    printf("Stream PaErrorText -> %s\n", Pa_GetErrorText(pa_close_stream));
    return 0;
  }

  Pa_Sleep(3 * 1000);

  pa_shutdown();
}
