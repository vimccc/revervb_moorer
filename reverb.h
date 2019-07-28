// (c) Copyright 2016, Sean Connelly (@voidqk), http://syntheti.cc
// MIT License
// reverb algorithm is based on Progenitor2 reverb effect from freeverb3:
//   http://www.nongnu.org/freeverb3/

#ifndef _REVERB_H_
#define _REVERB_H_
#include "wave.h"


#define clampi(v, min, max) \
	(v < min ? min : (v > max ? max : v))

#define clampf(v, min, max) \
	(v < min ? min : (v > max ? max : v))

#define db2lin(db)	\
	(powf(10.0f, 0.05f * db))


/* component 1:	delay */
#define REVERB_DS        9814 /* maximum size allowed for a delay */
typedef struct {
	int pos; /* current write position */
	int size; /* delay size */
	float buf[REVERB_DS]; /* delay buffer*/
} rv_delay_t;


/* component 2:	1st order IIR filter */
typedef struct {
	float a2; 	/* coefficients */
	float b1;
	float b2;
	/* state */
	float y1; 
} rv_iir1_t;


/* component 3:	biquad filter*/
typedef struct {
	float b0; 	/* biquad coefficients */
	float b1;
	float b2;
	float a1;
	float a2;
	float xn1; /* input[n - 1] */
	float xn2; /* input[n - 2] */
	float yn1; /* output[n - 1] */
	float yn2; /* output[n - 2] */
} rv_biquad_t;


/*component 4:	early reflection */
typedef struct {
	int             delaytblL[18], delaytblR[18];
	rv_delay_t  delayPWL     , delayPWR     ;
	rv_delay_t  delayRL      , delayLR      ;
	rv_biquad_t allpassXL    , allpassXR    ;
	rv_biquad_t allpassL     , allpassR     ;
	rv_iir1_t   lpfL         , lpfR         ;
	rv_iir1_t   hpfL         , hpfR         ;
	float wet1, wet2;
} rv_earlyref_t;


/* component 5:	oversampling*/	
#define REVERB_OF        4	/* maximum oversampling factor */
typedef struct {
	int factor;	/* oversampling factor [1 to REVERB_OF] */
	rv_biquad_t lpfU; /* lowpass filter used for upsampling */
	rv_biquad_t lpfD; /* lowpass filter used for downsampling */
} rv_oversample_t;


/* component 6:	dc cut */
typedef struct {
	float gain;
	float y1;
	float y2;
} rv_dccut_t;

/* component 7:	fractal noise cache */
#define REVERB_NS        (1<<15) /* must be a power of 2 because it's generated via fractal generator */
typedef struct {
	int pos; /* current read position in the buffer */
	float buf[REVERB_NS]; /* buffer filled with noise */
} rv_noise_t;

/* component 8:	low-frequency oscilator (LFO) */
typedef struct {
	float re;  /* real part */
	float im;  /* imaginary part */
	float sn;  /* sin of angle increment per sample */
	float co;  /* cos of angle increment per sample */
	int count; /* number of samples generated so far (used to apply small corrections over time) */
} rv_lfo_t;


/* component 9:	all-pass filter */
#define REVERB_APS       6299	/* maximum size */
typedef struct {
	int pos;
	int size;
	float feedback;
	float decay;
	float buf[REVERB_APS];
} rv_allpass1_t;


/* component 10:	2nd order all-pass filter */
#define REVERB_AP2S1     11437 /* maximum sizes of the two buffers */
#define REVERB_AP2S2     3449
typedef struct {
	int   pos1                 , pos2                 ;
	int   size1                , size2                ;
	float feedback1            , feedback2            ;
	float decay1               , decay2               ;
	float buf1[REVERB_AP2S1], buf2[REVERB_AP2S2];
} rv_allpass2_t;


/* component 11:	3rd order all-pass filter with modulation */
#define REVERB_AP3S1     8171 /* maximum sizes of the three buffers and maximum mod size of the first line */
#define REVERB_AP3M1     683
#define REVERB_AP3S2     4597
#define REVERB_AP3S3     7541
typedef struct {
	int   rpos1, wpos1                           , pos2                 , pos3                 ;
	int   size1, msize1                          , size2                , size3                ;
	float feedback1                              , feedback2            , feedback3            ;
	float decay1                                 , decay2               , decay3               ;
	float buf1[REVERB_AP3S1 + REVERB_AP3M1], buf2[REVERB_AP3S2], buf3[REVERB_AP3S3];
} rv_allpass3_t;


/* component 12:	modulated all-pass filter */
#define REVERB_APMS      8681 /* maximum size and maximum mod size */
#define REVERB_APMM      137
typedef struct {
	int rpos, wpos;
	int size, msize;
	float feedback;
	float decay;
	float z1;
	float buf[REVERB_APMS + REVERB_APMM];
} rv_allpassm_t;

