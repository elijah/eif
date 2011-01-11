/*************************************************************
 *  $Id: data.h,v 1.4 2003/10/09 17:11:18 marcolz Exp $
 *
 *  data.h
 *
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1991.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/

#ifndef	INC_EIF_DATA_H
#define	INC_EIF_DATA_H

#ifdef MAIN
#define vext
#define initint(name, val) int name = (val)
#define initany(type, name, val) type name = (val)
#else
#define vext extern
#define initint(name, val) extern int name
#define initany(type, name, val)  extern type name
#endif

initint(curout_isapipe, 0);
extern FILE	*curout;

/* The global flags */
vext control_struct	cntl;

/* The current game */
initint(game_number, -1);	/* The number of the current game */
initint(game_socket, -1);	/* Socket of the current game */
initint(game_time, 0);		/* Connect time of current game */
initint(game_btus, 0);		/* BTU's of current game */
vext char game_name[40];	/* Name of current game */
vext char game_host[128];
vext char game_port[8];
vext char game_country[20];
vext char game_rep[128];
initint(game_wait, 120);	/* Seconds to wait in read for data */
vext int game_xsize, game_ysize;
vext int game_protocol;
vext char game_dataf[128];


vext int Interrupt;		/* True if we got a sig_int */
vext int Pipe_Interrupt;	/* True if we got a sig_pipe */


#undef vext

#endif /* INC_EIF_DATA_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
