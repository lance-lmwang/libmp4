
LIB_PATH = ../src

CFLAGS = -I../include -O2 -g -Wall

DEBUGBUILD=yes

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
LDFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
LDFLAGS+=-pg
endif


CC = gcc

LIB_SRCS = $(LIB_PATH)/mp4_file.c $(LIB_PATH)/mp4_file_data_map.c \
    	  $(LIB_PATH)/lib_mp4.c  $(LIB_PATH)/mp4_util.c  \
          $(LIB_PATH)/mp4_bits.c $(LIB_PATH)/mp4_boxes.c \
          $(LIB_PATH)/mp4_free_box.c $(LIB_PATH)/mp4_ftyp_box.c \
          $(LIB_PATH)/mp4_moov_box.c $(LIB_PATH)/mp4_mdat_box.c \
          $(LIB_PATH)/mp4_mvhd_box.c $(LIB_PATH)/mp4_trak_box.c \
          $(LIB_PATH)/mp4_tkhd_box.c $(LIB_PATH)/mp4_tref_box.c \
          $(LIB_PATH)/mp4_edts_box.c $(LIB_PATH)/mp4_mdia_box.c \
          $(LIB_PATH)/mp4_mdhd_box.c $(LIB_PATH)/mp4_hdlr_box.c \
          $(LIB_PATH)/mp4_minf_box.c $(LIB_PATH)/mp4_padb_box.c \
          $(LIB_PATH)/mp4_vmhd_box.c $(LIB_PATH)/mp4_smhd_box.c \
          $(LIB_PATH)/mp4_hmhd_box.c $(LIB_PATH)/mp4_nmhd_box.c \
          $(LIB_PATH)/mp4_dinf_box.c $(LIB_PATH)/mp4_stbl_box.c \
          $(LIB_PATH)/mp4_stsd_box.c $(LIB_PATH)/mp4_stts_box.c \
          $(LIB_PATH)/mp4_ctts_box.c $(LIB_PATH)/mp4_stsc_box.c \
          $(LIB_PATH)/mp4_stsz_box.c $(LIB_PATH)/mp4_stz2_box.c \
          $(LIB_PATH)/mp4_stco_box.c $(LIB_PATH)/mp4_co64_box.c \
          $(LIB_PATH)/mp4_stss_box.c $(LIB_PATH)/mp4_stsh_box.c \
          $(LIB_PATH)/mp4_dref_box.c $(LIB_PATH)/mp4_mp4v_box.c \
          $(LIB_PATH)/mp4_url_box.c $(LIB_PATH)/mp4_urn_box.c \
          $(LIB_PATH)/mp4_udta_box.c $(LIB_PATH)/mp4_unknown_box.c \
          $(LIB_PATH)/mp4_sample_descs.c $(LIB_PATH)/mp4_esds_box.c \
          $(LIB_PATH)/mp4_mp4a_box.c $(LIB_PATH)/mp4_rtp_box.c \
          $(LIB_PATH)/mp4_tims_box.c $(LIB_PATH)/mp4_tsro_box.c \
          $(LIB_PATH)/mp4_snro_box.c $(LIB_PATH)/mp4_avc1_box.c \
          $(LIB_PATH)/mp4_btrt_box.c $(LIB_PATH)/mp4_m4ds_box.c \
          $(LIB_PATH)/mp4_avcC_box.c $(LIB_PATH)/mp4_descriptor.c \
          $(LIB_PATH)/mp4_default_descr.c $(LIB_PATH)/mp4_es_descr.c \
          $(LIB_PATH)/mp4_dec_config_descr.c $(LIB_PATH)/mp4_mp4s_box.c \
          $(LIB_PATH)/mp4_elst_box.c \
		  $(LIB_PATH)/mp4_debug.c $(LIB_PATH)/mp4_list.c

LIB_OBJS := $(patsubst %.c,%.o,$(LIB_SRCS))

LIB=../lib/libmp4_static.a

all: $(LIB)

$(LIB) : $(LIB_OBJS)
	ar rc $@ $(LIB_OBJS)
	-ranlib $@

depend:
	$(CC) -MM $(CFLAGS) $(LIB_SRCS) $(PROG_SRCS) 1>.depend

clean: 
	rm -f $(LIB_OBJS) $(LIB)

distclean: clean
	rm -f Makefile.bak .depend



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
