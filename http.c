#include "main.h"

// Creates the pollfd array
struct pollfd *init_poll(int fd) {
  struct pollfd *fds =
      (struct pollfd *)malloc(POLLFD_SIZE * (sizeof(struct pollfd)));
  if (fds == NULL) {
    perror("memory allocation failed()");
    exit(-1);
  }

  memset(fds, 0, POLLFD_SIZE * sizeof(struct pollfd));
  fds[0].fd = fd;
  fds[0].events = POLLIN;
  return fds;
}

// Accepts worker fds and updates the pollfd array
void accept_connections(int fd, struct pollfd *fds, int *nfds,
                        bool *end_server) {
  int new_sd;
  do {
    new_sd = accept(fd, NULL, NULL);
    if (new_sd < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        continue;
      }
      perror("accept() failed");
      *end_server = true;
      break;
    }
    fds[*nfds].fd = new_sd;
    fds[*nfds].events = POLLIN;
    (*nfds)++;

  } while (new_sd != -1);
}

// Handles send errors
void handle_send_error(int fd, struct pollfd *fds, int i, bool *end_server) {
  perror("send() failed");
  close(fd);
  close(fds[i].fd);
  *end_server = true;
}

// Gets the file path given a url
char *get_file_path(char *url) {
  FILE *stream;
  int path_len = strlen(url) + PATH_LEN;
  char *file_path = (char *)malloc(path_len);
  if (file_path == NULL) {
    perror("memory allocation()");
    exit(-1);
  }
  int result = 0;
  char* favicon_path = "/public/favicon.ico";
  if (strstr(url, "favicon.ico") != NULL) {
    result = snprintf(file_path, path_len, "./www%s", favicon_path);
  }
  else {
    result = snprintf(file_path, path_len, "./www%s", url);
  }
  if (result < 0 || result >= path_len) {
    perror("error creating path()");
    exit(-1);
  }
  return file_path;
}

// Reads the file contents into a string and returns it
char *read_file(size_t *file_size, char *url, bool *valid_response) {
  *valid_response = true;
  char *file_path = get_file_path(url);
  FILE *stream;
  
  // If the url ends in a '/' then it should point to the index.html file
  int url_size = strlen(url);
  if (*(url + url_size - 1) == '/') {
    strcat(file_path, "index.html");
  }

  stream = fopen(file_path, "rb");

  if (stream == NULL) {
    *valid_response = false;
    stream = fopen("./www/404.html", "rb");
    // Redirect the file to the 404 page if the dir/file does not exist
  }

  fseek(stream, 0, SEEK_END);

  *file_size = ftell(stream);

  char *file_content = (char *)malloc(*file_size);
  if (file_content == NULL) {
    perror("memory allocation()");
    free(file_content);
    exit(-1);
  }

  fseek(stream, 0, SEEK_SET);
  size_t bytesRead = fread(file_content, 1, *file_size, stream);
  
  if (bytesRead != *file_size) {
    perror("reading file()");
    exit(-1);
  }

  fclose(stream);

  return file_content;
}

// Creates the header for the response
char *create_header(size_t *file_size, char *url, bool *valid_response) {
  char *header = (char *)malloc(HEADER_LEN);
  if (header == NULL) {
    perror("memory allocation error");
    free(header);
    exit(-1);
  }

  char http_code[20];
  if (*valid_response) {
    strcpy(http_code, "200 OK");
  } else {
    strcpy(http_code, "404 Not Found");
  }

  char content_type[30];
  if (strstr(url, ".css") != NULL) {
    strncpy(content_type, "text/css", sizeof(content_type));
  } else if (strstr(url, ".png") != NULL) {
    strncpy(content_type, "image/png", sizeof(content_type));
  } else if (strstr(url, ".ico") != NULL) {
    strncpy(content_type, "image/x-icon", sizeof(content_type));
  } else if (strstr(url, ".jpg") != NULL) {
    strncpy(content_type, "image/jpeg", sizeof(content_type));
  } else if (strstr(url, ".woff2") != NULL) {
    strncpy(content_type, "font/woff2", sizeof(content_type));
  } else if (strstr(url, ".js") != NULL) {
    strncpy(content_type, "application/javascript", sizeof(content_type));
  } else {
    strncpy(content_type, "text/html", sizeof(content_type));
  }

  int result =
      snprintf(header, HEADER_LEN,
               "HTTP/1.1 %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n\r\n",
               http_code, content_type, *file_size);

  return header;
}

// Creates the response using the header and file content
char *create_response(char *header, char *body, size_t body_size) {
  size_t header_size = strlen(header);
  char *result = (char *)malloc(header_size + body_size);
  if (result == NULL) {
    perror("Memory allocation error");
    free(result);
    exit(-1);
  }
  memcpy(result, header, header_size);
  memcpy(result + header_size, body, body_size);

  result[header_size + body_size] = '\0';
  return result;
}

// Gets the response from the file provided by the url
const char *get_response(char *url, size_t *file_size) {
  bool valid_response;
  char *file = read_file(file_size, url, &valid_response);
  char *header = create_header(file_size, url, &valid_response);
  char *result = create_response(header, file, *file_size);
  free(file);
  free(header);

  return result;
}

// Handles client requests
void handle_client_request(int fd, struct pollfd *fds, int i,
                           bool *end_server) {
  char buffer[RECV_BUFFER];
  memset(buffer, 0, sizeof(buffer));
  ssize_t bytes_received;
  
  bytes_received = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    perror("recv() failed");
    return;
  }
  if (bytes_received == 0) {
    return;
  }

  char *request_buffer = buffer;
  char *method = strsep(&request_buffer, " ");
  char *url = strsep(&request_buffer, " ");
  char *version = strsep(&request_buffer, "\r\n");

  ssize_t bytes_sent;
  size_t file_size;
  const char *file_result = get_response(url, &file_size);
  
  bytes_sent = send(fds[i].fd, file_result, strlen(file_result) + file_size, 0);
  free((void *)file_result);

  if (bytes_sent < 0) {
    handle_send_error(fd, fds, i, end_server);
    return;
  }
}

// Accepts requests using the listener fd and the array of pollfds
void accept_request(int fd, struct pollfd *fds) {
  int timeout = TIMEOUT;
  int nfds = 1;
  bool end_server = false;

  do {
    int curr_size = nfds;
    int poll_result = poll(fds, nfds, timeout);

    if (poll_result == -1) {
      perror("poll() failed");
      break;
    }

    if (poll_result == 0) {
      continue;
    }

    for (int i = 0; i < curr_size; i++) {
      if (fds[i].revents == 0) {
        continue;
      }

      if (fds[i].fd == fd && fds[i].revents != POLLIN) {
        printf("Error! revents = %d\n", fds[i].revents);
        close(fds[i].fd);
        close(fd);
        free(fds);
        exit(-1);
      }

      if (fds[i].fd == fd) {
        accept_connections(fd, fds, &nfds, &end_server);

      } else {
        handle_client_request(fd, fds, i, &end_server);
      }
    }
  } while (!end_server);
}
