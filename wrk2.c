
#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <lame/lame.h>

int buffer_frames = 1024;
int bitrate = 128;
int channels = 2;
unsigned int samplerate = 44100;
snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

main (int argc, char *argv[]) {
  lame_global_flags *gfp;
  snd_pcm_t *chandle;
  prepare_lame(&gfp);
  prepare_alsa(&chandle);

  loop( chandle, gfp);


  exit(0);
}

prepare_lame ( lame_global_flags **gfp) {
  *gfp = lame_init();
  lame_set_num_channels(*gfp,channels);
  lame_set_in_samplerate(*gfp,samplerate);
  lame_set_out_samplerate(*gfp,44100);
  lame_set_brate(*gfp,bitrate);
  lame_set_mode(*gfp,1);
  lame_set_quality(*gfp,2);   /* 2=high  5 = medium  7=low */
  lame_set_bWriteVbrTag(*gfp,0);
  lame_init_params(*gfp);

}

prepare_alsa ( snd_pcm_t **chandle) {
  int err;
  snd_pcm_hw_params_t *hw_params;

  if ((err = snd_pcm_open (chandle, "hw:1,0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
    fprintf (stderr, "cannot open audio device (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_any (*chandle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_access (*chandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_format (*chandle, hw_params, format)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_rate_near (*chandle, hw_params, &samplerate, 0)) < 0) {
    fprintf (stderr, "cannot set sample samplerate (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params_set_channels (*chandle, hw_params, 2)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  if ((err = snd_pcm_hw_params (*chandle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    exit (1);
  }

  snd_pcm_hw_params_free (hw_params);

  if ((err = snd_pcm_prepare (*chandle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    exit (1);
  }
}

loop( snd_pcm_t *chandle, lame_global_flags *gfp) {
  int read_status, write_status, i;
  void *buffer = malloc(
    buffer_frames * channels * snd_pcm_format_width(format) / 8);
  int mp3buffer_bytes = buffer_frames * 5 / 4 + 7200;
  char *mp3buffer = malloc( mp3buffer_bytes);

  for (i = 0; 1; ++i) {

    read_status = snd_pcm_readi (chandle, buffer, buffer_frames);
    if (read_status < 0)   fprintf (stderr,
      "read from audio interface failed (%s)\n", snd_strerror (read_status)), exit (1);

    write_status = lame_encode_buffer_interleaved(
      gfp, (short int *)buffer, read_status, mp3buffer, mp3buffer_bytes );
    if (write_status < 0)   fprintf (stderr,
      "lame encode failed (%d)\n", write_status), exit (1);

    fwrite( mp3buffer, 1, write_status, stdout);

  }

  free(buffer);
  free(mp3buffer);

  snd_pcm_close (chandle);

  exit (0);
}
