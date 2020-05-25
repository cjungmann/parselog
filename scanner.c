#include <stdio.h>
#include <clargs.h>

// For low-level file access
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// for read(), etc.
#include <unistd.h>

// for memmove(), strstr()
#include <string.h>

#include <alloca.h>

typedef void (*string_processor_t)(const char *str);


// Line processor prototypes:
string_processor_t process_string = NULL;
void save_ipt_lines(const char* str);
void test_line_reader(const char *str);

// Clargs callbacks for setting line processor
void set_ipt_line_reader(const DefLine *option, const char *value)
{
   process_string = save_ipt_lines;
}

void set_test_line_reader(const DefLine *option, const char *value)
{
   process_string = test_line_reader;
}


// Prepare clargs target variables:
const char *Syslog="/var/log/syslog";
int flagHelp = 0;
unsigned int BuffSize = 16384;

DefLine defs[] = {
   { 'h', "Show help", &flagHelp, NULL },
   { 'b', "Change buffer size.",  &BuffSize, clargs_set_int },
   { 'f', "Set file to process.", &Syslog,   clargs_set_string },
   { 't', "Use test line reader", NULL,      set_test_line_reader },
   { 'i', "Use ipt line reader",  NULL,      set_ipt_line_reader },
   CLARGS_END_DEF
};

// Return pointer to next line if end-of-string is detecged
char* terminate_string(char *str, const char *end)
{
   char *ptr = str;

   while (ptr < end)
   {
      if (*ptr == '\r')
         *ptr++ = '\0';

      if (*ptr == '\n')
      {
         *ptr = '\0';
         return ptr+1;
      }

      ++ptr;
   }

   return NULL;
}

int read_lines(int file_handle)
{
   char *buff = alloca(BuffSize);
   char *str, *nextstr, *end_of_data;
   const char *end = &buff[BuffSize];

   size_t bytes_to_read;
   ssize_t bytes_read;

   // Prepare for first time through loop
   end_of_data = buff;
   bytes_to_read = BuffSize;

   while ((bytes_read = read(file_handle, end_of_data, bytes_to_read)))
   {
      if (bytes_read < bytes_to_read)
         end_of_data[bytes_read] = '\0';

      end_of_data += bytes_read;
      str = buff;

      while (str < end_of_data)
      {
         if ((nextstr = terminate_string(str, end)))
         {
            (*process_string)(str);
            str = nextstr;
         }
         else
         {
            int remaining_data = end_of_data - str;

            if (remaining_data < BuffSize)
            {
               memmove(buff, str, remaining_data );
               end_of_data = buff + remaining_data;
               bytes_to_read = end - end_of_data;
            }
            else
            {
               buff[BuffSize-1] = '\0';
               fprintf(stderr,
                       "Buffer too small (%u bytes) to contain a complete line.\n[32;1m%s[m.\n",
                       BuffSize, buff);

               return 1;
            }

            break;
         }  
      }
   }

   // Process the final line that remains in the buffer.
   (*process_string)(str);

   return 0;
}


int read_syslog(const char *filepath)
{
   int file_handle = open(filepath, O_RDONLY);
   if (file_handle > -1)
   {
      read_lines(file_handle);
      
      close(file_handle);
      return 0;
   }

   return 1;
}

/**
 * Alternate line processors follow:
 */

void save_ipt_line(const char *str)
{
   const char *ptr = str;
   while (*ptr)
   {
      ++ptr;
   }
}

void save_ipt_lines(const char* str)
{
   if (strstr(str, " IPT ") != NULL)
   {
      save_ipt_line(str);
   }
}

void test_line_reader(const char *str)
{
   printf("%s\n", str);
}

/**
 * CL arguments response functions
 */


int main(int argc, const char **argv)
{
   clargs_process( defs, argc, argv );
   if ( flagHelp )
   {
      clargs_show(defs);
      return 0;
   }

   if (!process_string)
      process_string = save_ipt_lines;

   printf("About to process %s.\n", Syslog);
   read_syslog(Syslog);

   return 0;
}
