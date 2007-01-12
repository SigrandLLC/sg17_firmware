#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <sys/file.h>

#define MAX_LINES 1024
#define MAX_LINE_SIZE 1024
// #define MAX_DB_SIZE (1024*32-100)
#define READ_BUFFER_LEN (1024*32)
#define WRITE_BUFFER_LEN READ_BUFFER_LEN
#define HEADER_LINE "KDB\n"
#define true 1
#define false 0
#define COUNT_NAME "kdb_lines_count"
#define DEBUG 1

#define FLOCK(f) {if(flock(fileno(f), LOCK_EX)) perror("flock");};
#define FUNLOCK(f) flock(fileno(f), LOCK_UN);

typedef struct {
	char *name;
	char *value;
} _db_record;

char db_filename[255];
FILE *db_file=NULL;
_db_record db_lines[MAX_LINES];
char db_header[MAX_LINE_SIZE];
int db_lines_count;
int quotation;
int make_local;
char local_str[]="local ";
int export;
int print_count;
int need_write;
int db_already_readed;

void show_usage(char *name)
{
    printf("Usage: %s [OPTIONS] ARG [: ARG] \n", name);
	printf("where  OPTIONS:= l|q|qq|e|c\n");
    printf("       ARG := { set key=value |\n");
    printf("        rm key | del key |\n");
    printf("        isset key |\n");
    printf("        list [key] |ls [key] |\n");
    printf("        slist key | sls key |\n");
    printf("        klist key | kls key |\n");
    printf("        listrm key | lrm key |\n");
    printf("        listadd key_=value | ladd key_=value |\n");
    printf("        rename key_old key_new | rn key_old key_new |\n");
    printf("        create [filename] |\n");
    printf("        import [filename] |\n");
    printf("        edit }\n");
    exit(1);
};

char *get_dbfilename()
{
	if(strlen(db_filename))
		return db_filename;

    if (getenv("KDB"))
        strcpy(db_filename, getenv("KDB"));
    else if(!getuid())
        strcpy(db_filename, "/etc/kdb");
    else 
        sprintf(db_filename, "%s/.kdb", getenv("HOME"));

    return  db_filename;
}

char *set_dbfilename(const char* filename)
{
	strcpy(db_filename, filename);
	return db_filename;
}


int db_open()
{
	db_file = fopen(get_dbfilename(), "r+");
	if (!db_file) {
		fprintf(stderr, "fopen '%s' %s\n", get_dbfilename(), strerror(errno));
		exit(1);
	};
	// lock the file
	FLOCK(db_file);
	return true;
}

int db_close()
{
	FUNLOCK(db_file);
	fclose(db_file);
};

int init()
{
	db_already_readed=0;
    db_lines_count=0;
    quotation=0;
	export=0;
	make_local=0;
	print_count=0;
    need_write=false;
    int i;
	db_filename[0]='\0';
    for(i=0; i<MAX_LINES; i++)
        db_lines[i].name=NULL, db_lines[i].value=NULL;

    return 0;
}

// Parse str "name=value" and put 'name' to *name and 'value' to *value
int parse_pair(const char* str, char* name, char* value)
{
    int len = strlen(str);
    if (!len)
        return false;
    char s[MAX_LINE_SIZE];
    strncpy(s, str, MAX_LINE_SIZE);

    char *eq=strchr(s, '=');
    if (!eq || eq==s) {
        fprintf(stderr, "Error: can't parse: %s", s);
        return false;
    }
    eq[0]='\0';
    strcpy(name, s );
    strcpy(value, eq+1);

/* 
    if (strchr(value, '\n')){
        fprintf(stderr, "Error: value can't contain \\n");
        return false;
    }
*/
    return true;
}

int parse_header()
{
    int result=true;
    char buf[MAX_LINE_SIZE];
    char *size, *crc;
    if (strncmp(HEADER_LINE, db_header, 3)){
        fprintf(stderr, "bad file format\n");
        return false;
    }

    return result;
}

