samples: sampleSpeak sampleSpeakWithCallback

ESPEAK_LOC = $(shell cd .. ; pwd)/usr
ESPEAK_LIB = $(ESPEAK_LOC)/lib

sampleSpeak: sampleSpeak.cpp
	clang++ -std=c++17 -g  -L $(ESPEAK_LIB) sampleSpeak.cpp -lespeak-ng -o sampleSpeak

sampleSpeak-run: sampleSpeak
	LD_LIBRARY_PATH=$(ESPEAK_LIB):$LD_LIBRARY_PATH ./sampleSpeak

sampleSpeakWithCallback: sampleSpeakWithCallback.cpp
	clang++ -std=c++17 -g  -L $(ESPEAK_LIB) sampleSpeakWithCallback.cpp -lespeak-ng -o sampleSpeakWithCallback

sampleSpeakWithCallback-run: sampleSpeakWithCallback
	LD_LIBRARY_PATH=$(ESPEAK_LIB):$LD_LIBRARY_PATH ./sampleSpeakWithCallback

espeak-ng:
	cd .. && ./configure --prefix=$(ESPEAK_LOC) && make en

clean:
	-rm sampleSpeak
	-rm sampleSpeakWithCallback
