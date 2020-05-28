// -*- compile-command: "cc -Wall -Werror -ggdb -DPSYSLOG_MAIN -o psyslog psyslog.c" -*-

#include <ctype.h>   // for tolower()
#include <string.h>  // for strcmp()
#include <stdio.h>   // for printf, sprintf
#include <stdlib.h>  // for atoi()
#include <assert.h>

#include "psyslog.h"

const char* month_names[] = {
   "jan", "feb", "mar", "apr",
   "may", "jun", "jul", "aug",
   "sep", "oct", "nov", "dec"
};

const char **month_names_end = &month_names[12];

int get_month_number(char *str)
{
   assert(strlen(str)==3);

   // avoid strcasecmp by using lower case:
   for (int i=0; i<3; ++i)
      str[i] = (char)tolower((unsigned char)str[i]);

   const char **mptr = month_names;

   while (mptr < month_names_end)
   {
      if (! strcmp(str, *mptr))
         return mptr - month_names;
      ++mptr;
   }

   return -1;
}

/**
 * @param str should already be a copy of the syslog string, so we can change parts.
 */
int parse_timestamp(SLDate *sldate, char *str)
{
   memset(sldate, 0 , sizeof(SLDate));

   int  scr_int;
   char scr_str[4];

   char *start = str;
   char *end = start + 3;
   *end = '\0';

   // Month is unique, do individually:
   scr_int = get_month_number(start);
   sprintf(scr_str, "%d", scr_int + 101);
   memcpy(sldate->month, &scr_str[1], 2);

   // Parse each 2-digit number, separated by one character
   // (either a space or a colon):
   typedef char twochar[2];

   twochar *part_ptr = &sldate->day;
   twochar *part_end = part_ptr + 4;

   while (part_ptr < part_end)
   {
      start = end + 1;
      end = start + 2;
      *end = '\0';
      scr_int = atoi(start);
      sprintf(scr_str, "%d", scr_int + 100);
      memcpy(part_ptr, &scr_str[1], 2);

      ++part_ptr;
   }

   return end + 1 - str;
}

int syslog_parse_line(const char *str, syslog_use_parsed sup)
{
   // Copy string for processing
   int len = strlen(str);

   if (len > 32)
   {
      char *dup = (char*)alloca(len+1);
      strcpy(dup, str);
      char *ptr = dup;

      SLDate sldate;
      const char *host = "";
      const char *tag = "";
      const char *msg = "";

      ptr += parse_timestamp(&sldate, ptr);
      host = ptr;

      ptr = strchr(host, ' ');
      if (ptr)
      {
         *ptr++ = '\0';
         tag = ptr;

         ptr = strchr(tag, ':');
         if (ptr)
         {
            *ptr++ = '\0';
            msg = ptr + 1;
         
            (*sup)(&sldate, host, tag, msg);
            return 1;
         }
      }
   }

   return 0;
}


#ifdef PSYSLOG_MAIN

const char *test_line = "May 20 11:27:25 firewall kernel: [6273161.612754] IPT DROP SPOOFED INPUT IN=lan OUT= MAC=01:00:5e:00:00:01:28:c6:8e:8d:d6:96:08:00 SRC=0.0.0.0 DST=224.0.0.1 LEN=32 TOS=0x00 PREC=0xC0 TTL=1 ID=0 DF PROTO=2";

int use_parsed(SLDate *date, const char *host, const char *tag, const char *msg)
{
   printf("In [32;1muse_parsed[m()\n");

   printf("date is [32;1m%s[m.\n", (const char *)date);
   printf("host is [32;1m%s[m.\n", host);
   printf("tag is [32;1m%s[m.\n", tag);
   printf("message is [32;1m%s[m.\n", msg);

   return 1;
}

void test_get_month_number(void)
{
   char may[] = "may";
   char april[] = "apr";
   char december[] = "dec";

   int val_may = get_month_number(may);
   int val_april = get_month_number(april);
   int val_december = get_month_number(december);

   printf("val_may = %d\nval_april = %d\nval_december = %d\n",
          val_may, val_april, val_december);
}

void test_syslog_parse_line(void)
{
   syslog_parse_line(test_line, use_parsed);
}

int main(int argc, const char **argv)
{
   /* test_get_month_number(); */
   test_syslog_parse_line();

   return 0;
}


#endif
