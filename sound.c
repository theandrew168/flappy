#include <stdio.h>
#include <stdlib.h>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include "sounds/flap.h"

// Compile via:
// gcc -std=c99 -pthread -Ivendor/include/ -Ires/ sound.c -ldl -lm

void
flap_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount);

    (void)pInput;
}

int
main(int argc, char* argv[])
{
    ma_decoder flap_decoder;
    ma_result result = ma_decoder_init_memory_wav(SOUND_FLAP_DATA, sizeof(SOUND_FLAP_DATA), NULL, &flap_decoder);
    if (result != MA_SUCCESS) {
        return -2;
    }

    ma_decoder_seek_to_pcm_frame(&flap_decoder, 100000);

    ma_device_config flap_config = ma_device_config_init(ma_device_type_playback);
    flap_config.playback.format   = flap_decoder.outputFormat;
    flap_config.playback.channels = flap_decoder.outputChannels;
    flap_config.sampleRate        = flap_decoder.outputSampleRate;
    flap_config.dataCallback      = flap_callback;
    flap_config.pUserData         = &flap_decoder;

    ma_device flap_device;
    if (ma_device_init(NULL, &flap_config, &flap_device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&flap_decoder);
        return -3;
    }

    if (ma_device_start(&flap_device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&flap_device);
        ma_decoder_uninit(&flap_decoder);
        return -4;
    }

    printf("Press Enter to quit...");
    getchar();

    ma_decoder_seek_to_pcm_frame(&flap_decoder, 0);

    printf("Press Enter to quit...");
    getchar();

    ma_device_uninit(&flap_device);
    ma_decoder_uninit(&flap_decoder);

    return 0;
}
