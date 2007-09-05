/* -*- Mode: c; c-basic-offset: 4; tab-width:4; indent-tabs-mode:t -*-
 * vim: set noet ts=4 sw=4: */
/*
 * libmp4 - a library for reading and writing mp4 files
 * Copyright (C) 2007-2008 Limin Wang(lance.lmwang@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "lib_mp4.h"
#include "string.h"
#include "stdlib.h"
#include "getopt.h"
#ifdef DMALLOC
#include <dmalloc.h>
#endif

static int dump_audio_track_info(mp4_handle_t * p_mp4, uint32_t track_id)
{
	uint32_t media_sub_type;
	uint32_t sample_rate, channels;
	uint8_t bits_per_sample;

	media_sub_type = mp4_get_media_sub_type(p_mp4, track_id);
	if (!media_sub_type)
		return -1;

	mp4_get_audio_info(p_mp4, track_id, &sample_rate, &channels,
					   &bits_per_sample, NULL);

	printf("audio info: sample_rate: %d, channels: %d, bits_per_sample: %d\n",
		   sample_rate, channels, bits_per_sample);

	return 0;
}

static int dump_video_track_info(mp4_handle_t * p_mp4, uint32_t track_id)
{
	uint32_t media_sub_type;
	uint32_t width, height;
	uint32_t nr_of_samples;
	uint32_t max_sample_size;

	nr_of_samples = mp4_get_nr_of_samples(p_mp4, track_id);
	max_sample_size = mp4_get_max_sample_size(p_mp4, track_id);
	media_sub_type = mp4_get_media_sub_type(p_mp4, track_id);

	mp4_get_visual_info(p_mp4, track_id, &width, &height);
	printf("video info: %dx%d\n", width, height);
	printf("max sample size: %d \n", max_sample_size);

	return 0;
}

static int show_h264_rap(mp4_handle_t * p_mp4, uint32_t track_id)
{
	uint32_t nr_of_samples, i;
	uint32_t max_sample_size;
	char *buf;
	uint32_t size;
	uint64_t DTS, CTS, duration;
	uint8_t is_sync;
	uint32_t timescale;

	nr_of_samples = mp4_get_nr_of_samples(p_mp4, track_id);
	max_sample_size = mp4_get_max_sample_size(p_mp4, track_id);
	timescale = mp4_get_media_timescale(p_mp4, track_id);

	buf = (char *) malloc(max_sample_size);
	if (!buf)
		return -1;

	printf("Track_%d h264 video RAP,\n", track_id);
	/* play every sample */
	for (i = 0; i < nr_of_samples; i++) {
		size = max_sample_size;
		mp4_read_sample(p_mp4, track_id, i, buf, &size, &DTS,
						&CTS, &duration, &is_sync);
		if (is_sync) {
			printf("RAP: sample_id: %d, DTS: %llu, CTS: %llu \n",
				   i, DTS * 1000000ULL / timescale,
				   CTS * 1000000ULL / timescale);
		}
	}

	free(buf);

	return 0;
}

static int show_mpeg_audio_rap(mp4_handle_t * p_mp4, uint32_t track_id)
{
	uint32_t nr_of_samples, i;
	uint32_t max_sample_size;
	char *buf;
	uint32_t size;
	uint64_t DTS, CTS, duration;
	uint8_t is_sync;
	uint32_t timescale;

	nr_of_samples = mp4_get_nr_of_samples(p_mp4, track_id);
	max_sample_size = mp4_get_max_sample_size(p_mp4, track_id);
	timescale = mp4_get_media_timescale(p_mp4, track_id);

	buf = (char *) malloc(max_sample_size);
	if (!buf)
		return -1;

	printf("Track_%d mpeg audio RAP, \n", track_id);
	/* play every sample */
	for (i = 0; i < nr_of_samples; i++) {

		size = max_sample_size;
		mp4_read_sample(p_mp4, track_id, i, buf, &size, &DTS,
						&CTS, &duration, &is_sync);
		if (is_sync) {
			printf("RAP: sample_id: %d, DTS: %llu, CTS: %llu \n",
				   i, DTS * 1000000ULL / timescale,
				   CTS * 1000000ULL / timescale);
		}

	}

	free(buf);

	return 0;
}

