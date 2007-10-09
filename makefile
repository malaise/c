SUBDIRS := boolean timeval wait_evt mutex socket socket_evt \
 get_line rusage sem_util \
 sig_util vt100 adjtime \
 asc catlock cirtail dt forker fread_float \
 gettimeofdays gorgy html2ascii init_so ipm ipx mallocer locale man2file \
 mu pcrypt putvar semctl shm status substit synchro term tcpdump unlink \
 udp_send udpspy

include $(HOME)/Makefiles/dir.mk

