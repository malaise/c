SUBDIRS := boolean mutex timeval socket socket_evt rusage sem_util sig_util\
 wait_evt get_line\
 16_unali adjtime asc ask byte_swapping catlock cirtail\
 delay dt enquire forker fread_float gettimeofdays gorgy init_so ipm ipx\
 locale localtime mallocer man2file name_of pcrypt Poing putvar\
 semctl shm synchro tcpdump time_spy\
 t_malloc t_time udp_send udpspy

include $(HOME)/Makefiles/dir.mk
