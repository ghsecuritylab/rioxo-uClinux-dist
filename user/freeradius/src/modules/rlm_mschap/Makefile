#
# Makefile
#
# Version:	$Id: Makefile,v 1.14.2.1.2.1 2006/02/10 19:48:05 nbk Exp $
#

TARGET		= rlm_mschap
SRCS		= rlm_mschap.c smbdes.c
HEADERS		=
RLM_CFLAGS	=
RLM_LDFLAGS	=
RLM_LIBS	=
RLM_UTILS   	= smbencrypt
RLM_INSTALL 	= smbencrypt-install

# over-ride the previous assignment if we're not building anything
include ../rules.mak

$(LT_OBJS): $(HEADERS)

smbencrypt: smbencrypt.lo smbdes.lo ../../lib/libradius.la
	$(LIBTOOL) --mode=link $(CC) $(LDFLAGS) $(RLM_LDFLAGS) \
		-o $@ $^ $(RLM_LIBS) $(LIBS)

smbencrypt-install:
	$(INSTALL) -d -m 755 $(R)$(bindir)
	$(LIBTOOL) --mode=install $(INSTALL) -m 755 $(INSTALLSTRIP) \
		smbencrypt$(EXEEXT) $(R)$(bindir)
