/*
 *    wave.c    --    source for wave application
 *
 *    Authored by Karl "p0lyh3dron" Kreuze on February 2, 2023
 * 
 *    This file defines the entire usage of the wave application.
 *    The entire source is here for the convenience of portability.
 * 
 *    Wave is meant to be a simple, lightweight, and minimalistic
 *    audio player for the Linux operating system.
 */
#include <stdio.h>

#include <alsa/asoundlib.h>

int main(int argc, char *argv[]) {
    unsigned int rate = 44100;
    unsigned int channels = 2;
    unsigned int buf_len = 1024;
    unsigned int playing = 1;
    unsigned int samples = buf_len / channels / 2;

    unsigned int ret;

    char *buffer;

    snd_pcm_t *device;
    snd_pcm_hw_params_t *params;

    /* Open a PCM device.  */
    if (ret = snd_pcm_open(&device, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        fprintf(stderr, "Error opening PCM device: %s", snd_strerror(ret));
        return 1;
    }

    /* Set parameters for playback.  */
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(device, params);

    /*
     *    Set interleaved mode:
     *
     *    Samples are stored as: LRLRLRLR...
     */
    if (ret = snd_pcm_hw_params_set_access(device, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        fprintf(stderr, "Error setting interleaved mode: %s", snd_strerror(ret));
        return 1;
    }

    /* Set 16-bit signed PCM format.  */
    if (ret = snd_pcm_hw_params_set_format(device, params, SND_PCM_FORMAT_S16_LE) < 0) {
        fprintf(stderr, "Error setting format: %s", snd_strerror(ret));
        return 1;
    }

    /* Set sample rate.  */
    if (ret = snd_pcm_hw_params_set_rate_near(device, params, &rate, 0) < 0) {
        fprintf(stderr, "Error setting sample rate: %s", snd_strerror(ret));
        return 1;
    }

    /* Set number of channels.  */
    if (ret = snd_pcm_hw_params_set_channels(device, params, channels) < 0) {
        fprintf(stderr, "Error setting channels: %s", snd_strerror(ret));
        return 1;
    }

    /* Apply parameters.  */
    if (ret = snd_pcm_hw_params(device, params) < 0) {
        fprintf(stderr, "Error applying parameters: %s", snd_strerror(ret));
        return 1;
    }

    /* Allocate buffer.  */
    buffer = malloc(buf_len);

    /* Start playback.  */
    do {
        /* Read from stdin.  */
        if (ret = read(0, buffer, buf_len) == 0) {
            /* End of file.  */
            playing = 0;
        }

        /* Write to PCM device.  */
        if (ret = snd_pcm_writei(device, buffer, samples) < 0) {
            /* Underrun.  */
            snd_pcm_prepare(device);
        } else if (ret < 0) {
            /* Error.  */
            fprintf(stderr, "Error writing to PCM device: %s", snd_strerror(ret));
            playing = 0;
        }
    } while (playing);

    /* Close PCM device.  */
    snd_pcm_close(device);

    return 0;
}

