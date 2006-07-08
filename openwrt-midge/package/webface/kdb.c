#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>
#include <string.h>
#include <sys/file.h>

#define MAX_LINES 1024
#define MAX_LINE_SIZE 1024
#define MAX_DB_SIZE (1024*32-100)
#define HEADER_LINE "KDB"
#define true 1
#define false 0

char db_filename[255];
FILE *db_file=NULL;
char *db_lines[MAX_LINES];
char db_header[MAX_LINE_SIZE];
int db_lines_count;
int db_size;
int quotation;
int export;
int need_write;

void show_usage(char *name)
{
    printf("Usage: %s [OPTIONS] ARG [: ARG] \n", name);
	printf("where  OPTIONS:= q|qq|e\n");
    printf("       ARG := { set key=value |\n");
    printf("        del key |\n");
    printf("        isset key |\n");
    printf("        list [key] |ls [key] |\n");
    printf("        slist key | sls key |\n");
    printf("        klist key | kls key |\n");
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

int init()
{
    db_lines_count=0;
    db_size=0;
    quotation=0;
	export=0;
    need_write=false;
    int i;
	db_filename[0]='\0';
    for(i=0; i<MAX_LINES; i++)
        db_lines[i]=NULL;

    return 0;
}

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

/*
    if (! (size=strstr(db_header, "SIZE="))) 
       return false;
    size
  */   

    return result;
}

int sort_func(const void *pa, const void *pb){
	const char *a=(*(const char**)pa);
	const char *b=(*(const char**)pb);
	
	return strcmp(a, b);
}

int db_sort(){
	qsort(db_lines, db_lines_count, sizeof(char*), sort_func);
}

inline int print_pair(const char* name, const char* value)
{
	switch(quotation) {
		case 0: 
			if (name)
				printf(export?"export %s=%s\n":"%s=%s\n", name, value);
			else
				printf("%s\n", value);
			break;
		case 1:
			if (name)
				printf(export?"export \"%s\"=\"%s\"\n":"%s=\"%s\"\n", name, value);
			else
				printf("\"%s\"\n", value);
			break;
		default:
			if (name)
				printf(export?"export '%s'='%s'\n":"%s='%s'\n", name, value);
			else
				printf("'%s'\n", value);
			break;
	}

    return true;
}

int db_add(const char *str)
{

    db_lines[db_lines_count]=strdup(str);
    db_size+=strlen(db_lines[db_lines_count]+1);
    db_lines_count++;
}

int db_read()
{
    int result=true;
    char *buf=NULL;

	// check if file readed already
	if (db_lines_count)
		return true;

	db_file = fopen(get_dbfilename(), "r");
	if (!db_file) {
		fprintf(stderr, "fopen '%s' %s\n", get_dbfilename(), strerror(errno));
		exit(1);
	};
	// read header
    if( ! fgets(db_header, sizeof(db_header), db_file) ) {
        perror("fgets");
        result=false;
    }
	// check header
    if (result && (!parse_header()))
        result=false;

    if (result && (!(buf=malloc(MAX_DB_SIZE+100)))){
        perror("malloc");
        result=false;
    }

    if (result) {
        size_t readed = fread(buf, 1, MAX_DB_SIZE+10, db_file);
        if (ferror(db_file) || (readed<5) ) {
            perror("fread");
            result=false;
        };
        fclose(db_file);
        // terminate buffer
        buf[MAX_DB_SIZE]='\0';

        if (result) {
            char *s=buf;
            int i=0;
            int f=true;
            while ( s ) {
                if (s[0]=='\0') {
                    //fprintf(stderr, "DEBUG: found good end of data\n");
                    break;
                }
                int len=strlen(s);
                if (len>=MAX_DB_SIZE) {
                    fprintf(stderr, "string lenght is greater than buffer\n");
                    result=false;
                    break;
                }
                if ( (buf+MAX_DB_SIZE) <= s ){
                    fprintf(stderr, "DEBUG: out of buffer\n");
                    break;
                }
                db_add(s);
                s=s+len+1;
            }
        }
            
    }
    if (buf)
        free(buf);
    return result;
}

int db_write()
{
    int i, len, written;
    int result=true;
    char *buf=NULL;
    char strbuf[MAX_LINE_SIZE+2];
    char *s;
    if ( ! (buf=malloc(MAX_DB_SIZE+100)) ){
        perror("malloc");
        result=false;
    };
    // clear buffer
    memset(buf, 0, MAX_DB_SIZE+100);
    len=sprintf(buf, "%s\n", HEADER_LINE);
    s=buf+len;
	// sorts the data
	db_sort();
	// fills the output buffer
    for(i=0; i<db_lines_count; i++) {
        len=sprintf(s, "%s", db_lines[i]); 
        s+=len+1;
        if (s > (buf+MAX_DB_SIZE)) {
            fprintf(stderr, "Out of memory\n");
            result=false;
            break;
        };
    };
    if (result) {
		// open file
        db_file = fopen(get_dbfilename(), "w");
        if (!db_file) {
            perror("fopen");
            exit(1);
        };
        s+=5;
        fwrite(buf, 1, (s-buf), db_file);
        if (ferror(db_file)) 
            perror("fwrite"), result=false;
        fclose(db_file);
    }
    if (buf)
        free(buf);
        
    return result;
}

