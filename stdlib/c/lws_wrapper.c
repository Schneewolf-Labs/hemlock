// libwebsockets FFI wrapper for Hemlock
// Provides both HTTP and WebSocket functionality in a single library

#include <libwebsockets.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ========== HTTP SUPPORT ==========

typedef struct {
    char *body;
    size_t body_len;
    size_t body_capacity;
    char *headers;
    size_t headers_len;
    size_t headers_capacity;
    int status_code;
    int complete;
    int failed;
} http_response_t;

static int http_callback(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    http_response_t *resp = (http_response_t *)user;

    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lwsl_user("HTTP connection established\n");
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            lwsl_err("HTTP connection error\n");
            resp->failed = 1;
            resp->complete = 1;
            break;

        case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
            resp->status_code = lws_http_client_http_response(wsi);
            break;

        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
            // Accumulate response body
            if (resp->body_len + len >= resp->body_capacity) {
                resp->body_capacity = (resp->body_len + len + 1) * 2;
                resp->body = realloc(resp->body, resp->body_capacity);
            }
            memcpy(resp->body + resp->body_len, in, len);
            resp->body_len += len;
            resp->body[resp->body_len] = '\0';
            break;

        case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
            resp->complete = 1;
            break;

        case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
            resp->complete = 1;
            break;

        default:
            break;
    }

    return 0;
}

// HTTP GET request
http_response_t* lws_http_get(const char *url) {
    struct lws_context_creation_info info;
    struct lws_client_connect_info connect_info;
    struct lws_context *context;
    http_response_t *resp;
    struct lws *wsi;

    // Allocate response structure
    resp = calloc(1, sizeof(http_response_t));
    resp->body_capacity = 4096;
    resp->body = malloc(resp->body_capacity);
    resp->body[0] = '\0';
    resp->headers_capacity = 1024;
    resp->headers = malloc(resp->headers_capacity);
    resp->headers[0] = '\0';

    // Parse URL (simplified - should use proper URL parser)
    char host[256] = {0};
    char path[512] = "/";
    int port = 80;
    int ssl = 0;

    if (strncmp(url, "https://", 8) == 0) {
        ssl = 1;
        port = 443;
        sscanf(url + 8, "%255[^/]%511s", host, path);
    } else if (strncmp(url, "http://", 7) == 0) {
        sscanf(url + 7, "%255[^/]%511s", host, path);
    } else {
        free(resp->body);
        free(resp->headers);
        free(resp);
        return NULL;
    }

    // Create context
    memset(&info, 0, sizeof(info));
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.port = CONTEXT_PORT_NO_LISTEN;

    static const struct lws_protocols protocols[] = {
        { "http", http_callback, sizeof(http_response_t), 0, 0, NULL, 0 },
        { NULL, NULL, 0, 0, 0, NULL, 0 }
    };
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        free(resp->body);
        free(resp->headers);
        free(resp);
        return NULL;
    }

    // Connect
    memset(&connect_info, 0, sizeof(connect_info));
    connect_info.context = context;
    connect_info.address = host;
    connect_info.port = port;
    connect_info.path = path;
    connect_info.host = host;
    connect_info.origin = host;
    connect_info.method = "GET";
    connect_info.protocol = protocols[0].name;
    connect_info.userdata = resp;

    if (ssl) {
        connect_info.ssl_connection = LCCSCF_USE_SSL;
    }

    wsi = lws_client_connect_via_info(&connect_info);
    if (!wsi) {
        lws_context_destroy(context);
        free(resp->body);
        free(resp->headers);
        free(resp);
        return NULL;
    }

    // Event loop
    while (!resp->complete && !resp->failed) {
        lws_service(context, 50);
    }

    lws_context_destroy(context);

    if (resp->failed) {
        free(resp->body);
        free(resp->headers);
        free(resp);
        return NULL;
    }

    return resp;
}

// HTTP POST request
http_response_t* lws_http_post(const char *url, const char *body, const char *content_type) {
    // Similar to GET but with POST method and body
    // Implementation left as exercise - follows same pattern
    return NULL;
}

// Free HTTP response
void lws_http_response_free(http_response_t *resp) {
    if (resp) {
        free(resp->body);
        free(resp->headers);
        free(resp);
    }
}

// Response accessors
int lws_response_status(http_response_t *resp) {
    return resp ? resp->status_code : 0;
}

const char* lws_response_body(http_response_t *resp) {
    return resp ? resp->body : "";
}

const char* lws_response_headers(http_response_t *resp) {
    return resp ? resp->headers : "";
}

// ========== WEBSOCKET SUPPORT ==========

typedef struct {
    struct lws_context *context;
    struct lws *wsi;
    char *recv_buffer;
    size_t recv_len;
    size_t recv_capacity;
    int message_type;
    int closed;
    int failed;
} ws_connection_t;