/*#define RANDOM_SEEK*/

static int extract_h264_track(mp4_handle_t * p_mp4, uint32_t track_id)
{
	uint32_t nr_of_samples, i;
	uint32_t max_sample_size;
	char *buf;
	uint32_t size;
	uint64_t DTS, CTS, duration;
	uint8_t is_sync;
	FILE *temp;
	uint8_t nr_of_seq_hdr, nr_of_pic_hdr;
	char **seq_hdr, **pic_hdr;
	uint16_t *pic_hdr_size, *seq_hdr_size;
	uint8_t header[4] = { 0, 0, 0, 1 };
	uint8_t nalu_size;
	char dumpname[128];
	uint32_t remain;
	uint32_t nal_size;
	uint32_t timescale;
	int j;
#ifdef RANDOM_SEEK
	uint64_t seek_time;
	uint64_t when;
#endif

	nr_of_samples = mp4_get_nr_of_samples(p_mp4, track_id);
	max_sample_size = mp4_get_max_sample_size(p_mp4, track_id);
	mp4_get_track_h264_seq_pic_headers(p_mp4, track_id,
									   &nr_of_seq_hdr, &seq_hdr,
									   &seq_hdr_size, &nr_of_pic_hdr,
									   &pic_hdr, &pic_hdr_size);
	mp4_get_track_h264_dec_config_info(p_mp4, track_id, &nalu_size, NULL,
									   NULL, NULL, NULL);
	timescale = mp4_get_media_timescale(p_mp4, track_id);

	sprintf(dumpname, "track_%d.h264", track_id);

	temp = fopen(dumpname, "wb");
	if (!temp)
		return -1;

	for (i = 0; i < nr_of_seq_hdr; i++) {
		fwrite(header, 4, 1, temp);
		fwrite(seq_hdr[i], seq_hdr_size[i], 1, temp);
	}

	for (i = 0; i < nr_of_pic_hdr; i++) {
		fwrite(header, 4, 1, temp);
		fwrite(pic_hdr[i], pic_hdr_size[i], 1, temp);
	}
	buf = (char *) malloc(max_sample_size);
	if (!buf)
		return -1;

#ifdef RANDOM_SEEK
	seek_time = 9600000;
	when = seek_time * timescale / 1000000ULL;
	printf("timescale: %d, when: %lld \n", timescale, when);
	i = mp4_get_sample_from_time(p_mp4, track_id,
								 when, MP4_SEARCH_SYNC_BACKWARD);
	printf("sync: %d \n", i);
#else
	i = 0;

#endif
	printf("Start save h264 raw data...\n");
	/* play every sample */
	for (; i < nr_of_samples; i++) {
		size = max_sample_size;
		mp4_read_sample(p_mp4, track_id, i, buf, &size, &DTS,
						&CTS, &duration, &is_sync);
		/*printf("read sample_id: %d, sample_size: %d \n", i, size); */

		char *ptr = buf;
		remain = size;

		while (remain) {
			nal_size = 0;
			for (j = 0; j < nalu_size; j++) {
				nal_size |= ((uint8_t) * ptr);
				if (j + 1 < nalu_size)
					nal_size <<= 8;
				remain--;
				ptr++;
			}

			fwrite(header, 4, 1, temp);
			fwrite(ptr, nal_size, 1, temp);
			ptr += nal_size;
			remain -= nal_size;

			/*  printf("nalu_size: %d, nal_size: %d, remain: %d \n",
			   nalu_size, nal_size, remain); */

		}

		/*
		   printf("sample[%d] size: %d, DTS: %llu, CTS: %llu,"
		   "duration: %llu, is_sync: %d \n",
		   i, size, DTS, CTS, duration, is_sync); */
	}
	printf("Finish save h264 raw data\n");

	fclose(temp);
	free(buf);

	return 0;
}

/* AAC dec specific info */

