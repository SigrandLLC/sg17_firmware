#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>

#define DEBUG
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
//
//  EXAMPLE TREE
//
//  *ROOT*
//    |       |-t-t-t-t-t
//    n-n-n-n-n       |
//      |   |         a-a-a-a-a
//      |   z-z-z-z-z
//      |           |
//      y-y-y-y-y   q-q-q-q-q
//        |
//        r-r-r-r
//      

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

	const char *get_name() { return name; }
	const char *get_data() { return data; }

	void set_namedata(const char* name, const char* data) {
		set_name(name);
		set_data(data);
	}

	const char *set_name(const char *str) { assert(str); free(name); return name = strdup(str); }
	const char *set_data(const char *str) { assert(str); free(data); return data = strdup(str); }

	node *get_next() { return next; }
	node *get_prev() { return prev; }
	node *get_root() { return root; }
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
		debug("node[%x]::get_first()\n", this);
		if ( prev == NULL )
			return this;
		assert(prev != NULL);
		return prev->get_first();
	}

	node *get_last() {
		debug("node[%x]::get_last()\n", this);
		assert(this);
		if ( next == NULL )
			return this;
		assert( next != NULL );
		return next->get_last();
	}

	node *find_name(const char* str, int maxlevel=INT_MAX) {
		debug("node[%x]::find_name(%s, %d)\n", this, str, maxlevel);
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
		debug("node[%x]::get_lastchild()\n", this);
		assert (fchild != NULL);
		return fchild->get_last();
	}

	node *insert_after(node* n) {
		debug("node[%x]::insert_after(%x)\n", this, n);
		assert(n);
		node* t = next;
		next=n;
		next->prev=this;
		next->next=t;
		if (t)
			t->prev=next;
	}

	node *insert_before(node* n) {
		debug("node[%x]::insert_before(%x)\n", this, n);
		assert(n);
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
	}
	
	node *add_child(node* child) {
		debug("node[%x]::add_child(%x)\n", this, child);
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

	void unserialize(FILE *stream=stdin, int level=0) {
		char *buf=NULL;
		size_t strsize=0;

		if ( getline( &buf, &strsize, stream ) != -1 ) {
			if ( parse_pair( buf ) ) {
				;
			};


			free(buf);
		}


	}

	void dump(int level=0) {
		for (int i=0; i < level; i++)
			printf("\t");
		printf("node[%x]: %s=%s\n", this, name, data);

		node *c=fchild;
		while ( c ) {
			c->dump(level+1);
			c=c->get_next();
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

	printf("dump:\n");
	root->dump();

	printf("find:\n");
	node *n = root->find_name("n2");
	if (n) 
		n->dump();

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

