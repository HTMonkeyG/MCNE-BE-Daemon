#include <stdlib.h>
#include <string.h>

#define MAX_HEIGHT 8

typedef struct {
  char* value;
  int length;
} Buffer;

typedef struct {
  Buffer* key;
  Buffer* value;
  void* next;
} ListNode;
