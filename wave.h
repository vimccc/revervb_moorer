#ifndef _WAV_H_
#define _WAV_H_
#include <stdint.h>

#define clampi(v, min, max) \
	(v < min ? min : (v > max ? max : v))
#define clampf(v, min, max) \
 	( v < min ? min : (v > max ? max : v)) 

typedef struct {
	float L; /*left channel sample*/
	float R; /*right channel sample*/
}sample_t;

typedef struct {
	sample_t *samples;
	int size; /*number of sample*/
	int rate; /*sample per second*/
}snd_t, *snd_t_ptr;

/*pcm data*/
typedef struct pcm_data{
	uint32_t num;	/*number of sample*/
	int flag_left;	/*flag of left channel*/
	int flag_right;	/*flag of right channel*/
	uint16_t *left;	/*data of left channel*/
	uint16_t *right; /*data of right channel*/
}pcm_t;

/*structure of RIFF chunk*/
typedef struct WAVE_riff{
	unsigned char id[4];
	uint32_t size;
	unsigned char format[4];
}riff_t;

/*structure of FORMAT chunk*/
typedef struct WAVE_format{
	unsigned char id[4];
	uint32_t size;
	uint16_t audio_fmt;	/*Audio Format*/
	uint16_t num_channel; /*Number of Channels*/
	uint32_t rate_sample; /*Sample Rate*/
	uint32_t rate_byte;	/*Byte Rate*/
	uint16_t align;	/*Block Align*/
	uint16_t bits;	/*Bit per Sample*/
}fmt_t;

/*structure of DATA chunk*/
typedef struct WAVE_data{
	unsigned char id[4];
	uint32_t size;
	uint8_t *data;
}data_t;

/*structure of WAVE file*/
typedef struct WAVE{
	riff_t riff;	/*RIFF chunk*/
	fmt_t format;	/*FORMAT chunk*/	
	data_t data;	/*DATA chunk*/
}wave_t;

#endif