static int ws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len) {
    ws_connection_t *conn = (ws_connection_t *)user;

    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            lwsl_user("WebSocket connection established\n");
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            // Accumulate received data
            if (conn->recv_len + len >= conn->recv_capacity) {
                conn->recv_capacity = (conn->recv_len + len + 1) * 2;
                conn->recv_buffer = realloc(conn->recv_buffer, conn->recv_capacity);
            }
            memcpy(conn->recv_buffer + conn->recv_len, in, len);
            conn->recv_len += len;

            if (lws_is_final_fragment(wsi)) {
                conn->recv_buffer[conn->recv_len] = '\0';
                conn->message_type = lws_frame_is_binary(wsi) ? 2 : 1;
            }
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            // Ready to send
            break;

        case LWS_CALLBACK_CLOSED:
            conn->closed = 1;
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            conn->failed = 1;
            conn->closed = 1;
            break;

        default:
            break;
    }

    return 0;
}

// WebSocket connect
ws_connection_t* lws_ws_connect(const char *url) {
    struct lws_context_creation_info info;
    struct lws_client_connect_info connect_info;
    ws_connection_t *conn;

    conn = calloc(1, sizeof(ws_connection_t));
    conn->recv_capacity = 4096;
    conn->recv_buffer = malloc(conn->recv_capacity);
    conn->recv_buffer[0] = '\0';

    // Parse URL
    char host[256] = {0};
    char path[512] = "/";
    int port = 80;
    int ssl = 0;

    if (strncmp(url, "wss://", 6) == 0) {
        ssl = 1;
        port = 443;
        sscanf(url + 6, "%255[^/]%511s", host, path);
    } else if (strncmp(url, "ws://", 5) == 0) {
        sscanf(url + 5, "%255[^/]%511s", host, path);
    } else {
        free(conn->recv_buffer);
        free(conn);
        return NULL;
    }

    // Create context
    memset(&info, 0, sizeof(info));
    info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.port = CONTEXT_PORT_NO_LISTEN;

    static const struct lws_protocols ws_protocols[] = {
        { "ws", ws_callback, sizeof(ws_connection_t), 4096, 0, NULL, 0 },
        { NULL, NULL, 0, 0, 0, NULL, 0 }
    };
    info.protocols = ws_protocols;

    conn->context = lws_create_context(&info);
    if (!conn->context) {
        free(conn->recv_buffer);
        free(conn);
        return NULL;
    }

    // Connect
    memset(&connect_info, 0, sizeof(connect_info));
    connect_info.context = conn->context;
    connect_info.address = host;
    connect_info.port = port;
    connect_info.path = path;
    connect_info.host = host;
    connect_info.origin = host;
    connect_info.protocol = ws_protocols[0].name;
    connect_info.userdata = conn;

    if (ssl) {
        connect_info.ssl_connection = LCCSCF_USE_SSL;
    }

    conn->wsi = lws_client_connect_via_info(&connect_info);
    if (!conn->wsi) {
        lws_context_destroy(conn->context);
        free(conn->recv_buffer);
        free(conn);
        return NULL;
    }

    // Wait for connection
    int timeout = 100;
    while (timeout-- > 0 && !conn->closed && !conn->failed) {
        lws_service(conn->context, 10);
    }

    if (conn->failed) {
        lws_context_destroy(conn->context);
        free(conn->recv_buffer);
        free(conn);
        return NULL;
    }

    return conn;
}

// WebSocket send text
int lws_ws_send_text(ws_connection_t *conn, const char *text) {
    if (!conn || conn->closed) return -1;

    size_t len = strlen(text);
    unsigned char *buf = malloc(LWS_PRE + len);
    memcpy(buf + LWS_PRE, text, len);

    int result = lws_write(conn->wsi, buf + LWS_PRE, len, LWS_WRITE_TEXT);
    free(buf);

    lws_service(conn->context, 0);
    return result >= 0 ? 0 : -1;
}

// WebSocket send binary
int lws_ws_send_binary(ws_connection_t *conn, const unsigned char *data, size_t len) {
    if (!conn || conn->closed) return -1;

    unsigned char *buf = malloc(LWS_PRE + len);
    memcpy(buf + LWS_PRE, data, len);

    int result = lws_write(conn->wsi, buf + LWS_PRE, len, LWS_WRITE_BINARY);
    free(buf);

    lws_service(conn->context, 0);
    return result >= 0 ? 0 : -1;
}

// WebSocket close
void lws_ws_close(ws_connection_t *conn) {
    if (conn) {
        if (conn->context) {
            lws_context_destroy(conn->context);
        }
        free(conn->recv_buffer);
        free(conn);
    }
}

// Check if closed
int lws_ws_is_closed(ws_connection_t *conn) {
    return conn ? conn->closed : 1;
}