int sort_func(const void *pa, const void *pb){
	_db_record *a=(_db_record*)pa;
	_db_record *b=(_db_record*)pb;
	
	return strcmp(a->name, b->name);
}

int db_sort(){
	qsort(db_lines, db_lines_count, sizeof(char*), sort_func);
}

inline int print_pair(const char* name, const char* value)
{
	char *prefix="";
	if (make_local)
		prefix="local ";
	else if (export)
		prefix="export ";
	if (DEBUG) fprintf(stderr, "DEBUG: print_pair(): %s%s=%s\n", prefix, name, value);
	switch(quotation) {
		case 0: 
			if (name)
				printf("%s%s=%s\n", prefix, name, value);
			else
				printf("%s\n", value);
			break;
		case 1:
			if (name)
				printf("%s%s=\"%s\"\n", prefix, name, value);
			else
				printf("\"%s\"\n", value);
			break;
		default:
			if (name)
				printf("%s%s='%s'\n", prefix, name, value);
			else
				printf("'%s'\n", value);
			break;
	}

    return true;
}

inline int db_add_namevalue(const char* name, const char* value)
{
	db_lines[db_lines_count].name=strdup(name);
	db_lines[db_lines_count].value=strdup(value);
    db_lines_count++;
	return true;
}

int db_add(const char *str)
{
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
	if ( parse_pair(str, name, value) ) 
		return db_add_namevalue(name, value);
	else
		return false;
}

char* db_unserialize(void *buf)
{
	char *pointer=(char*)buf;
	char l_name[MAX_LINE_SIZE], l_value[MAX_LINE_SIZE];
	int i=0;
	if (!pointer[0]) return NULL;
	while (pointer[i])
		l_name[i]=pointer[i++];
	l_name[i]='\0';
	pointer=buf+i+1;

	i=0;
	if (!pointer[0]) return NULL;
	while (pointer[i])
		l_value[i]=pointer[i++];
	l_value[i]='\0';
	pointer=buf+i+1;
	if (DEBUG) fprintf(stderr, " DEBUG: unserialize '%s=%s'\n", l_name, l_value);
	if (db_add_namevalue(l_name, l_value))
		return pointer;
	else 
		return NULL;
}

int db_serialize(int index, void *buf, int *size)
{
	int name_len=strlen(db_lines[index].name);
	int value_len=strlen(db_lines[index].value);
	char *pointer=buf;
    memset(pointer, 0, *size);
	memcpy(pointer, db_lines[index].name, name_len);
	if (DEBUG) fprintf(stderr, " DEBUG: serialize '%s=", pointer);
	pointer+=name_len+1;
	memcpy(pointer, db_lines[index].value, value_len);
	pointer+=value_len+1;
	*size=pointer-(char*)buf; // and 2 bytes of end-zero byte	
	if (DEBUG) fprintf(stderr, "%s'\n", pointer);
	return true;
}


int db_read()
{
    int result=true;
    char *buf=NULL;
	char *pointer=NULL;

	// check if file readed already
	if (db_already_readed)
		return true;

	if (! db_open() )
		return false;

	// read header
	if ( fread(db_header, 1, sizeof(HEADER_LINE), db_file) < sizeof(HEADER_LINE) ) {
		perror("fread");
		return false;
	};

	// check header
    if (! parse_header() )
        return false;

    if (! (buf=malloc(READ_BUFFER_LEN)) ) {
        perror("malloc");
        return false;
    }

	// clean buf
    memset(buf, 0, (READ_BUFFER_LEN));
	size_t readed = fread(buf, 1, READ_BUFFER_LEN-1, db_file);
	if (DEBUG) fprintf(stderr, " DEBUG: readed '%d'\n", readed);
	db_already_readed = true;

	pointer=buf;
	while( pointer = db_unserialize(pointer) ) ;
            
    if (buf)
        free(buf);
    return true;
}

