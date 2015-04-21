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

union number {
    int i;
    double d;
};

struct case_info {
    int case_num;
    int funct_num;
    int num_matched;
    enum {INTEGER, DOUBLE} number_type;
    union number a;
    union number b;
};

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

static char *_factor(struct case_info*);
static char *_fib(struct case_info*);
static char *_add(struct case_info*);
static char *_sub(struct case_info*);
static char *_mul(struct case_info*);
static char *_div(struct case_info*);
static char *_exp(struct case_info*);

static char* (*folder_functions[])(struct case_info*) = {
    _factor,
    _fib,
    _add,
    _sub,
    _mul,
    _div,
    _exp,
};

/*
int isPrime(int n)
{
    int i;
    if(n == 2)
        return 1;
    for(i = 2; i < sqrt(n); i++)
    {
        if(n % i == 0)
            return 0;
    }
    return 1;
}

int* primeFactor(int n)
{
    if(n < 0)
    {
        printf("Bad\n");
    }
    static int primes[200];
    int i, count = 0;
    for(i = 2; i < n; i++)
    {
        if(n % i == 0)
        {
            if(isPrime(i) == 1)
            {
                primes[count] = i;
                count++;
            }
        }
    }
    primes[count] = '\0';
    return primes;
}
*/

static char *_add(struct case_info *current_case)
{
    static char string[1000];
    if (current_case->number_type == INTEGER)
        sprintf(string, "%d\n", current_case->a.i + current_case->b.i);
    else
        sprintf(string, "%lf\n", current_case->a.d + current_case->b.d);
    return string;
}

static char *_factor(struct case_info *current_case)
{
    static char string[1000];
    if (current_case->number_type == INTEGER)
        sprintf(string, "%d\n", current_case->a.i + current_case->b.i);
    else
        sprintf(string, "%lf\n", current_case->a.d + current_case->b.d);
    return string;
}

static char *_fib(struct case_info *current_case)
{
    static char string[1000];
    if (current_case->number_type == INTEGER)
        sprintf(string, "%d\n", current_case->a.i + current_case->b.i);
    else
        sprintf(string, "%lf\n", current_case->a.d + current_case->b.d);
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
    int i;
    // union number a, b;
    int ia, ib, numMatched, intNumMatched, doubleNumMatched, isDouble = 0;
    double da, db;
    for (i = 0; i < 7; i++) {
        if (strcmp(path, folders[i]) == 0) {
            case_value = 0;
            break;
        } else if (strncmp(path, folders[i], strlen(folders[i])) == 0) {
            if (strcmp(path + strlen(folders[i]), doc_path) == 0) {
                case_value = 1;
                break;
            }

            doubleNumMatched = sscanf(path + strlen(folders[i]), "/%lf/%lf", &da, &db);
            if ((intNumMatched = sscanf(path + strlen(folders[i]), "/%d/%d", &ia, &ib)) > doubleNumMatched)
                numMatched = intNumMatched;
            else {
                numMatched = doubleNumMatched;
                isDouble = 1;
            }

            if (numMatched == 1) {
                if (i != 0 && i != 1)
                    case_value = 2;
                else
                    case_value = 3;
                break;
            } else if (numMatched == 2) {
                if (i == 0 || i == 1) // incorrect number of arguments
                    case_value = 4;
                else
                    case_value = 5;
                break;
            } else {
                case_value = -1;
                break;
            }
        }
    }

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
    return_struct.num_matched = numMatched;

    return return_struct;
}

static int mathfs_getattr(const char *path, struct stat *stbuf)
{
    struct case_info current_case;
    char *content;
    int res = -ENOENT;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        res = 0;
    }

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

