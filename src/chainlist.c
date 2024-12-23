#include "chainlist.h"

void insert(ListNode* head, Buffer* key, Buffer* value) {
  ListNode* node = malloc(sizeof(ListNode));
  node->next = head->next;
  head->next = node;
  node->key = key;
  node->value = value;
}

Buffer* get(ListNode* head, Buffer* key) {
  for (ListNode* node = head; node->next; node = node->next) {
    if (node->key->length != key->length)
      continue;
    if (!memcmp(node->key->value, key->value, key->length))
      return node->value;
  }
  return NULL;
}

Buffer* set(ListNode* head, Buffer* key, Buffer* value) {
  for (ListNode* node = head; node->next; node = node->next) {
    if (node->key->length != key->length)
      continue;
    if (!memcmp(node->key->value, key->value, key->length))
      return node->value = value;
  }
  insert(head, key, value);
  return value;
}