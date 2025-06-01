#include "web-server/http_request.h"
#include "web-server/http_response.h"
#include "web-server/web_server.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WRITING_FILE 1

typedef struct Context {
  char *body;
  size_t length;
  size_t sent;
} Context;

typedef struct RootContext {
  char *file_content;
  size_t content_length;
  size_t sent_count;
} RootContext;

void write_server_error(HttpResponse *res) {
  res_set_status_code(res, HTTP_STATUS_SERVER_ERROR);
  res_finish_headers(res);
  res_finish_write(res);
}

void root_cleanup(RootContext *context) {
  if (context != NULL) {
    free(context->file_content);
    free(context);
  }
}

void root(HttpRequest *req, HttpResponse *res) {
  RootContext *context = (RootContext *)res_get_context(res);
  if (context == NULL) {
    context = malloc(sizeof(RootContext));
    if (context == NULL) {
      write_server_error(res);
      return;
    }
    const HttpQuery *query = req_get_query(req);
    const char *lang = query_get(query, "lang");
    const char *file = "..\\..\\html\\index.html";
    const char *lan = "en-US";

    if (lang == NULL) {
    } else if (strcmp(lang, "he") == 0) {
      file = "..\\..\\html\\index.he.html";
      lan = "he-IL";
    } else if (strcmp(lang, "fr") == 0) {
      file = "..\\..\\html\\index.fr.html";
      lan = "fr-FR";
    }

    context->file_content = malloc(sizeof(char) * 256);
    if (context->file_content == NULL) {
      write_server_error(res);
      free(context);
      return;
    }

    FILE *stream;
    if (fopen_s(&stream, file, "r") != 0) {
      write_server_error(res);
      free(context->file_content);
      free(context);
      return;
    }
    context->content_length = 0;
    context->sent_count = 0;
    char *buffer = context->file_content;
    while (true) {
      if (fgets(buffer, 255, stream) == NULL && !feof(stream)) {
        write_server_error(res);
        free(context->file_content);
        free(context);
        fclose(stream);
        return;
      }

      size_t read = strlen(buffer);
      context->content_length += read;
      if (read == 0) {
        break;
      }

      char *temp =
          realloc(context->file_content, context->content_length + 255);
      if (temp == NULL) {
        write_server_error(res);
        free(context->file_content);
        free(context);
        fclose(stream);
        return;
      }

      context->file_content = temp;
      buffer = context->file_content + context->content_length;
    }

    fclose(stream);
    res_set_context(res, context);
    res_set_status_code(res, HTTP_STATUS_OK);
    HttpHeaders *headers = res_get_headers(res);
    headers_add(headers, "Content-Type", "text/html; charset=utf-8");
    char cnt_len[255];
    snprintf(cnt_len, sizeof(cnt_len), "%zu", context->content_length);
    headers_add(headers, "Content-Length", cnt_len);
    res_finish_headers(res);
  }

  while (res_can_write(res) && context->sent_count != context->content_length) {
    size_t wrote = res_wirte(res, context->file_content + context->sent_count,
                             context->content_length - context->sent_count);
    context->sent_count += wrote;
  }

  if (context->sent_count == context->content_length) {
    res_finish_write(res);
  }
}

typedef struct Student {
  int id;
  char fname[50];
  char lname[50];
  int grade;
} Student;

static Student *students = NULL;
static size_t students_capacity = 0;
static size_t students_length = 0;

static void cleanup(Context *context) {
  if (context != NULL) {
    free(context->body);
    free(context);
  }
}

static void students_get(HttpRequest *req, HttpResponse *res) {
  Context *context = res_get_context(res);
  if (context != NULL) {
    while (res_can_write(res) && context->sent != context->length) {
      size_t wrote = res_wirte(res, context->body + context->sent,
                               context->length - context->sent);
      context->sent += wrote;
    }

    if (context->sent == context->length) {
      res_finish_write(res);
    }
    return;
  }

  context = malloc(sizeof(Context));
  if (context == NULL) {
    write_server_error(res);
    return;
  }

  size_t length = students_length;
  //"{"id":%9d,"fname":"some name","lname":"morename","grade":1414}"
  size_t size = 1 + ((6 + 9 + 10 + 49 + 11 + 49 + 10 + 3 + 1 + 1) * length) + 1;
  char *students_json = malloc(size + 1);
  if (students_json == NULL) {
    write_server_error(res);
    free(context);
    return;
  }

  size_t index = 0;
  students_json[index++] = '[';
  for (size_t i = 0; i < length; i++) {
    index += sprintf_s(students_json + index, size - index + 1,
              "{\"id\":%9d,\"fname\":\"%s\",\"lname\":\"%s\",\"grade\":%3d}",
              students[i].id, students[i].fname, students[i].lname,
              students[i].grade);
    if(i != length - 1){
      students_json[index++] = ',';
    }
  }
  students_json[index++] = ']';
  students_json[index] = 0;

  context->body = students_json;
  context->length = index;
  context->sent = 0;
  res_set_context(res, context);

  res_set_status_code(res, HTTP_STATUS_OK);
  HttpHeaders *headers = res_get_headers(res);
  headers_add(headers, "Content-Type", "application/json;  charset=utf-8");
  char len[50] = { 0 };
  sprintf_s(len, 50, "%zu", context->length);
  headers_add(headers, "Content-Length", len);
  res_finish_headers(res);
}

static void students_post(HttpRequest *req, HttpResponse *res) {
  Context *context = res_get_context(res);
  if (context != NULL) {
    while (res_can_write(res) && context->sent != context->length) {
      size_t wrote = res_wirte(res, context->body + context->sent,
                               context->length - context->sent);
      context->sent += wrote;
    }

    if (context->sent == context->length) {
      res_finish_write(res);
    }
    return;
  }

  context = malloc(sizeof(Context));
  if (context == NULL) {
    write_server_error(res);
    return;
  }

  size_t length = students_length;
  //"{"id":%9d,"fname":"some name","lname":"morename","grade":1414}"
  size_t size = 1 + ((6 + 9 + 10 + 49 + 11 + 49 + 10 + 3 + 1 + 1) * length);
  char *students_json = malloc(size + 1);
  if (students_json == NULL) {
    write_server_error(res);
    free(context);
    return;
  }

  size_t index = 0;
  students_json[index++] = '[';
  for (size_t i = 0; i < length; i++) {
    index += sprintf_s(students_json + index, size - index + 1,
              "{\"id\":%9d,\"fname\":\"%s\",\"lname\":\"%s\",\"grade\":%3d}",
              students[i].id, students[i].fname, students[i].lname,
              students[i].grade);
    if(i != length - 1){
      students_json[index++] = ',';
    }
  }
  students_json[index++] = ']';
  students_json[index] = 0;

  context->body = students_json;
  context->length = index;
  context->sent = 0;
  res_set_context(res, context);
}

int main() {
  WebServer *server = create_web_server();
  web_server_map_get(server, "/", root, (void (*)(void *))root_cleanup);
  web_server_map_get(server, "/students", students_get, (void (*)(void *))cleanup);
  web_server_map_post(server, "/students", students_post, (void (*)(void *))cleanup);
  return web_server_run(server);
}
