#include "zc_io.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

struct zc_file {
    int fd;
    int reader;
    void *ptr;
    sem_t r_mut;
    sem_t w_mut;
    size_t size;
    size_t offset;
};

// Exercise 1 
zc_file *zc_open(const char *path) {
    int fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    struct stat buf;
    fstat(fd, &buf);
    off_t size = buf.st_size;

    if (fd >= 0)
    {
        void *ptr;
        zc_file *file = malloc(sizeof(zc_file));
        sem_init(&file->r_mut, 0, 1);
        sem_init(&file->w_mut, 0, 1);
        file->fd = fd;
        file->offset = 0;
        file->reader = 0;

        if (size)
        {
            ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        }
        else
        {
            ptr = mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            size = 1;
        }
        file->size = size;
        file->ptr = ptr;
        return file;
    }
    return NULL;
}

int zc_close(zc_file *file) {
    int ans = close(file->fd);
    munmap(file->ptr, file->size);
    free(file);
    return ans;
}

const char *zc_read_start(zc_file *file, size_t *size) {
    if (file->offset > file->size)    return NULL;

    sem_wait(&file->r_mut);
    file->reader++;
    if (file->reader == 1)    sem_wait(&file->w_mut);
    sem_post(&file->r_mut);

    if (file->offset + *size > file->size)
    {
        char *chunk = (char *)file->ptr + file->offset;
        *size = file->size - file->offset;
        file->offset += *size;
        return chunk;
    }
    else
    {
        char *chunk = (char *)file->ptr + file->offset;
        file->offset += *size;
        return chunk;
    }
}

void zc_read_end(zc_file *file) {
    sem_wait(&file->r_mut);
    file->reader--;
    if (!file->reader)    sem_post(&file->w_mut);
    sem_post(&file->r_mut);
}

//Exercise 2
char *zc_write_start(zc_file *file, size_t size) {
    sem_wait(&file->w_mut);
    if (file->offset + size < file->size)
    {
        char *chunk = (char *)file->ptr + file->offset;
        file->offset += size;
        return chunk;
    }
    else
    {
        ftruncate(file->fd, file->offset + size);
        file->ptr = mremap(file->ptr, file->size, file->offset + size, MREMAP_MAYMOVE);
        if (file->ptr == (void *)-1)    return NULL;      
        char *chunk = (char *)file->ptr + file->offset;
        file->offset += size;
        return chunk;
    }
}

void zc_write_end(zc_file *file) {
    sem_post(&file->w_mut);
}


//Exercise 3
off_t zc_lseek(zc_file *file, long offset, int whence) {
    sem_wait(&file->w_mut);

    switch (whence)
    {
        case SEEK_SET:
            if (offset < 0)
            {
                sem_post(&file->w_mut);
                return -1;
            }
            file->offset = offset;
            sem_post(&file->w_mut);
            return file->offset;
        case  SEEK_CUR:
            if ((long)file->offset + offset < 0)
            {
                sem_post(&file->w_mut);
                return -1;
            }
            file->offset += offset;
            sem_post(&file->w_mut);
            return file->offset;
        case SEEK_END:
            if ((long)file->size + offset < 0)
            {
                sem_post(&file->w_mut);
                return -1;
            }
            file->offset = file->size + offset;
            sem_post(&file->w_mut);
            return file->offset;
        default:
            sem_post(&file->w_mut);
            return -1;
    }
}

//Exercise 4
int zc_copyfile(const char *source, const char *dest) {
    zc_file *dst = zc_open(dest);
    zc_file *src = zc_open(source);

    ftruncate(dst->fd, src->size);
    dst->ptr = mremap(dst->ptr, dst->size, src->size, MREMAP_MAYMOVE);
    dst->size = src->size;
    memcpy(dst->ptr, src->ptr, src->size);

    if (dst->ptr == (void *)-1)    return -1;
    return 0;
}
