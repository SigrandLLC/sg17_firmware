#define PORT_BASED 0

struct statistics_t {
	unsigned long rx, tx;
};

struct switch_t {
	struct statistics_t stat[27];
};

#define TAG_BASED 1
