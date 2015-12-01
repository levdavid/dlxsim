#ifdef SUN

/* NOTE: File descriptor f is represented byu 1<<f */

#define FD_SETSIZE	(sizeof(int)*8)
#define FD_CLR(fd,fdset)( \
	(fdset)->fds_bits[0] &= ~(((int)1) << ((fd))))
#define FD_SET(fd,fdset)( \
	(fdset)->fds_bits[0] |=  (((int)1) << (fd)))
#define FD_ZERO(fdset)  ((fdset)->fds_bits[0] = 0)
#define FD_ISSET(fd,fdset)( \
	(fdset)->fds_bits[0] &  (((int)1) << ((fd))))
#endif SUN
