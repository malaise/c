include $(HOME)/Makefiles/common.mk
SUBDIRS := boolean timeval wait_evt mutex socket socket_evt \
 get_line rusage sem_util sig_util vt100 dynlist \
 adjtime asc catlock cirtail delta_time dos2unix dt forker fread_float \
 gettimeofdays gorgy html2ascii init_so ipm ipx locale mallocer man2file \
 mapcode mheap mlist mu pcre pcrypt putvar semctl shm status substit synchro \
 tcpdump term udp_send unlink udpspy

include $(TEMPLATES)/dir.mk

