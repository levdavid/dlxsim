# This Makefile is for use when distributing dlxsim to UNIX systems.
# It is simplified not to include any Sprite-specific stuff, and it
# makes copies of library source files that may not be present on
# all UNIX systems.  For HP-UX systems, use the second definition
# of LIBS below.

LIBS = tcl/tcl.a
#LIBS = tcl/tcl.a -lBSD

CC = gcc
#CC = cc
CFLAGS = -g -I. -Itcl
DEST = /tmp_mnt/home/ginger/pnh/bly/bin/$(MACHINE)/
DEST = ../bin
TARGET = dlxsim

OBJS = asm.o cop0.o getput.o io.o main.o sim.o stop.o sym.o trap.o

LIBOBJS = Hash.o HashChainSearch.o Hash_CreateEntry.o Hash_DeleteEntry.o \
	Hash_EnumFirst.o Hash_EnumNext.o Hash_FindEntry.o Hash_InitTable.o

CSRCS = asm.c cop0.c getput.c io.c main.c sim.c stop.c sym.c trap.c

$(TARGET): $(OBJS) $(LIBOBJS) $(LIBS)
	rm -f $(TARGET)
	$(CC) -g ${OBJS} ${LIBOBJS} ${LIBS} -o $(TARGET)
	cp $(TARGET) $(DEST)


tcl/tcl.a:
	cd tcl; make

install: $(TARGET)
	rm -f $(DEST)$(TARGET).old
	mv $(DEST)$(TARGET) $(DEST)$(TARGET).old
	chmod 755 $(TARGET)
	mv $(TARGET) $(DEST)$(TARGET)

clean:
	rm -f ${OBJS} ${LIBOBJS} $(TARGET)

Hash.o : Hash.c hash.h
HashChainSearch.o : HashChainSearch.c hash.h
Hash_CreateEntry.o : Hash_CreateEntry.c hash.h
Hash_DeleteEntry.o : Hash_DeleteEntry.c hash.h
Hash_DeleteTable.o : Hash_DeleteTable.c hash.h
Hash_EnumFirst.o : Hash_EnumFirst.c hash.h
Hash_EnumNext.o : Hash_EnumNext.c hash.h
Hash_FindEntry.o : Hash_FindEntry.c hash.h
Hash_InitTable.o : Hash_InitTable.c hash.h
asm.o : asm.c dlx.h asm.h gp.h sym.h
cop0.o : cop0.c dlx.h
getput.o : getput.c asm.h gp.h dlx.h sym.h
io.o : io.c dlx.h
main.o : main.c dlx.h
sim.o : sim.c asm.h dlx.h sym.h
stop.o : stop.c asm.h dlx.h sym.h
sym.o : sym.c dlx.h sym.h
trap.o : trap.c asm.h dlx.h sym.h traps.h