int find_key(const char *key)
{
    int i;
    char name[MAX_LINE_SIZE];
    int key_len=strlen(key);

    if ( !key_len )
        return false;

    strcpy(name, key);
    strcat(name, "=");
    key_len++;

    for(i=0; i<db_lines_count; i++) {
        if (!strncmp(name, db_lines[i], key_len)) 
            return i;
    }

    return -1;
}

int get(const char *key)
{
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    db_read();
    int index = find_key(key);
    if (index != -1)
        if (parse_pair(db_lines[index], name, value)) {
            print_pair(NULL, value);
            return true;
        }
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

int set(const char *str)
{
    int result=false;
    int index;
    char iname[MAX_LINE_SIZE], ivalue[MAX_LINE_SIZE];

    db_read();
    if (!parse_pair(str, iname, ivalue)) 
        return false;

    index = find_key(iname);
    if (index != -1) {
        free(db_lines[index]);
        db_lines[index] = strdup(str);
        result = true;
    } else {
        int len=strlen(str);
        if ((db_lines_count < (MAX_LINES-1)) && (db_size+len < MAX_DB_SIZE )) {
            db_lines_count++;
            db_size+=len;
            db_lines[db_lines_count-1] = strdup(str);
            result=true;
        } else {
            fprintf(stderr, "Error: File too big: lines: %d, size: %d\n", db_lines_count, db_size);
            return false;
        }
    }
    if (result)
        need_write=true;

    return result;
}

int del(const char *key)
{
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    db_read();
    int index = find_key(key);
    if (index != -1) {
        while( index < (db_lines_count-1)){
            db_lines[index]=db_lines[index+1];
            index++;
        }
        db_lines_count--;
		need_write=true;
        return true;
    } else {
        fprintf(stderr, "%s not found\n", key);
        return false;
    }
}

int sublist(const char *key)
{
    int result=true;
    int i;
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    int key_len=strlen(key);

    db_read();
    for(i=0; i<db_lines_count; i++) {
        parse_pair(db_lines[i], name, value);
        
        if(key && key_len){
            if (!strncmp(key, name, key_len)) {
                char *s=name+key_len;
                print_pair(s, value);
            }
        } 
    }

    return result;
}

int keylist(const char *key)
{
    int result=true;
    int i;
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    int key_len=strlen(key);

    db_read();
    for(i=0; i<db_lines_count; i++) {
        parse_pair(db_lines[i], name, value);
        
        if(key && key_len){
            if (!strncmp(key, name, key_len))
                print_pair(NULL, name);
        } else
            print_pair(NULL, name);
    }

    return result;
}

int list(const char *key)
{
    int result=true;
    int i;
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    int key_len=strlen(key);

    db_read();
    for(i=0; i<db_lines_count; i++) {
        parse_pair(db_lines[i], name, value);
        
        if(key && key_len){
            if (!strncmp(key, name, key_len))
                print_pair(name, value);
        } else
            print_pair(name, value);
    }

    return result;
}

int db_create(const char *filename)
{
    if (strlen(filename))
		set_dbfilename(filename);
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
            if(parse_pair(buf, name, value))
                db_add(buf);
            else {
                result=false;
                break;
            };
        };
    };
    fclose(file);
    if (result) 
        result = db_write();

    return result;
}

int test()
{
    int i;
    char name[MAX_LINE_SIZE], value[MAX_LINE_SIZE];
    for(i=0; i<db_lines_count; i++)
        if ( parse_pair(db_lines[i], name, value) )
            fprintf(stderr, "test: parse '%s': name='%s', value='%s'\n", db_lines[i], name, value);
        else
            return false;
    return true;
}

int main(int argc, char **argv)
{
    int ch, i;
    int result=false;
    char cmd[MAX_LINE_SIZE];
    char param[MAX_LINE_SIZE];

    init();

    while ((ch = getopt(argc, argv, "qef:")) != -1){
        switch(ch) {
            case 'f': set_dbfilename(optarg);
                     break;
            case 'q': quotation++;
                     break;
            case 'e': export++;
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


		if ( (optind >= argc) || (strcmp(":", argv[optind])==0)) {
			strcpy(param, "");
		} else {
			strcpy(param, argv[optind]);
			optind++;
		}

		optind++;


		if ((!strcmp(cmd, "list")) || (!strcmp(cmd, "ls")))
			result = list(param);
		else if ((!strcmp(cmd, "slist")) || (!strcmp(cmd, "sublist"))|| (!strcmp(cmd, "sls")))
			result = sublist(param);
		else if ((!strcmp(cmd, "klist")) || (!strcmp(cmd, "kls")))
			result = keylist(param);
		else if (!strcmp(cmd, "get"))
			result = get(param);
		else if (!strcmp(cmd, "set"))
			result = set(param);
		else if (!strcmp(cmd, "isset"))
			result = isset(param);
		else if (!strcmp(cmd, "del"))
			result = del(param);
		else if (!strcmp(cmd, "edit"))
			result = edit(argv[0]);
		else if (!strcmp(cmd, "create"))
			result = db_create(param);
		else if (!strcmp(cmd, "import"))
			result = db_import(param);
		else 
			show_usage(argv[0]);
		if (!result)
			break;

	}
    if (result) {
		if(need_write)
			db_write();

        return 0;
	} else 
        return 1;
}
