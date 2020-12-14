#pragma once

struct tms
{
  clock_t tms_utime;		/* User CPU time.  */
  clock_t tms_stime;		/* System CPU time.  */

  clock_t tms_cutime;		/* User CPU time of dead children.  */
  clock_t tms_cstime;		/* System CPU time of dead children.  */
};


/* Store the CPU time used by this process and all its
   dead children (and their dead children) in BUFFER.
   Return the elapsed real time, or (clock_t) -1 for errors.
   All times are in CLK_TCKths of a second.  */
extern clock_t times (struct tms *__buffer);
