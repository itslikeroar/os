#ifndef MATHFS_H
#define MATHFS_H

union number {
	int i;
	double d;
};

struct case_info {
	int case_num;
	int funct_num;
	enum {INTEGER, DOUBLE} number_type;
	union number a;
	union number b;
};

static char *_fac(struct case_info*);
static char *_fib(struct case_info*);
static char *_add(struct case_info*);
static char *_sub(struct case_info*);
static char *_mul(struct case_info*);
static char *_div(struct case_info*);
static char *_exp(struct case_info*);

#endif