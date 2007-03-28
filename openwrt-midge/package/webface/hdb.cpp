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
extern "C"
{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/file.h>
#include <sys/types.h>
}


#define DEBUG
#define READ_BUF_SIZE 2048
#define MAX_LEVEL INT_MAX

#define BIT(x) (1UL << (x))
#define SET_BIT(target,bit)        ((target) |= (bit))
#define CLEAR_BIT(target,bit)      ((target) &= ~(bit))
#define TEST_BIT(target,bit)       ((target) & (bit))

#define PRINT_QUOT		BIT(0)
#define PRINT_DQUOT		BIT(1)
#define PRINT_LOCAL		BIT(2)
#define PRINT_EXPORT		BIT(3)
#define PRINT_COUNT		BIT(4)
#define PRINT_NAME		BIT(5)
#define PRINT_CHAINNAME	BIT(6)
#define PRINT_DATA		BIT(7)
#define DO_NOT_ESCAPE		BIT(8)


int debug_level = 2;

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
//    |             parent              |
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


class hdb;
hdb *app = NULL;

char *level_delimiter=" ";
char *node_delimiter="_";

class node {
	node *parent;
	node *fchild;
	node *next;
	node *prev;

	char *name;
	char *data;
	char *pos_str;

	void init_values() {
		parent=NULL; 
		fchild=NULL;
		next=NULL;
		prev=NULL;
		name=strdup("");
		data=strdup("");
		pos_str=strdup("");
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
		while ( (n = fchild) ) {
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
	const char *set_data(const char *str) { 
			debug(6, "node['%s']::set_data('%s')\n", get_name(), str); 
			assert(str); 
			char buf[512];
			int value, str_value;
			if ( str[0] == '+' && str[1] == '=' ) {
				value=atoi(data);
				str_value=atoi(&str[2]);
				snprintf(buf, sizeof(buf), "%d", value+str_value);
				free(data); 
				return data = strdup(buf);
			} else if ( str[0] == '-' && str[1] == '=' ) {
				value=atoi(data);
				str_value=atoi(&str[2]);
				snprintf(buf, sizeof(buf), "%d", value-str_value);
				free(data); 
				return data = strdup(buf);
			} else
				return data = strdup(str); 
	}
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

	int get_levelpos() {
		int i = 0;
		node* n = get_first();

		if ( n == NULL)
			return -1;

		debug(7, "node['u']::get_levelpos(): get_first(): '%s'\n", n->get_name()); 
		while ( n != this ) {
			assert( n != NULL);
			i++;
			n = n->get_next();
		}
		debug(7, "node['u']::get_levelpos('%d')\n", i); 
		return i;
	}
	
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
		debug(7, "node['%s']::get_first(): \n", get_name());
		node *n = prev;
		if ( !n ) {
			debug(7, "node['%s']\n", get_name());
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
		debug(5, "node['%s']\n", get_name());
		return next->get_last();
	}
	
	int get_level(int curr_level=0) {
		if ( ! get_parent() )
			return curr_level;
		return get_parent()->get_level(curr_level+1);
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
			parent->fchild=prev;
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

	// buggy
	node* swap_with_next() {
		if (! get_next() )
			return NULL;

		debug(3, "node['%s']::swap_with_next(): next['%s']\n", get_name(), get_next()->get_name());
		if ( get_parent() ) {
			get_next()->parent = get_parent();
			get_parent()->fchild = get_next();
			parent = NULL;
		}

		if ( get_prev() ) {
			get_prev()->next = get_next();
			debug(4, "node['%s']::swap_with_next(): setting get_prev()[%s]->next = get_next()['%s']\n", get_name(), get_prev()->get_name(), get_next()?get_next()->get_name():"NULL");
		}


		node* n = get_next();
		debug(4, "node['%s']::swap_with_next(): n[%s]\n", get_name(), n?n->get_name():"NULL");
		node* t = n->get_next();
		debug(4, "node['%s']::swap_with_next(): t[%s]\n", get_name(), t?t->get_name():"NULL");
		debug(4, "node['%s']::swap_with_next(): setting n[%s]->prev to [%s]\n", get_name(), n->get_name(), get_prev()?get_prev()->get_name():"NULL");

		n->prev = get_prev();
		n->next = this;

		prev = n;
		next = t;
		if ( next )
			next->prev = this;
		return this;
	}

	node* move_left() {
		debug(2, "node['%s']::move_left()\n", get_name());
		node *p = get_prev();
		if ( p ) {
			remove();
			return p->insert_before( this );
		} else
			return NULL;
	}

	node* move_right() {
		debug(2, "node['%s']::move_right()\n", get_name());
		node *n = get_next();
		if ( n ) {
			remove();
			return n->insert_after( this );
		} else 
			return NULL;
	}

	int compare_name(node* n) {
		assert( n != NULL );
		if (! n )
			return 0;
		debug(5, "node['%s']::compare_name(['%s'])\n", get_name(), n->get_name());

		// try to compare integers
		int i1, i2;
		if ( (i1 = atoi(get_name())) && (i2 = atoi(n->get_name())) )
			return i1-i2;
		else
			return strcmp(get_name(), n->get_name());
	}

	// very simple sort
	void sort() {
		debug(3, "node['%s']::sort()\n", get_name());
		node *n=fchild;
		while ( n ) {
			if ( n->get_next() && ( n->compare_name(n->get_next()) > 0 ) ) {
				n->move_right();
				n = fchild;
				continue;
			}
			n=n->get_next();
		}

		n=fchild;
		while ( n ) {
			if ( n->get_fchild() )
				n->sort();
			n = n->get_next();
		}
		return;
	}

	int renumber_child() {
		debug(3, "node['%s']::renumber_child()\n", get_name());
		char str[32];
		int i = 0;
		node *n=fchild;
		while ( n ) {
			snprintf(str, sizeof(str), "%d", i);
			n->set_name(str);
			n = n->get_next();
			i++;
		}

		return i;
	}

	node *remove() {
		debug(3, "node['%s'-%d]::remove()\n", get_name(), get_levelpos());
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

	int serialize_to_file(FILE *file, int flags=0) {
		debug(3, "node['%s']::serialize_to_file(flags=%d)\n", get_name(), flags);
		return serialize(file, 0, flags);
	}

	int serialize(FILE *stream=stdout, int level=0, int flags=0) {
		debug(3, "node['%s']::serialize(%d, %d, %d)\n", get_name(), stream, level, flags);

		int i;
		for ( i=0; i < level; i++ )
			fprintf(stream, level_delimiter);

		char *esc_name = TEST_BIT(flags, DO_NOT_ESCAPE) ? strdup(name) : str_escape(name);
		char *esc_data = TEST_BIT(flags, DO_NOT_ESCAPE) ? strdup(data) : str_escape(data);

		// print 'name=data' or just 'name'
		if ( strlen(esc_data) )
			fprintf(stream, "%s=%s\n", esc_name, esc_data);
		else
			fprintf(stream, "%s\n", esc_name);

		free(esc_data);
		free(esc_name);

		node *c = fchild;
		while ( c ) {
			c->serialize(stream, level+1, flags);
			c = c->get_next();
		}

		return true;
	}

	node *unserialize(const char* str, int level=0) {
		node *n = this;
		int i;
		int my_level = get_level();
		char *unescaped_str=str_unescape(str);

		debug(3, "  node['%s']::unserialize(): my_level=%d, level=%d, read_buf='%s'\n", get_name(), my_level, level, unescaped_str);
		if ( level == my_level ) {
			n = new_sibling(unescaped_str);
		} else if ( level > my_level ) {
			n = new_child(unescaped_str);

		} else if ( level < my_level ) {
			n = this;
			for ( i = 0; i < (my_level - level); i++) {
				n = n->get_parent();
				assert( n != NULL );
			}
			n = n->new_sibling(unescaped_str);
		}
		return n;
	}

	int unserialize_from_file(FILE *file=stdin) {

		uint readed_level = 0;
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

	char *get_fullchain() {
		char str_buf1[512];
		char str_buf2[512];
		char *result;
		memset((void*)str_buf1, 0, sizeof(str_buf1));
		memset((void*)str_buf2, 0, sizeof(str_buf2));

		node *n = this;

		while (n) {
			// dot not include ROOT node
			if ( ! n->get_parent() )
				break;
			strncpy(str_buf2, str_buf1, sizeof(str_buf2));
			strncpy(str_buf1, n->get_name(), sizeof(str_buf1));
			if ( n != this)
				strncat(str_buf1, node_delimiter, sizeof(str_buf1));
			strncat(str_buf1, str_buf2, sizeof(str_buf1));
			n = n->get_parent();
		}
		result = (char*) malloc(strlen(str_buf1)+1);
		strcpy(result, str_buf1);
		return result;
	}

	void print_fullchain(int maxlevel=MAX_LEVEL) {
		char *chain_name;
		node *c = fchild;

		while ( c ) {
			if ( strlen( c->get_data() ) ) {
				chain_name = c->get_fullchain();
				printf("%s=%s\n", chain_name, c->get_data());
				free(chain_name);
			}
			c->print_fullchain(maxlevel-1);
			c = c->get_next();
		}
	}

	void walk(bool (*walk_func)(node*, void*, int), void* func_data, int options) {
		node *c = fchild;
		if (! walk_func(this, func_data, options) )
			return;

		while ( c ) {
			c->walk(walk_func, func_data, options);
			c = c->get_next();
		}
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
		
		if ( (next_name = chain_split_chain(str_buf)) ) {
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
		
		if ( (next_name = chain_split_chain(str_buf)) ) {
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


	static void print_pair(const char* name, const char* value, int print_opt) {
		char *prefix="";
		char *format="";
		char *quot_str="";

		if ( print_opt & PRINT_LOCAL )
			prefix = "local ";
		else if ( print_opt & PRINT_EXPORT )
			prefix = "export ";

		quot_str="";
		if ( print_opt & PRINT_QUOT ) 
			quot_str = "'";
		else if ( print_opt & PRINT_DQUOT )
			quot_str = "\"";

		if ( (! name) && (! value) )
			format="\n";
		else if ( ( (! name) && value) || ( name && (! value) ) )
			printf( "%s\n", name?name:value);
		else
			printf( "%s%s=%s%s%s\n", prefix, name?name:"", quot_str, value?value:"", quot_str);
		
	}

};


bool walk_print (node* n, void* func_data, int opt);


/*************************************************************************************************************/

class hdb {

	FILE *db_file;
	char *db_filename;
	int opt;
	int need_write;
	int is_db_already_readed;
	node *root;
	node *current_root;
public:

	hdb() {
		debug(4, "hdb::hdb()\n");
		db_file = NULL;
		db_filename = strdup("");
		opt = 0;
		need_write = false;
		is_db_already_readed = false;
		root = new node("root");
		current_root = root;
	};

	~hdb() {
		debug(4, "hdb::~hdb()\n");
		delete root;
	}


	void show_usage(char *name)
	{
		printf("Usage: %s [OPTIONS] ARG [: ARG] \n", name);
		printf("where  OPTIONS:= d|l|q|qq|e|c\n");
		printf("       ARG := { set nodechain=[+=|-=]value |\n");
		printf("        rm nodechain |\n");
		printf("        list [pattern] | ls [pattern] |\n");
		printf("        slist key | sls key |\n");
		printf("        rename oldnodechain newname |\n");
		printf("        import [filename] |\n");
		printf("        create [filename] |\n");
		printf("        edit }\n");
		return;
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
			return false;
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
			debug(4, "hdb::db_open(): fail\n");
			return false;
		}

		if ( ! root->unserialize_from_file(db_file) )
			return false;
		is_db_already_readed=true;
		return true;
	}

	int db_write() {
		db_load();
		debug(3, "hdb::db_write() trunkating file\n");
		rewind(db_file);
		ftruncate(fileno(db_file), 0);
		debug(3, "hdb::db_write() serializing\n");
		return root->serialize_to_file(db_file);
	}

	int db_close() {
		if (! db_file)
			return true;
		debug(5, "hdb::db_close(): unlocking\n");
		flock(fileno(db_file), LOCK_UN);
		debug(5, "hdb::db_close(): closing\n");
		fclose(db_file);
		db_file=NULL;
		is_db_already_readed = false;
		return true;
	}

	int hdb_print (const char* str, int loptions) {
		db_load();
		node *n = current_root;
		if ( n ) {
			debug(3, "hdb::hdb_list('%s'): found node['%s']\n", str, n->get_name());
			n->walk(walk_print, (void*) str, opt|loptions);
		} else {
			debug(3, "hdb::hdb_list('%s'): node not found\n", str);
		}
		
		return true;
	}
	int hdb_slist (const char* str) {
		db_load();
		node *n = current_root->chain_find_node(str);
		if ( n ) {
			debug(3, "hdb::hdb_slist('%s'): found node['%s']\n", str, n->get_name());
			n->chain_print_node("");
		} else {
			debug(3, "hdb::hdb_slist('%s'): node not found\n", str);
		}
		
		return true;
	}

#define CMD_SORT 0
#define CMD_MOVE_LEFT 1
#define CMD_MOVE_RIGHT 2
#define CMD_RM 3

	int hdb_cmd(int cmd, const char* name) {
		db_load();
		node *n = current_root->chain_find_node(name);
		if ( n ) {
			debug(3, "hdb::cmd(%d, '%s'): found node['%s']\n", cmd, name, n->get_name());
			switch (cmd) {
				case CMD_SORT:
					n->sort();
					break;
				case CMD_MOVE_LEFT:
					n->move_left();
					break;
				case CMD_MOVE_RIGHT:
					n->move_right();
					break;
				case CMD_RM:
					debug(3, "hdb::hdb_cmd(rm, '%s'): found node['%s'], deleting\n", name, n->get_name());
					n->remove();
					delete n;
					break;

			}
			need_write++;
		} else {
			debug(3, "hdb::hdb_move_right('%s'): node not found\n", name);
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
		current_root->serialize_to_file(stdout, DO_NOT_ESCAPE);
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
		
		// get data from environment
		if ( ! strcmp("%ENV", data) ) {
			data = getenv(chain);
			if ( ! data )
				data="";
		};

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

	int hdb_dump (const char* str) {
		db_load();
		root->dump();
		return true;
	}

	int hdb_create(const char *filename) {
		FILE *f;
		const char *lfilename;
		
		if (! filename || !strlen(filename)) {
			lfilename = get_dbfilename();
		} else {
			lfilename = filename;
			set_dbfilename(filename);
		}

		debug(4, "hdb::hdb_create(%s): creating '%s'\n", filename, lfilename);
		if (! (f=fopen(lfilename, "w")))
			return false;
		db_file=f;
		is_db_already_readed=true;

		if (! ( db_write() && db_close() ))
			return false;

		is_db_already_readed=false;

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
			return false;
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
		char tmpname[128];
		char editor[255];
		char buf[255];
		snprintf(tmpname, sizeof(tmpname), "/tmp/hdb.tmp.%d.%d", (int)time(NULL), getpid());

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
    int ch;
    int result=false;
	int quotation = 0;

	debug(4, "hdb::main(): parsing cmdline, optind=%d\n", optind);
	optind=1;
    while ((ch = getopt(argc, argv, "dD:qlecf:")) != -1){
		debug(5, "hdb::main(): getopt() returns %c: \n", ch);
        switch(ch) {
            case 'D': node_delimiter = strdup(optarg);
                     break;
            case 'f': set_dbfilename(optarg);
                     break;
            case 'd': debug_level++;
                     break;
            case 'q': quotation++;
                     break;
            case 'l': opt |= PRINT_LOCAL;
                     break;
            case 'e': opt |= PRINT_EXPORT; 
                     break;
            case 'c': opt |= PRINT_COUNT;
                     break;
            default: show_usage(argv[0]);
                     break;
        }
    }
	if ( quotation ) {
		opt |= (quotation==1)?PRINT_DQUOT:0;
		opt |= (quotation==2)?PRINT_QUOT:0;
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

		debug(4, "hdb::main(): cmd: '%s'\n", cmd);

		if ( (!strcmp(cmd, "slist")) || (!strcmp(cmd, "sls")) )
			result = hdb_slist(param0);
		else if ( (! strcmp(cmd, "ls")) || (!strcmp(cmd, "list")))
			result = hdb_print(param0, PRINT_CHAINNAME|PRINT_DATA);
		else if (! strcmp(cmd, "get"))
			result = hdb_print(param0, PRINT_DATA);
		else if (! strcmp(cmd, "lskeys"))
			result = hdb_print(param0, PRINT_NAME);
		else if (! strcmp(cmd, "lschains"))
			result = hdb_print(param0, PRINT_CHAINNAME);
		else if (! strcmp(cmd, "set") )
			result = hdb_set(param0);
		else if (! strcmp(cmd, "rename") )
			result = hdb_rename(param0, param1), optind++;
		else if (! strcmp(cmd, "dump") )
			result = hdb_dump(param0);
		else if (! strcmp(cmd, "show") )
			result = hdb_show(param0);
		else if (! strcmp(cmd, "rm") )
			result = hdb_cmd(CMD_RM, param0);
		else if (! strcmp(cmd, "mv_right") )
			result = hdb_cmd(CMD_MOVE_RIGHT, param0);
		else if (! strcmp(cmd, "mv_left") )
			result = hdb_cmd(CMD_MOVE_LEFT, param0);
		else if (! strcmp(cmd, "sort") )
			result = hdb_cmd(CMD_SORT, param0);
		else if (! strcmp(cmd, "edit") )
			result = hdb_edit(argv[0]);
		else if (!strcmp(cmd, "export") )
			result = hdb_export(argv[0]);
		else if (! strcmp(cmd, "import") )
			result = hdb_import(argv[0]);
		else if (! strcmp(cmd, "create") )
			result = hdb_create(param1);
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


extern "C"
{
#ifdef SHELL
int hdbcmd (int argc, char **argv) {
#else
int main (int argc, char **argv) {
#endif
	if (! app)
		app = new hdb;
	int result =  app->main(argc, argv);

#ifndef SHELL
	delete app;
	app = NULL;
#endif

	return result;
}

}



/*************************************************************************************************************/

// print callback
bool walk_print (node* n, void* func_data, int options) {
	char *chain_name;
	
	if (strlen(n->get_data())) {
		char *pattern = (char*) func_data;
		chain_name = n->get_fullchain();
		char strbuf[strlen(chain_name)+strlen(n->get_data())+16];
		snprintf(strbuf, sizeof(strbuf), "%s=%s", chain_name, n->get_data());

		int o = options&(PRINT_CHAINNAME|PRINT_NAME|PRINT_DATA);

		debug(5, "walk_print(node[%s], pattern='%s' opt=%d, o=%d)\n", n->get_name(), pattern, options, o);
		if ( match_wildcard(pattern, strbuf) ) {
			switch(o) {
				case PRINT_CHAINNAME|PRINT_DATA:
					node::print_pair(chain_name, n->get_data(), options);
					break;
				case PRINT_NAME|PRINT_DATA:
					node::print_pair(n->get_name(), n->get_data(), options);
					break;
				case PRINT_DATA:
					node::print_pair(NULL, n->get_data(), options);
					break;
				case PRINT_NAME:
					node::print_pair(n->get_name(), NULL, options);
					break;
				case PRINT_CHAINNAME:
					node::print_pair(chain_name, NULL, options);
					break;
			};
		}

		free(chain_name);
	}

	return true;
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

/* Wildcard code from ndtpd */
/*
 * Copyright (c) 1997, 98, 2000, 01  
 *    Motoyuki Kasahara
 *    ndtpd-3.1.5
 */

/*
 * Do wildcard pattern matching.
 * In the pattern, the following characters have special meaning.
 * 
 *   `*'    matches any sequence of zero or more characters.
 *   '\x'   a character following a backslash is taken literally.
 *          (e.g. '\*' means an asterisk itself.)
 *
 * If `pattern' matches to `string', 1 is returned.  Otherwise 0 is
 * returned.
 */
int match_wildcard(const char *pattern, const char *string)
{
    const char *pattern_p = pattern;
    const char *string_p = string;

    while (*pattern_p != '\0') {
	if (*pattern_p == '*') {
	    pattern_p++;
	    if (*pattern_p == '\0')
		return 1;
	    while (*string_p != '\0') {
		if (*string_p == *pattern_p
		    && match_wildcard(pattern_p, string_p))
		    return 1;
		string_p++;
	    }
	    return 0;
	} else {
	    if (*pattern_p == '\\' && *(pattern_p + 1) != '\0')
		pattern_p++;
	    if (*pattern_p != *string_p)
		return 0;
	}
	pattern_p++;
	string_p++;
    }

    return (*string_p == '\0');
}