/*
 * ADTS Header:
 *  MPEG-2 version 56 bits (byte aligned)
 *  MPEG-4 version 56 bits (byte aligned) - note - changed for 0.99 version
 *
 * syncword                     12 bits
 * id                           1 bit
 * layer                        2 bits
 * protection_absent            1 bit
 * profile                      2 bits
 * sampling_frequency_index     4 bits
 * private                      1 bit
 * channel_configuraton         3 bits
 * original                     1 bit
 * home                         1 bit
 * copyright_id                 1 bit
 * copyright_id_start           1 bit
 * aac_frame_length             13 bits
 * adts_buffer_fullness         11 bits
 * num_raw_data_blocks          2 bits
 *
 * if (protection_absent == 0)
 *  crc_check                   16 bits
 */

/*
 * AAC Config in ES:
 *
 * AudioObjectType          5 bits
 * samplingFrequencyIndex   4 bits
 * if (samplingFrequencyIndex == 0xF)
 *  samplingFrequency   24 bits
 * channelConfiguration     4 bits
 * GA_SpecificConfig
 *  FrameLengthFlag         1 bit 1024 or 960
 *  DependsOnCoreCoder      1 bit (always 0)
 *  ExtensionFlag           1 bit (always 0)
 */

#define NUM_ADTS_SAMPLING_RATES  16

uint32_t adts_sampling_rates[NUM_ADTS_SAMPLING_RATES] = {
	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
	16000, 12000, 11025, 8000, 7350, 0, 0, 0
};

static uint32_t aac_get_samplerate(uint8_t * p_config)
{
	uint8_t index;

	index = ((p_config[0] << 1) | (p_config[1] >> 7)) & 0xF;

	if (index == 0xF) {
		return (p_config[1] & 0x7F) << 17
			| p_config[2] << 9 | p_config[3] << 1 | (p_config[4] >> 7);
	}
	return adts_sampling_rates[index];

}

static uint8_t aac_get_mpeg4_type(uint8_t * p_config)
{
	uint8_t mpeg4_type;

	mpeg4_type = ((p_config[0] >> 3) & 0x1f);
	if (mpeg4_type == 0x1f) {
		mpeg4_type = 32 + (((p_config[0] & 0x7) << 3) |
						   ((p_config[1] >> 5) & 0x7));
	}

	return mpeg4_type;
}

static uint32_t aac_get_channels(uint8_t * p_config)
{
	uint8_t index;
	uint8_t adjust = 0;
	uint32_t nr_ch;

	index = ((p_config[0] << 1) | (p_config[1] >> 7)) & 0xF;

	if (index == 0xF) {
		adjust = 3;
	}

	nr_ch = (p_config[1 + adjust] >> 3) & 0xF;

	return nr_ch;
}

static uint8_t aac_find_sampling_rate_index(uint32_t samplerate)
{
	uint8_t i;

	for (i = 0; i < NUM_ADTS_SAMPLING_RATES; i++) {
		if (samplerate == adts_sampling_rates[i]) {
			return i;
		}
	}

	return NUM_ADTS_SAMPLING_RATES - 1;
}

