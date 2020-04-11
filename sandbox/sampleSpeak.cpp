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

#define sandbox_fields_reflection_espeak_class_espeak_VOICE(f, g, ...)                       \
  f(const char*, name, FIELD_NORMAL, ##__VA_ARGS__) g()       \
  f(const char*, languages, FIELD_NORMAL, ##__VA_ARGS__) g()  \
  f(const char*, identifier, FIELD_NORMAL, ##__VA_ARGS__) g() \
  f(unsigned char, gender, FIELD_NORMAL, ##__VA_ARGS__) g()   \
  f(unsigned char, age, FIELD_NORMAL, ##__VA_ARGS__) g()      \
  f(unsigned char, variant, FIELD_NORMAL, ##__VA_ARGS__) g()  \
  f(unsigned char, xx1, FIELD_NORMAL, ##__VA_ARGS__) g()      \
  f(int, score, FIELD_NORMAL, ##__VA_ARGS__) g()              \
  f(void*, spare, FIELD_NORMAL, ##__VA_ARGS__) g()

#define sandbox_fields_reflection_espeak_allClasses(f, ...)                  \
  f(espeak_VOICE, espeak, ##__VA_ARGS__)                                      \

rlbox_load_structs_from_library(espeak);


int main(int argc, char* argv[]) {

  // create rlbox sandbox
  rlbox::rlbox_sandbox<rlbox_noop_sandbox> sandbox;
  sandbox.create_sandbox();

  // init espeak
  if (sandbox.invoke_sandbox_function(espeak_Initialize, AUDIO_OUTPUT_PLAYBACK, 500, nullptr, 0).unverified_safe_because("is int") < 0) {
    return -1;
  }

  tainted<char*,  rlbox_noop_sandbox> klatt = sandbox.malloc_in_sandbox<char>(6);
  std::strcpy(klatt.UNSAFE_unverified(), "klatt");

  tainted<char*,  rlbox_noop_sandbox> en = sandbox.malloc_in_sandbox<char>(3);
  std::strcpy(klatt.UNSAFE_unverified(), "en");

  tainted<espeak_VOICE*, rlbox_noop_sandbox> voice = sandbox.malloc_in_sandbox<espeak_VOICE>();
  rlbox::memset(sandbox, voice, 0, sizeof(espeak_VOICE)); // Zero out the voice first
  voice->languages = en;
  voice->name = klatt;
  voice->variant = 2;
  voice->gender = 1;

  sandbox.invoke_sandbox_function(espeak_SetVoiceByProperties, voice);

  char text[13] = {"Hello world!"};
  size_t size = strlen(text)+1;    
  tainted<char*,  rlbox_noop_sandbox> tainted_text = sandbox.malloc_in_sandbox<char>(size);
  std::strcpy(tainted_text.UNSAFE_unverified(), text);
  tainted<size_t, rlbox_noop_sandbox> tainted_size = strlen(text)+1;    

  sandbox.invoke_sandbox_function(espeak_Synth, tainted_text, size, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, nullptr, nullptr );

  sandbox.invoke_sandbox_function(espeak_Synchronize);

  sandbox.invoke_sandbox_function(espeak_Terminate);
  sandbox.destroy_sandbox();

  printf("DONE saying %s\n", text); 
  return 0;
}
