#
#  Makefile for CSC501 Xinu.
#

INCLUDE = -I../h
CFLAGS  = -m32 -march=i586 -fno-stack-protector -fno-builtin -c -Wall -O ${DEFS} ${INCLUDE} -std=gnu99
SDEFS	= -I../h
ASFLAGS = --32

CC      =       /usr/bin/gcc
cPP     =       /usr/bin/gcc -E
AS      =       /usr/bin/as
LD      =       /usr/bin/ld
MAKETD  =	/usr/X11R6/bin/makedepend
AWK	=	awk
LIB     =       ../lib
XINU    =       ./xinu

# Amount to move loaded image down in memory
BRELOC  =	0x100000
BOOTPLOC=	0x150000

#------------------------------------------------------------------------
# source files
#------------------------------------------------------------------------
COM =	comcntl.c	comgetc.c	comiin.c	cominit.c	\
	cominput.c	comoutput.c	comread.c	comintr.c

MON =   monitor.c       monarp.c        monbootp.c      monip.c         \
	monnet.c        monudp.c        mongpq.c        ethintr.c       \
	ethwrite.c      ethinit.c       ethdemux.c      ethwstrt.c      \
	ethrom.c        monboot.c       montimer.c      ethcmd.c	\
	monpci.c	mon3com.c

SYS =	blkcmp.c	blkequ.c	main.c		stacktrace.c	\
	chprio.c	clkinit.c	close.c		conf.c		\
	control.c	create.c	evec.c		freebuf.c	\
	freemem.c	getbuf.c	getc.c		getitem.c	\
	getmem.c	getpid.c	getprio.c	getstk.c	\
	gettime.c	gpq.c		i386.c		init.c		\
	insert.c	insertd.c	ioerr.c		ionull.c	\
	kill.c		kprintf.c	kputc.c		mark.c		\
	mkpool.c	newqueue.c	open.c		panic.c		\
	poolinit.c	putc.c		queue.c		read.c		\
	ready.c		receive.c	recvclr.c	recvtim.c	\
	resched.c	resume.c	scount.c	screate.c	\
	sdelete.c	send.c		setdev.c	setnok.c	\
	signal.c	signaln.c	sleep.c		sleep10.c	\
	sleep100.c	sleep1000.c	sreset.c	suspend.c	\
	unsleep.c	userret.c	wait.c		wakeup.c	\
	write.c		xdone.c		pci.c           shutdown.c \

TTY =	ttyalloc.c	ttycntl.c	ttygetc.c	ttyiin.c	\
	ttyinit.c	ttynew.c	ttyopen.c	ttyputc.c	\
	ttyread.c	ttywrite.c	ttyoin.c

PG  =   get_bs.c        release_bs.c    read_bs.c       write_bs.c      \
        control_reg.c   bsm.c           policy.c        \
        frame.c         pfint.c         dump32.c        vcreate.c       \
        xm.c            vgetmem.c       vfreemem.c       \
        paging_sc.c     paging_fifo.c

SRC = ${COM} ${TTY} ${MON} ${SYS}

#------------------------------------------------------------------------
# object files
#------------------------------------------------------------------------
COMOBJ = ${COM:%.c=%.o}

MONOBJ = ${MON:%.c=%.o}

SYSOBJ = ${SYS:%.c=%.o}

TTYOBJ = ${TTY:%.c=%.o}

PGOBJ = ${PG:%.c=%.o}

XOBJ = startup.o initialize.o intr.o clkint.o ctxsw.o pfintr.o

OBJ =	${COMOBJ} ${MONOBJ} ${SYSOBJ} ${TTYOBJ}		\
	${PGOBJ}					\
	moncksum.o monclkint.o comint.o ethint.o montftp.o

#------------------------------------------------------------------------
# make targets
#------------------------------------------------------------------------
all: xinu.elf

xinu.elf: Makefile Configuration ../h/conf.h ${XOBJ} ${OBJ} ${LIB}/libxc.a
	$(LD) -m elf_i386 -dn -Ttext 0x10000 -e start ${XOBJ} ${OBJ} ${LIB}/libxc.a \
	-o ${XINU}.elf

