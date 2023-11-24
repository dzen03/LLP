#include "file.h"
#include "dynamic_store.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int static_store_read(FILE* file, int64_t addr, void* memory, int64_t size)
{
  if (fseek64(file, addr, SEEK_SET))
    return -1;

  if (fread(memory, size, 1, file) != 1)
  {
    return -2;
  }

  return 0;
}

int static_store_write(FILE* file, int64_t addr, const void* const data, int64_t size)
{
  if (fseek64(file, addr, SEEK_SET) != 0)
    return -1;
  if (fwrite(data, size, 1, file) != 1)
    return -2;

  return 0;
}

int static_store_write_and_free(FILE* file, int64_t addr, void* data, int64_t size)
{
  int res;
  if ((res = static_store_write(file, addr, data, size)) == 0)
    free(data);

  return res;
}

int dynamic_store_read_(FILE* file, int64_t addr, struct dynamic_store* store)
{
  if (fseek64(file, addr, SEEK_SET))
    return -1;

  if (fread(&store->header, sizeof(struct dynamic_store_header), 1, file) != 1)
  {
    return -2;
  }

  if (fread(store->data, sizeof(uint8_t), store->header.length, file) != store->header.length)
  {
    return -2;
  }

  return 0;
}

int dynamic_store_write_(FILE* file, int64_t addr, const struct dynamic_store* const store)
{
  if (fseek64(file, addr, SEEK_SET))
    return -1;
  if (fwrite(&store->header, sizeof(struct dynamic_store_header), 1, file) != 1)
    return -2;
  if (fwrite(store->data, sizeof(uint8_t), DYNAMIC_STORE_DATA_LENGTH, file) != DYNAMIC_STORE_DATA_LENGTH)
    return -2;

  fflush(file);

  return 0;
}

int dynamic_store_write_and_free_(FILE* file, int64_t addr, struct dynamic_store* store)
{
  int res;
  if ((res = dynamic_store_write_(file, addr, store)) == 0)
  {
    free(store->data);
  }
  return res;
}

int64_t fsizeo(FILE* file)
{
  int64_t current_position = ftello(file);
  if (fseeko(file, 0, SEEK_END) != 0)
    return -1;
  int64_t size = ftello(file);
  if (fseeko(file, current_position, SEEK_SET) != 0)
    return -1;
  return size;
}

struct file file_init(char* filename)
{
  struct file file = {0};
  file.descriptor = fopen(filename, "rb+");
  if (file.descriptor == NULL) // no file, need to create
    file.descriptor = fopen(filename, "wb+");

  if (file.descriptor == NULL)
    exit_with_error("Can't open or create file");

  // move to the file begin, because init position depends on the system
  fseek64(file.descriptor, 0, SEEK_SET);

  if (file.descriptor == NULL)
    return file;

  if (fsizeo(file.descriptor) > 0)
  {
    if (fread(&file.metadata, sizeof(file.metadata), 1, file.descriptor) != 1)
    {
      fclose(file.descriptor);
      return (struct file){NULL};
    }
  }
  else
  {
    if (fwrite(&file.metadata, sizeof(file.metadata), 1, file.descriptor) != 1)
    {
      fclose(file.descriptor);
      return (struct file){NULL};
    }
  }

  return file;
}

void file_close(FILE* file)
{
  fclose(file);
}

// start of platform dependant functions
#if defined(LAB1_WINDOWS_) // WINDOWS
#include <fileapi.h>

int64_t ftell64(FILE* file)
{
  return _ftelli64(file);
}

int fseek64(FILE* file, int64_t offset, int whence)
{
  return _fseeki64(file, offset, whence);
}

int ftrunc(FILE* file, int64_t length)
{
  int64_t current_position = ftello(file);
  if (fseeko(file, 0, SEEK_END) != 0)
    return -1;
  int res = SetEndOfFile(file);
  if (fseeko(file, current_position, SEEK_SET) != 0)
    return -1;
  return (res != 0 ? 0 : -1);
}

#elif defined (LAB1_POSIX_) // POSIX compatible (UNIX || MACOS)
#include <unistd.h>

int64_t ftell64(FILE* file)
{
  return ftello(file);
}

int fseek64(FILE* file, int64_t offset, int whence)
{
  return fseeko(file, offset, whence);
}

int ftrunc(FILE* file, int64_t length)
{
  return ftruncate(fileno(file), length);
}

#endif // end


