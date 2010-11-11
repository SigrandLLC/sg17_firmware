char *str_escape(const char *source);
char *str_unescape(const char *source);
int match_wildcard(const char *pattern, const char *string);
int is_wildcarded(const char* str);
int copyfile(const char *, const char *);
char *get_dbfilename();
char *set_dbfilename(const char* filename);
int db_open();
int db_close();
int kdbinit();
inline int db_add(const char* name, const char* value, int dup);
// Parse str "name=value" and put 'name' to *name and 'value' to *value
int parse_pair(const char* str, char* name, char* value);
int parse_header(char *header);
int db_unserialize(const char *buf);
int db_serialize(int index, char *buf);
int db_read();
int sort_func(const void *pa, const void *pb);
int db_sort();
inline int print_pair(const char* name, const char* value);
int db_set_index(int index, const char *name, const char *value, int dup);
int find_key(const char *key);
int db_set(const char *name, const char* value);
int db_del_index(int index);
/*
	md5 = 1 — generate MD5 file
	md5 = 0 — do not generate MD5 file
*/
int db_write(int md5);
void print_count(int count);
int import(const char *filename);
int export(const char *filename);
int edit(const char* me);
int rename(const char *oldkey, const char* newkey);
int get(const char *key);
int get1(const char *key);
int isset(const char *key);
int set(const char *str);
// delete with wildcard matching
int del(const char *str);
int sublist(const char *key);
// keylist with wildcard matching
int keylist(const char *key);
int kdbcmd(int argc, char **argv);
