/************************************************************************** 
 *
 * Copyright (c) 2005,2007    Vladislav Moskovets (webface-dev(at)vlad.org.ua)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *************************************************************************** */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/types.h>


#define DEBUG
#define READ_BUF_SIZE 2048
#define MAX_LEVEL INT_MAX


int debug_level = 1;

#ifdef DEBUG
void debug(int level,char *format, ...){
	if ( level > debug_level ) 
		return;
	for (int i = 0; i < level; i++)
		fprintf(stderr, "\t");
	fprintf(stderr, "DEBUG%d: ", level);
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}
#else
inline void _debug(int level, char* format, ...){}
#define debug if(0)_debug
#endif


//                    ^
//    +---------------------------------+
//    |              root               |
//  <-| prev                       next |->
//    |             fchild              |
//    +---------------------------------+
//                    v
//
//  RULES:
// 1. absolute root: next=NULL, prev=NULL, parent=NULL
// 2. first node: prev=NULL
// 3. not first node: parent=NULL
// 4. last: next=NULL
//
//  EXAMPLE TREE
//
//  *ROOT*
//    |           |-t1-t2-t3-t4-t5
//    n1-n2-n3-n4-n5             |
//      |   |                    a1-a2-a3-a4-a5
//      |   z1-z2-z3-z4-z5
//      |            |
//      y1-y2-y3     q1-q2-q3-q4-q5
//      |
//      r1-r2-r3-r4
//      
// 1. q4->get_parent(): returns z4
// 2. q4->get_first(): returns q1
// 3. q4->get_last(): returns q5
// 4. q5->get_parent()->get_first(): returns z1
// 5. q5->get_parent()->get_last(): returns z5
// 6. q4->get_first()->get_prev(): returns NULL
// 7. q4->get_absolute_root(): returns ROOT

char *str_escape(const char *source);
char *str_unescape(const char *source);
int match_wildcard(const char *pattern, const char *string);
int is_wildcarded(const char* str);

char *level_delimiter=" ";
char *node_delimiter="_";

class node {
	node *parent;
	node *fchild;
	node *next;
	node *prev;

	char *name;
	char *data;

	void init_values() {
		parent=NULL; 
		fchild=NULL;
		next=NULL;
		prev=NULL;
		name=strdup("");
		data=strdup("");
	}
public:
	node() { 
		init_values();
	}

	node(const char* name, const char* data) { 
		init_values();
		set_namedata(name, data);
	}
	node(node* r) {
		init_values();
		parent=r;
	}

	node(node* r, const char* name, const char* data) {
		init_values();
		parent=r;
		set_namedata(name, data);
	}

	node(const char* pair) {
		init_values();
		set_pair(pair);
	}

	~node() {
		free_childs();
		assert( data != NULL);
		assert( name != NULL);
		free(data);
		free(name);
	}

	void free_childs() {
		debug(3, "node['%s']::free_childs() \n", get_name());
		node *n;
		while ( n = fchild ) {
			//n->free_childs();
			n->remove();
			delete n;
		}
	}

	const char *get_name() { return name; }
	const char *get_data() { return data; }

	void set_namedata(const char* name, const char* data) {
		set_name(name);
		set_data(data);
	}

	const char *set_name(const char *str) { debug(6, "node['%s']::set_name('%s')\n", get_name(), str); assert(str); free(name); return name = strdup(str); }
	const char *set_data(const char *str) { debug(6, "node['%s']::set_data('%s')\n", get_name(), str); assert(str); free(data); return data = strdup(str); }
	void set_parent(node* newparent) { assert(newparent); parent = newparent; }

	node *get_next() { return next; }
	node *get_prev() { return prev; }
	node *get_parent() { 
		if ( get_prev() ) 
			return get_first()->get_parent();
		else 
			return parent;
	}

	inline bool is_absolute_root() {
		return get_parent()?true:false;
	}

