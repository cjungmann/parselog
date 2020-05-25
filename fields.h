
typedef struct _field_item
{
   const char  *name;
   int         index;

   struct _field_item *next;
} FItem;

void field_init(FItem *item);
FItem* field_seek(FItem *list, const char *name);
FItem* field_add(FItem *list, const char *name);
FItem* field_manage(FItem *list, const char *name);
void fields_delete(FItem *item);

FItem* init_field_names(const char *str);