/* component 13:	comb filter  */
#define REVERB_CS        4229 /* maximum size of the buffer */
typedef struct {
	int pos;
	int size;
	float buf[REVERB_CS];
} rv_comb_t;


/* the whole reverb module */
typedef struct {
	rv_earlyref_t   earlyref;
	rv_oversample_t oversampleL, oversampleR;
	rv_dccut_t      dccutL     , dccutR     ;
	rv_noise_t      noise;
	rv_lfo_t        lfo1;
	rv_iir1_t       lfo1_lpf;
	rv_allpassm_t   diffL[10]  , diffR[10]  ;
	rv_allpass1_t    crossL[4]  , crossR[4]  ;
	rv_iir1_t       clpfL      , clpfR      ; // cross LPF
	rv_delay_t      cdelayL    , cdelayR    ; // cross delay
	rv_biquad_t     bassapL    , bassapR    ; // bass all-pass
	rv_biquad_t     basslpL    , basslpR    ; // bass lowpass
	rv_iir1_t       damplpL    , damplpR    ; // dampening lowpass
	rv_allpassm_t   dampap1L   , dampap1R   ; // dampening all-pass (1)
	rv_delay_t      dampdL     , dampdR     ; // dampening delay
	rv_allpassm_t   dampap2L   , dampap2R   ; // dampening all-pass (2)
	rv_delay_t      cbassd1L   , cbassd1R   ; // cross-fade bass delay (1)
	rv_allpass2_t   cbassap1L  , cbassap1R  ; // cross-fade bass allpass (1)
	rv_delay_t      cbassd2L   , cbassd2R   ; // cross-fade bass delay (2)
	rv_allpass3_t   cbassap2L  , cbassap2R  ; // cross-fade bass allpass (2)
	rv_lfo_t        lfo2;
	rv_iir1_t       lfo2_lpf;
	rv_comb_t       combL      , combR      ;
	rv_biquad_t     lastlpfL   , lastlpfR   ;
	rv_delay_t      lastdelayL , lastdelayR ;
	rv_delay_t      inpdelayL  , inpdelayR  ;
	int outco[32];
	float loopdecay;
	float wet1, wet2;
	float wander;
	float bassb;
	float ertolate; /* early reflection mix parameters */
	float erefwet;
	float dry;
} reverb_state_t;


/* all types of reverb effect*/
typedef enum {
	REVERB_PRESET_DEFAULT,
	REVERB_PRESET_SMALLHALL1,
	REVERB_PRESET_SMALLHALL2,
	REVERB_PRESET_MEDIUMHALL1,
	REVERB_PRESET_MEDIUMHALL2,
	REVERB_PRESET_LARGEHALL1,
	REVERB_PRESET_LARGEHALL2,
	REVERB_PRESET_SMALLROOM1,
	REVERB_PRESET_SMALLROOM2,
	REVERB_PRESET_MEDIUMROOM1,
	REVERB_PRESET_MEDIUMROOM2,
	REVERB_PRESET_LARGEROOM1,
	REVERB_PRESET_LARGEROOM2,
	REVERB_PRESET_MEDIUMER1,
	REVERB_PRESET_MEDIUMER2,
	REVERB_PRESET_PLATEHIGH,
	REVERB_PRESET_PLATELOW,
	REVERB_PRESET_LONGREVERB1,
	REVERB_PRESET_LONGREVERB2
} reverb_preset_t;


/* preset all components of the reverb module according to the input */
void reverb_preset(reverb_state_t *state, int rate, reverb_preset_t preset);


/* init the component of the reverb module, called by reverb_preset() */
void reverb_init(reverb_state_t *rv,
	int rate,             /* input sample rate (samples per second) */
	int oversamplefactor, /* how much to oversample [1 to 4] */
	float ertolate,       /* early reflection amount [0 to 1] */
	float erefwet,        /* dB, final wet mix [-70 to 10] */
	float dry,            /* dB, final dry mix [-70 to 10] */
	float ereffactor,     /* early reflection factor [0.5 to 2.5] */
	float erefwidth,      /* early reflection width [-1 to 1] */
	float width,          /* width of reverb L/R mix [0 to 1] */
	float wet,            /* dB, reverb wetness [-70 to 10] */
	float wander,         /* LFO wander amount [0.1 to 0.6] */
	float bassb,          /* bass boost [0 to 0.5] */
	float spin,           /* LFO spin amount [0 to 10] */
	float inputlpf,       /* Hz, lowpass cutoff for input [200 to 18000] */
	float basslpf,        /* Hz, lowpass cutoff for bass [50 to 1050] */
	float damplpf,        /* Hz, lowpass cutoff for dampening [200 to 18000] */
	float outputlpf,      /* Hz, lowpass cutoff for output [200 to 18000] */
	float rt60,           /* reverb time decay [0.1 to 30] */
	float delay           /* seconds, amount of delay [-0.5 to 0.5] */
);


/* this function will process the input sound based on the state passed */
void reverb_process(reverb_state_t *state, int size, sample_t *input,
	sample_t *output);

#endif