/* make AAC adts header, adts is free by outside */
static int make_adts_header(char **adts, uint8_t is_aac, uint8_t profile,
							uint32_t samplerate,
							uint32_t channels, uint32_t size)
{
	char *data;
	uint32_t data_size;
	uint8_t sr_index;
	uint32_t framesize;

	data_size = 7;

	framesize = data_size + size;

	data = malloc(data_size * sizeof(char));
	if (!data) {
		printf("malloc failed \n");
		return -1;
	}
	memset(data, 0, data_size * sizeof(char));
	*adts = data;

	data[0] += 0xFF;			/* 8b: syncword */
	data[1] += 0xF0;			/* 4b: syncword */

	/* 1b: mpeg id = 0 */
	data[1] += ((is_aac == 1) ? 1 : 0) << 3;

	/* 2b: layer = 0 */
	data[1] += 1;				/* 1b: protection absent */

	data[2] += ((profile << 6) & 0xC0);	/* 2b: profile */

	/* the samplerate index */
	sr_index = aac_find_sampling_rate_index(samplerate);

	data[2] += ((sr_index << 2) & 0x3C);	/* 4b: sampling_frequency_index */
	/* 1b: private = 0 */
	data[2] += ((channels >> 2) & 0x1);	/* 1b: channel_configuration */

	data[3] += ((channels << 6) & 0xC0);	/* 2b: channel_configuration */
	/* 1b: original */
	/* 1b: home */
	/* 1b: copyright_id */
	/* 1b: copyright_id_start */
	data[3] += ((framesize >> 11) & 0x3);	/* 2b: aac_frame_length */

	data[4] += ((framesize >> 3) & 0xFF);	/* 8b: aac_frame_length */

	data[5] += ((framesize << 5) & 0xE0);	/* 3b: aac_frame_length */
	data[5] += ((0x7FF >> 6) & 0x1F);	/* 5b: adts_buffer_fullness */

	data[6] += ((0x7FF << 2) & 0x3F);	/* 6b: adts_buffer_fullness */
	/* 2b: num_raw_data_blocks */

	return data_size;
}

static int extract_mpeg_audio_track(mp4_handle_t * p_mp4, uint32_t track_id)
{
	uint32_t nr_of_samples, i;
	uint32_t max_sample_size;
	char *buf;
	uint32_t size;
	uint64_t DTS, CTS, duration;
	uint8_t is_sync;
	FILE *temp;
	char dumpname[128];
	uint8_t object_type_indication;
	uint8_t stream_type;
	uint8_t up_stream;
	uint32_t buf_size_db;
	uint32_t max_bitrate;
	uint32_t avg_bitrate;
	int is_aac;
	uint8_t *dsi_data = NULL;
	uint32_t dsi_size;
	uint8_t profile;
	uint8_t mpeg4_type;
	uint8_t sound_version;
	uint32_t samplerate;
	uint32_t channels;

	nr_of_samples = mp4_get_nr_of_samples(p_mp4, track_id);
	max_sample_size = mp4_get_max_sample_size(p_mp4, track_id);

	sprintf(dumpname, "track_%d", track_id);
	mp4_get_esds_dec_config_info(p_mp4, track_id, &object_type_indication,
								 &stream_type, &up_stream, &buf_size_db,
								 &max_bitrate, &avg_bitrate);
	switch (object_type_indication) {
	case 0x66:
	case 0x67:
	case 0x68:
		strcat(dumpname, ".aac");
		is_aac = 1;
		mp4_get_esds_dec_specific_info(p_mp4, track_id, &dsi_data, &dsi_size);
		if (!dsi_data)
			printf("no dec specific info \n");
		profile = object_type_indication - 0x66;
		break;
	case 0x40:
		strcat(dumpname, ".aac");
		is_aac = 2;
		mp4_get_esds_dec_specific_info(p_mp4, track_id, &dsi_data, &dsi_size);
		if (!dsi_data)
			printf("no dec specific info \n");
		/* The mpeg4 audio type (AAC, CELP, HXVC, ...)
		   is the first 5 bits of the ES configuration */
		mpeg4_type = aac_get_mpeg4_type(dsi_data);
		profile = mpeg4_type - 1;

		break;
	default:
		printf("unsupport type: 0x%x \n", object_type_indication);
		return -1;
	}

	if (!dsi_data || dsi_size < 2) {
		mp4_get_audio_info(p_mp4, track_id, NULL, NULL, NULL, &sound_version);
		if (sound_version == 1) {
			mp4_get_audio_info(p_mp4, track_id, &samplerate, &channels, NULL,
							   NULL);
		} else {
			printf("invalid mp4 audio \n");
			return -1;
		}
	} else {
		samplerate = aac_get_samplerate(dsi_data);
		channels = aac_get_channels(dsi_data);
	}

	temp = fopen(dumpname, "wb");
	if (!temp)
		return -1;

	/* write dsi */
	if (dsi_data)
		fwrite(dsi_data, dsi_size, 1, temp);

	buf = (char *) malloc(max_sample_size);
	if (!buf)
		return -1;

	printf("Start save raw mpeg audio data... \n");
	/* play every sample */
	for (i = 0; i < nr_of_samples; i++) {
		char *adts_hdr = NULL;
		uint32_t adts_hdr_size;

		size = max_sample_size;
		mp4_read_sample(p_mp4, track_id, i, buf, &size, &DTS,
						&CTS, &duration, &is_sync);

		adts_hdr_size = make_adts_header(&adts_hdr, is_aac, profile,
										 samplerate, channels, size);
		/* write adts header */
		fwrite(adts_hdr, adts_hdr_size, 1, temp);

		/* free the hdr */
		free(adts_hdr);

		/* write the sample */
		fwrite(buf, size, 1, temp);

		/*
		   printf("sample[%d] size: %d, DTS: %llu, CTS: %llu,"
		   "duration: %llu, is_sync: %d \n",
		   i, size, DTS, CTS, duration, is_sync); */
	}
	printf("Finish save raw mpeg audio data\n");

	fclose(temp);
	free(buf);

	return 0;
}

