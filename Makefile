major=2
minor=0
rel=4
CC=cc

OPTS=-O3 -I./include -D_LARGEFILE_SOURCE -D_LARGEFILE_SOURCE64 -D_FILE_OFFSET_BITS=64 -fpic

OBJS=convert_float_ibm_to_ieee32.o  extract_wgdos_row.o rlencode.o wgdos_decode_row_parameters.o \
	convert_float_ieee32_to_ibm.o  logerrors.o uascii.o wgdos_expand_row_to_data.o \
	extract_bitmaps.o pack_ppfield.o unpack_ppfield.o wgdos_pack.o extract_nbit_words.o \
	read_wgdos_bitmaps.ibm.o wgdos_decode_field_parameters.o wgdos_unpack.o

libmo_unpack: $(OBJS)
	mkdir -p lib
	ar -cr lib/libmo_unpack.a $(OBJS)
	ld -shared -soname libmo_unpack.so.$(major) $(OBJS) -o lib/libmo_unpack.so.$(major).$(minor).$(rel)
	@echo $(shell cd lib; ln -s libmo_unpack.so.$(major).$(minor).$(rel) libmo_unpack.so.$(major).$(minor) && \
    ln -s libmo_unpack.so.$(major).$(minor) libmo_unpack.so.$(major) && \
    ln -s libmo_unpack.so.$(major) libmo_unpack.so )

%.o: %.c
	$(CC) -c $(OPTS) $< -o $@

clean:
	rm -f *.o lib/libmo_unpack.so*
