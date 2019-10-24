/******************************************************************************/
/*                                                                            */
/* This document contains confidential and proprietary information.           */
/* Copyright (C) 2008 by Scully Signal Systems. All rights reserved.          */
/*                                                                            */
/******************************************************************************
/*                                                                       */
/*  Author: Ken Langlais      July 2, 2008                               */
/*                                                                       */
/*  Revision history:                                                    */
/*    July 2, 2008   creation                                            */
/*                                                                       */
/*************************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

FILE *build_date_file; 					/* download File Handle and Status. */


main()
{
struct tm *newtime;
char am_pm[] = "AM";
time_t long_time;


    /* Display operating system-style date and time. */
//    _strtime( time_buf );

//    _strdate( date_buf );

    time( &long_time );                /* Get time as long integer. */
    newtime = localtime( &long_time ); /* Convert to local time. */

  if ((build_date_file = fopen("..\\Tools\\build_date.c","w")) == NULL)
	 {
    printf("Unable to open ..\\Tools\\build_date.c\n\r");
	    exit(1);
	 }
 // fprintf(build_date_file,"#include \"common.h\"\n\r");
  //fprintf(build_date_file,"#include <stdio.h>\n\r");
 // fprintf(build_date_file,"#include <string.h>\n\r");

//	fprintf(build_date_file,"int dprintf(const char *format, ...);\n\r");
	if ( newtime->tm_hour == 0 )
	{
		newtime->tm_hour = 12;
	} else
	{
		if( newtime->tm_hour >= 12 )        /* Set up extension. */
    {
      strcpy( am_pm, "PM" );
      if ( newtime->tm_hour >= 13)  /* Change Military time to standard time */
      {
        newtime->tm_hour -= 12;
      }
    }
  }
	if( newtime->tm_hour > 12 )        /* Convert from 24-hour */
            newtime->tm_hour -= 12;    /*   to 12-hour clock.  */
    if( newtime->tm_hour == 0 )        /*Set hour to 12 if midnight. */
            newtime->tm_hour = 12;
  fprintf(build_date_file,"char b_date[] = \"%.7s %02d, %04d %02d:%02d:%02d %s\";\n",
    asctime( newtime ), newtime->tm_mday, newtime->tm_year+1900, newtime->tm_hour, newtime->tm_min, newtime->tm_sec,am_pm);
//	fprintf(build_date_file,"char b_time[] = \"%d:%02d%s\";\n",
//		newtime->tm_hour, newtime->tm_min, am_pm);
// fprintf(build_date_file,"void build_date(void)");
// fprintf(build_date_file,"\n{\n");




//    fprintf(build_date_file,"\tprintf(\"Built on %02d/%02d/%d: %d:%02d%s\");",
//    newtime->tm_mon+1, newtime->tm_mday, newtime->tm_year+1900,
//   newtime->tm_hour, newtime->tm_min, am_pm );
//	fprintf(build_date_file,"\tprintf(\"Built on %s %s\");",date_buf,time_buf);
//  fprintf(build_date_file,"\n}\n\n");

	fclose(build_date_file);
	exit(0);
}
