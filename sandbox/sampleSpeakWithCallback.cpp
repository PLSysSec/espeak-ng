/* 
   TITLE: Simple C/C++ Program showing use of speak_lib.h 
   AUTHOR:Dhananjay Singh
   LICENSE: GPLv2
*/
#include <string.h>
#include <malloc.h>
#include "../usr/include/espeak-ng/speak_lib.h"

#define RLBOX_SINGLE_THREADED_INVOCATIONS
#define RLBOX_USE_STATIC_CALLS() rlbox_noop_sandbox_lookup_symbol

#include "rlbox/rlbox.hpp"
#include "rlbox/rlbox_noop_sandbox.hpp"

using namespace rlbox;

#define sandbox_fields_reflection_espeak_class_espeak_VOICE(f, g, ...) \
  f(const char*, name, FIELD_NORMAL, ##__VA_ARGS__) g()                \
  f(const char*, languages, FIELD_NORMAL, ##__VA_ARGS__) g()           \
  f(const char*, identifier, FIELD_NORMAL, ##__VA_ARGS__) g()          \
  f(unsigned char, gender, FIELD_NORMAL, ##__VA_ARGS__) g()            \
  f(unsigned char, age, FIELD_NORMAL, ##__VA_ARGS__) g()               \
  f(unsigned char, variant, FIELD_NORMAL, ##__VA_ARGS__) g()           \
  f(unsigned char, xx1, FIELD_NORMAL, ##__VA_ARGS__) g()               \
  f(int, score, FIELD_NORMAL, ##__VA_ARGS__) g()                       \
  f(void*, spare, FIELD_NORMAL, ##__VA_ARGS__) g()

#define sandbox_fields_reflection_espeak_class_espeak_EVENT(f, g, ...) \
  f(espeak_EVENT_TYPE , type, FIELD_NORMAL, ##__VA_ARGS__) g()         \
  f(unsigned int, unique_identifier,  FIELD_NORMAL, ##__VA_ARGS__) g() \
  f(int, text_position,  FIELD_NORMAL, ##__VA_ARGS__) g()              \
  f(int, length,  FIELD_NORMAL, ##__VA_ARGS__) g()                     \
  f(int, audio_position,  FIELD_NORMAL, ##__VA_ARGS__) g()             \
  f(int, sample,  FIELD_NORMAL, ##__VA_ARGS__) g()                     \
  f(void*, user_data,  FIELD_NORMAL, ##__VA_ARGS__) g()                \
  f(long, id,  FIELD_NORMAL, ##__VA_ARGS__) g()


#define sandbox_fields_reflection_espeak_allClasses(f, ...)            \
  f(espeak_VOICE, espeak, ##__VA_ARGS__)                               \
  f(espeak_EVENT, espeak, ##__VA_ARGS__)                               \




rlbox_load_structs_from_library(espeak);



// HELPER FUNCTIONS
const char *WordToString(unsigned int word);
static void Write4Bytes(FILE *f, int value);
int OpenWavFile(const char *path, int rate);
static void CloseWavFile();
// HELPER FUNCTIONS


FILE *f_wavfile = NULL;
int samplerate;

#define BUF_LEN 1323 /* 60*22.05 */

tainted<int, rlbox_noop_sandbox>
  synthCallback(rlbox_sandbox<rlbox_noop_sandbox> &sandbox,
                tainted<short*, rlbox_noop_sandbox> t_wav,
                tainted<int, rlbox_noop_sandbox> t_numsamples,
                tainted<espeak_EVENT*, rlbox_noop_sandbox> events) {

  int numsamples = t_numsamples.copy_and_verify([](int n) { return n; });

  if (numsamples <= 0 || numsamples > BUF_LEN) {
    return 0;
  }

  // Close wave file if we're done
  if (t_wav == nullptr) {
    printf("closing file\n");
    CloseWavFile();
    return 0;
  }

  events->type.copy_and_verify([&events](int ty) {
      if (ty == espeakEVENT_SAMPLERATE) {
        int number = static_cast<int>(events->id.UNSAFE_unverified());
        if (number > 0) {
          samplerate = number;
          printf("setting sample rate = %d\n", samplerate);
        }
      }
  });

  // Open wave file if it is not open
  if(f_wavfile == NULL){
    printf("opening file\n");
    if(OpenWavFile("/tmp/hello.wav", samplerate) != 0){
      return 1;
    }
  }

  auto wav = t_wav.copy_and_verify_range(
    [](std::unique_ptr<short[]> arr) {
        return std::move(arr);
    }, numsamples*2);

  fwrite(wav.get(), numsamples*2, 1, f_wavfile);
  printf("writing %d\n", numsamples*2);

  return 0;
}


int main(int argc, char* argv[]) {

  // create rlbox sandbox
  rlbox::rlbox_sandbox<rlbox_noop_sandbox> sandbox;
  sandbox.create_sandbox();

  // init espeak
  if (sandbox.invoke_sandbox_function(espeak_Initialize, AUDIO_OUTPUT_SYNCHRONOUS, 60, nullptr, 0).unverified_safe_because("is int") < 0) {
    return -1;
  }
  sandbox.invoke_sandbox_function(espeak_SetParameter, espeakWORDGAP, 20,0);

  // set synth callback
  auto callback = sandbox.register_callback(synthCallback);
  sandbox.invoke_sandbox_function(espeak_SetSynthCallback, callback);

  auto voice = sandbox.malloc_in_sandbox<char>(7);
  std::strcpy(voice.UNSAFE_unverified(), "en-lt1");
  sandbox.invoke_sandbox_function(espeak_SetVoiceByName, voice);

  char text[13] = {"Hello world!"};
  size_t size = strlen(text)+1;    
  auto tainted_text = sandbox.malloc_in_sandbox<char>(size);
  std::strcpy(tainted_text.UNSAFE_unverified(), text);
  auto tainted_size = strlen(text)+1;    

  sandbox.invoke_sandbox_function(espeak_Synth, tainted_text, size, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, nullptr, nullptr);

  sandbox.invoke_sandbox_function(espeak_Synchronize);

  sandbox.invoke_sandbox_function(espeak_Terminate);
  sandbox.destroy_sandbox();

  return 0;
}


// COPIED FROM espeak-sample/audacityLabelSpeak.cpp

////////////////////////////////////////////////////////////////////////////
// Static functions, copy/pasted from espeak. Should be refactored to reuse.
////////////////////////////////////////////////////////////////////////////

static void Write4Bytes(FILE *f, int value)
{//=================================
// Write 4 bytes to a file, least significant first
	int ix;

	for(ix=0; ix<4; ix++)
	{
		fputc(value & 0xff,f);
		value = value >> 8;
	}
}

int OpenWavFile(const char *path, int rate)
//===================================
{
	static unsigned char wave_hdr[44] = {
		'R','I','F','F',0x24,0xf0,0xff,0x7f,'W','A','V','E','f','m','t',' ',
		0x10,0,0,0,1,0,1,0,  9,0x3d,0,0,0x12,0x7a,0,0,
		2,0,0x10,0,'d','a','t','a',  0x00,0xf0,0xff,0x7f};

	if(path == NULL)
		return(2);

	if(path[0] == 0)
		return(0);

	if(strcmp(path,"stdout")==0)
		f_wavfile = stdout;
	else
		f_wavfile = fopen(path,"wb");

	if(f_wavfile == NULL)
	{
		fprintf(stderr,"Can't write to: '%s'\n",path);
		return(1);
	}


	fwrite(wave_hdr,1,24,f_wavfile);
	Write4Bytes(f_wavfile,rate);
	Write4Bytes(f_wavfile,rate * 2);
	fwrite(&wave_hdr[32],1,12,f_wavfile);
	return(0);
}   //  end of OpenWavFile



static void CloseWavFile()
//========================
{
	unsigned int pos;

	if((f_wavfile==NULL) || (f_wavfile == stdout))
		return;

	fflush(f_wavfile);
	pos = ftell(f_wavfile);

	fseek(f_wavfile,4,SEEK_SET);
	Write4Bytes(f_wavfile,pos - 8);

	fseek(f_wavfile,40,SEEK_SET);
	Write4Bytes(f_wavfile,pos - 44);

	fclose(f_wavfile);
	f_wavfile = NULL;

} // end of CloseWavFile
