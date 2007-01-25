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

	node *find_name(const char* str, int level=INT_MAX) {
		debug("node[%x]::find_name(%s, %d)\n", this, str, level);
		node *found=NULL;
		if ( ! strcmp(str, name) )
			return this;

		if ( level <= 0 )
			return NULL;

		node *n=fchild;
		while ( n ) {
			found=n->find_name(str, level-1);
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
	root->get_last()->new_child();
	root->get_last()->new_child();
	root->get_last()->new_child();
	root->new_child()->set_name("n1");
	root->new_child()->set_data("d1");
	root->new_child();
	root->get_last()->new_child();
	root->get_last()->new_child();
	root->get_last()->new_child()->new_child();
	root->get_last()->new_child()->new_child()->new_child();
	root->get_last()->new_child()->new_child()->get_last()->new_child();
	root->get_last()->new_child()->new_child()->get_last()->new_child()->set_namedata("n2", "d2");
	root->get_last()->get_last()->new_child()->get_last()->new_child();
	root->get_first()->new_child()->new_child()->get_last()->new_child();
	root->get_first()->new_child()->new_child()->get_last()->new_child();
	root->get_last()->get_first()->get_last()->get_first()->new_child();

	printf("dump:\n");
	root->dump();

	printf("find:\n");
	node *n = root->find_name("n2");
	if (n) 
		n->dump();

	return 0;
}
