#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#include "http.h"

#define MAX_HEADERS 32

typedef struct {
    char *key;
    char *value;
} KeyValuePair;

typedef struct {
    char *action;
    char *path;
    char *version;
    void *payload;
    size_t payload_size;
    int header_count;
    KeyValuePair headers[MAX_HEADERS];
} RequestData;

/* Find the end of HTTP headers in a buffer */
static char *find_header_end(const char *buffer, size_t buffer_len) {
    /* Look for \r\n\r\n */
    for (size_t i = 0; i < buffer_len - 3; i++) {
        if (buffer[i] == '\r' && buffer[i+1] == '\n' &&
            buffer[i+2] == '\r' && buffer[i+3] == '\n') {
            return (char *)buffer + i + 4;
        }
    }

    /* Look for \n\n */
    for (size_t i = 0; i < buffer_len - 1; i++) {
        if (buffer[i] == '\n' && buffer[i+1] == '\n') {
            return (char *)buffer + i + 2;
        }
    }

    return NULL;
}

/* Trim whitespace from a string */
static char *trim(char *str) {
    if (!str) return NULL;
    
    /* Trim leading whitespace */
    while (*str && isspace((unsigned char)*str)) str++;
    
    /* All spaces? */
    if (!*str) return str;
    
    /* Trim trailing whitespace */
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    
    return str;
}

/* Free all memory associated with request data */
static void free_request_data(RequestData *data) {
    if (!data) return;
    
    free(data->action);
    free(data->path);
    free(data->version);
    free(data->payload);
    
    for (int i = 0; i < data->header_count; i++) {
        free(data->headers[i].key);
        free(data->headers[i].value);
    }
    
    free(data);
}

/**
 * httprequest_parse_headers
 * 
 * Populate a `req` with the contents of `buffer`, returning the number of bytes used from `buf`.
 */
ssize_t httprequest_parse_headers(HTTPRequest *req, char *buffer, ssize_t buffer_len) {
    if (!req || !buffer || buffer_len <= 0) return -1;
    
    /* Clear request */
    memset(req, 0, sizeof(HTTPRequest));
    
    /* Find end of headers */
    char *headers_end = find_header_end(buffer, buffer_len);
    if (!headers_end) return -1;
    
    ssize_t headers_len = headers_end - buffer;
    
    /* Allocate internal data structure */
    RequestData *req_data = calloc(1, sizeof(RequestData));
    if (!req_data) return -1;
    
    /* Make a working copy of the headers */
    char *headers_copy = malloc(headers_len + 1);
    if (!headers_copy) {
        free(req_data);
        return -1;
    }
    memcpy(headers_copy, buffer, headers_len);
    headers_copy[headers_len] = '\0';
    
    /* Parse first line (request line) */
    char *saveptr = NULL;
    char *line = strtok_r(headers_copy, "\r\n", &saveptr);
    if (!line) {
        free(headers_copy);
        free(req_data);
        return -1;
    }
    
    /* Extract method, path, version */
    char *saveptr2 = NULL;
    char *method = strtok_r(line, " ", &saveptr2);
    if (!method) {
        free(headers_copy);
        free(req_data);
        return -1;
    }
    req_data->action = strdup(method);
    if (!req_data->action) {
        free(headers_copy);
        free(req_data);
        return -1;
    }
    
    char *path = strtok_r(NULL, " ", &saveptr2);
    if (!path) {
        free(req_data->action);
        free(headers_copy);
        free(req_data);
        return -1;
    }
    req_data->path = strdup(path);
    if (!req_data->path) {
        free(req_data->action);
        free(headers_copy);
        free(req_data);
        return -1;
    }
    
    char *version = strtok_r(NULL, "\r\n", &saveptr2);
    if (!version) {
        free(req_data->path);
        free(req_data->action);
        free(headers_copy);
        free(req_data);
        return -1;
    }
    req_data->version = strdup(version);
    if (!req_data->version) {
        free(req_data->path);
        free(req_data->action);
        free(headers_copy);
        free(req_data);
        return -1;
    }
    
    /* Parse header lines */
    while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
        /* If we've reached max headers, stop parsing */
        if (req_data->header_count >= MAX_HEADERS) break;
        
        /* Find colon separator */
        char *colon = strchr(line, ':');
        if (!colon) continue; /* Invalid header, skip */
        
        /* Split at colon */
        *colon = '\0';
        char *key = trim(line);
        char *value = trim(colon + 1);
        
        /* Store header if both key and value are non-empty */
        if (*key && *value) {
            req_data->headers[req_data->header_count].key = strdup(key);
            if (!req_data->headers[req_data->header_count].key) {
                /* Memory allocation failed */
                free_request_data(req_data);
                free(headers_copy);
                return -1;
            }
            
            req_data->headers[req_data->header_count].value = strdup(value);
            if (!req_data->headers[req_data->header_count].value) {
                /* Memory allocation failed */
                free(req_data->headers[req_data->header_count].key);
                free_request_data(req_data);
                free(headers_copy);
                return -1;
            }
            
            req_data->header_count++;
        }
    }
    
    /* Cleanup working copy */
    free(headers_copy);
    
    /* Store parsed data in req */
    req->action = req_data->action;
    req->path = req_data->path;
    req->version = req_data->version;
    req->payload = NULL;
    req->_internal = req_data;
    
    return headers_len;
}

