RLBox-sandboxed espeak examples. The validators should be improved. The
espeak-ng library should also be put in a Wasm sandbox. The examples only show
uses of RLBox API.

Original example source from: <https://github.com/mondhs/espeak-sample>

# sampleSpeak

Build espeak-ng and sample program, then run it:

```
make espeak-ng
make sampleSpeak
make sampleSpeak-run
```

This example is not great because we would be passing in a `FILE*` into the sanbox.

# sampleSpeakWithCallback

Build espeak-ng and sample program, then run it:

```
make espeak-ng
make sampleSpeakWithCallback
make sampleSpeakWithCallback-run
```

This is a much better example. The library is used to process data but the writing to file is dont in the app.
