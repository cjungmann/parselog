#include <stdio.h>
#include <clargs.h>

#include "psyslog.h"
#include "line_reader.h"
#include "fields.h"


// for memmove(), strstr()
#include <string.h>

#include <alloca.h>   // for alloca
#include <stdlib.h>   // for malloc/free

#include <time.h>     // for clock() for timing alternative run paths


typedef struct _text_link
{
   char *line;
   struct _text_link *next;
} TLink;

typedef void(*text_link_deleter_t)(TLink *link);

void delete_text_chain_ordered(TLink *link)
{
   TLink *next;
   while (link)
   {
      next = link->next;

      free(link->line);
      free(link);

      link = next;
   }
}

void delete_text_chain_recursive(TLink *link)
{
   if (link)
   {
      if (link->next)
         delete_text_chain_recursive(link->next);

      free(link->line);
      free(link);
   }
}

void tlink_nonfreer(TLink* link) { ; }

text_link_deleter_t text_link_deleter = NULL;




typedef struct _log_link
{
   SLDate sldate;
   char   *host;
   char   *tag;
   char   *msg;

   struct _log_link *next;
} LLink;

typedef void(*log_link_deleter_t)(LLink *link);

void free_log_link(LLink *link)
{
   free(link->host);
   free(link->tag);
   free(link->msg);
   free(link);
}

void delete_log_chain_ordered(LLink *link)
{
   LLink *next;
   while (link)
   {
      next = link->next;
      free_log_link(link);

      link = next;
   }
}

void delete_log_chain_recursive(LLink *link)
{
   if (link)
   {
      if (link->next)
         delete_log_chain_recursive(link->next);

      free_log_link(link);
   }
}

void llink_nonfreer(LLink* link) { ; }

log_link_deleter_t log_link_deleter = NULL;

typedef void (*memfreer_t)(void*);
memfreer_t memfreer = NULL;
void noopfree(void* p) { ; }

// Prepare clargs target variables:
const char*  Syslog="/var/log/syslog";
int          ShowHelp = 0;
unsigned int BuffSize = 16384;
unsigned int LineBuffSize = 512;
int          PlainText = 0;       // File processor flag
int          StackSave = 0;       // Line save method
int          RecursiveDelete = 0; // 
int          MemoryRepeats = 1;
const char*  FieldNames = NULL;

#define getmem(C) ( StackSave ? alloca((C)) : malloc((C)) )

DefLine defs[] = {
   { 'h', "Show help",                          &ShowHelp,      NULL },
   { 'b', "Change line-reader buffer size.",    &BuffSize,      clargs_set_int },
   { 'l', "Change duplicate line buffer size.", &LineBuffSize,  clargs_set_int }, 
   { 'p', "Set file to process.",               &Syslog,        clargs_set_string },
   { 't', "File is plain text.",                &PlainText,     NULL },
   { 'r', "Set repeat count.",                  &MemoryRepeats, clargs_set_int },
   { 's', "Save lines in stack.",               &StackSave,     NULL },
   /* { 'f', "Comma-separated field names", &FieldNames, clargs_set_string }, */
   CLARGS_END_DEF
};

typedef void (*file_processor_t)(LRScope *scope);
file_processor_t file_processor = NULL;

// Four possible processors, Text or Log file,
// saving lines to Heap or Stack

void text_file_processor(LRScope *scope)
{
   TLink *new, *tail = NULL, *root = NULL;
   char *newline;
   int linelen;

   const char *line, *line_end;
   while (lr_get_line(scope, &line, &line_end))
   {
      linelen = line_end - line;
      newline = (char*)getmem( 1 + linelen );
      if (newline)
      {
         memcpy(newline, line, linelen);

         new = (TLink*)getmem(sizeof(TLink));
         if (new)
         {
            memset(new, 0, sizeof(TLink));
            new->line = newline;

            if (tail)
            {
               tail->next = new;
               tail = new;
            }
            else
               root = tail = new;
         }
         else
         {
            fprintf(stderr, "Out of memory; abandoning the loop\n");
            (*memfreer)(newline);
            break;
         }
      }
      else
      {
         fprintf(stderr, "Out of memory; abandoning the loop\n");
         break;
      }
   }

   (*text_link_deleter)(root);
}