clean: FRC
	rm -rf .d* *.o *.errs *.bak *nm* core ${XINU} ${XINU}.elf tags version
	rm -rf ../h/conf.h
	echo '0' > vn
	(cd ${LIB}/libxc; ${MAKE} clean)

depend: makedep
	sed -e '1,/^# DO NOT DELETE THIS LINE/!d' Makefile > Makefile.base
	cat Makefile.base makedep > Makefile
	rm Makefile.base makedep makedep1 makedeperrs
	@rm -f ../h/conf.h

undepend:
	sed -e '1,/^# DO NOT DELETE THIS LINE/!d' Makefile > Makefile.base
	mv Makefile.base Makefile

CFILES = ../*/*.c ../*/*/*.c

makedep: FRC
	@cat /dev/null >makedep
	@touch ../h/conf.h
	-(for i in ${CFILES} ; do \
		${CC} -M -w ${DEFS} -I../h ./$$i >> makedep; done) \
		2>makedeperrs
	sed -e 's,^\(.*\)\.o:,\1.o:,' makedep > makedep1
	${AWK} ' { if ($$1 != prev) { print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$3) > 78) { print rec; rec = $$0; } \
			else rec = rec " " $$3 } } \
		END { print rec } ' makedep1 > makedep
	@cat makedeperrs
	@(if [ -s makedeperrs ]; then false; fi)

xdepend: FRC
	${MAKETD} ${DEFS} -I../h ${CFILES}

run:
	@qemu-system-i386 -nographic -device isa-debug-exit,iobase=0xf4,iosize=0x04 -kernel ${XINU}.elf ||:

debug:
	@qemu-system-i386 -s -S -nographic -device isa-debug-exit,iobase=0xf4,iosize=0x04 -kernel ${XINU}.elf ||:

#------------------------------------------------------------------------
#
#------------------------------------------------------------------------
tags: FRC
	ctags -R ..

../config/config:
	make -C ../config/ -f ../config/Makefile clean all

../h/conf.h ../sys/conf.c: ../config/config Configuration
	../config/config; mv conf.c ../sys 

${LIB}/libxc.a: FRC
	(cd ${LIB}/libxc; make install)

intr.o: ../sys/intr.S
	${CPP} ${SDEFS} ../sys/intr.S | ${AS} ${ASFLAGS} -o intr.o

clkint.o: ../sys/clkint.S
	${CPP} ${SDEFS} ../sys/clkint.S | ${AS} ${ASFLAGS} -o clkint.o

ctxsw.o: ../sys/ctxsw.S
	${CPP} ${SDEFS} ../sys/ctxsw.S | ${AS} ${ASFLAGS} -o ctxsw.o

pfintr.o: ../paging/pfintr.S
	${CPP} ${SDEFS} ../paging/pfintr.S | ${AS} ${ASFLAGS} -o pfintr.o

ethint.o: ../mon/ethint.S
	${CPP} ${SDEFS} ../mon/ethint.S | ${AS} ${ASFLAGS} -o ethint.o

startup.o: ../sys/startup.S Makefile
	${CPP} ${SDEFS} -DBRELOC=${BRELOC} -DBOOTPLOC=${BOOTPLOC} \
	../sys/startup.S | ${AS} ${ASFLAGS} -o startup.o 

moncksum.o: ../mon/moncksum.S
	${CPP} ${SDEFS} ../mon/moncksum.S | ${AS} ${ASFLAGS} -o moncksum.o

monclkint.o: ../mon/monclkint.S
	${CPP} ${SDEFS} ../mon/monclkint.S | ${AS} ${ASFLAGS} -o monclkint.o

montftp.o: ../mon/montftp.c
	${CC} ${CFLAGS} -DBOOTPLOC=${BOOTPLOC} ../mon/montftp.c -o montftp.o

comint.o: ../com/comint.S
	${CPP} ${SDEFS} ../com/comint.S | ${AS} ${ASFLAGS} -o comint.o

initialize.o: $(OBJ) startup.o paging_sc.o paging_fifo.o
	sh mkvers.sh
	${CC} -c ${CFLAGS} -DVERSION=\""`cat version`"\" ../sys/initialize.c

${MONOBJ}:
	 ${CC} ${CFLAGS} ../mon/`basename $@ .o`.[c]
${COMOBJ}:
	 ${CC} ${CFLAGS} ../com/`basename $@ .o`.[c]
${SYSOBJ}:
	 ${CC} ${CFLAGS} ../sys/`basename $@ .o`.[c]
${TTYOBJ}:
	 ${CC} ${CFLAGS} ../tty/`basename $@ .o`.[c]	 
${PGOBJ}:
	${CC} ${CFLAGS} ../paging/`basename $@ .o`.[c]

FRC:
#------------------------------------------------------------------------

# DO NOT DELETE THIS LINE - maketd DEPENDS ON IT

comcntl.o: ../com/comcntl.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h
comgetc.o: ../com/comgetc.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h ../h/stdio.h
comiin.o: ../com/comiin.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/com.h
cominit.o: ../com/cominit.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/com.h \
 ../h/stdio.h
cominput.o: ../com/cominput.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/io.h \
 ../h/stdio.h
comintr.o: ../com/comintr.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/com.h \
 ../h/stdio.h
comoutput.o: ../com/comoutput.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/tty.h \
 ../h/com.h ../h/stdio.h
comread.o: ../com/comread.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
lex.yy.o: ../config/lex.yy.c /usr/include/stdc-predef.h ../h/stdio.h \
 /usr/include/string.h \
 /usr/include/x86_64-linux-gnu/bits/libc-header-start.h \
 /usr/include/features.h /usr/include/features-time64.h \
 /usr/include/x86_64-linux-gnu/bits/wordsize.h \
 /usr/include/x86_64-linux-gnu/bits/timesize.h \
 /usr/include/x86_64-linux-gnu/sys/cdefs.h \
 /usr/include/x86_64-linux-gnu/bits/long-double.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/13/include/stddef.h \
 /usr/include/x86_64-linux-gnu/bits/types/locale_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h \
 /usr/include/strings.h /usr/include/errno.h \
 /usr/include/x86_64-linux-gnu/bits/errno.h /usr/include/linux/errno.h \
 /usr/include/x86_64-linux-gnu/asm/errno.h \
 /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
 /usr/include/stdlib.h /usr/include/x86_64-linux-gnu/bits/waitflags.h \
 /usr/include/x86_64-linux-gnu/bits/waitstatus.h \
 /usr/include/x86_64-linux-gnu/bits/floatn.h \
 /usr/include/x86_64-linux-gnu/bits/floatn-common.h \
 /usr/include/x86_64-linux-gnu/sys/types.h \
 /usr/include/x86_64-linux-gnu/bits/types.h \
 /usr/include/x86_64-linux-gnu/bits/typesizes.h \
 /usr/include/x86_64-linux-gnu/bits/time64.h \
 /usr/include/x86_64-linux-gnu/bits/types/clock_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/time_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/timer_t.h \
 /usr/include/x86_64-linux-gnu/bits/stdint-intn.h /usr/include/endian.h \
 /usr/include/x86_64-linux-gnu/bits/endian.h \
 /usr/include/x86_64-linux-gnu/bits/endianness.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap.h \
 /usr/include/x86_64-linux-gnu/bits/uintn-identity.h \
 /usr/include/x86_64-linux-gnu/sys/select.h \
 /usr/include/x86_64-linux-gnu/bits/select.h \
 /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h \
 /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h \
 /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h \
 /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h \
 /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h \
 /usr/include/x86_64-linux-gnu/bits/atomic_wide_counter.h \
 /usr/include/x86_64-linux-gnu/bits/struct_mutex.h \
 /usr/include/x86_64-linux-gnu/bits/struct_rwlock.h /usr/include/alloca.h \
 /usr/include/x86_64-linux-gnu/bits/stdlib-float.h \
 /usr/include/inttypes.h \
 /usr/lib/gcc/x86_64-linux-gnu/13/include/stdint.h /usr/include/stdint.h \
 /usr/include/x86_64-linux-gnu/bits/wchar.h \
 /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h \
 /usr/include/x86_64-linux-gnu/bits/stdint-least.h /usr/include/unistd.h \
 /usr/include/x86_64-linux-gnu/bits/posix_opt.h \
 /usr/include/x86_64-linux-gnu/bits/environments.h \
 /usr/include/x86_64-linux-gnu/bits/confname.h \
 /usr/include/x86_64-linux-gnu/bits/getopt_posix.h \
 /usr/include/x86_64-linux-gnu/bits/getopt_core.h \
 /usr/include/x86_64-linux-gnu/bits/unistd_ext.h
y.tab.o: ../config/y.tab.c /usr/include/stdc-predef.h \
 /usr/include/stdlib.h \
 /usr/include/x86_64-linux-gnu/bits/libc-header-start.h \
 /usr/include/features.h /usr/include/features-time64.h \
 /usr/include/x86_64-linux-gnu/bits/wordsize.h \
 /usr/include/x86_64-linux-gnu/bits/timesize.h \
 /usr/include/x86_64-linux-gnu/sys/cdefs.h \
 /usr/include/x86_64-linux-gnu/bits/long-double.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs.h \
 /usr/include/x86_64-linux-gnu/gnu/stubs-64.h \
 /usr/lib/gcc/x86_64-linux-gnu/13/include/stddef.h \
 /usr/include/x86_64-linux-gnu/bits/waitflags.h \
 /usr/include/x86_64-linux-gnu/bits/waitstatus.h \
 /usr/include/x86_64-linux-gnu/bits/floatn.h \
 /usr/include/x86_64-linux-gnu/bits/floatn-common.h \
 /usr/include/x86_64-linux-gnu/sys/types.h \
 /usr/include/x86_64-linux-gnu/bits/types.h \
 /usr/include/x86_64-linux-gnu/bits/typesizes.h \
 /usr/include/x86_64-linux-gnu/bits/time64.h \
 /usr/include/x86_64-linux-gnu/bits/types/clock_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/clockid_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/time_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/timer_t.h \
 /usr/include/x86_64-linux-gnu/bits/stdint-intn.h /usr/include/endian.h \
 /usr/include/x86_64-linux-gnu/bits/endian.h \
 /usr/include/x86_64-linux-gnu/bits/endianness.h \
 /usr/include/x86_64-linux-gnu/bits/byteswap.h \
 /usr/include/x86_64-linux-gnu/bits/uintn-identity.h \
 /usr/include/x86_64-linux-gnu/sys/select.h \
 /usr/include/x86_64-linux-gnu/bits/select.h \
 /usr/include/x86_64-linux-gnu/bits/types/sigset_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/__sigset_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h \
 /usr/include/x86_64-linux-gnu/bits/types/struct_timespec.h \
 /usr/include/x86_64-linux-gnu/bits/pthreadtypes.h \
 /usr/include/x86_64-linux-gnu/bits/thread-shared-types.h \
 /usr/include/x86_64-linux-gnu/bits/pthreadtypes-arch.h \
 /usr/include/x86_64-linux-gnu/bits/atomic_wide_counter.h \
 /usr/include/x86_64-linux-gnu/bits/struct_mutex.h \
 /usr/include/x86_64-linux-gnu/bits/struct_rwlock.h /usr/include/alloca.h \
 /usr/include/x86_64-linux-gnu/bits/stdlib-float.h ../h/stdio.h \
 ../config/lex.yy.c /usr/include/string.h \
 /usr/include/x86_64-linux-gnu/bits/types/locale_t.h \
 /usr/include/x86_64-linux-gnu/bits/types/__locale_t.h \
 /usr/include/strings.h /usr/include/errno.h \
 /usr/include/x86_64-linux-gnu/bits/errno.h /usr/include/linux/errno.h \
 /usr/include/x86_64-linux-gnu/asm/errno.h \
 /usr/include/asm-generic/errno.h /usr/include/asm-generic/errno-base.h \
 /usr/include/inttypes.h \
 /usr/lib/gcc/x86_64-linux-gnu/13/include/stdint.h /usr/include/stdint.h \
 /usr/include/x86_64-linux-gnu/bits/wchar.h \
 /usr/include/x86_64-linux-gnu/bits/stdint-uintn.h \
 /usr/include/x86_64-linux-gnu/bits/stdint-least.h /usr/include/unistd.h \
 /usr/include/x86_64-linux-gnu/bits/posix_opt.h \
 /usr/include/x86_64-linux-gnu/bits/environments.h \
 /usr/include/x86_64-linux-gnu/bits/confname.h \
 /usr/include/x86_64-linux-gnu/bits/getopt_posix.h \
 /usr/include/x86_64-linux-gnu/bits/getopt_core.h \
 /usr/include/x86_64-linux-gnu/bits/unistd_ext.h
ethcmd.o: ../mon/ethcmd.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/moni386.h ../h/./mon/moneepro.h ../h/stdio.h
ethdemux.o: ../mon/ethdemux.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/./mon/moneepro.h ../h/stdio.h
ethinit.o: ../mon/ethinit.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/./mon/moneepro.h ../h/./mon/mon3com.h ../h/./mon/monintel.h \
 ../h/./mon/monpci.h ../h/stdio.h
ethintr.o: ../mon/ethintr.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/./mon/moneepro.h ../h/stdio.h
ethrom.o: ../mon/ethrom.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/./mon/moneepro.h ../h/stdio.h
ethwrite.o: ../mon/ethwrite.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/stdio.h
ethwstrt.o: ../mon/ethwstrt.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/./mon/moneepro.h ../h/stdio.h
mon3com.o: ../mon/mon3com.c /usr/include/stdc-predef.h ../h/mon/moni386.h \
 ../h/mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/mon/mon3com.h ../h/mon/monpci.h ../h/mon/monitor.h \
 ../h/./mon/moncom.h ../h/./mon/monnetwork.h ../h/./mon/monitor.h \
 ../h/./mon/moni386.h ../h/stdio.h
monarp.o: ../mon/monarp.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/stdio.h
monboot.o: ../mon/monboot.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/stdio.h
monbootp.o: ../mon/monbootp.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/stdio.h
mongpq.o: ../mon/mongpq.c /usr/include/stdc-predef.h ../h/./mon/monconf.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/monnetwork.h \
 ../h/./mon/monsystypes.h ../h/./mon/monip.h ../h/./mon/monether.h \
 ../h/./mon/monudp.h ../h/./mon/monbootp.h ../h/./mon/monarp.h \
 ../h/./mon/monnetif.h ../h/./mon/moni386.h ../h/stdio.h
monip.o: ../mon/monip.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/stdio.h
monitor.o: ../mon/monitor.c /usr/include/stdc-predef.h \
 ../h/./mon/moncom.h ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h \
 ../h/./mon/monconf.h ../h/./mon/monip.h ../h/./mon/monether.h \
 ../h/./mon/monudp.h ../h/./mon/monbootp.h ../h/./mon/monarp.h \
 ../h/./mon/monnetif.h ../h/./mon/monitor.h ../h/./mon/moni386.h \
 ../h/bufpool.h ../h/mark.h ../h/stdio.h
monnet.o: ../mon/monnet.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/moni386.h ../h/./mon/monitor.h ../h/./mon/moncom.h \
 ../h/bufpool.h ../h/mark.h ../h/stdio.h
monpci.o: ../mon/monpci.c /usr/include/stdc-predef.h \
 ../h/mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/mon/mon3com.h ../h/mon/monpci.h ../h/mon/monitor.h \
 ../h/./mon/moncom.h ../h/./mon/monnetwork.h ../h/./mon/monitor.h \
 ../h/./mon/moni386.h ../h/stdio.h
montftp.o: ../mon/montftp.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/./mon/montftp.h ../h/stdio.h
montimer.o: ../mon/montimer.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/stdio.h
monudp.o: ../mon/monudp.c /usr/include/stdc-predef.h \
 ../h/./mon/monnetwork.h ../h/./mon/monsystypes.h ../h/./mon/monconf.h \
 ../h/./mon/monip.h ../h/./mon/monether.h ../h/./mon/monudp.h \
 ../h/./mon/monbootp.h ../h/./mon/monarp.h ../h/./mon/monnetif.h \
 ../h/./mon/monitor.h ../h/./mon/moncom.h ../h/./mon/moni386.h \
 ../h/./mon/montftp.h ../h/stdio.h
bsm.o: ../paging/bsm.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h \
 ../h/proc.h
control_reg.o: ../paging/control_reg.c /usr/include/stdc-predef.h \
 ../h/conf.h ../h/kernel.h ../h/systypes.h ../h/mem.h
dump32.o: ../paging/dump32.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h
frame.o: ../paging/frame.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h \
 ../h/proc.h
get_bs.o: ../paging/get_bs.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h \
 ../h/proc.h
paging_fifo.o: ../paging/paging_fifo.c /usr/include/stdc-predef.h \
 ../h/paging_fifo.h ../h/paging.h ../h/kernel.h ../h/systypes.h \
 ../h/conf.h ../h/mem.h
paging_sc.o: ../paging/paging_sc.c /usr/include/stdc-predef.h \
 ../h/paging_sc.h ../h/paging.h ../h/kernel.h ../h/systypes.h ../h/conf.h \
 ../h/mem.h
pfint.o: ../paging/pfint.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h \
 ../h/proc.h
policy.o: ../paging/policy.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h \
 ../h/paging_fifo.h ../h/paging_sc.h
read_bs.o: ../paging/read_bs.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/mark.h ../h/bufpool.h \
 ../h/proc.h ../h/paging.h ../h/kernel.h
release_bs.o: ../paging/release_bs.c /usr/include/stdc-predef.h \
 ../h/conf.h ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h \
 ../h/kernel.h ../h/proc.h
vcreate.o: ../paging/vcreate.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/i386.h ../h/io.h ../h/kernel.h ../h/systypes.h ../h/mem.h \
 ../h/paging.h ../h/kernel.h ../h/proc.h ../h/sem.h
vfreemem.o: ../paging/vfreemem.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h \
 ../h/proc.h
vgetmem.o: ../paging/vgetmem.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h \
 ../h/proc.h
write_bs.o: ../paging/write_bs.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/mark.h \
 ../h/bufpool.h ../h/paging.h ../h/kernel.h
xm.o: ../paging/xm.c /usr/include/stdc-predef.h ../h/conf.h ../h/kernel.h \
 ../h/systypes.h ../h/mem.h ../h/paging.h ../h/kernel.h ../h/proc.h
blkcmp.o: ../sys/blkcmp.c /usr/include/stdc-predef.h
blkequ.o: ../sys/blkequ.c /usr/include/stdc-predef.h ../h/kernel.h \
 ../h/systypes.h ../h/conf.h ../h/mem.h
chprio.o: ../sys/chprio.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h
clkinit.o: ../sys/clkinit.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/sleep.h ../h/i386.h \
 ../h/stdio.h ../h/q.h
close.o: ../sys/close.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
conf.o: ../sys/conf.c /usr/include/stdc-predef.h ../h/conf.h
control.o: ../sys/control.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
create.o: ../sys/create.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/i386.h ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h \
 ../h/sem.h ../h/io.h ../h/paging.h ../h/kernel.h
evec.o: ../sys/evec.c /usr/include/stdc-predef.h ../h/conf.h ../h/i386.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/sem.h \
 ../h/tty.h ../h/q.h ../h/io.h ../h/stdio.h
freebuf.o: ../sys/freebuf.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/mark.h ../h/bufpool.h \
 ../h/stdio.h
freemem.o: ../sys/freemem.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/stdio.h
getbuf.o: ../sys/getbuf.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/mark.h ../h/bufpool.h \
 ../h/stdio.h
getc.o: ../sys/getc.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
getitem.o: ../sys/getitem.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/q.h
getmem.o: ../sys/getmem.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/stdio.h
getpid.o: ../sys/getpid.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h
getprio.o: ../sys/getprio.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
getstk.o: ../sys/getstk.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/stdio.h
gettime.o: ../sys/gettime.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/date.h
gpq.o: ../sys/gpq.c /usr/include/stdc-predef.h ../h/conf.h ../h/kernel.h \
 ../h/systypes.h ../h/mem.h ../h/q.h ../h/stdio.h
i386.o: ../sys/i386.c /usr/include/stdc-predef.h ../h/icu.h ../h/i386.h \
 ../h/kernel.h ../h/systypes.h ../h/conf.h ../h/mem.h
init.o: ../sys/init.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
initialize.o: ../sys/initialize.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/i386.h ../h/io.h ../h/kernel.h ../h/systypes.h ../h/mem.h \
 ../h/paging.h ../h/kernel.h ../h/paging_fifo.h ../h/paging_sc.h \
 ../h/proc.h ../h/q.h ../h/sem.h ../h/sleep.h ../h/tty.h
insert.o: ../sys/insert.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/q.h
insertd.o: ../sys/insertd.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/q.h
ioerr.o: ../sys/ioerr.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h
ionull.o: ../sys/ionull.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h
kill.o: ../sys/kill.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/sem.h \
 ../h/io.h ../h/q.h ../h/stdio.h ../h/paging.h ../h/kernel.h
kprintf.o: ../sys/kprintf.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
kputc.o: ../sys/kputc.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
main.o: ../sys/main.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h \
 ../h/paging.h ../h/kernel.h
mark.o: ../sys/mark.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/mark.h
mkpool.o: ../sys/mkpool.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/mark.h ../h/bufpool.h \
 ../h/stdio.h
newqueue.o: ../sys/newqueue.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/q.h
open.o: ../sys/open.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
panic.o: ../sys/panic.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
pci.o: ../sys/pci.c /usr/include/stdc-predef.h ../h/conf.h ../h/kernel.h \
 ../h/systypes.h ../h/mem.h ../h/pci.h ../h/stdio.h
poolinit.o: ../sys/poolinit.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/mark.h ../h/bufpool.h \
 ../h/stdio.h
putc.o: ../sys/putc.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
queue.o: ../sys/queue.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/q.h
read.o: ../sys/read.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
ready.o: ../sys/ready.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h
receive.o: ../sys/receive.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
recvclr.o: ../sys/recvclr.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
recvtim.o: ../sys/recvtim.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h ../h/stdio.h
resched.o: ../sys/resched.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/paging.h ../h/kernel.h
resume.o: ../sys/resume.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
scount.o: ../sys/scount.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/sem.h
screate.o: ../sys/screate.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h ../h/sem.h \
 ../h/stdio.h
sdelete.o: ../sys/sdelete.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h ../h/sem.h \
 ../h/stdio.h
send.o: ../sys/send.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
setdev.o: ../sys/setdev.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h
setnok.o: ../sys/setnok.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
shutdown.o: ../sys/shutdown.c /usr/include/stdc-predef.h ../h/kernel.h \
 ../h/systypes.h ../h/conf.h ../h/mem.h ../h/stdio.h
signal.o: ../sys/signal.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h ../h/sem.h \
 ../h/stdio.h
signaln.o: ../sys/signaln.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h ../h/sem.h \
 ../h/stdio.h
sleep.o: ../sys/sleep.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h ../h/stdio.h
sleep10.o: ../sys/sleep10.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h ../h/stdio.h
sleep100.o: ../sys/sleep100.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h ../h/stdio.h
sleep1000.o: ../sys/sleep1000.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h ../h/stdio.h
sreset.o: ../sys/sreset.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h ../h/sem.h \
 ../h/stdio.h
ssclock.o: ../sys/ssclock.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h
stacktrace.o: ../sys/stacktrace.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/stdio.h
suspend.o: ../sys/suspend.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/stdio.h
unsleep.o: ../sys/unsleep.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h ../h/stdio.h
userret.o: ../sys/userret.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h
wait.o: ../sys/wait.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h ../h/sem.h \
 ../h/stdio.h
wakeup.o: ../sys/wakeup.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/proc.h ../h/q.h \
 ../h/sleep.h
write.o: ../sys/write.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h
xdone.o: ../sys/xdone.c /usr/include/stdc-predef.h ../h/kernel.h \
 ../h/systypes.h ../h/conf.h ../h/mem.h ../h/stdio.h
ttyalloc.o: ../tty/ttyalloc.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/stdio.h
ttycntl.o: ../tty/ttycntl.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/stdio.h
ttygetc.o: ../tty/ttygetc.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
ttyiin.o: ../tty/ttyiin.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
ttyinit.o: ../tty/ttyinit.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
ttynew.o: ../tty/ttynew.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
ttyoin.o: ../tty/ttyoin.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/stdio.h
ttyopen.o: ../tty/ttyopen.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
ttyputc.o: ../tty/ttyputc.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
ttyread.o: ../tty/ttyread.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h ../h/stdio.h
ttywrite.o: ../tty/ttywrite.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/tty.h
abs.o: ../lib/libxc/abs.c /usr/include/stdc-predef.h
atof.o: ../lib/libxc/atof.c /usr/include/stdc-predef.h ../h/ctype.h
atoi.o: ../lib/libxc/atoi.c /usr/include/stdc-predef.h
atol.o: ../lib/libxc/atol.c /usr/include/stdc-predef.h
bzero.o: ../lib/libxc/bzero.c /usr/include/stdc-predef.h
ctype_.o: ../lib/libxc/ctype_.c /usr/include/stdc-predef.h ../h/ctype.h
doprnt.o: ../lib/libxc/doprnt.c /usr/include/stdc-predef.h
doscan.o: ../lib/libxc/doscan.c /usr/include/stdc-predef.h ../h/ctype.h
ecvt.o: ../lib/libxc/ecvt.c /usr/include/stdc-predef.h
fgets.o: ../lib/libxc/fgets.c /usr/include/stdc-predef.h
fprintf.o: ../lib/libxc/fprintf.c /usr/include/stdc-predef.h
fputs.o: ../lib/libxc/fputs.c /usr/include/stdc-predef.h
gets.o: ../lib/libxc/gets.c /usr/include/stdc-predef.h
index.o: ../lib/libxc/index.c /usr/include/stdc-predef.h
printf.o: ../lib/libxc/printf.c /usr/include/stdc-predef.h
puts.o: ../lib/libxc/puts.c /usr/include/stdc-predef.h
qsort.o: ../lib/libxc/qsort.c /usr/include/stdc-predef.h
rand.o: ../lib/libxc/rand.c /usr/include/stdc-predef.h
rindex.o: ../lib/libxc/rindex.c /usr/include/stdc-predef.h
scanf.o: ../lib/libxc/scanf.c /usr/include/stdc-predef.h ../h/conf.h \
 ../h/kernel.h ../h/systypes.h ../h/mem.h ../h/io.h ../h/tty.h
sprintf.o: ../lib/libxc/sprintf.c /usr/include/stdc-predef.h
strcat.o: ../lib/libxc/strcat.c /usr/include/stdc-predef.h
strchr.o: ../lib/libxc/strchr.c /usr/include/stdc-predef.h
strcmp.o: ../lib/libxc/strcmp.c /usr/include/stdc-predef.h
strcpy.o: ../lib/libxc/strcpy.c /usr/include/stdc-predef.h
strlen.o: ../lib/libxc/strlen.c /usr/include/stdc-predef.h
strncat.o: ../lib/libxc/strncat.c /usr/include/stdc-predef.h
strncmp.o: ../lib/libxc/strncmp.c /usr/include/stdc-predef.h
strncpy.o: ../lib/libxc/strncpy.c /usr/include/stdc-predef.h
swab.o: ../lib/libxc/swab.c /usr/include/stdc-predef.h
