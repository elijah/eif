/*************************************************************
 *  $Id: cntl.h,v 1.3 2003/10/07 21:56:24 marcolz Exp $
 *
 *  cntl.h
 *
 *  This file describes some general data structures.
 *
 *  See init.c for initial values.
 *
 */
/*******************************************************
 *  Copyright (C) Doug Hay, 1989,90,91.
 *  Permission to use and abuse this code, as long
 *  as this copyright notice stays intact and with the
 *  code.  No warranty implied.  This code supplied as is.
 *******************************************************/

#ifndef INC_EIF_CNTL_H
#define INC_EIF_CNTL_H

typedef unsigned char flag_t;

/**************************
 * Startup options.
 */
typedef struct {
	flag_t	readstartup;
	char	startupf[40];
} startup_opt_struct;

/**************************
 * User info from passwd file.
 */
typedef struct {
	int	uid;
	char	*name;
	char	*homedir;
	char	*shell;
} user_info_struct;

/**************************
 *  The global control structure.
 *
 *  Where all the global control flags live.
 *
 *  See init.c for default values.
 */
typedef struct {
	char	*pname;		/* Program name as passed in arg0 */
	int	argc;
	char	**argv;

	int  pid;

	startup_opt_struct st;
	user_info_struct usr;
} control_struct;

#endif /* INC_EIF_CNTL_H */

/* vim:ts=8:ai:sw=8:syntax=c
 */
