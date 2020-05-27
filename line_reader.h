
typedef struct _line_reader_scope
{
   char *buffer;
   char *buffer_end;

   char *data_end;
   char *line_ptr;

   int  eof;

   int  file_handle;
} LRScope;

int lr_init_scope(LRScope *scope, char *buffer, int buffsize, int file_handle);
int lr_get_line(LRScope *scope, const char** line, const char** line_end);
