/* 
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char *doc_path = "/doc";
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

static char *_factor(void*, void*);
static char *_fib(void*, void*);
static char *_add(void*, void*);
static char *_sub(void*, void*);
static char *_mul(void*, void*);
static char *_div(void*, void*);
static char *_exp(void*, void*);

static char* (*folder_functions[])(void*, void*) = {
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
// unsigned int* nfib(int n)
// {
//     static unsigned int fib[1000000];
//     int fib1 = 1;
//     int fib2 = 1;
//     int total = 0;
//     int count = 0;
//     if(n == 0)
//     {
//         fib[0] = 0;
//         count = 1;
//     }
//     else if(n == 1)
//     {
//         fib[0] = 1;
//         count = 1;
//     }
//     else if(n >= 2)
//     {
//         fib[0] = 1;
//         fib[1] = 1;
//         count = 2;
//         while(count != n)
//         {
//             total = fib1 + fib2;
//             fib1 = fib2;
//             fib2 = total;
//             fib[count] = total;
//             count++;
//         }
//     }
//     fib[count] = '\0';
//     return fib;
// }

static char *_add(void *x1, void *x2)
{
    int *a =(int*) x1;
    int *b = (int*) x2;
    static char sum_string[1000];
    sprintf(sum_string, "%d\n", *a + *b);
    return sum_string;
}

static char *_factor(void *x1, void *x2)
{
    int *a =(int*) x1;
    int *b = (int*) x2;
    static char sum_string[1000];
    sprintf(sum_string, "%d\n", *a + *b);
    return sum_string;
}

static char *_fib(void *x1, void *x2)
{
    int *a =(int*) x1;
    int *b = (int*) x2;
    static char sum_string[1000];
    sprintf(sum_string, "%d\n", *a + *b);
    return sum_string;
}

static char *_sub(void *x1, void *x2)
{
    int *a =(int*) x1;
    int *b = (int*) x2;
    static char sum_string[1000];
    sprintf(sum_string, "%d\n", *a + *b);
    return sum_string;
}

static char *_mul(void *x1, void *x2)
{
    int *a =(int*) x1;
    int *b = (int*) x2;
    static char sum_string[1000];
    sprintf(sum_string, "%d\n", *a + *b);
    return sum_string;
}

static char *_div(void *x1, void *x2)
{
    int *a =(int*) x1;
    int *b = (int*) x2;
    static char sum_string[1000];
    sprintf(sum_string, "%d\n", *a + *b);
    return sum_string;
}

static char *_exp(void *x1, void *x2)
{
    int *a =(int*) x1;
    int *b = (int*) x2;
    static char sum_string[1000];
    sprintf(sum_string, "%d\n", *a + *b);
    return sum_string;
}
// int sub(int x1, int x2)
// {
//     return x1 - x2;
// }
// double div(int x1, int x2)
// {
//     if(x2 == 0)
//         return 0;
//     else
//         return x1 / x2;
// }

// int exp1(int x1, int x2)
// {
//     int i;
//     int total = 1;
//     for(i = 0; i < x2; i++)
//     {
//         total = total * x1;
//     }
//     return total;
// }

int find_case(path) {
    int case_value;
    int return_value = 0;
    int i;
    for (i = 0; i < 7; i++) {
        if (strcmp(path, folders[i]) == 0) {
            case_value = 1;
            break;
        } else if (strncmp(path, folders[i], strlen(folders[i])) == 0) {
            int a, b, numMatched;
            if (strcmp(path + strlen(folders[i]), doc_path) == 0) {
                case_value = 2;
                break;
            }

            numMatched = sscanf(path + strlen(folders[i]), "/%d/%d", &a, &b);
            // printf("%s matched %d\n", path, numMatched);

            if (numMatched == 1) {
                if (i != 0 && i != 1) {
                    case_value = 2;
                } else {
                    case_value = 4;
                }
                break;
            } else if (numMatched == 2) {
                if (i == 0 || i == 1) {
                    // printf("incorrect number of arguments to %s\n", folders[i] + 1);
                    case_value = 5;
                } else {
                    case_value = 6;
                }
                break;
            } else {
                // printf("wat, numMatched: %d\n", numMatched);
                case_value = 7;
                break;
            }
        }
    }
    return_value = i << 16;
    return_value += case_value;

    return return_value;
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
    int res = -ENOENT;
    printf("#####################getattr(\"%s\")\n", path);

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        res = 0;
    }
    else if (strcmp(path, hello_path) == 0) {
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = strlen(hello_str);
        res = 0;
    }

    printf("#####################getattr path: '%s'\n", path);

    // char temp[1000];

    int i;
    for (i = 0; i < 7; i++) {
        if (strcmp(path, folders[i]) == 0) {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
            res = 0;
            break;
        } else if (strncmp(path, folders[i], strlen(folders[i])) == 0) {
            int a, b, numMatched;
            if (strcmp(path + strlen(folders[i]), doc_path) == 0) {
                stbuf->st_mode = S_IFREG | 0444;
                stbuf->st_nlink = 1;
                stbuf->st_size = strlen(doc_str[i]);
                res = 0;
                break;
            }

            numMatched = sscanf(path + strlen(folders[i]), "/%d/%d", &a, &b);
            // printf("%s matched %d\n", path, numMatched);

            if (numMatched == 1) {
                if (i != 0 && i != 1) {
                    stbuf->st_mode = S_IFDIR | 0755;
                    stbuf->st_nlink = 2;
                    res = 0;
                } else {
                    char *content = folder_functions[i](&a, &b);
                    stbuf->st_mode = S_IFREG | 0444;
                    stbuf->st_nlink = 1;
                    stbuf->st_size = strlen(content);
                    res = 0;
                }
                break;
            } else if (numMatched == 2) {
                if (i == 0 || i == 1) {
                    printf("incorrect number of arguments to %s\n", folders[i] + 1);
                } else {
                    char *content = folder_functions[i](&a, &b);
                    // sprintf(temp, "%d", c);
                    stbuf->st_mode = S_IFREG | 0444;
                    stbuf->st_nlink = 1;
                    stbuf->st_size = strlen(content);
                    res = 0;
                }
                break;
            } else {
                printf("wat, numMatched: %d\n", numMatched);
                break;
            }
        }
    }

    return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    int found = 0;
    printf("####################### path: '%s'\n", path);
    if (strcmp(path, "/") == 0)
    {
        filler(buf, hello_path + 1, NULL, 0);
        int i;
        for (i = 0; i < 7; i++)
            filler(buf, folders[i] + 1, NULL, 0);
        found = 1;
    }

    // int i;
    // for (i = 0; i < 7; i++)
    // {
    //     if (strcmp(path, folders[i]) == 0)
    //     {
    //         filler(buf, doc_path + 1, NULL, 0);
    //         found = 1;
    //     }
    // }
    int i;
    for (i = 0; i < 7; i++) {
        if (strcmp(path, folders[i]) == 0) {
            filler(buf, doc_path + 1, NULL, 0);
            found = 1;
            break;
        } else if (strncmp(path, folders[i], strlen(folders[i])) == 0) {
            int a, b, numMatched;
            if (strcmp(path + strlen(folders[i]), doc_path) == 0) {
                break;
            }

            numMatched = sscanf(path + strlen(folders[i]), "/%d/%d", &a, &b);
            // printf("%s matched %d\n", path, numMatched);

            if (numMatched == 1) {
                if (i != 0 && i != 1) {
                    found = 1;
                }
                break;
            } else if (numMatched == 2) {
                if (i == 0 || i == 1) {
                    printf("incorrect number of arguments to %s\n", folders[i] + 1);
                } else {
                    
                }
                break;
            } else {
                printf("wat, numMatched: %d\n", numMatched);
                break;
            }
        }
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

static int hello_open(const char *path, struct fuse_file_info *fi)
{
    int retval = -ENOENT;
    if (strcmp(path, hello_path) == 0)
    {
        if ((fi->flags & 3) != O_RDONLY)
            return -EACCES;
        return 0;
    }

    // printf("!!!!!!!!!!!!!!!!trying to open: %s\n", path);

    // int i;
    // for (i = 0; i < 7; i++) {
    //     if (strncmp(path, folders[i], strlen(folders[i])) == 0)
    //     {
    //         if (strcmp(path + strlen(folders[i]), doc_path) != 0)
    //             continue;

    //         if ((fi->flags & 3) != O_RDONLY)
    //             return -EACCES;

    //         return 0;
    //     }
    // }

    // return -ENOENT;
    
    int i;
    for (i = 0; i < 7; i++) {
        if (strcmp(path, folders[i]) == 0) {
            break;
        } else if (strncmp(path, folders[i], strlen(folders[i])) == 0) {
            int a, b, numMatched;
            if (strcmp(path + strlen(folders[i]), doc_path) == 0) {
                retval = 0;
                break;
            }

            numMatched = sscanf(path + strlen(folders[i]), "/%d/%d", &a, &b);
            // printf("%s matched %d\n", path, numMatched);

            if (numMatched == 1) {
                if (i != 0 && i != 1) {

                } else {
                    retval = 0;
                }
                break;
            } else if (numMatched == 2) {
                if (i == 0 || i == 1) {
                    printf("incorrect number of arguments to %s\n", folders[i] + 1);
                } else {
                    retval = 0;
                }
                break;
            } else {
                printf("wat, numMatched: %d\n", numMatched);
                break;
            }
        }
    }

    if ((fi->flags & 3) != O_RDONLY)
        retval = -EACCES;
    
    return retval;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi)
{
    int retval = -ENOENT;
    size_t len;

    // printf("hello_read: %d\n", path);
    
    //(void) fi;
    if(strcmp(path, hello_path) == 0)
    {
        len = strlen(hello_str);
        if (offset < len) {
            if (offset + size > len)
                size = len - offset;
            memcpy(buf, hello_str + offset, size);
        } else
            size = 0;
        return size;
    }
    

    // printf("\t\ttrying to read: %s\n", path);

    // int i = 0;
    // for (i = 0; i < 7; i++)
    // {
    //     if (strncmp(path, folders[i], strlen(folders[i])) == 0)
    //     {
    //         if (strncmp(path + strlen(folders[i]), doc_path, strlen(doc_path)) == 0)
    //         {
    //             len = strlen(doc_str[i]);
    //             if (offset < len)
    //             {
    //                 if (offset + size > len)
    //                     size = len - offset;
    //                 memcpy(buf, doc_str[i] + offset, size);
    //             }
    //             else
    //                 size = 0;
    //             return size;
    //         }
    //     }
    // }
    int i;
    for (i = 0; i < 7; i++) {
        if (strcmp(path, folders[i]) == 0) {
            break;
        } else if (strncmp(path, folders[i], strlen(folders[i])) == 0) {
            int a, b, numMatched;
            if (strcmp(path + strlen(folders[i]), doc_path) == 0) {
                len = strlen(doc_str[i]);
                if (offset < len)
                {
                    if (offset + size > len)
                        size = len - offset;
                    memcpy(buf, doc_str[i] + offset, size);
                }
                else
                    size = 0;
                retval = size;
                break;
            }

            numMatched = sscanf(path + strlen(folders[i]), "/%d/%d", &a, &b);
            // printf("%s matched %d\n", path, numMatched);

            if (numMatched == 1) {
                if (i != 0 && i != 1) {

                } else {
                    char *content = folder_functions[i](&a, &b);
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
                break;
            } else if (numMatched == 2) {
                if (i == 0 || i == 1) {
                    printf("incorrect number of arguments to %s\n", folders[i] + 1);
                } else {
                    char *content = folder_functions[i](&a, &b);
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
                break;
            } else {
                printf("wat, numMatched: %d\n", numMatched);
                break;
            }
        }
    }

    return retval;
}

static struct fuse_operations hello_oper = {
    .getattr    = hello_getattr,
    .readdir    = hello_readdir,
    .open       = hello_open,
    .read       = hello_read,
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &hello_oper, NULL);
}

