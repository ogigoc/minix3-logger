# Makefile for Logging Server (LS)
PROG=	ls
SRCS=	main.c util.c logger.c

DPADD+=	${LIBSYS}
LDADD+=	-lsys

BOOT_FILE = /boot/minix_latest/mod11_ls.gz
hdboot: $(PROG)
	if [ -d staging ]; then echo Removing staging directory...; rm -rf staging; fi
	echo Creating empty staging directory...
	mkdir -p staging
	echo Copying binary to staging...
	cp $(PROG) staging/$(PROG)
	echo Stripping symbols from the binary...
	strip --strip-all staging/$(PROG)
	echo Gzipping binary...
	gzip staging/$(PROG)
	echo Copying binary from staging to boot directory...
	cp -f staging/$(PROG).gz $(BOOT_FILE)
	echo Done. Reboot to run the new server.

.include <minix.service.mk>