/**
 * httprequest_read
 * 
 * Populate a `req` from the socket `sockfd`, returning the number of bytes read to populate `req`.
 */
ssize_t httprequest_read(HTTPRequest *req, int sockfd) {
    if (!req || sockfd < 0) return -1;
    
    /* Initialize request */
    memset(req, 0, sizeof(HTTPRequest));
    
    /* Buffer for reading data */
    char buffer[8192] = {0};
    ssize_t total_read = 0;
    ssize_t bytes_read;
    char *headers_end = NULL;
    
    /* Read until we find end of headers or buffer is full */
    while (total_read < (ssize_t)sizeof(buffer) - 1) {
        bytes_read = read(sockfd, buffer + total_read, sizeof(buffer) - total_read - 1);
        if (bytes_read <= 0) break;
        
        total_read += bytes_read;
        buffer[total_read] = '\0';
        
        /* Check if we've found end of headers */
        headers_end = find_header_end(buffer, total_read);
        if (headers_end) break;
    }
    
    if (!headers_end || total_read == 0) return -1;
    
    /* Parse headers */
    ssize_t headers_len = httprequest_parse_headers(req, buffer, total_read);
    if (headers_len < 0) return -1;
    
    /* Check for Content-Length header */
    const char *content_length_str = httprequest_get_header(req, "Content-Length");
    if (content_length_str) {
        /* Parse content length with validation */
        char *endptr;
        long content_length = strtol(content_length_str, &endptr, 10);
        
        /* Validate the parsed content length */
        if (*endptr != '\0' || content_length < 0 || content_length > INT_MAX) {
            /* Invalid Content-Length value */
            httprequest_destroy(req);
            return -1;
        }
        
        if (content_length > 0) {
            /* Get internal data structure */
            RequestData *req_data = (RequestData *)req->_internal;
            if (!req_data) {
                return -1; /* Should never happen */
            }
            
            /* Allocate memory for payload */
            void *payload = malloc((size_t)content_length);
            if (!payload) {
                httprequest_destroy(req);
                return -1;
            }
            
            /* Copy payload data we've already read */
            size_t payload_already_read = 0;
            if (total_read > headers_len) {
                payload_already_read = (size_t)(total_read - headers_len);
                if (payload_already_read > (size_t)content_length) {
                    payload_already_read = (size_t)content_length;
                }
                memcpy(payload, buffer + headers_len, payload_already_read);
            }
            
            /* Read remaining payload if needed */
            size_t remaining = (size_t)content_length - payload_already_read;
            size_t offset = payload_already_read;
            
            while (remaining > 0) {
                bytes_read = read(sockfd, (char*)payload + offset, remaining);
                if (bytes_read <= 0) break;
                
                offset += (size_t)bytes_read;
                remaining -= (size_t)bytes_read;
            }
            
            /* Store payload in request */
            req_data->payload = payload;
            req_data->payload_size = (size_t)content_length;
            req->payload = payload;
        }
    }
    
    return total_read;
}

/**
 * httprequest_get_action
 * 
 * Returns the HTTP action verb for a given `req`.
 */
const char *httprequest_get_action(HTTPRequest *req) {
    return req ? req->action : NULL;
}

/**
 * httprequest_get_header
 * 
 * Returns the value of the HTTP header `key` for a given `req`.
 */
const char *httprequest_get_header(HTTPRequest *req, const char *key) {
    if (!req || !key || !req->_internal) return NULL;
    
    RequestData *req_data = (RequestData *)req->_internal;
    
    for (int i = 0; i < req_data->header_count; i++) {
        if (strcasecmp(req_data->headers[i].key, key) == 0) {
            return req_data->headers[i].value;
        }
    }
    
    return NULL;
}

/**
 * httprequest_get_path
 * 
 * Returns the requested path for a given `req`.
 */
const char *httprequest_get_path(HTTPRequest *req) {
    return req ? req->path : NULL;
}

/**
 * httprequest_destroy
 * 
 * Destroys a `req`, freeing all associated memory.
 */
void httprequest_destroy(HTTPRequest *req) {
    if (!req || !req->_internal) return;
    
    /* Get internal data structure */
    RequestData *req_data = (RequestData *)req->_internal;
    
    /* Free all memory */
    free_request_data(req_data);
    
    /* Reset request */
    req->action = NULL;
    req->path = NULL;
    req->version = NULL;
    req->payload = NULL;
    req->_internal = NULL;
}