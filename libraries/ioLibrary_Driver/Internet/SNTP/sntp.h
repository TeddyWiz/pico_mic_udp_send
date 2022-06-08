/*
 * sntp.h
 *
 *  Created on: 2014. 12. 15.
 *      Author: Administrator
 */

#ifndef SNTP_H_
#define SNTP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * @brief Define it for Debug & Monitor DNS processing.
 * @note If defined, it dependens on <stdio.h>
 */
//#define _SNTP_DEBUG_

#define	MAX_SNTP_BUF_SIZE	sizeof(ntpformat)		///< maximum size of DNS buffer. */

/* Time Base Conversion Macros
 *
 * The NTP timebase is 00:00 Jan 1 1900.  The local
 * time base is 00:00 Jan 1 1970.  Convert between
 * these two by added or substracting 70 years
 * worth of time.  Note that 17 of these years were
 * leap years.
 */
#define TIME_BASEDIFF           ((((uint64_t)70*365 + 17) * 24*3600))
#define TIME_NTP_TO_LOCAL(t)	((t)-TIME_BASEDIFF)
#define TIME_LOCAL_TO_NTP(t)	((t)+TIME_BASEDIFF)

/* for ntpclient */
typedef signed char s_char;
typedef unsigned long long tstamp;
typedef unsigned int tdist;

typedef struct _ntpformat
{

	uint8_t  dstaddr[4];        /* destination (local) address */
	char    version;        /* version number */
	char    leap;           /* leap indicator */
	char    mode;           /* mode */
	char    stratum;        /* stratum */
	char    poll;           /* poll interval */
	s_char  precision;      /* precision */
	tdist   rootdelay;      /* root delay */
	tdist   rootdisp;       /* root dispersion */
	char    refid;          /* reference ID */
	tstamp  reftime;        /* reference time */
	tstamp  org;            /* origin timestamp */
	tstamp  rec;            /* receive timestamp */
	tstamp  xmt;            /* transmit timestamp */


} ntpformat;

typedef struct _datetime
{
	uint16_t yy;
	uint8_t mo;
	uint8_t dd;
	uint8_t hh;
	uint8_t mm;
	uint8_t ss;
} datetime;

#define ntp_port		123                     //ntp server port number
#define SECS_PERDAY		86400UL             	// seconds in a day = 60*60*24
#define UTC_ADJ_HRS		9              	        // SEOUL : GMT+9
#define EPOCH			1900                    // NTP start year

void get_seconds_from_ntp_server(uint8_t *buf, uint16_t idx);
void SNTP_init(uint8_t s, uint8_t *ntp_server, uint8_t tz, uint8_t *buf);
int8_t SNTP_run(datetime *time);
tstamp changedatetime_to_seconds(void);
void calcdatetime(tstamp seconds);

/* User-defined function */
/*
 * @brief Get current timestamp by SNTP sever
 * @return    The value is as the follow
 *            @ref 0 - Failed to get current timestamp
 *            @timestamp - Succeeded to get current timestamp
 */
int64_t SNTP_get_current_timestamp(void);

#ifdef __cplusplus
}
#endif

#endif /* SNTP_H_ */
