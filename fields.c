// -*- compile-command: "cc -Wall -Werror -ggdb -DFIELDS_MAIN -o fields fields.c" -*-
#include "fields.h"
#include <string.h>
#include <stdlib.h>

void field_init(FItem *item)
{
   memset(item, 0, sizeof(FItem));
}

FItem *field_seek(FItem *list, const char *name)
{
   FItem *ptr = list;
   while (ptr)
   {
      if ( ! strcmp(name, ptr->name) )
         return ptr;

      ptr = ptr->next;
   }

   return NULL;
}

FItem *field_add(FItem *list, const char *name)
{
   FItem *newitem;
   int nlen = strlen(name);
   char *newname = (char*)malloc(nlen+1);
   if (newname)
   {
      newitem = (FItem *)malloc(sizeof(FItem));
      if (!newitem)
      {
         free(newname);
      }
      else
      {
         field_init(newitem);

         strcpy(newname, name);
         newitem->name = newname;

         if (list)
         {
            FItem *ptr = list;
            while (ptr->next)
               ptr = ptr->next;

            newitem->index = ptr->index + 1;
            ptr->next = newitem;
         }

         return newitem;
      }
   }

   return NULL;
}

FItem *field_manage(FItem *list, const char *name)
{
   FItem *retval = field_seek(list, name);
   if (!retval)
      retval = field_add(list, name);

   return retval;
}

void fields_delete(FItem *item)
{
   FItem *ptr = item;

   while (ptr)
   {
      FItem *next = ptr->next;
      if (ptr->name)
         free((void*)ptr->name);
      free((void*)ptr);
      ptr = next;
   }
}


#ifdef FIELDS_MAIN
int main(int argc, const char **argv)
{
   FItem *root = NULL;

   root = field_manage(root, "Date");
   field_manage(root, "Manage");
   field_manage(root, "IN");

   fields_delete(root);
      
   
   return 0;
}

#endif
