typedef struct _syslog_date {
   char month[2];
   char day[2];
   char hour[2];
   char minute[2];
   char second[2];
   char eos;
} SLDate;

typedef int (*syslog_use_parsed)(SLDate *date, const char *host, const char *tag, const char *msg);

int syslog_parse_line(const char *str, syslog_use_parsed sup);

int syslog_break_line(char *str,
                      SLDate *sldate,
                      char **host,
                      char **tag,
                      char **msg);
