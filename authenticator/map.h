typedef struct _map_entry {
  char *key;
  char *value;
} map_entry;

typedef struct _bucket {
  map_entry entry;
  struct _bucket *next;
} bucket;

typedef struct _bucket_list {
  bucket *begin;
} bucket_list;

typedef struct _map {
  int capacity;
  int size;
  bucket_list *table;
} map;

map *map_new(int capacity);
void map_add(map *m, char *key, char *value);
char *map_get(map *m, char *key);
void map_destroy(map *m);
void map_display(map *m);
