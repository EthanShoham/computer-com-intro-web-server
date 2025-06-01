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

void write_not_found(HttpResponse *res) {
  res_set_status_code(res, HTTP_STATUS_NOT_FOUND);
  res_finish_headers(res);
  res_finish_write(res);
}

void write_bad_request(HttpResponse *res) {
  res_set_status_code(res, HTTP_STATUS_BAD_REQUEST);
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
    headers_add(headers, "Content-Language", lan);
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

  const HttpQuery *query = req_get_query(req);
  const char *id = query_get(query, "id");
  int id_num = -1;
  if (id != NULL && !sscanf_s(id, "%9d", &id_num)) {
    id_num = -1;
  }

  size_t length = id_num == -1 ? students_length : 1;
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
  if (id_num != -1) {
    index--;
  }
  for (size_t i = 0; i < students_length; i++) {
    if (id_num != -1 && students[i].id != id_num) {
      continue;
    }

    index += sprintf_s(
        students_json + index, size - index + 1,
        "{\"id\":%9d,\"fname\":\"%s\",\"lname\":\"%s\",\"grade\":%3d}",
        students[i].id, students[i].fname, students[i].lname,
        students[i].grade);

    students_json[index++] = ',';
  }

  if (students_json[index - 1] == ',') {
    students_json[index - 1] = ']';
  } else {
    students_json[index++] = ']';
  }

  if (id_num != -1) {
    index--;
  }

  students_json[index] = 0;

  if (id_num != -1 && students_json[0] == 0) {
    write_not_found(res);
    free(context);
    return;
  }

  context->body = students_json;
  context->length = index;
  context->sent = 0;
  res_set_context(res, context);

  res_set_status_code(res, HTTP_STATUS_OK);
  HttpHeaders *headers = res_get_headers(res);
  headers_add(headers, "Content-Type", "application/json;  charset=utf-8");
  char len[50] = {0};
  sprintf_s(len, 50, "%zu", context->length);
  headers_add(headers, "Content-Length", len);
  res_finish_headers(res);
}

static void students_post(HttpRequest *req, HttpResponse *res) {
  Context *context = res_get_context(res);
  if (context == NULL) {
    if (!req_start_read(req)) {
      write_bad_request(res);
      return;
    }

    char buff[257] = {0};
    size_t read = req_read(req, buff, 256);
    buff[read] = 0;
    Student s = {0};
    int scanned = sscanf_s(buff,
                           "{\"id\":%9d,\"fname\":\"%49[a-zA-Z "
                           "]\",\"lname\":\"%49[a-zA-Z ]\",\"grade\":%3d}",
                           &s.id, &s.fname, (unsigned)sizeof(s.fname), &s.lname,
                           (unsigned)sizeof(s.lname), &s.grade);
    if (scanned != 4) {
      write_bad_request(res);
      return;
    }

    for (size_t i = 0; i < students_length; i++) {
      if (students[i].id == s.id) {
        write_bad_request(res);
        return;
      }
    }

    if (students_length == students_capacity) {
      students_capacity = students_capacity + 2;
      Student *temp = realloc(students, sizeof(Student) * students_capacity);
      if (temp == NULL) {
        write_server_error(res);
        return;
      }

      students = temp;
    }

    context = malloc(sizeof(Context));
    if (context == NULL) {
      write_server_error(res);
      return;
    }

    context->body = malloc(sizeof(char) * (read + 1));
    if (context->body == NULL) {
      free(context);
      write_server_error(res);
      return;
    }

    students[students_length++] = s;
    res_set_status_code(res, HTTP_STATUS_CREATED);
    HttpHeaders *headers = res_get_headers(res);
    headers_add(headers, "Content-Type", "application/json");
    char head_buff[25] = {0};
    sprintf_s(head_buff, sizeof(head_buff), "%zu", read);
    headers_add(headers, "Content-Length", head_buff);
    sprintf_s(head_buff, sizeof(head_buff), "/students?id=%9d", s.id);
    headers_add(headers, "Content-Location", head_buff);
    headers_add(headers, "Cache-Control", "no-cache");
    res_finish_headers(res);

    strcpy_s(context->body, read + 1, buff);
    context->length = read;
    context->sent = 0;
    res_set_context(res, context);
    return;
  }

  while (res_can_write(res) && context->sent < context->length) {
    size_t wrote = res_wirte(res, context->body + context->sent,
                             context->length - context->sent);
    context->sent += wrote;
  }

  if (context->sent == context->length) {
    res_finish_write(res);
  }
}

