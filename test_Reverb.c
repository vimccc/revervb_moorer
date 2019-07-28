 /*test module*/

 #include<stdio.h>
 #include<stdlib.h>
 #include<string.h>
 #include "wave.h"
 #include "reverb.h"

 #define FILE_NAME "./test.wav"
 #define OUTPUT_FILE "./result.wav"
 #define _TEST_ 

 #define CHANNELS_DOUBLE 2
 #define CHANNELS_SINGLE 1
 #define BYTE_MASK 0xff


/*convert pcm_t type data into  sample_t type*/
static void  convert_pcm2sample(pcm_t *pcm, sample_t *input){
	for (int i=0; i < pcm->num; i++){
		int16_t L = (int16_t) pcm->left[i];
		int16_t R = (int16_t)pcm->right[i];
		if (pcm->flag_left){
			if (L < 0){
				input[i].L = (float)L / 32768.0f;
				input[i].R = (float)L / 32768.0f;
			}
			else{
				input[i].L = (float)L / 32767.0f;
				input[i].R = (float)L / 32768.0f;
				}
		}
		if (pcm->flag_right){
			if (R < 0)
				input[i].R = (float)R / 32768.0f;
			else
				input[i].R = (float)R / 32767.0f;
		}

	}
	
#ifdef _TEST_	
	printf("convert pcm to sample success\n");
#endif

}


 /*convert sample_t type data into pcm type*/
static void  convert_sample2pcm(pcm_t *pcm, sample_t *output){
	 for (int i=0; i < pcm->num; i++){
	 		float L = clampf(output[i].L, -1, 1);
			float R = clampf(output[i].R, -1, 1);
			if (L < 0)
				pcm->left[i] = (uint16_t)(L * 32768.0f);
			else
				pcm->left[i] = (uint16_t)(L * 32767.0f);
			if (R < 0)
				pcm->right[i] = (uint16_t)(R * 32768.0f);
			else
				pcm->right[i] = (uint16_t)(R * 32767.0f);
	 }

#ifdef _TEST_	
	 printf("convert sample to pcm success\n");
#endif

 }

 /*parse the type of the file*/
static void parse_wave_data(wave_t *wave, pcm_t *pcm){
	/*case double channel*/
	if (wave->format.num_channel == CHANNELS_DOUBLE){
		pcm->flag_left = 1;
		pcm->flag_right = 1;
		int byte_per_sample = (int) (wave->format.bits/8);
		uint32_t num_of_sample = (uint32_t)(wave->data.size/byte_per_sample/2);
		pcm->num = num_of_sample;
		pcm->left = (uint16_t*) malloc( sizeof(uint16_t)*num_of_sample);
		pcm->right = (uint16_t*) malloc( sizeof(uint16_t)*num_of_sample);

		for (int i = 0; i < num_of_sample; i++){
			for (int j = 0; j < byte_per_sample;j++){
				pcm->left[i] = (pcm->left[i]>>8)+ (wave->data.data[i*byte_per_sample*2 + j]<<8);
				pcm->right[i] =(pcm->right[i]>>8)+ (wave->data.data[i*byte_per_sample*2 + j \
								+ byte_per_sample]<<8);
			}
		}

	}
	/*case single channel*/
	else if (wave->format.num_channel == CHANNELS_SINGLE){
		pcm->flag_left = 1;
		pcm->flag_right = 0;
		int byte_per_sample = (int) (wave->format.bits/8);
		uint32_t num_of_sample = (uint32_t)(wave->data.size/byte_per_sample);
		pcm->num = num_of_sample;
		pcm->left = (uint16_t*) malloc( sizeof(uint16_t)*num_of_sample);
		pcm->right = (uint16_t*) malloc( sizeof(uint16_t)*num_of_sample);

		for (int i = 0; i < num_of_sample; i++){
			for (int j = 0; j < byte_per_sample;j++){
				pcm->left[i] = (pcm->left[i]>>8)+ (wave->data.data[i*byte_per_sample + j]<<8);
				pcm->right[i] = pcm->left[i];
			}
		}

	}
#ifdef _TEST_	
		 printf("parse wave data success\n");
#endif

 }

