# Emscripten

## Build

```
cd trunk
mkdir build
cd build
emcmake cmake ..
emmake make
```

## Link

```
em++ -flto -O3 -fno-rtti -fno-exceptions *.o ../../../bin/libtinyxml.a -o index.html -sUSE_SDL=2 -sUSE_SDL_MIXER=2 -sASYNCIFY -sASYNCIFY_IGNORE_INDIRECT -sASYNCIFY_ONLY=@../../../../../funcs.txt -sENVIRONMENT=web -sSTACK_SIZE=262144 -sINITIAL_MEMORY=128mb --preload-file ../../../../levels@levels/ --preload-file ../../../../res/images@images/ --preload-file ../../../../res/sounds@sounds/ --closure 1 -sEXPORTED_RUNTIME_METHODS=['allocate']
```
