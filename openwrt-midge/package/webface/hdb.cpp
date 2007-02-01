#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>

#define DEBUG
#define READ_BUF_SIZE 2048


#ifdef DEBUG
void debug(char *format, ...){
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}
#else
inline void _debug(char* format, ...){}
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
// 1. absolute root: next=NULL, prev=NULL, root=NULL
// 2. first: prev=NULL
// 3. not first: root=NULL
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
// 1. q5->get_root()->get_first(): returns z1
// 2. q5->get_root()->get_last(): returns z5
// 3. q4->get_first(): return q1
// 4. q4->get_root(): return z4
// 4. q4->get_first()->get_prev(): return NULL

char *str_escape(const char *source);
char *str_unescape(const char *source);
int match_wildcard(const char *pattern, const char *string);
int is_wildcarded(const char* str);

class node {
	node *root;
	node *fchild;
	node *next;
	node *prev;

	char* name;
	char* data;

	void init_values() {
		root=NULL; 
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
		root=r;
	}

	node(node* r, const char* name, const char* data) {
		init_values();
		root=r;
		set_namedata(name, data);
	}

	node(const char* pair) {
		init_values();
		parse_pair(pair);
	}

	const char *get_name() { return name; }
	const char *get_data() { return data; }

	void set_namedata(const char* name, const char* data) {
		set_name(name);
		set_data(data);
	}

	const char *set_name(const char *str) { assert(str); free(name); return name = strdup(str); }
	const char *set_data(const char *str) { assert(str); free(data); return data = strdup(str); }
	void set_root(node* newroot) { assert(newroot); root = newroot; }

	node *get_next() { return next; }
	node *get_prev() { return prev; }
	node *get_root() { 
		if ( get_prev() ) 
			return get_first()->get_root();
		else 
			return root;
	}

	node *get_absolute_root() {
		node *n = get_root();
		while ( ! n )
			n = get_root();
		return n;
	}

	node *get_fchild() { return fchild; }
	
	bool parse_pair(const char* str) {
		int len = strlen(str);
		if (!len)
			return false;
		char s[len+10];

		strcpy(s, str);
		if (s[len-1]=='\n')
			s[len-1]='\0';

		char *eq=strchr(s, '=');
		if (!eq || eq==s) {
			fprintf(stderr, "Error: can't parse: %s", s);
			return false;
		}
		eq[0]='\0';
		strcpy(name, s );
		strcpy(data, eq+1);

		return true;
	}

	node *get_first() {
		debug("node['%s']::get_first(): ", get_name());
		node *n = prev;
		if ( !n ) {
			debug("node['%s']\n", get_name());
			return this;
		}
		
		// look for first: on first *prev should be NULL
		while ( true ) {
			if ( n->get_prev() )
				n = n->get_prev();
			else {
				debug("node['%s']\n", get_name());
				return n;
			}
		}
		
	}

	node *get_last() {
		debug("node['%s']::get_last():  ", get_name());
		assert(this);
		if ( next == NULL ) {
			debug("node['%s']\n", get_name());
			return this;
		}
		assert( next != NULL );
		return next->get_last();
	}
	
	int get_level(int curr_level=0) {
		if ( ! get_root() )
			return curr_level;
		get_root()->get_level(curr_level+1);
	}

	node *find_name(const char* str, int maxlevel=INT_MAX) {
		debug("node[%x]::find_name('%s', %d)\n", this, str, maxlevel);
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
		debug("node['%s']::get_lastchild()\n", get_name());
		assert (fchild != NULL);
		return fchild->get_last();
	}

	node *insert_after(node* n) {
		assert(n);
		debug("node['%s']::insert_after('%s')\n", get_name(), n->get_name());
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
		debug("node['%s']::insert_before('%s')\n", get_name(), n->get_name());
		node* t=prev;
		prev=n;
		prev->prev=t;
		prev->next=this;
		if (t) {
			t->next=prev;
		}
		if (root) { 
			prev->root=root;
			root=NULL;
		}
		return n;
	}
	
	node *add_child(node* child) {
		assert(child);
		debug("node['%s']::add_child('%s')\n", get_name(), child->get_name());
		assert (child);
		if ( fchild == NULL ) {
			fchild = child;
			child->root=this;
		} else {
			get_lastchild()->insert_after(child);
		}
		return child;
	}
			
		
	node *new_child(){
		return add_child(new node(this));
	}	

