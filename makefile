SUBDIRS := boolean mutex timeval wait_evt socket socket_evt rusage sem_util \
 sig_util get_line vt100 adjtime gorgy\
 16_unali Poing asc ask byte_swapping catlock cirtail\
 delay dt enquire forker formfeed fread_float gettimeofdays html2ascii\
 init_so ipm ipx\
 locale localtime mallocer man2file name_of mu pause pcrypt putvar\
 semctl shm status substit synchro tcpdump term time_spy\
 t_malloc t_time udp_send udpspy unlink

include $(HOME)/Makefiles/dir.mk
