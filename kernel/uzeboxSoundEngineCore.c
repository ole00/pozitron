#include "defines.h"
#include "kernel.h"

#ifndef MIXER_WAVES
#error MIXER_VAVES are not defined
#endif

	unsigned char waves[] = {
		#include MIXER_WAVES 
	};
	
	uint16_t steptable[] = {
		#include "data/steptable.inc"
	};
	struct MixerStruct mixer; 

	uint16_t mix_buf[MIX_BUF_SIZE];
	volatile uint16_t *mix_pos;
	volatile unsigned char mix_bank = 0;
	unsigned char sound_enabled = 1;

	void InitSoundPort(void) {
		int i;
		for (i = 0; i < WAVE_CHANNELS; i++) {
			mixer.channels.type.wave[i].position = 0;
		}
	}	


	//mixes sound and music tracks into the mix_buf
	void MixTracks(uint16_t stage) {
		uint16_t* buf = mix_buf;
		uint16_t i;

		uint16_t start;
		uint16_t end;

		if (mix_bank) {
			buf += MIX_BANK_SIZE;
		}
		if (!sound_enabled) {
			return;
		}
#if ENABLE_MIXER==1
		start = stage * 88;
		end = start + 88;
		if (end > MIX_BANK_SIZE) {
			end = MIX_BANK_SIZE;
		}

		for (i = start; i < end;i++)
		{
			uint16_t result;
			uint16_t sample;
			struct MixerWaveChannelStruct* channel;

			channel = &mixer.channels.type.wave[2];
			channel->positionFrac += channel->step;
			sample = waves[channel->position + (channel->positionFrac >> 8)] + 128;
			result = (sample * channel->volume) >> 3;

			channel = &mixer.channels.type.wave[1];
			channel->positionFrac += channel->step;
			sample = waves[channel->position + (channel->positionFrac >> 8)] + 128;
			result += (sample * channel->volume) >> 3;

			channel = &mixer.channels.type.wave[0];
			channel->positionFrac += channel->step;
			sample = waves[channel->position + (channel->positionFrac >> 8)] + 128;
			result += (sample * channel->volume) >> 3;

#if SOUND_CHANNEL_4_ENABLE == 1
#if MIXER_CHAN4_TYPE == 0
			{
				struct MixerNoiseChannelStruct* noise;		
				noise = &mixer.channels.type.noise;

				if (noise->divider == 0) {
					uint8_t xorBit;
					uint8_t barrel = noise->barrel;

					//reset the divider
					noise->divider = noise->params >> 1;
					noise->barrel >>= 1;
					xorBit = noise->barrel & 1;
					noise->barrel ^= barrel;
					
					if (xorBit) {
						noise->barrel |= 0x2000;
					}  else {
						noise->barrel &= ~0x2000;
					}
					if (!(noise->params & 1)) { // bit0: 7/15 bits lfsr
						if (xorBit) {
							noise->barrel |= 0x20;
						}  else {
							noise->barrel &= ~0x20;
						}
					}
				} else {
					noise->divider--;
				}
				result += ((noise->barrel & 0xFF) * noise->volume) >> 3;
			}
#else
			channel = &mixer.channels.type.pcm;
			channel->positionFrac +=channel->step;
			//FIME - use the PCM position
			sample = waves[channel->position + (channel->positionFrac >> 8)] + 128;
			result += (sample * channel->volume) >> 3;
#endif
#endif
			buf[i] = result >> 4; // the audio DAC will use top 12bits: 4..15

		}

#endif

	}

	void SetMixerNote(unsigned char channel, unsigned char note) {
#if SOUND_MIXER == MIXER_TYPE_VSYNC
		if (channel < 3) 
#else
		if (channel != 3) 
#endif
		{
			mixer.channels.type.wave[channel].step = steptable[note]; 
		}
	}
	
	void SetMixerWave(unsigned char channel, unsigned char patchIndex) {
#if MIXER_CHAN4_TYPE == 0
		if (patchIndex == 0xfe) {
			mixer.channels.type.noise.params &= 0xfe;	
		} else
		if (patchIndex == 0xff) {
			mixer.channels.type.noise.params |= 0xfe;	
		} else
#endif
		{
			mixer.channels.type.wave[channel].position = 0; //((uint16_t)patchIndex) << 8;
		}
	}

	void EnableSoundEngine(void) {
		sound_enabled = 1;
	}

	void DisableSoundEngine(void) {
		sound_enabled = 0;
	}
