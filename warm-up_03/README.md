# Sound Maker and Audio Player on Mac OS
## Group Information
1. Jainam Shah
2. Jiajie Yang

## Instructions to Run Our Programs
### The program that makes some noise and displays the properties of the default speaker
1. Compile gcc 'file_name.c' -framework CoreAudio -o 'exe_file_name'
2. Run $ ./exe_file_name

### The program that retrieves audio raw data from a video file and display on terminal
1. Use alias 'multithreadcompile' defined in the 'Setup' section.
2. Compile with $ cairogtkFFmpegcompile 'exe_file_name' 'file_name.c'.
3. Run $ ./exe_file_name video_file_name

## Introduction
The project is mainly about making noise by providing some data to your default audio output device and extracting some raw audio data from a video file.

## Setup
    * Download and install brew from [Homebrew](https://brew.sh/)
    * Install GTK+3 from [Homebrew Formulae](https://formulae.brew.sh/formula/gtk+3)
    * Intall Cairo from [Homebrew Formulae] (https://formulae.brew.sh/formula/cairo)
    * Intall FFmpeg from [Homebrew Formulae] (https://formulae.brew.sh/formula/ffmpeg)
    * alias gtkcompile="gcc `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -o"
    * alias ffmpegcompile="gcc -I/opt/homebrew/Cellar/ffmpeg/5.1.2/include -L/opt/homebrew/Cellar/ffmpeg/5.1.2/lib -lavcodec -lavformat -lavutil -lswscale -o"
    * alias cairogtkFFmpegcompile="gcc -I/opt/homebrew/Cellar/ffmpeg/5.1.2/include -L/opt/homebrew/Cellar/ffmpeg/5.1.2/lib -lavcodec -lavformat -lavutil -lswscale `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -o"
    * alias multithreadcompile="gcc -I/opt/homebrew/Cellar/ffmpeg/5.1.2/include -L/opt/homebrew/Cellar/ffmpeg/5.1.2/lib -lavcodec -lavformat -lavutil -lswscale `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lpthread -o"

## Log
### D1 : Nov21: Coding
#### Create Lab-7