	node *get_absolute_root() {
		node *n = get_parent();
		while ( ! n )
			n = get_parent();
		return n;
	}

	inline node *get_fchild() { return fchild; }
	
	bool set_pair(const char* str) {
		debug(5, "node['%s']::set_pair('%s')\n", get_name(), str);
		int len = strlen(str);
		if (!len) {
			fprintf(stderr, "Error: can't parse: '%s'", str);
			return false;
		}
		char s[len+10];

		strcpy(s, str);
		if (s[len-1]=='\n')
			s[len-1]='\0';

		char *eq=strchr(s, '=');
		if ( eq == s ) {
			set_data( eq+1 );
		} else if ( eq ) { 
			eq[0]='\0';
			set_name( s );
			set_data( eq+1 );
		} else {
			set_name( s );
		}

		return true;
	}

	node *get_first() {
		debug(5, "node['%s']::get_first(): ", get_name());
		node *n = prev;
		if ( !n ) {
			debug(5, "node['%s']\n", get_name());
			return this;
		}
		
		// look for first: on first *prev should be NULL
		while ( true ) {
			if ( n->get_prev() )
				n = n->get_prev();
			else {
				debug(5, "node['%s']\n", get_name());
				return n;
			}
		}
		
	}

	node *get_last() {
		debug(5, "node['%s']::get_last():  ", get_name());
		assert(this);
		if ( next == NULL ) {
			debug(5, "node['%s']\n", get_name());
			return this;
		}
		assert( next != NULL );
		return next->get_last();
	}
	
	int get_level(int curr_level=0) {
		if ( ! get_parent() )
			return curr_level;
		get_parent()->get_level(curr_level+1);
	}

	node *find_name(const char* str, int maxlevel=MAX_LEVEL) {
		debug(3, "node['%s']::find_name('%s', %d)\n", get_name(), str, maxlevel);
		node *found=NULL;
		if ( ! strcmp(str, name) )
			return this;

		if ( maxlevel <= 0 )
			return NULL;

		node *n=fchild;
		while ( n ) {
			found=n->find_name(str, maxlevel-1);
			if ( found )
				return found;
			n=n->get_next();
		}
		return found;
	}
	int get_childcount() {
		int i = 0;
		node *n=fchild;
		while ( n ) {
			n=n->get_next();
			i++;
		}
		return i;
	}

	node *get_lastchild() {
		debug(4, "node['%s']::get_lastchild()\n", get_name());
		assert (fchild != NULL);
		return fchild->get_last();
	}

	node *insert_after(node* n) {
		assert(n);
		debug(3, "node['%s']::insert_after('%s')\n", get_name(), n->get_name());
		node* t = next;
		next=n;
		next->prev=this;
		next->next=t;
		if (t)
			t->prev=next;
		return n;
	}

	node *insert_before(node* n) {
		assert(n);
		debug(3, "node['%s']::insert_before('%s')\n", get_name(), n->get_name());
		node* t=prev;
		prev=n;
		prev->prev=t;
		prev->next=this;
		if (t) {
			t->next=prev;
		}
		if (parent) { 
			prev->parent=parent;
			parent=NULL;
		}
		return n;
	}
	
	node *add_child(node* child) {
		assert(child);
		debug(3, "node['%s']::add_child('%s')\n", get_name(), child->get_name());
		assert (child);
		if ( fchild == NULL ) {
			fchild = child;
			child->parent=this;
		} else {
			get_lastchild()->insert_after(child);
		}
		return child;
	}