	node *new_child(const char* str){
		debug("node['%s']::new_child('%s')\n", get_name(), str);
		return add_child(new node(str));
	}	


	node *new_sibling(const char* str){
		debug("node['%s']::new_sibling('%s')\n", get_name(), str);
		return get_last()->insert_after(new node(str));
	}	

	node *new_sibling(){
		return get_last()->insert_after(new node(this));
	}	

	void dump(int level=0) {
		for (int i=0; i < level; i++)
			printf("\t");
		printf("node['%s']: data=%s, root='%s'\n", get_name(), get_data(), get_root()?get_root()->get_name():"NULL");

		node *c=fchild;
		while ( c ) {
			c->dump(level+1);
			c=c->get_next();
		}
	}

	int serialize(FILE *stream=stdout, int level=0) {
		int i;
		for ( i=0; i < level; i++ )
			fprintf(stream, "\t");
		char *esc_name = str_escape(name);
		char *esc_data = str_escape(data);

		fprintf(stream, "%s=%s\n", esc_name, esc_data);

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

		debug("  node['%s']::unserialize(): my_level=%d, level=%d, read_buf='%s'\n", get_name(), my_level, level, str);
		if ( level == my_level ) {
			debug("     node::unserialize(): equal\n");
			n = new_sibling(str);
		} else if ( level > my_level ) {
			debug("     node::unserialize(): greater\n");
			n = new_child(str);

		} else if ( level < my_level ) {
			debug("     node::unserialize(): less\n");
			n = this;
			for ( i = 0; i < (my_level - level); i++) {
				debug("         node::unserialize(): i=%d\n", i);
				n = n->get_root();
				assert( n != NULL );
			}
			n = n->new_sibling(str);
		}
		return n;
	}

	static int unserialize_file(const char* filename, node* node_root) {
		FILE *file; 
		if ( ! (file = fopen(filename, "r")) ) {
			perror("fopen:");
			return false;
		};

		int i;
		int curr_level = 0;
		int readed_level = 0;
		char read_buf[READ_BUF_SIZE];
		char *pair_begin;
		node *curr_node = node_root;

		fgets(read_buf, sizeof(read_buf), file);
		if ( strcmp(read_buf, "/\n") ) {
			debug("  node::unserialize_file(): bad format\n");
			return false;
		}
		node_root->parse_pair(read_buf);

		while ( fgets(read_buf, sizeof(read_buf), file) ) {

			if ( read_buf[strlen(read_buf)-1] == '\n' )
				read_buf[strlen(read_buf)-1]='\0';

			pair_begin = NULL;
			debug("  -------------\n");
			for ( readed_level = 0; readed_level < sizeof(read_buf); readed_level++ )
				if ( read_buf[readed_level] != '\t' ) {
					pair_begin = &read_buf[readed_level];
					break;
				}
			curr_node = curr_node->unserialize(pair_begin, readed_level);
		}

	}
};

int main ()
{
	node *root=new node;
	root->new_child();
	root->new_child();
	root->new_child();
	root->new_child();
	root->new_child();
	root->new_child()->set_name("n1");
	root->new_child()->set_data("d1");
	root->new_child();
	root->new_child();
	root->new_child();
	root->new_child();
	root->new_child()->new_child();
	root->new_child()->new_child()->new_child()->parse_pair("name1=value1");
	root->new_child()->new_child()->get_last()->new_child();
	root->new_child()->new_child()->get_last()->new_child()->set_namedata("n2", "d2");
	root->get_last()->new_child()->get_last()->new_child();
	root->new_child()->new_child()->get_last()->new_child();
	root->new_child()->new_child()->get_last()->new_child();
	root->get_first()->get_last()->get_first()->new_child();
	printf("get_level(): %d\n", root->new_child()->new_child()->new_child()->new_child()->get_level());

	printf("dump:\n");
	root->dump();

	printf("find:\n");
	node *n = root->find_name("n2");
	if (n) 
		n->dump();

	printf("serialize:\n");
	root->serialize(stdout);

	root = new node;
	root->set_name("/");
	printf("unserialize_file:\n");
	node::unserialize_file("fixtures.hdb", root);

	printf("dump:\n");
	root->dump();

	printf("serialize:\n");
	root->serialize(stdout);


	return 0;
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