static int dump_hint_track_info(mp4_handle_t * p_mp4, uint32_t track_id)
{
	printf("hint info: \n");
	return 0;
}

static int dump_track_info(mp4_handle_t * p_mp4, uint32_t index)
{
	uint32_t track_id;
	uint8_t is_track_enable;
	uint64_t track_duration;
	uint64_t media_duration;
	uint32_t timescale;
	uint32_t media_type;
	uint32_t media_sub_type;
	char media_type_str[5];
	char media_sub_type_str[5];
	uint32_t nr_of_samples;

	/* get the track id */
	track_id = mp4_get_track_id(p_mp4, index);
	is_track_enable = mp4_is_track_enabled(p_mp4, track_id);
	track_duration = mp4_get_track_duration(p_mp4, track_id);
	timescale = mp4_get_media_timescale(p_mp4, track_id);
	media_duration = mp4_get_media_duration(p_mp4, track_id);

	printf("Track # %d Info: - ", index);
	printf("Track_id: %d, is_track_enable: %d, duration: %llu \n",
		   track_id, is_track_enable, track_duration);
	printf("\tMedia info: timescale: %d, media_duration: %llu, ",
		   timescale, media_duration*1000000ULL/timescale);
	media_type = mp4_get_media_type(p_mp4, track_id);
	mp4_fourcc_to_str(media_type, media_type_str, 5);

	media_sub_type = mp4_get_media_sub_type(p_mp4, track_id);
	mp4_fourcc_to_str(media_sub_type, media_sub_type_str, 5);

	nr_of_samples = mp4_get_nr_of_samples(p_mp4, track_id);
	printf("type: %s, sub type: %s, %d samples \n",
		   media_type_str, media_sub_type_str, nr_of_samples);

	switch (media_type) {
	case MP4_FOURCC('s', 'o', 'u', 'n'):
		dump_audio_track_info(p_mp4, track_id);
		break;
	case MP4_FOURCC('v', 'i', 'd', 'e'):
		dump_video_track_info(p_mp4, track_id);
		break;
	case MP4_FOURCC('h', 'i', 'n', 't'):
		dump_hint_track_info(p_mp4, track_id);
		break;
	default:
		printf("unsupport media type: %d \n", media_type);
		break;
	}

	return 0;
}

static int extract_track_info(mp4_handle_t * p_mp4, uint32_t index)
{
	uint32_t track_id;
	uint32_t media_type;
	uint32_t media_sub_type;

	/* get the track id */
	track_id = mp4_get_track_id(p_mp4, index);
	media_type = mp4_get_media_type(p_mp4, track_id);
	media_sub_type = mp4_get_media_sub_type(p_mp4, track_id);

	switch (media_type) {
	case MP4_FOURCC('s', 'o', 'u', 'n'):
		if (media_sub_type == MP4_FOURCC('m', 'p', '4', 'a'))
			extract_mpeg_audio_track(p_mp4, track_id);
		break;
	case MP4_FOURCC('v', 'i', 'd', 'e'):
		if (media_sub_type == MP4_FOURCC('a', 'v', 'c', '1'))
			extract_h264_track(p_mp4, track_id);
		break;
	case MP4_FOURCC('h', 'i', 'n', 't'):
		printf("extract hint track haven't supported \n");
		break;
	default:
		printf("unsupportted media type: %d \n", media_type);
		break;
	}

	return 0;
}

