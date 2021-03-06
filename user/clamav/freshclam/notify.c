/*
 *  Copyright (C) 2002 - 2005 Tomasz Kojm <tkojm@clamav.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */
#ifdef        _MSC_VER
#include <windows.h>
#include <winsock.h>
#endif


#if HAVE_CONFIG_H
#include "clamav-config.h"
#endif

#ifdef BUILD_CLAMD

#include <stdio.h>
#ifdef	HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <sys/types.h>
#ifndef	C_WINDOWS
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <string.h>

#include "shared/cfgparser.h"
#include "shared/output.h"
#include "notify.h"

#ifndef	C_WINDOWS
#define	closesocket(s)	close(s)
#endif

int notify(const char *cfgfile)
{
	char buff[20];
#ifndef	C_WINDOWS
	struct sockaddr_un server;
#endif
        struct sockaddr_in server2;
	struct hostent *he;
	struct cfgstruct *copt;
	const struct cfgstruct *cpt;
	int sockd, bread;
	const char *socktype;


    if((copt = getcfg(cfgfile, 1)) == NULL) {
	logg("^Clamd was NOT notified: Can't find or parse configuration file %s\n", cfgfile);
	return 1;
    }

#ifndef	C_WINDOWS
    if((cpt = cfgopt(copt, "LocalSocket"))->enabled) {
	socktype = "UNIX";
	server.sun_family = AF_UNIX;
	strncpy(server.sun_path, cpt->strarg, sizeof(server.sun_path));

	if((sockd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	    logg("^Clamd was NOT notified: Can't create socket endpoint for %s\n", cpt->strarg);
	    perror("socket()");
	    freecfg(copt);
	    return 1;
	}

	if(connect(sockd, (struct sockaddr *) &server, sizeof(struct sockaddr_un)) < 0) {
	    closesocket(sockd);
	    logg("^Clamd was NOT notified: Can't connect to clamd through %s\n", cpt->strarg);
	    perror("connect()");
	    freecfg(copt);
	    return 1;
	}

    } else
#endif
    if((cpt = cfgopt(copt, "TCPSocket"))->enabled) {

	socktype = "TCP";
#ifdef PF_INET
	if((sockd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
#else
	if((sockd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
	    logg("^Clamd was NOT notified: Can't create TCP socket\n");
	    perror("socket()");
	    freecfg(copt);
	    return 1;
	}

	server2.sin_family = AF_INET;
	server2.sin_port = htons(cpt->numarg);

	if((cpt = cfgopt(copt, "TCPAddr"))->enabled) {
	    if ((he = gethostbyname(cpt->strarg)) == 0) {
		perror("gethostbyname()");
		logg("^Clamd was NOT notified: Can't resolve hostname '%s'\n", cpt->strarg);
		freecfg(copt);
		return 1;
	    }
	    server2.sin_addr = *(struct in_addr *) he->h_addr_list[0];
	} else
	    server2.sin_addr.s_addr = inet_addr("127.0.0.1");


	if(connect(sockd, (struct sockaddr *) &server2, sizeof(struct sockaddr_in)) < 0) {
	    closesocket(sockd);
	    logg("^Clamd was NOT notified: Can't connect to clamd on %s:%d\n",
		    inet_ntoa(server2.sin_addr), ntohs(server2.sin_port));
	    perror("connect()");
	    freecfg(copt);
	    return 1;
	}

    } else {
	logg("^Clamd was NOT notified: No socket specified in %s\n", cfgfile);
	freecfg(copt);
	return 1;
    }

    if(send(sockd, "RELOAD", 6, 0) < 0) {
	logg("^Clamd was NOT notified: Could not write to %s socket\n", socktype);
	perror("write()");
	closesocket(sockd);
	freecfg(copt);
	return 1;
    }

    /* TODO: Handle timeout */
    memset(buff, 0, sizeof(buff));
    if((bread = recv(sockd, buff, sizeof(buff), 0)) > 0)
	if(!strstr(buff, "RELOADING")) {
	    logg("^Clamd was NOT notified: Unknown answer from clamd: '%s'\n", buff);
	    closesocket(sockd);
	    freecfg(copt);
	    return 1;
	}

    closesocket(sockd);
    logg("Clamd successfully notified about the update.\n");
    freecfg(copt);
    return 0;
}

#endif
