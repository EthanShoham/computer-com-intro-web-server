#include "http_headers_internal.h"
#include "key_value_pair.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct HttpHeaders {
  KeyValuePair *headers;
  size_t capacity;
  size_t length;
} HttpHeaders;

HttpHeaders *create_http_headers() {
  HttpHeaders *headers = malloc(sizeof(HttpHeaders));
  if (headers == NULL) {
    return NULL;
  }

  headers->length = 0;
  headers->capacity = 0;
  headers->headers = NULL;
  return headers;
}
void destroy_http_headers(HttpHeaders *headers) {
  if (headers == NULL) {
    return;
  }

  for (size_t i = 0; i < headers->length; i++) {
    free(headers->headers[i].key);
    free(headers->headers[i].value);
  }

  free(headers->headers);
  headers->headers = NULL;
  free(headers);
}

const char *headers_get(const HttpHeaders *headers, const char *key) {
  if (headers == NULL || headers->headers == NULL || headers->length == 0) {
    return NULL;
  }

  for (size_t i = 0; i < headers->length; i++) {
    if (_stricmp(headers->headers[i].key, key) == 0) {
      return headers->headers[i].value;
    }
  }

  return NULL;
}
bool headers_add(HttpHeaders *headers, const char *key, const char *value) {
  assert(headers && "headers_add: headers is null, did you forgot to call "
                    "create_http_headers()?");
  assert(key && "headers_add: key cannot be null.");
  assert(value && "headers_add: value cannot be null.");

  if (headers_get(headers, key) != NULL) {
    return false;
  }

  if (headers->headers == NULL) {
    headers->headers = malloc(sizeof(KeyValuePair) * 1);
    if (headers->headers == NULL) {
      return false;
    }

    headers->capacity = 1;
  }

  if (headers->length == headers->capacity) {
    KeyValuePair *temp;
    temp = realloc(headers->headers,
                   sizeof(KeyValuePair) * (headers->capacity + 1));
    if (temp == NULL) {
      return false;
    }

    headers->headers = temp;
    headers->capacity++;
  }

  size_t key_length = strlen(key) + 1;
  size_t value_length = strlen(value) + 1;

  char *key_copy = malloc(sizeof(char) * key_length);
  if (key_copy == NULL) {
    return false;
  }
  char *value_copy = malloc(sizeof(char) * value_length);
  if (value_copy == NULL) {
    free(key_copy);
    return false;
  }

  if (strcpy_s(key_copy, key_length, key)) {
    free(key_copy);
    free(value_copy);
    return false;
  }

  if (strcpy_s(value_copy, value_length, value)) {
    free(key_copy);
    free(value_copy);
    return false;
  }

  headers->headers[headers->length].key = key_copy;
  headers->headers[headers->length].value = value_copy;
  headers->length++;
  return true;
}
const char *headers_get_key_at_index(HttpHeaders *headers, size_t index) {
  assert(index < headers->length);

  return headers->headers[index].key;
}
const char *headers_get_value_at_index(HttpHeaders *headers, size_t index) {
  assert(index < headers->length);

  return headers->headers[index].value;
}
size_t headers_count(const HttpHeaders *headers) { return headers->length; }
