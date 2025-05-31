#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "http_query_internal.h"
#include "key_value_pair.h"

// list of key value pairs extracted from the url
typedef struct HttpQuery {
  KeyValuePair *params;
  size_t length;
  size_t capacity;
} HttpQuery;

// the key is case sensetive, returns null if key not found,
HttpQuery* create_http_query() {
  HttpQuery *query = malloc(sizeof(HttpQuery));
  if(query == NULL) {
    return NULL;
  }

  query->length = 0;
  query->capacity = 0;
  query->params = NULL;
  return  query;
}
void destroy_http_query(HttpQuery* query) {
  if(query == NULL) {
    return;
  }

  for (size_t i = 0; i < query->length; i++) {
    free(query->params[i].key);
    free(query->params[i].value);
  }

  free(query->params);
  query->params = NULL;
  free(query);
}

const char* query_get(const HttpQuery* query, const char* key) {
  if(query == NULL || query->params == NULL || query->length == 0) {
    return NULL;
  }

  for (size_t i =0; i < query->length;i++) {
    if(strcmp(query->params[i].key, key) == 0) {
      return query->params[i].value;
    }
  }

  return NULL;
}
bool query_add(HttpQuery* query, const char* key, const char* value) {
  assert(query && "query_add: query is null, did you forgot to call create_http_query()?");
  assert(key && "query_add: key cannot be null.");
  assert(value && "query_add: value cannot be null.");

  if(query->params == NULL) {
    query->params = malloc(sizeof(KeyValuePair) * 1);
    if(query->params == NULL) {
      return false;
    }

    query->capacity = 1;
  }

  if(query->length == query->capacity) {
    KeyValuePair *temp;
    temp = realloc(query->params, sizeof(KeyValuePair) * (query->capacity + 1));
    if(temp == NULL) {
      return false;
    }

    query->params = temp;
    query->capacity++;
  }

  size_t key_length = strlen(key) + 1;
  size_t value_length = strlen(value) + 1;

  char* key_copy = malloc(sizeof(char) * key_length);
  if(key_copy == NULL) {
    return false;
  }
  char* value_copy = malloc(sizeof(char) * value_length);
  if(value_copy == NULL) {
    free(key_copy);
    return false;
  }

  if(strcpy_s(key_copy, key_length, key)) {
    free(key_copy);
    free(value_copy);
    return false;
  }

  if(strcpy_s(value_copy, value_length, value))  {
    free(key_copy);
    free(value_copy);
    return false;
  }

  query->params[query->length].key = key_copy;
  query->params[query->length].value = value_copy;
  query->length++;
  return true;
}