/*save the result in a .wav file*/
 static void save_wave_data(wave_t *wave, pcm_t *pcm){
	/*case double channel*/
	if (wave->format.num_channel == CHANNELS_DOUBLE){
		int num_of_sample = pcm->num;
		int byte_per_sample = (int) (wave->format.bits/8);
		for (int i = 0; i < num_of_sample; i++){
			for (int j = 0; j < byte_per_sample;j++){
				wave->data.data[i*byte_per_sample*2+j] = pcm->left[i]& BYTE_MASK;
				pcm->left[i] = pcm->left[i]>>8;
				wave->data.data[i*byte_per_sample*2+j+byte_per_sample] = pcm->right[i] & BYTE_MASK;
				pcm->right[i] = pcm->right[i]>>8;
			}
		}

	}
	/*case single channel*/
	else if (wave->format.num_channel == CHANNELS_SINGLE){
		int num_of_sample = pcm->num;
		int byte_per_sample = (int) (wave->format.bits/8);

		for (int i = 0; i < num_of_sample; i++){
			for (int j = 0; j < byte_per_sample;j++){
				wave->data.data[i*byte_per_sample+j] = pcm->left[i]& BYTE_MASK;
				pcm->left[i] = pcm->left[i]>>8;
			}
		}

	}
#ifdef _TEST_	
			 printf("save wave data success\n");
#endif

 }

 int main()
 {	
	FILE *fp;
	fp = fopen(FILE_NAME, "rb+");
	if (fp == NULL){
		printf("open file faied\n");
		exit(-1);
	}

	wave_t wave;	/*content of file*/
	pcm_t pcm;	/*valid data*/
	sample_t *input, *output;

	/*get all content of file*/
	fread(&wave, 36, 1, fp);
	fread(&wave.data, 8, 1, fp);
	wave.data.data = (uint8_t *)malloc(wave.data.size);
	fread(wave.data.data, wave.data.size, 1, fp);
	parse_wave_data(&wave, &pcm); /*parse the valid data of file*/ 

#ifdef _TEST_	
	printf("data size:%d\n", wave.data.size);
	printf("bit per sample:%d\n", wave.format.bits);
	printf("number of sample:%d\n", pcm.num);
	printf("number of channel:%d\n", wave.format.num_channel);
#endif
	/* apply for buffer for input and output*/
	input = (sample_t*) malloc(sizeof(sample_t) * pcm.num);
	memset(input, 0, sizeof(sample_t) * pcm.num);
	output = (sample_t *) malloc(sizeof(sample_t) * pcm.num);
	memset(output, 0, sizeof(sample_t) * pcm.num);
	convert_pcm2sample(&pcm, input);
	/*pocess the data*/
	reverb_state_t rv;
	memset(&rv, 0, sizeof(reverb_state_t));
	reverb_preset(&rv, wave.format.rate_sample, REVERB_PRESET_SMALLHALL1);
	reverb_process(&rv, pcm.num, input, output);


	/*save the result*/
	FILE *fp_out;
	convert_sample2pcm(&pcm, output);
	save_wave_data(&wave, &pcm);
	fp_out = fopen(OUTPUT_FILE, "wb+");
	fwrite(&wave, 36, 1, fp_out);
	fwrite(&wave.data, 8, 1, fp_out);
	fwrite(wave.data.data, wave.data.size, 1, fp_out);

#ifdef _TEST_	
	;
#endif
	/*	free the buffer */
	if (wave.data.data != NULL)
		free(wave.data.data);	
	if (pcm.left != NULL)
		free(pcm.left);
	if (pcm.right != NULL)
		free(pcm.right);	
	if (input != NULL)
		free(input);
	if (output != NULL)
		free(output);

	fclose(fp);
	fclose(fp_out);

	return 0;
 }