	node *remove() {
		debug(3, "node['%s']::remove()\n", get_name());
		if ( (parent != NULL) && (prev != NULL) )
			debug(1, "parent: %x, prev: %x\n", parent, prev);

		if ( get_prev() ) {
			debug(5, "node['%s']::remove(): updating node['%s'].next = node['%s']\n", get_name(), get_prev()->get_name(), get_next()? get_next()->get_name():"NULL");
			get_prev()->next=get_next();
		}
		
		// handle parent updation
		if ( parent ) {
			debug(5, "node['%s']::remove(): updating node['%s'].fchild = node['%s']\n", get_name(), parent->get_name(), get_next()? get_next()->get_name():"NULL");
			parent->fchild = get_next();
			if (get_next()) {
				debug(5, "node['%s']::remove(): updating node['%s'].parent = node['%s']\n", get_name(), get_next()->get_name(), parent->get_name());
				get_next()->parent=parent;
			}
		}
			
		if ( get_next() ) {
			debug(5, "node['%s']::remove(): updating node['%s'].prev = node['%s']\n", get_name(), get_next()->get_name(), get_prev()? get_prev()->get_name():"NULL");
			get_next()->prev=get_prev();
		}

		parent=NULL;
		next=NULL;
		prev=NULL;
		return this;
	}
			
		
	node *new_child(){
		return add_child(new node(this));
	}	

	node *new_child(const char* str){
		debug(3, "node['%s']::new_child('%s')\n", get_name(), str);
		return add_child(new node(str));
	}	


	node *new_sibling(const char* str){
		debug(3, "node['%s']::new_sibling('%s')\n", get_name(), str);
		return get_last()->insert_after(new node(str));
	}	

	node *new_sibling(){
		return get_last()->insert_after(new node(this));
	}	

	void dump(int level=0) {
		for (int i=0; i < level; i++)
			fprintf(stderr, level_delimiter);
		fprintf(stderr, "node['%s']: data=%s, parent='%s'\n", get_name(), get_data(), get_parent()?get_parent()->get_name():"NULL");

		node *c=fchild;
		while ( c ) {
			c->dump(level+1);
			c=c->get_next();
		}
	}

	int serialize_to_file(FILE *file) {
		debug(3, "node['%s']::serialize_to_file()\n", get_name());
		return serialize(file, 0);
	}

	int serialize(FILE *stream=stdout, int level=0) {
		debug(3, "node['%s']::serialize(%d, %d)\n", get_name(), stream, level);

		int i;
		for ( i=0; i < level; i++ )
			fprintf(stream, level_delimiter);
		char *esc_name = str_escape(name);
		char *esc_data = str_escape(data);

		// print 'name=data' or just 'name'
		if ( strlen(esc_data) )
			fprintf(stream, "%s=%s\n", esc_name, esc_data);
		else
			fprintf(stream, "%s\n", esc_name);

		free(esc_data);
		free(esc_name);

		node *c = fchild;
		while ( c ) {
			c->serialize(stream, level+1);
			c = c->get_next();
		}

		return true;
	}

	node *unserialize(const char* str, int level=0) {
		node *n;
		int i;
		int my_level = get_level();

		debug(3, "  node['%s']::unserialize(): my_level=%d, level=%d, read_buf='%s'\n", get_name(), my_level, level, str);
		if ( level == my_level ) {
			n = new_sibling(str);
		} else if ( level > my_level ) {
			n = new_child(str);

		} else if ( level < my_level ) {
			n = this;
			for ( i = 0; i < (my_level - level); i++) {
				n = n->get_parent();
				assert( n != NULL );
			}
			n = n->new_sibling(str);
		}
		return n;
	}

	int unserialize_from_file(FILE *file=stdin) {

		int i;
		int curr_level = 0;
		int readed_level = 0;
		char read_buf[READ_BUF_SIZE];
		char *pair_begin;
		node *curr_node = this;

		fgets(read_buf, sizeof(read_buf), file);
		curr_node->set_pair(read_buf);

		while ( fgets(read_buf, sizeof(read_buf), file) ) {

			if ( read_buf[strlen(read_buf)-1] == '\n' )
				read_buf[strlen(read_buf)-1]='\0';

			pair_begin = NULL;
			debug(5, "-------------\n");
			for ( readed_level = 0; readed_level < sizeof(read_buf); readed_level++ )
				if ( read_buf[readed_level] != level_delimiter[0] ) {
					pair_begin = &read_buf[readed_level];
					break;
				}
			curr_node = curr_node->unserialize(pair_begin, readed_level);
		}
		return true;
	}

