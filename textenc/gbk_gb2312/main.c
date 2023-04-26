#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "gbk2gb2312.h"

int readfromfile (const char *szFile, char *buf, int size)
{
    int fd = open (szFile, O_RDONLY);
    if (fd < 0)
    {
        return -1;
    }

    read (fd, buf, size);

    close (fd);
    fd = -1;

    return 0;
}

int writetofile (const char *szFile, const char *buf, int size)
{
    int fd = open (szFile, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    if (fd < 0)
    {
        return -1;
    }

    write (fd, buf, size);

    close (fd);
    fd = -1;

    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf ("need file name\n");
        return 0;
    }

    const char *szFilename = argv[1];
    const char *szOutputFilename = argv[2];

    printf ("filename: intput:%s, output:%s\n", szFilename, szOutputFilename);

    char inputbuf[512] = {};
    char outputbuf[512] = {};

    if (readfromfile (szFilename, inputbuf, sizeof(inputbuf)) < 0)
    {
        printf ("read from file failed: %s\n", szFilename);
        return -1;
    }

    int nLen = strlen (inputbuf);

    printf ("input:\n");
    for (int i = 0; i < nLen + 1; ++i)
    {
        printf ("%hhx ", ((unsigned char *)inputbuf)[i]);
    }
    printf ("\n");

    int outputsize = sizeof(outputbuf);
    enc_conv_gbk2gb2312 (inputbuf, sizeof(inputbuf), outputbuf, outputsize, '*');

    printf ("output:\n");
    for (int i = 0; i < nLen + 1; ++i)
    {
        printf ("%hhx ", ((unsigned char *)outputbuf)[i]);
    }
    printf ("\n");

    if (writetofile (szOutputFilename, outputbuf, strlen (outputbuf)) < 0)
    {
        printf ("write to file failed: %s\n", szOutputFilename);
        return -1;
    }

    printf ("\n");

    return 0;
}