/* show track random access point(RAP) */
static int show_track_rap(mp4_handle_t * p_mp4, uint32_t index)
{
	uint32_t track_id;
	uint32_t media_type;
	uint32_t media_sub_type;

	/* get the track id */
	track_id = mp4_get_track_id(p_mp4, index);
	media_type = mp4_get_media_type(p_mp4, track_id);
	media_sub_type = mp4_get_media_sub_type(p_mp4, track_id);

	switch (media_type) {
	case MP4_FOURCC('s', 'o', 'u', 'n'):
		if (media_sub_type == MP4_FOURCC('m', 'p', '4', 'a'))
			show_mpeg_audio_rap(p_mp4, track_id);
		break;
	case MP4_FOURCC('v', 'i', 'd', 'e'):
		if (media_sub_type == MP4_FOURCC('a', 'v', 'c', '1'))
			show_h264_rap(p_mp4, track_id);
		break;
	case MP4_FOURCC('h', 'i', 'n', 't'):
		printf("Hint track haven't any RAP\n");
		break;
	default:
		printf("Unsupported media type: %d \n", media_type);
		break;
	}

	return 0;
}

void usage()
{
	printf("usage: mp4_tool <options> mp4_file\n");
	printf("Options: \n");
	printf("-h, -?, --help            List the more commonly used options\n");
	printf("-a, --add <string>        Add a track to the mp4 file,(Todo) \n");
	printf("-e, --extract <integer>   Extract a track from the mp4 file \n");
	printf("-i, --info <integer>      List tracks info from the mp4 file \n");
	printf("-r, --show_rap <integer>  Show one track random access point \n");
	printf("-v, --verbosity <integer>  Control amount of the log \n");

	return;
}

static struct option long_options[] = {
	{"add", required_argument, NULL, 'a'},
	{"extract", required_argument, NULL, 'e'},
	{"help", no_argument, NULL, 'h'},
	{"info", required_argument, NULL, 'i'},
	{"show_rap", required_argument, NULL, 'r'},
	{"verbosity", required_argument, NULL, 'v'},

	/* end */
	{0, 0, 0, 0}
};

