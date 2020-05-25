// -*- compile-command: "cc -Wall -Werror -ggdb -DFIELDS_MAIN -o fields fields.c" -*-
#include "fields.h"
#include <string.h>
#include <stdlib.h>

void field_init(FItem *item)
{
   memset(item, 0, sizeof(FItem));
}

FItem* field_seek(FItem *list, const char *name)
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

FItem* field_add(FItem *list, const char *name)
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

/**
 * Return an existing field name, or add a new field
 * name and return it.
 *
 * I hate this function name, but I already used
 * field_add(), and I can't think of a better name.
 */
FItem* field_manage(FItem *list, const char *name)
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

/**
 * Initialize a new FItem chain with a comma-separated
 * list of names.
 */
FItem *init_field_names(const char *str)
{
   FItem *root = NULL;

   // Make non-const copy of the string:
   char *dupstr = (char*)alloca(1+strlen(str));
   strcpy(dupstr, str);

   char *ptr = dupstr;
   int ndx;

   while (*ptr)
   {
      ndx = strcspn(ptr, ",");
      if ( ptr[ndx] == ',' )
         ptr[ndx] = '\0';
      else
         // Last element, prepare ndx so increment points to original end-of-string:
         --ndx;

      if (root)
         field_manage(root, ptr);
      else
         root = field_manage(NULL, ptr);

      ptr += ndx+1;
   }

   return root;
}


#ifdef FIELDS_MAIN

#include <stdio.h>

void display_field_names(FItem *root)
{
   while (root)
   {
      printf("%d: %s\n", root->index, root->name);
      root = root->next;
   }

   printf("\n");
}

int main(int argc, const char **argv)
{
   FItem *root = NULL;

   printf("Populate fields with individual field names.\n");
   root = field_manage(root, "Date");
   field_manage(root, "Manage");
   field_manage(root, "IN");
   display_field_names(root);

   fields_delete(root);
   root = NULL;

   printf("Populate fields with a CSV fields list.\n");
   root = init_field_names("one,two,three,four,five,six");
   display_field_names(root);

   printf("Add a couple more field names.\n");
   field_manage(root, "seven");
   field_manage(root, "eight");
   display_field_names(root);

   return 0;
}

#endif