int db_write()
{
    int i, len, written;
    int result=true;
	// sorts the data
	//db_sort();
	rewind(db_file);

	fwrite(HEADER_LINE, 1, sizeof(HEADER_LINE), db_file);

	// write all db_lines
    for(i=0; i<db_lines_count; i++) {
		char buf[MAX_LINE_SIZE*2];
		int size=sizeof(buf);
		db_serialize(i, &buf, &size);
		fwrite(buf, 1, size, db_file);
    };
	
    return result;
}

int find_key(const char *key)
{
    int i;

    if ( !key )
        return false;

    for(i=0; i<db_lines_count; i++) 
        if (!strcmp(key, db_lines[i].name)) 
            return i;

    return -1;
}

int rename(const char *oldkey, const char* newkey)
{
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE], s[MAX_LINE_SIZE];
	db_read();
    int index = find_key(oldkey);
	if ( index == -1 ) 
		return false;
	// release memory
	free(db_lines[index].name);
	db_lines[index].name=strdup(newkey);
	return need_write=true;
}

int get(const char *key)
{
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    db_read();
    int index = find_key(key);
    if (index != -1) {
		print_pair(NULL, db_lines[index].value);
		return true;
	} else
		return false;
}

int isset(const char *key)
{
    db_read();
    return (find_key(key) !=-1);
}

int edit(const char* me)
{
    int result=true;
    char *tmpname=tmpnam(NULL);
    char editor[255];
    char buf[255];
    if (!tmpname)
        return false;
    if (getenv("EDITOR"))
        sprintf(editor, "%s %s", getenv("EDITOR"), tmpname) ;
    else
        sprintf(editor, "vi %s", tmpname);

    
    sprintf(buf, "%s list > %s", me, tmpname);
    if (system(buf))
        return false;
    
    if (! system(editor))  {
        result=db_import(tmpname);
        remove(tmpname);
    }

    return result;
}

int set_namevalue(const char *name, const char* value)
{
    int result=false;
    int index;
    db_read();

    index = find_key(name);
    if (index != -1) {
		free(db_lines[index].value);
        db_lines[index].value = strdup(value);
        result = true;
		if (DEBUG) fprintf(stderr, "DEBUG: replaced to '%s'\n", value);
    } else {
		result=db_add_namevalue(name, value);
    }
    if (result)
        need_write=true;

    return result;
}

int set(const char *str)
{
    char iname[MAX_LINE_SIZE], ivalue[MAX_LINE_SIZE];

    if (!parse_pair(str, iname, ivalue)) 
        return false;

	return set_namevalue(iname, ivalue);

}

int del_index(int index)
{
    if (index != -1) {
		// TODO hm, maybe free(db_lines[index]) or not? unimportantly
        while( index < (db_lines_count-1)){
            db_lines[index]=db_lines[index+1];
            index++;
        }
		if (DEBUG) fprintf(stderr, " DEBUG: deleted db_lines[%d]\n", index-1);
        db_lines_count--;
		need_write=true;
        return true;
    } else {
        return false;
    }

}
int del(const char *key)
{
    char name[MAX_LINE_SIZE];
    db_read();
    int index = find_key(key);
	if ( !del_index(index) ) {
        fprintf(stderr, "%s not found\n", key);
	}
}

int sublist(const char *key)
{
    int result=true;
    int i,count=0;
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    int key_len=strlen(key);

    db_read();
    for(i=0; i<db_lines_count; i++) {
        //parse_pair(db_lines[i], name, value);
        
        if(key && key_len){
            if (!strncmp(key, db_lines[i].name, key_len)) {
                char *s=db_lines[i].name+key_len;
                print_pair(s, value);
				count++;
            }
        } 
    }
	if (print_count) {
		sprintf(value, "%d", count);
		print_pair(COUNT_NAME, value);
	};

    return result;
}