	// in: str:'sys_name_value'  replace first '_'  to '\0', 
	// returns pointer to 'name_value'
	// returns NULL if delimiter not found
	char *chain_split_chain(char *str) {
		char *delim = strchr(str, node_delimiter[0]);

		if ( delim ) {
			delim[0]='\0';
			delim++;
			return delim;
		};
		return NULL;
	}

	// in: str:'sys_fw_filter_policy'
	// returns node with name policy
	// returns NULL if chain not found
	node* chain_find_node(const char *str) {
		debug(3, "node['%s']::chain_find_node('%s')\n", get_name(), str);
		char str_buf[512];
		char *next_name;
		node *n;
		strncpy(str_buf, str, sizeof(str_buf));
		
		if ( next_name = chain_split_chain(str_buf) ) {
			n = find_name(str_buf, 1);
			if ( n ) {
				return n->chain_find_node( next_name );
			} else {
				debug(5, "node['%s']::chain_find_node('%s'): child '%s' not found\n", get_name(), str, str_buf);
				return NULL;
			}
		} else {
			n = find_name(str_buf, 1);
			if ( n )
				debug(5, "node['%s']::chain_find_node('%s'): child '%s' found!\n", get_name(), str, str_buf);
			return n;
		}
	}
	
	// in: str:'sys_fw_filter_policy'
	// returns created node with name policy
	node* chain_add_node(const char *str) {
		debug(3, "node['%s']::chain_add_node('%s')\n", get_name(), str);
		char str_buf[512];
		char *next_name;
		node *n;
		strncpy(str_buf, str, sizeof(str_buf));
		
		if ( next_name = chain_split_chain(str_buf) ) {
			n = find_name(str_buf, 1);
			if ( n ) {
				return n->chain_add_node( next_name );
			} else {
				debug(3, "node['%s']::chain_add_node('%s'): child '%s' not found, creating\n", get_name(), str, str_buf);
				n = new_child(str_buf);
				return n->chain_add_node(next_name);
			}
		} else {
			n = find_name(str_buf, 1);
			if ( n ) {
				debug(5, "node['%s']::chain_add_node('%s'): child '%s' found!\n", get_name(), str, str_buf);
			} else {
				debug(5, "node['%s']::chain_add_node('%s'): child '%s' not found, creating\n", get_name(), str, str_buf);
				return new_child(str_buf);
			}
			return n;
		}
	}

	void chain_print_node(const char* prefix) {
		debug(3, "node['%s']::chain_print_node('%s')\n", get_name(), prefix);
		char str_buf[512];
		strncpy(str_buf, prefix, sizeof(str_buf));
		
		if ( strlen(get_data() ) ) {
			debug(4, "node['%s']::chain_print_node('%s'): get_data()='%s'\n", get_name(), prefix, get_data());
			printf("%s%s=%s\n", str_buf, get_name(), get_data());
		}

		snprintf(str_buf, sizeof(str_buf), "%s%s%s", prefix,  get_name(), node_delimiter);
		
		node *c=fchild;
		while ( c ) {
			c->chain_print_node(str_buf);
			c=c->get_next();
		}
	}

};




/*************************************************************************************************************/


class hdb {

	FILE *db_file;
	char *db_filename;
	int quotation;
	int make_local;
	char *local_str;
	int make_export;
	int need_print_count;
	int need_write;
	int is_db_already_readed;
	node *root;
	node *current_root;
public:

	hdb() {
		db_file = NULL;
		db_filename = strdup("");
		quotation = 0;
		make_local = false;
		local_str = strdup("local ");
		make_export = false;
		need_print_count = false;
		need_write = false;
		is_db_already_readed = false;
		root = new node("root");
		current_root = root;
	};

