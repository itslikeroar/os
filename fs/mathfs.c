/* 
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall mathfs.c `pkg-config fuse --cflags --libs` -o mathfs
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include "mathfs.h"

static char *doc_path = "/doc";
static const char *folders[] = {
	"/factor",
	"/fib",
	"/add",
	"/sub",
	"/mul",
	"/div",
	"/exp",
};
static const char *doc_str[] = {
	"factors\n",
	"gets the nth fib\n",
	"adds\n",
	"subtracts\n",
	"multiplies\n",
	"divides\n",
	"a^b\n"
};

static char* (*folder_functions[])(struct case_info*) = {
	_fac,
	_fib,
	_add,
	_sub,
	_mul,
	_div,
	_exp,
};

static char *_add(struct case_info *current_case)
{
	static char string[1000];
	if (current_case->number_type == INTEGER)
		sprintf(string, "%d\n", current_case->a.i + current_case->b.i);
	else
		sprintf(string, "%lf\n", current_case->a.d + current_case->b.d);
	return string;
}

int isPrime(int n)
{
    int i;
    if(n == 2)
        return 1;
    for (i = 2; i <= sqrt(n); i++) {
        if(n % i == 0)
            return 0;
    }
    return 1;
}

static char *_fac(struct case_info *current_case)
{
    static char *intError = "Argument not an Integer\n";
    static char *negError = "Argument is below zero\n";
    static char primes[1000];
    char mychar[100];
    int i, j = 0, count = 0, flag = 0;;
    if (current_case->number_type == DOUBLE)
        return intError;
    else if (current_case->a.i < 0)
        return negError;
    else {
        for(i = 2; i <= current_case->a.i; i++) {
            if(current_case->a.i % i == 0) {
                if (isPrime(i) == 1) {
                    sprintf(mychar, "%d\n", i);
                    if (strlen(mychar) > 1) {
                        while (mychar[j] != '\0') {
                            primes[count] = mychar[j];
                            count++;
                            j++;
                        }
                    } else {
                        primes[count] = mychar[j];
                        count++;
                        j++;
                    }
                    j = 0;
                    flag = 1;
                }
            }
            if(flag == 1) {
                primes[count] = '\n';
                flag = 0;
            }
        }
    }
    primes[count] = '\0';
    return primes;
}

static char *_fib(struct case_info *current_case)
{
	static char string[1000];
	char buf[100];
	unsigned long int i, a = 0, b = 1;

	// printf("MANY HERE: %d\n", current_case->a.i);
	if (current_case->number_type != INTEGER || current_case->a.i <= 0)
		return NULL;

	// put the first one fib num in the string
	sprintf(string, "0\n");

	if (current_case->a.i > 1)
		sprintf(string, "0\n1\n");
	
	a = 1;
	b = 1;
	for (i = 2; i < current_case->a.i; i++) {
		unsigned long int temp = b;
		sprintf(buf, "%lu\n", b);
		strcat(string, buf);
		b = a + b;
		a = temp;
		if (b < a) {
			sprintf(string, "Overflow error\n");
			break;
		}
	}

	return string;
}

static char *_sub(struct case_info *current_case)
{
	static char sub_string[1000];
	if (current_case->number_type == INTEGER)
		sprintf(sub_string, "%d\n", current_case->a.i - current_case->b.i);
	else
		sprintf(sub_string, "%lf\n", current_case->a.d - current_case->b.d);
	return sub_string;
}

static char *_mul(struct case_info *current_case)
{
	static char string[1000];
	if (current_case->number_type == INTEGER)
		sprintf(string, "%d\n", current_case->a.i * current_case->b.i);
	else
		sprintf(string, "%lf\n", current_case->a.d * current_case->b.d);
	return string;
}

static char *_div(struct case_info *current_case)
{
	static char string[1000];
	if (current_case->number_type == INTEGER && current_case->b.i != 0)
		sprintf(string, "%d\n", current_case->a.i / current_case->b.i);
	else if (current_case->number_type == DOUBLE && current_case->b.d != 0.0)
		sprintf(string, "%lf\n", current_case->a.d / current_case->b.d);
	else
		return NULL;
	return string;
}

static char *_exp(struct case_info *current_case)
{
	static char string[1000];
	if (current_case->number_type == INTEGER) {
		int total = 1;
		int i;
		for (i = 0; i < current_case->b.i; i++)
			total *= current_case->a.i;
		sprintf(string, "%d\n", total);
	} else
		return NULL;
		// sprintf(string, "%lf\n", current_case->a.d + current_case->b.d);
	return string;
}

struct case_info find_case(const char *path) {
	struct case_info return_struct;
	int case_value;
	// int return_value = 0;
	int i, j;
	int num_slashes = 0;
	// union number a, b;
	int ia, ib, numMatched, isDouble = 0;
	double da, db;
	for (i = 0; i < 7; i++) {
		if (strcmp(path, folders[i]) == 0) {
			case_value = 0;	// path is one of the folders
			break;
		} else if (strncmp(path, folders[i], strlen(folders[i])) == 0) {
			if (strcmp(path + strlen(folders[i]), doc_path) == 0) {
				case_value = 1;	// path is doc in one of the folders
				break;
			}

			if (sscanf(path + strlen(folders[i]), "/%d.%d/%d.%d", &ia, &ia, &ia, &ia) == 4 ||
				sscanf(path + strlen(folders[i]), "/%d/%d.%d", &ia, &ia, &ia) == 3 ||
				sscanf(path + strlen(folders[i]), "/%d.%d/%d", &ia, &ia, &ia) == 3 ||
				sscanf(path + strlen(folders[i]), "/%d.%d", &ia, &ia) == 2)
				isDouble = 1;

			if (isDouble)
				numMatched = sscanf(path + strlen(folders[i]), "/%lf/%lf", &da, &db);
			else
				numMatched = sscanf(path + strlen(folders[i]), "/%d/%d", &ia, &ib);

			if (numMatched == 1) {	// one argument given
				if (i == 0 || i == 1)
					case_value = 3;	// function takes 1 argument
				else
					case_value = 2;	// function takes 2 arguments
				break;
			} else if (numMatched == 2) {	// two arguments given
				if (i == 0 || i == 1)
					case_value = 4;	// function takes 1 argument
				else
					case_value = 5;	// function taken 2 arguments
				break;
			} else {
				case_value = -1;	// invalid path
				break;
			}
		}
	}

	for (j = 0; j < strlen(path); j++) {
		if (path[j] == '/') {
			num_slashes++;
		}
		if (num_slashes > 2 && (i == 0 || i == 1)) {
			case_value = -1;
			break;
		} else if (num_slashes > 3) {
			case_value = -1;
			break;
		}
	}

	printf("####### path: '%s'\tcase: %d\n", path, case_value);

	if (isDouble) {
		return_struct.number_type = DOUBLE;
		return_struct.a.d = da;
		return_struct.b.d = db;
	} else {
		return_struct.number_type = INTEGER;
		return_struct.a.i = ia;
		return_struct.b.i = ib;
	}

	return_struct.case_num = case_value;
	return_struct.funct_num = i;
	// return_struct.num_matched = numMatched;

	return return_struct;
}

static int mathfs_getattr(const char *path, struct stat *stbuf)
{
	struct case_info current_case;
	char *content;
	int res = -ENOENT;

	memset(stbuf, 0, sizeof(struct stat));
	stbuf->st_mtime = stbuf->st_atime = stbuf->st_ctime = time(0);
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		res = 0;
	} else {
		current_case = find_case(path);

		if (current_case.case_num == 0 || current_case.case_num == 2) {
			stbuf->st_mode = S_IFDIR | 0755;
			stbuf->st_nlink = 2;
			res = 0;
		} else if (current_case.case_num == 1) {
			stbuf->st_mode = S_IFREG | 0444;
			stbuf->st_nlink = 1;
			stbuf->st_size = strlen(doc_str[current_case.funct_num]);
			res = 0;
		} else if (current_case.case_num == 3 || current_case.case_num == 5) {
			content = folder_functions[current_case.funct_num](&current_case);
			if (content != NULL) {
				stbuf->st_mode = S_IFREG | 0444;
				stbuf->st_nlink = 1;
				stbuf->st_size = strlen(content);
				res = 0;
			}
			else
				printf("here!!!!!!!!!!!!!!!!!!!!! %d\n", current_case.funct_num);
		}
	}
	return res;
}

static int mathfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	struct case_info current_case;
	int found = 0;
	
	if (strcmp(path, "/") == 0)
	{
		int i;
		for (i = 0; i < 7; i++)
			filler(buf, folders[i] + 1, NULL, 0);
		found = 1;
	}
	else
	{
		current_case = find_case(path);

		if (current_case.case_num == 0) {
			filler(buf, doc_path + 1, NULL, 0);
			found = 1;
		} else if (current_case.case_num == 2)
			found = 1;
	}

	if (found == 1)
	{
		filler(buf, ".", NULL, 0);
		filler(buf, "..", NULL, 0);
		return 0;
	}
	else
		return -ENOENT;
}

static int mathfs_open(const char *path, struct fuse_file_info *fi)
{
	struct case_info current_case;
	int retval = -ENOENT;
	
	current_case = find_case(path);

	if (current_case.case_num == 2 || current_case.case_num == 3 || current_case.case_num == 5)
		retval = 0;

	if ((fi->flags & 3) != O_RDONLY)
		retval = -EACCES;
	
	return retval;
}

static int mathfs_read(const char *path, char *buf, size_t size, off_t offset,
			  struct fuse_file_info *fi)
{
	struct case_info current_case;
	int retval = -ENOENT;
	const char *content = NULL;
	size_t len;

	current_case = find_case(path);

	if (current_case.case_num == 1)
		content = doc_str[current_case.funct_num];
	else if (current_case.case_num == 3 || current_case.case_num == 5)
		content = folder_functions[current_case.funct_num](&current_case);

	if (content != NULL) {
		len = strlen(content);
		if (offset < len)
		{
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, content + offset, size);
		}
		else
			size = 0;
		retval = size;
	}

	return retval;
}

static struct fuse_operations mathfs_oper = {
	.getattr    = mathfs_getattr,
	.readdir    = mathfs_readdir,
	.open       = mathfs_open,
	.read       = mathfs_read,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &mathfs_oper, NULL);
}