static void students_put(HttpRequest *req, HttpResponse *res) {
  const HttpQuery *query = req_get_query(req);
  const char *id = query_get(query, "id");
  int id_num = -1;
  if (id == NULL || !sscanf_s(id, "%9d", &id_num)) {
    write_bad_request(res);
    return;
  }

  if (!req_start_read(req)) {
    write_bad_request(res);
    return;
  }

  char buff[257] = {0};
  size_t read = req_read(req, buff, 256);
  buff[read] = 0;
  Student s = {0};
  s.id = id_num;
  int scanned = sscanf_s(
      buff,
      "{\"fname\":\"%49[a-zA-Z ]\",\"lname\":\"%49[a-zA-Z ]\",\"grade\":%3d}",
      &s.fname, (unsigned)sizeof(s.fname), &s.lname, (unsigned)sizeof(s.lname),
      &s.grade);
  if (scanned != 3) {
    write_bad_request(res);
    return;
  }

  for (size_t i = 0; i < students_length; i++) {
    if (students[i].id == s.id) {
      students[i] = s;
      HttpHeaders *headers = res_get_headers(res);
      char buff[25] = {0};
      sprintf_s(buff, sizeof(buff), "/students?id=%9d", s.id);
      headers_add(headers, "Content-Location", buff);
      res_set_status_code(res, HTTP_STATUS_NO_CONTENT);
      res_finish_headers(res);
      res_finish_write(res);
      return;
    }
  }

  if (students_length == students_capacity) {
    students_capacity = students_capacity + 2;
    Student *temp = realloc(students, sizeof(Student) * students_capacity);
    if (temp == NULL) {
      write_server_error(res);
      return;
    }

    students = temp;
  }

  students[students_length++] = s;
  res_set_status_code(res, HTTP_STATUS_CREATED);
  HttpHeaders *headers = res_get_headers(res);
  sprintf_s(buff, sizeof(buff), "/students?id=%9d", s.id);
  headers_add(headers, "Content-Location", buff);
  res_finish_headers(res);
  res_finish_write(res);
  return;
}

static void students_delete(HttpRequest *req, HttpResponse *res) {
  const HttpQuery *query = req_get_query(req);
  const char *id = query_get(query, "id");
  int id_num = -1;
  if (id == NULL || !sscanf_s(id, "%9d", &id_num)) {
    write_bad_request(res);
    return;
  }

  for (size_t i = 0; i < students_length; i++) {
    if (students[i].id == id_num) {
      Student zero = {0};
      students[i] = students[students_length - 1];
      students[students_length - 1] = zero;
      students_length--;

      res_set_status_code(res, HTTP_STATUS_NO_CONTENT);
      res_finish_headers(res);
      res_finish_write(res);
      return;
    }
  }

  write_not_found(res);
}

int main() {
  students = malloc(sizeof(Student) * 2);
  if (students == NULL) {
    return 1;
  }
  students_capacity = 2;
  students_length = 2;
  students[0].id = 903488341;
  strcpy_s(students[0].fname, 50, "Jon");
  strcpy_s(students[0].lname, 50, "Doe");
  students[0].grade = 85;
  students[1].id = 522333421;
  strcpy_s(students[1].fname, 50, "Mick");
  strcpy_s(students[1].lname, 50, "Salvaski");
  students[1].grade = 94;

  WebServer *server = create_web_server();
  web_server_map_get(server, "/", root, (void (*)(void *))root_cleanup);
  web_server_map_get(server, "/students", students_get,
                     (void (*)(void *))cleanup);
  web_server_map_post(server, "/students", students_post,
                      (void (*)(void *))cleanup);
  web_server_map_put(server, "/students", students_put, NULL);
  web_server_map_delete(server, "/students", students_delete, NULL);
  int res = web_server_run(server);

  free(students);
  return res;
}