	void show_usage(char *name)
	{
		printf("Usage: %s [OPTIONS] ARG [: ARG] \n", name);
		printf("where  OPTIONS:= d|l|q|qq|e|c\n");
		printf("       ARG := { set pattern=value |\n");
		printf("        rm pattern | del pattern |\n");
		printf("        isset key |\n");
		printf("        list [pattern] | ls [pattern] |\n");
		printf("        slist key | sls key |\n");
		printf("        klist pattern | kls pattern |\n");
		printf("        listrm key | lrm key |\n");
		printf("        listadd key_=value | ladd key_=value |\n");
		printf("        rename oldkey newkey | rn oldkey newkey |\n");
		printf("        create [filename] |\n");
		printf("        import [filename] |\n");
		printf("        edit }\n");
		exit(1);
	};
	
	// Parse str "name=value" and return 'name' 
	char* get_pair_name(const char* str)
	{
		debug(4, "hdb::get_pair_name('%s')\n", str);
		int len = strlen(str);
		if (!len)
			return NULL;
		char s[len+10];

		strcpy(s, str);

		char *eq = strchr(s, '=');
		if ( eq ) {
			eq[0]='\0';
			return strdup(s);
		}
		return NULL;
	}
	
	// Parse str "name=value" and return 'value' 
	char* get_pair_data(const char* str)
	{
		debug(4, "hdb::get_pair_value('%s')\n", str);
		int len = strlen(str);
		if (!len)
			return NULL;
		char s[len+10];

		strcpy(s, str);

		char *eq = strchr(s, '=');
		if ( eq ) {
			eq[0]='\0';
			eq++;
			len=strlen(eq);
			if (eq[len-1]=='\n')
				eq[len-1]='\0';
			return strdup(eq);
		}
		return NULL;
	}

	const char *get_dbfilename() { 
		if ( strlen(db_filename) )
			return db_filename;
		if ( getenv("HDB") ) {
			free( db_filename );
			db_filename = strdup( getenv("HDB") );
		} else if ( ! getuid() ) {
			free( db_filename );
			db_filename = strdup( "/etc/hdb" );
		} else {
			char str_buf[ strlen( getenv("HOME") ) + 10 ];
			sprintf(str_buf, "%s/.hdb", getenv("HOME"));
			free(db_filename);
			db_filename = strdup( str_buf );
		}

		return db_filename; 
	};

	const char *set_dbfilename(const char* filename) {
		free(db_filename);
		db_filename = strdup(filename);
		return db_filename;
	}

	int db_open()
	{
		debug(3, "Using %s as database\n", get_dbfilename());
		db_file = fopen(get_dbfilename(), "r+");
		if (!db_file) {
			fprintf(stderr, "fopen '%s' %s\n", get_dbfilename(), strerror(errno));
			exit(1);
		};
		// lock the file
		if( flock( fileno(db_file), LOCK_EX) )
		   	perror("flock");

		return true;
	}

	int db_load() {
		if ( is_db_already_readed ) {
			debug(3, "hdb::db_load(): already readed\n");
			return true;
		}

		if ( ! db_open() ) {
			debug(4, "hdb::db_open(): fail, exiting\n");
			return false;
		}

		if ( ! root->unserialize_from_file(db_file) )
			return false;
		is_db_already_readed=true;
		return true;
	}

	int db_write() {
		db_load();
		rewind(db_file);
		ftruncate(fileno(db_file), 0);
		root->serialize_to_file(db_file);
	}

	int db_close() {
		if (! db_file)
			return true;
		flock(fileno(db_file), LOCK_UN);
		db_file=NULL;
		is_db_already_readed = false;
		return true;
	}
		

