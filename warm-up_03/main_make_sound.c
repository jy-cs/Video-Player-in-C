// Compile
// gcc lab_7_make_sound_submit.c -framework CoreAudio -o e-test
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>
#include <CoreAudio/CoreAudio.h>

OSStatus my_fill_buffer(AudioObjectID inDevice, const AudioTimeStamp *inNow, const AudioBufferList *inInputData, const AudioTimeStamp *inInputTime, AudioBufferList *outOutputData, const AudioTimeStamp *inOutputTime, void *__nullable inClientData)
{
    AudioBuffer buffer = outOutputData->mBuffers[0];
    // pi is defined as M_PI
    int left = 1, t = 0, freq = 89, frame = 0, total = buffer.mDataByteSize / (sizeof(float));
    float amp = 0;
    while (frame < total)
    {
        amp = 0.125 + 0.125 * cos((float)(t / 2700));
        if (++freq > 255)
            freq = 89;
        float val = amp * sin((float)(M_PI * freq * (t % (12 * INT8_MAX)) / 96000));
        if (left)
            *(((float *)buffer.mData) + frame) = val;
        else
            for (int channel = 1; channel < buffer.mNumberChannels; channel++)
                *(((float *)buffer.mData) + (frame + channel)) = val;
        if (t == 2 * INT16_MAX)
        {
            t = 0;
            left = 1 - left;
        }
        else
            t++;
        frame += buffer.mNumberChannels;
    }
    return noErr;
}

void printCoreAudioErrorAndExit(OSStatus error)
{
    if (error != noErr)
    {
        printf("Error: %d", error);
        exit(-1);
    }
}

int main(int argc, char **argv)
{
    // Create a struct to query to correct parameter
    AudioObjectPropertyAddress property = (AudioObjectPropertyAddress){
        .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
        .mElement = kAudioObjectPropertyElementMain,
        .mScope = kAudioObjectPropertyScopeGlobal};
    // tell how much memory should be allocated for the result of AudioObjectGetPropertyData
    UInt32 propertySize;
    printCoreAudioErrorAndExit(AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &property, 0, NULL, &propertySize));
    // prompt to stop the execution
    printf("Please press \'return\' to stop:\n");
    // print out the index of the speaker
    printf("-> The index of the speaker is %lu", (unsigned long)kAudioObjectSystemObject);
    // Create a variable of the expected output type
    AudioObjectID defaultOutputDevice;
    // Try to call the function to receive the desired property
    printCoreAudioErrorAndExit(AudioObjectGetPropertyData(kAudioObjectSystemObject, &property, 0, NULL, &propertySize, &defaultOutputDevice));
    // let the audio device pull the audio data, by calling a function in regular
    //     intervals, in which you can provide a chunk of audio data.
    AudioDeviceIOProcID procID;
    printCoreAudioErrorAndExit(AudioDeviceCreateIOProcID(defaultOutputDevice, my_fill_buffer, NULL, &procID));
    // We need to start the device
    printCoreAudioErrorAndExit(AudioDeviceStart(defaultOutputDevice, procID));
    getchar();
    // stop the playback.
    printCoreAudioErrorAndExit(AudioDeviceStop(defaultOutputDevice, NULL));
    // After the playback has stopped we will clean up the resources and terminate the program.
    printCoreAudioErrorAndExit(AudioDeviceDestroyIOProcID(defaultOutputDevice, procID));
    return 0;
}