void log_file_processor(LRScope *scope)
{
   // for now...
   LLink *new, *tail = NULL, *root = NULL;

   const char *line, *line_end;
   char *dupline = (char*)getmem(LineBuffSize);
   
   int linelen;

   SLDate sldate;
   char *host, *tag, *msg;

   while (lr_get_line(scope, &line, &line_end))
   {
      new = NULL;

      // Make non-const copy of line in which
      // characters can be replaced with '\0'.
      linelen = line_end - line;

      if (linelen > LineBuffSize)
         goto line_too_long;

      memcpy(dupline, line, linelen);
      dupline[linelen] = '\0';

      if (syslog_break_line(dupline, &sldate, &host, &tag, &msg))
      {
         new = (LLink*)getmem(sizeof(LLink));
         if (new)
         {
            memset(new, 0, sizeof(LLink));
            memcpy(&new->sldate, &sldate, sizeof(SLDate));

            linelen = strlen(host) + 1;
            new->host = (char*)getmem(linelen);
            if (new->host)
            {
               memcpy(new->host, host, linelen);

               linelen = strlen(tag) + 1;
               new->tag = (char*)getmem(linelen);
               if (new->tag)
               {
                  memcpy(new->tag, tag, linelen);

                  linelen = strlen(msg) + 1;
                  new->msg = (char*)getmem(linelen);
                  if (new->msg)
                  {
                     memcpy(new->msg, msg, linelen);

                     if (tail)
                     {
                        tail->next = new;
                        tail = new;
                     }
                     else
                        root = tail = new;
                  }
                  else
                     goto memfailure;  // for new->msg
               }
               else
                  goto memfailure;     // for mew->tag
            }
            else
               goto memfailure;        // for new->host
         }
         else
            goto memfailure;           // for new (LLink*)
      }
   }

   goto cleanup;

  line_too_long:
   fprintf(stderr, "linelen %d is too long for line buffer %d.\n", linelen, LineBuffSize);
   goto cleanup;

  memfailure:
   fprintf(stderr, "Out of memory, abandoning the loop.\n");
   if (new)
      (*log_link_deleter)(new);

  cleanup:

   (*log_link_deleter)(root);

   // Delete worker 
   (*memfreer)(dupline);
}



// Picks the appropriate processor according to user selections:
void set_processors(void)
{
   // Set memory allocator
   if (StackSave)
      memfreer = noopfree;
   else
      memfreer = free;

   // Set memory freer (or not)
   if (PlainText)
   {
      file_processor = text_file_processor;
      if (StackSave)
         text_link_deleter = tlink_nonfreer;
      else
         text_link_deleter = RecursiveDelete ? delete_text_chain_ordered : delete_text_chain_recursive;
   }
   else
   {
      file_processor = log_file_processor;
      if (StackSave)
         log_link_deleter = llink_nonfreer;
      else
      log_link_deleter = RecursiveDelete ? delete_log_chain_ordered : delete_log_chain_recursive;
   }
}


int read_file(const char *filepath)
{
   int file_handle = open(filepath, O_RDONLY);
   if (file_handle > -1)
   {
      LRScope scope;
      char *buffer = (char*)alloca(BuffSize);

      clock_t start, end;
      int repeats = 0;

      start = clock();

      while ( repeats++ < MemoryRepeats )
      {
         lseek( file_handle, 0, SEEK_SET);

         if (lr_init_scope(&scope, buffer, BuffSize, file_handle))
         {
            const char *line, *line_end;
            while (lr_get_line(&scope, &line, &line_end))
               (*file_processor)(&scope);
         }
      }

      end = clock();
      
      close(file_handle);

      printf("%ld clock ticks, %f seconds to run %d iterations.\n",
             end - start,
             (double)(end - start) / CLOCKS_PER_SEC,
             MemoryRepeats);

      return 0;
   }

   return 1;
}


int main(int argc, const char **argv)
{
   clargs_process( defs, argc, argv );
   if ( ShowHelp )
   {
      clargs_show(defs);
      return 0;
   }

   /* FItem *fields = NULL; */

   /* if ( FieldNames ) */
   /*    fields = init_field_names(FieldNames); */

   set_processors();

   printf("About to process %s.\n", Syslog);
   read_file(Syslog);

   return 0;
}
