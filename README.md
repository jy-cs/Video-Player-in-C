# Local Video Player
## Group Members
1. Jainam Shah
2. Jiajie Yang

## Instructions to Run Our Programs
1. Use alias 'avmulticompile' defined in the 'Setup' section
2. Compile with $ avmulticompile 'exe_file_name' 'file_name.c'
3. Run $ ./avmulticompile video_file_name

## Introduction
The project is mainly about using ffmpeg, GTK Window, and Cairo graphics to perform a video player by decoding, displaying and synchronizing video and audio channels.

## Setup
    * Download and install brew from [Homebrew](https://brew.sh/)
    * Install GTK+3 from [Homebrew Formulae](https://formulae.brew.sh/formula/gtk+3)
    * Intall Cairo from [Homebrew Formulae] (https://formulae.brew.sh/formula/cairo)
    * Intall FFmpeg from [Homebrew Formulae] (https://formulae.brew.sh/formula/ffmpeg)
    * alias gtkcompile="gcc `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -o"
    * alias ffmpegcompile="gcc -I/opt/homebrew/Cellar/ffmpeg/5.1.2/include -L/opt/homebrew/Cellar/ffmpeg/5.1.2/lib -lavcodec -lavformat -lavutil -lswscale -o"
    * alias cairogtkFFmpegcompile="gcc -I/opt/homebrew/Cellar/ffmpeg/5.1.2/include -L/opt/homebrew/Cellar/ffmpeg/5.1.2/lib -lavcodec -lavformat -lavutil -lswscale `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -o"
    * alias multithreadcompile="gcc -I/opt/homebrew/Cellar/ffmpeg/5.1.2/include -L/opt/homebrew/Cellar/ffmpeg/5.1.2/lib -lavcodec -lavformat -lavutil -lswscale `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lpthread -o"
    * alias avmulticompile="gcc -I/opt/homebrew/Cellar/ffmpeg/5.1.2/include -L/opt/homebrew/Cellar/ffmpeg/5.1.2/lib -lavcodec -lavformat -lavutil -lswscale `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -lpthread -framework CoreAudio -o"

## Log
### D1 - D4 : Sept 27 - 30: Getting Familiar with FFmpeg
#### Preparation
Explore websites, [FFmpeg](http://ffmpeg.org/doxygen/4.0/decode__video_8c_source.html),
                  [An ffmpeg and SDL Tutorial](http://dranger.com/ffmpeg/tutorial01.html), and
                  [<stdio.h> Library](https://pubs.opengroup.org/onlinepubs/009696799/functions/popen.html)
Read the related parts of the documentation, such as opening a video file, decoding a video file, and creating a buff to store the pixel info of frames.

### D5 : Oct 01: Coding and Debugging
#### Coding
Using popen in <stdio.h> library to open our sample video file, ffmpeg_input_video_1.mp4. Write ppm and pgm files by calling fread and fwrite. Convert ppm and pgm file using oneline source [converter] (https://convertio.co/ppm-jpg/) to check if the output file is the actual frame we pulled out. The corresponding tasks are completed in files, "test_2_simple_read_gray_scale.c", "test_3_img_ppm_out.c", and "test_4_img_pgm_out.c". 

#### Debugging
Facing a challenge: cannot include the library, <libavcodec/avcodec.h>, and compile.
Search for a solution, with sources [fatal error - stack overflow] (https://stackoverflow.com/questions/12127397/fatal-error-libavcodec-avcodec-h-no-such-file-or-directory-compilation-terminat).

### D6 : Oct 02: Coding and Debugging
#### Coding
Generate GTK windows to display the frame. To draw the gray scaled frame onto the GTK screen, we called methods'cairo_image_surface_create', 'cairo_surface_flush', 'cairo_image_surface_get_stride', 'cairo_surface_mark_dirty', 'cairo_set_source_surface', and 'cairo_paint' in our callback function 'on_draw_event_gray'. The corresponding files are "test_5_display_gray.c", and "test_6_display_both.c".

#### Debugging
Find some other solutions to debug the file in [FFmpeg](http://ffmpeg.org/doxygen/4.0/decode__video_8c_source.html). The corresponding task is reflected in the files of "ffmpeg-debug.c", and "Makefile".

### D7 : Oct 03: Problem Fixing and Finalizing
#### Problem
We wanted to display the content of the pgm file, which is the coloured frame before transforming into gray scale. However, 'fwrite' will write pixel values into a binary file that cannot be read. Therefore, we use 'fprintf' and placeholder '%X' to print out the hexadecimal values into a text file, "frame_pixel_ppm.txt". The finalized code for presentation on Tuesday is "test_7_display_pixelValOut.c".

### D8 : Oct 06: Decoding Problem Solved
#### Problem
Same problem in Debugging section in D5.

#### Solution
In the file of "test_8_decode_modify.c", we merge parts of code from [demuxing_decoding.c](http://www.ffmpeg.org/doxygen/trunk/demuxing_decoding_8c-example.html) into [decode_video.c](http://ffmpeg.org/doxygen/4.0/decode__video_8c_source.html).

#### Extension
Figure out which part of the combined code is redundant. Make the decode file more concise and efficient.

### D9 : Oct 25: Code Revisit
File name re-submitted: "lab_3_resubmit_3.c"
#### Revision - 1
Q: Why two pgm saving functions?
One is removed in this file, and the only one left is located at line-130.

#### Revision - 2
In stead of using "AV_CODEC_ID_MPEG1VIDEO" decoder, we provide "avcodec_find_decoder(video_dec_ctx->codec_id)" for multiple type of decoders at line-352.

#### Revision - 3
Use a method called sws_scale(...) in <libswscale/swscale.h> to convert the format from YUV420P to RGB24 at line-110. To do the conversion, we created two methods at line-81 and line-103, which are called allocPicture(...) and avFrameConvertPixelFormat(...) respectively.

#### Revision - 4
Display the converted AVFrame at line-464 onto a GTK window by using a frame structure field called data[0].

#### Revision - 5
Q: what is read from file?
The method that is called "read_AVFrame_info" was removed.

### D1 : Oct 16: Exploring timers
#### Sources
1. [gtk.org](https://docs.gtk.org/glib/struct.Timer.html#description) 
2. [GTK timer](https://stackoverflow.com/questions/8352027/gtk-timer-how-to-make-a-timer-within-a-frame)

### D2 : Oct 17: Coding and Problem Solving
#### Problem
- In the file of "test-4-refined-grid.c", the GTK window can only dislay fixed-size images.

#### Solution
- Take the max over the width dimensions and height dimensions and make it the dimension of the window.

### D3 : Oct 18: Problem Solving
#### Problem
- In the file of "test-5-load-refinded-grid.c", the animation is performed on partial image when the image dimension either height or width is not a multiple of the number of grid.

#### Solution
- Use double instead of int to represent image dimensions.

#### Extension
- Figure out how threading is used to perform a timer in GTK windows.

### D1 : Nov3: Coding
#### Merging Lab-3 and Lab-4
Merge decoder and GTK timer into one file.

### D1 : Nov15: Coding
#### Create Lab-6

#### Problem Solving
1. create two threads, gtk can only be run on the main threads
2. decrease buffer size from 256 to 1
3. solve memory leak by adding av_freep() at line-262 and wait and signal at lines 267 and 303

### D1 : Nov28: Coding
#### Create Lab-8

### D2 : Nov29: Problem Solving
#### Unsigned int to float
Instead of reading 4 bytes and bit shifting, we cast ->data to (float *)

#### Pop sound
unresolved: printf in read_audio_frame_data() cannot be removed

### D1 : Nov 30: Coding
#### Create Lab-9

### D2 : Dec 03: Problem Solving
#### Synchronization
Synchronize from the beginning of the video(1sec/fps * nread) instead of the beginning of each frame(1sec/fps)