	int hdb_list (const char* str) {
		
		db_load();
		node *n = current_root->chain_find_node(str);
		if ( n ) {
			debug(3, "hdb::hdb_list('%s'): found node['%s']\n", str, n->get_name());
			n->chain_print_node("");
		} else {
			debug(3, "hdb::hdb_list('%s'): node not found\n", str);
		}
		
		return true;
	}

	int hdb_rename (const char* name, const char* newname) {
		db_load();
		node *n = current_root->chain_find_node(name);
		if ( n ) {
			debug(3, "hdb::hdb_rename('%s', '%s'): found node['%s'], renaming\n", name, newname, n->get_name());
			n->set_name(newname);
			need_write++;
		} else {
			debug(3, "hdb::hdb_rename('%s', '%s'): node not found\n", name, newname);
		}
		return true;
	}

	int hdb_show (const char* str) {
		db_load();
		current_root->serialize();
		return true;
	}

	int hdb_set (const char* str) {
		char *chain;
		char *data;

		db_load();
		if ( ! (chain = get_pair_name(str)) )
			return false;
		if ( ! (data = get_pair_data(str)) )
			return false;
		
		debug(4, "hdb::hdb_set('%s'): chain='%s', data='%s'\n", str, chain, data);
		node *n = current_root->chain_find_node(chain);
		if ( n ) {
			debug(3, "hdb::hdb_set('%s'): found node['%s']\n", str, n->get_name());
			n->set_data(data);
		} else {
			debug(3, "hdb::hdb_set('%s'): node not found\n", str);
			n = current_root->chain_add_node(chain);
			n->set_data(data);
		}
		
		need_write++;
		return true;
	}

	int hdb_rm (const char* str) {
		db_load();
		
		debug(4, "hdb::hdb_rm('%s')\n", str);
		node *n = current_root->chain_find_node(str);
		if ( n ) {
			debug(3, "hdb::hdb_set('%s'): found node['%s'], deleting\n", str, n->get_name());
			n->remove();
			delete n;
		}
		
		root->dump();
		need_write++;
		return true;
	}

	int hdb_dump (const char* str) {
		char *name;
		char *data;

		db_load();
		root->dump();
		return true;
	}

	int hdb_export(const char* filename)
	{
		db_load();
		return current_root->serialize();
	}

	int hdb_import(const char* filename)
	{
		db_close();

		FILE *file;
		if ( ! strcmp(filename, "-"))
			file=stdin;
		else
			file = fopen(filename, "r");
		if (!file) {
			perror("fopen(): ");
			exit(1);
		};

		root->unserialize_from_file(file);
		fclose(file);

		is_db_already_readed=true;
		need_write++;

		return db_open();
	}

	int hdb_edit(const char* me) 
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

		
		sprintf(buf, "%s export > %s", me, tmpname);
		if (system(buf))
			return false;
		
		if (! system(editor))  {
			result= hdb_import(tmpname);
			remove(tmpname);
		}

		return result;
	}

	int main(int argc, char **argv);
	
};