/* test code for libmp4 library */
int main(int argc, char *argv[])
{
	mp4_handle_t *p_mp4;
	uint32_t verbosity = MP4_DETAILS_NONE;
	uint32_t mp4_timescale;
	uint64_t mp4_duration;
	uint64_t mp4_creation_time;
	uint64_t mp4_modification_time;
	uint32_t nr_tracks;
	int i = 0;
	int c;
	int show_track_info = 0;
	int show_help = 0;
	int show_rap = 0;
	int do_extract_track = 0;
	uint32_t extract_track_id = 0xFFFFFFFF;
	uint32_t rap_track_id = 0xFFFFFFFF;
	int nr_add_track = 0;
	int option_index = 0;
	char *add_track_file_name[256];
	char *mp4_file_name;

	while ((c = getopt_long(argc, argv,
							"a:e:h?ir:v:",
							long_options, &option_index)) != -1) {
		switch (c) {
		case 'a':
			add_track_file_name[nr_add_track++] = optarg;
			break;

		case 'h':
		case '?':
			show_help = 1;
			break;

		case 'i':
			show_track_info = 1;
			break;

		case 'e':
			do_extract_track = 1;
			extract_track_id = atoi(optarg);
			break;

		case 'v':
			verbosity |= MP4_DETAILS_ERROR;
			if (optarg) {
				uint32_t level;
				if (sscanf(optarg, "%u", &level) == 1) {
					if (level >= 2) {
						verbosity |= MP4_DETAILS_WARNING;
					}
					if (level >= 3) {
						verbosity |= MP4_DETAILS_READ;
					}
					if (level >= 4) {
						verbosity |= MP4_DETAILS_WRITE;
					}
					if (level >= 5) {
						verbosity = MP4_DETAILS_FIND;
					}
					if (level >= 6) {
						verbosity = MP4_DETAILS_TABLE;
					}
					if (level >= 7) {
						verbosity = MP4_DETAILS_SAMPLE;
					}
					if (level >= 8) {
						verbosity = MP4_DETAILS_MEMLEAK;
					}
					if (level >= 9) {
						verbosity = MP4_DETAILS_ALL;
					}
				}
			}
			break;

		case 'r':
			show_rap = 1;
			rap_track_id = atoi(optarg);

			break;
		default:
			show_help = 1;
			break;
		}

	}

	/* show usage and exit */
	if (show_help) {
		usage();
		return -1;
	}

	if (optind > argc - 1) {
		printf("no input mp4 file, Run %s -h for a list of options.\n",
			   argv[0]);
		return -1;
	}

	mp4_file_name = argv[optind++];

	/* set verbosity */
	mp4_set_verbosity(verbosity);

	/* show tracks info */
	if (show_track_info == 1) {
		if (!mp4_probe(mp4_file_name)) {
			printf("unsupported file \n");
			return -1;
		}

		p_mp4 = mp4_open(mp4_file_name, MP4_OPEN_READ);
		if (p_mp4 == NULL) {
			printf("invalid mp4 file \n");
			return -1;
		}

		mp4_timescale = mp4_get_timescale(p_mp4);
		mp4_duration = mp4_get_duration(p_mp4);
		mp4_get_creation_time(p_mp4, &mp4_creation_time,
							  &mp4_modification_time);
		nr_tracks = mp4_get_nr_of_tracks(p_mp4);

		printf("\n");
		printf("* Movie info *\n");
		printf("\tTimescale: %d, duration: %llu, "
			   "creation_time: %llu, modification_time: %llu \n",
			   mp4_timescale, mp4_duration*1000000ULL/mp4_timescale,
			   mp4_creation_time, mp4_modification_time);
		printf("\tNumber of tracks: %d \n", nr_tracks);

		printf("\n");

		/* track index start from 0 */
		for (i = 0; i < nr_tracks; i++) {
			dump_track_info(p_mp4, i);
		}

		mp4_close(p_mp4);
	}

	/* extract track to raw data */
	if (do_extract_track) {
		if (!mp4_probe(mp4_file_name)) {
			printf("unsupported file \n");
			return -1;
		}

		p_mp4 = mp4_open(mp4_file_name, MP4_OPEN_READ);
		if (p_mp4 == NULL) {
			printf("invalid mp4 file \n");
			return -1;
		}

		nr_tracks = mp4_get_nr_of_tracks(p_mp4);

		if (extract_track_id != 0xFFFFFFFF) {
			if (extract_track_id > nr_tracks) {
				printf("invalid info track id(%d) \n", extract_track_id);
				return -1;
			}

			extract_track_info(p_mp4, extract_track_id);
		} else {
			/* track index start from 0 */
			for (i = 0; i < nr_tracks; i++) {
				extract_track_info(p_mp4, i);
			}
		}

		mp4_close(p_mp4);
	}

	/* show track random access point */
	if (show_rap) {
		if (!mp4_probe(mp4_file_name)) {
			printf("unsupported file \n");
			return -1;
		}

		p_mp4 = mp4_open(mp4_file_name, MP4_OPEN_READ);
		if (p_mp4 == NULL) {
			printf("invalid mp4 file \n");
			return -1;
		}

		nr_tracks = mp4_get_nr_of_tracks(p_mp4);

		if (rap_track_id != 0xFFFFFFFF) {
			if (rap_track_id > nr_tracks) {
				printf("invalid rap track id(%d) \n", rap_track_id);
				return -1;
			}

			show_track_rap(p_mp4, rap_track_id);
		} else {
			/* track index start from 0 */
			for (i = 0; i < nr_tracks; i++) {
				show_track_rap(p_mp4, i);
			}
		}

		mp4_close(p_mp4);
	}

	return 0;
}
