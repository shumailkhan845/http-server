#ifndef FILE_H 
#define FILE_H

struct file_data
{
    char *data;
    int size;
    int status;
};
struct file_data read_file(const char *filepath);
#endif