int keylist(const char *key)
{
    int result=true;
    int i,count=0;
    char ls_count[MAX_LINE_SIZE];
    int key_len=strlen(key);

    db_read();
    for(i=0; i<db_lines_count; i++) {
        
        if(key && key_len){
            if (!strncmp(key, db_lines[i].name, key_len))
                print_pair(NULL, db_lines[i].name), count++;
        } else
            print_pair(NULL, db_lines[i].name), count++;
    }
	if (print_count) {
		sprintf(ls_count, "%d", count);
		print_pair(COUNT_NAME, ls_count);
	};

    return result;
}


int listrm(const char *key)
{
    int result=true;
    int i, count=0, index;
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE], prefix_name[MAX_LINE_SIZE], tmps[MAX_LINE_SIZE], new_name[MAX_LINE_SIZE];
	char *s;
	_db_record local_lines[MAX_LINES];
	int local_lines_count=0;
    int key_len=strlen(key);
    db_read();

	s=strrchr(key, '_');
	if(!s) {
		return false;
	};
	strncpy(prefix_name, key, s-key+1);
	if (DEBUG) fprintf(stderr, "DEBUG: prefix_name=%s\n", prefix_name);
	
	// finds all key_[0-9]+ keys, and move it to local_lines array
	i=0;
	while(true) {
		snprintf(tmps, sizeof(tmps), "%s%d", prefix_name, i);
		index=find_key(tmps);
		if ( index >= 0 ) {
			if (DEBUG) fprintf(stderr, "DEBUG: db_lines[%d]='%s' \n", index, db_lines[index]);
			// if index != rm_index - add pair to temp local_lines array
			if ( strcmp(db_lines[index].name, key) ) {
				strcpy(name, db_lines[index].name);
				strcpy(value, db_lines[index].value);
				// finds last '_'
				s=strrchr(name, '_');
				// truncate name after last '_'
				s[1]='\0';
				snprintf(new_name, sizeof(new_name), "%s%d", name, local_lines_count);
				if (DEBUG) fprintf(stderr, "  DEBUG: new_name=%s value=%s \n", new_name, value);
				local_lines[local_lines_count].name=strdup(new_name);
				local_lines[local_lines_count++].value=strdup(value);
				if (DEBUG) fprintf(stderr, "  DEBUG: added to local_lines '%s=%s' \n", local_lines[local_lines_count-1].name, local_lines[local_lines_count-1].value);
			};
			del_index(index);
		} else {
			break;
		};
		i++;
	}
	// yes, we have some keys=values
	if (i) {
		i=0;
		for(i=0; i<local_lines_count; i++) {
			set_namevalue(local_lines[i].name, local_lines[i].value);
			if (local_lines[i].name)
				free(local_lines[i].name), free(local_lines[i].value);
		}
	}
	return result;
}

int listadd(const char* str)
{
    int result=true;
	int i,index;
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE], s[MAX_LINE_SIZE];
    db_read();
	
	parse_pair(str, name, value);
	
	// check for '_' at the end of string
	if ( name[strlen(name)-1] != '_' ) {
		fprintf(stderr, "Error: key should ends with '_'\n");
		return false;
	};
		
	for ( i=0; i < 10000; i++) {
		snprintf(s, sizeof(s), "%s%d", name, i);
		index=find_key(s);
		if (DEBUG) fprintf(stderr, "  DEBUG: s=%s index=%d\n", s, index);
		if ( index >= 0 )
			continue;
		snprintf(s, sizeof(s), "%s%d=%s", name, i, value);
		result=set(s);
		break;
	};

	return result;
}

int list(const char *key)
{
    int result=true;
    int i, count=0;
    int key_len=strlen(key);
    char ls_count[MAX_LINE_SIZE];

    db_read();
    for(i=0; i<db_lines_count; i++) {
        
        if(key && key_len){
            if (!strncmp(key, db_lines[i].name, key_len))
                print_pair(db_lines[i].name, db_lines[i].value), count++;
        } else
            print_pair(db_lines[i].name, db_lines[i].value), count++;
    }
	if (print_count) {
		sprintf(ls_count, "%d", count);
		print_pair(COUNT_NAME, ls_count);
	};

    return result;
}