int hdb::main (int argc, char **argv)
{
    int ch, i;
    int result=false;

    while ((ch = getopt(argc, argv, "dD:qlecf:")) != -1){
        switch(ch) {
            case 'D': node_delimiter = strdup(optarg);
                     break;
            case 'f': set_dbfilename(optarg);
                     break;
            case 'd': debug_level++;
                     break;
            case 'q': quotation++;
                     break;
            case 'l': make_local++;
                     break;
            case 'e': make_export++;
                     break;
            case 'c': need_print_count++;
                     break;
            default: show_usage(argv[0]);
                     break;
        }
    }

    if ( argc <= optind ) 
        show_usage(argv[0]);

	while(true) {
		if ( optind >= argc ) 
			break;

		char *cmd = argv[optind];
		char *param0 = argv[optind+1];
		char *param1 = argv[optind+2];

		optind++;
		// if cmd == ':' then go to next cmd
		if ( cmd[0]==':' )
			continue;

		if ( (optind >= argc) || param0[0]==':' )
			param0 = "";
		if ( (optind+1 >= argc) || param1[0]==':' )
			param1 = "";


		if ( (!strcmp(cmd, "list")) || (!strcmp(cmd, "ls")) )
			result = hdb_list(param0);
		else if (!strcmp(cmd, "set"))
			result = hdb_set(param0);
		else if (!strcmp(cmd, "rename"))
			result = hdb_rename(param0, param1), optind++;
		else if (!strcmp(cmd, "dump"))
			result = hdb_dump(param0);
		else if (!strcmp(cmd, "show"))
			result = hdb_show(param0);
		else if (!strcmp(cmd, "rm"))
			result = hdb_rm(param0);
		else if (!strcmp(cmd, "edit"))
			result = hdb_edit(argv[0]);
		else if (!strcmp(cmd, "export"))
			result = hdb_export(argv[0]);
		else if (!strcmp(cmd, "import"))
			result = hdb_import(argv[0]);
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


int main (int argc, char **argv) {
	hdb app;
	app.main(argc, argv);
}




























// copyright (c) 2003-2005 chisel <storlek@chisel.cjb.net>
/* adapted from glib. in addition to the normal c escapes, this also escapes the comment character (#)
 *  * as \043.  */
char *str_escape(const char *source)
{
	const char *p = source;
	/* Each source byte needs maximally four destination chars (\777) */
	char *dest = (char*)calloc(4 * strlen(source) + 1, sizeof(char));
	char *q = dest;

	while (*p) {
		switch (*p) {
		case '\a':
			*q++ = '\\';
			*q++ = 'a';
		case '\b':
			*q++ = '\\';
			*q++ = 'b';
			break;
		case '\f':
			*q++ = '\\';
			*q++ = 'f';
			break;
		case '\n':
			*q++ = '\\';
			*q++ = 'n';
			break;
		case '\r':
			*q++ = '\\';
			*q++ = 'r';
			break;
		case '\t':
			*q++ = '\\';
			*q++ = 't';
			break;
		case '\v':
			*q++ = '\\';
			*q++ = 'v';
			break;
		case '\\': case '"': case '\'': 
			*q++ = '\\';
			*q++ = *p;
			break;
		default:
			if ((*p <= ' ') || (*p >= 0177) || (*p == '=') || (*p == '#')) {
				*q++ = '\\';
				*q++ = '0' + (((*p) >> 6) & 07);
				*q++ = '0' + (((*p) >> 3) & 07);
				*q++ = '0' + ((*p) & 07);
			} else {
				*q++ = *p;
			}
			break;
		}
		p++;
	}

	*q = 0;

	return dest;
}

/* opposite of str_escape. (this is glib's 'compress' function renamed more clearly)
 * TODO: it'd be nice to handle \xNN as well... */
char *str_unescape(const char *source)
{
	const char *p = source;
	const char *octal;
	char *dest = (char*)calloc(strlen(source) + 1, sizeof(char));
	char *q = dest;

	while (*p) {
		if (*p == '\\') {
			p++;
			switch (*p) {
			case '0'...'7':
				*q = 0;
				octal = p;
				while ((p < octal + 3) && (*p >= '0') && (*p <= '7')) {
					*q = (*q * 8) + (*p - '0');
					p++;
				}
				q++;
				p--;
				break;
			case 'a':
				*q++ = '\a';
				break;
			case 'b':
				*q++ = '\b';
				break;
			case 'f':
				*q++ = '\f';
				break;
			case 'n':
				*q++ = '\n';
				break;
			case 'r':
				*q++ = '\r';
				break;
			case 't':
				*q++ = '\t';
				break;
			case 'v':
				*q++ = '\v';
				break;
			default:		/* Also handles \" and \\ */
				*q++ = *p;
				break;
			}
		} else {
			*q++ = *p;
		}
		p++;
	}
	*q = 0;

	return dest;
}

