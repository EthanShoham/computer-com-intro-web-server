#include "web-server/web_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WRITING_FILE 1

typedef struct RootContext {
  char *file_content;
  size_t content_length;
  size_t sent_count;
} RootContext;

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
      res_set_status_code(res, HTTP_STATUS_SERVER_ERROR);
      res_finish_headers(res);
      res_finish_write(res);
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
      res_set_status_code(res, HTTP_STATUS_SERVER_ERROR);
      res_finish_headers(res);
      res_finish_write(res);
      free(context);
      return;
    }

    FILE *stream;
    if (fopen_s(&stream, file, "r") != 0) {
      res_set_status_code(res, HTTP_STATUS_SERVER_ERROR);
      res_finish_headers(res);
      res_finish_write(res);
      free(context->file_content);
      free(context);
      return;
    }
    context->content_length = 0;
    context->sent_count = 0;
    char *buffer = context->file_content;
    while (true) {
      if (fgets(buffer, 255, stream) == NULL && !feof(stream)) {
        res_set_status_code(res, HTTP_STATUS_SERVER_ERROR);
        res_finish_headers(res);
        res_finish_write(res);
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
        res_set_status_code(res, HTTP_STATUS_SERVER_ERROR);
        res_finish_headers(res);
        res_finish_write(res);
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
} Student;

int main() {
  WebServer *server = create_web_server();
  web_server_map_get(server, "/", root, (void (*)(void *))root_cleanup);

  return web_server_run(server);
}