int db_create(const char *filename)
{
    if (strlen(filename))
		set_dbfilename(filename);

	// create or truncate file
	FILE *f=fopen(get_dbfilename(), "w");
	if (!f) {
		perror("fopen");
		return false;
	}
	fclose(f);

	db_open();
	need_write=true;
   
    return true;
};

int db_import(const char *filename)
{
    FILE *file;
    int result=true;
    char buf[MAX_LINE_SIZE];
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    if (!filename || (!strlen(filename)) || (!strcmp("-", filename)) )
        file=stdin;
    else {
        file = fopen(filename, "r+");
        if (!file) {
            perror("fopen");
            result=false;
        };
    };
    if (result) {
        while( fgets(buf, MAX_LINE_SIZE, file) ) {
            if (ferror(file)) {
                result=false;
                break;
            }
            int len=strlen(buf);
            if (len && buf[len-1]=='\n')
                buf[len-1]='\0';
            if(parse_pair(buf, name, value) && (strcmp(name, COUNT_NAME)))
                db_add(buf);
            else {
                result=false;
                break;
            };
        };
    };
    fclose(file);
	db_open();
    if (result) 
        need_write=true;

    return result;
}

int main(int argc, char **argv)
{
    int ch, i;
    int result=false;
    char cmd[MAX_LINE_SIZE];
    char param[MAX_LINE_SIZE];

    init();

    while ((ch = getopt(argc, argv, "qlecf:")) != -1){
        switch(ch) {
            case 'f': set_dbfilename(optarg);
                     break;
            case 'q': quotation++;
                     break;
            case 'l': make_local++;
                     break;
            case 'e': export++;
                     break;
            case 'c': print_count++;
                     break;
            default: show_usage(argv[0]);
                     break;
        }
    }

    if ( argc <= optind ) 
        show_usage(argv[0]);

	
	/*fprintf(stderr, "argc=%d, optind=%d\n", argc, optind);
	for ( i=0; i<argc; i++)
		fprintf(stderr, "  argv[%d]=%s\n", i, argv[i]); */
		
    
	while(true) {
		if ( optind >= argc ) 
			break;

		strcpy(cmd, argv[optind]);
		optind++;
		// if cmd == ':' then go to next cmd
		if ( cmd[0]==':' )
			continue;


		if ( (optind >= argc) || (strcmp(":", argv[optind])==0)) {
			strcpy(param, "");
		} else {
			strcpy(param, argv[optind++]);
		}

		if ( (!strcmp(cmd, "list")) || (!strcmp(cmd, "ls")) )
			result = list(param);
		else if (!strcmp(cmd, "set"))
			result = set(param);
		else if (!strcmp(cmd, "get"))
			result = get(param);
		else if ( (!strcmp(cmd, "sls")) || (!strcmp(cmd, "slist")) || (!strcmp(cmd, "sublist")) )
			result = sublist(param);
		else if ( (!strcmp(cmd, "listrm")) || (!strcmp(cmd, "lrm")) )
			result = listrm(param);
		else if ( (!strcmp(cmd, "listadd")) || (!strcmp(cmd, "ladd")) )
			result = listadd(param);
		else if (!strcmp(cmd, "isset"))
			result = isset(param);
		else if ( (!strcmp(cmd, "klist")) || (!strcmp(cmd, "kls")) )
			result = keylist(param);
		else if ( (!strcmp(cmd, "del")) || (!strcmp(cmd, "rm")) )
			result = del(param);
		else if ( !strcmp(cmd, "edit") )
			result = edit(argv[0]);
		else if ( !strcmp(cmd, "rename") )
			result = rename(param, argv[optind++]);
		else if ( !strcmp(cmd, "create") )
			result = db_create(param);
		else if ( !strcmp(cmd, "import") )
			result = db_import(param);
		else 
			show_usage(argv[0]);
		if (!result)
			break;
		optind++;

	}
    if (result && need_write)
		db_write();
	db_close();
	return !result;
}
