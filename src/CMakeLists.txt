include_directories(.)

add_library(mo_unpack SHARED convert_float_ibm_to_ieee32.c convert_float_ieee32_to_ibm.c extract_bitmaps.c extract_nbit_words.c extract_wgdos_row.c logerrors.c pack_ppfield.c read_wgdos_bitmaps.ibm.c rlencode.c uascii.c unpack_ppfield.c wgdos_decode_field_parameters.c wgdos_decode_row_parameters.c wgdos_expand_row_to_data.c wgdos_pack.c wgdos_unpack.c)

set_target_properties(mo_unpack PROPERTIES SOVERSION 3)

install(TARGETS mo_unpack DESTINATION lib)
install(FILES wgdosstuff.h DESTINATION include)

