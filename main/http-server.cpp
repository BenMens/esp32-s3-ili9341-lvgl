#include "http-server.hpp"

#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_vfs.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <spiffs.hpp>

#define TAG "http-server"

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

/* Scratch buffer size */
#define SCRATCH_BUFSIZE 8192

struct fileServerData {
    /* Base path of file storage */
    char basePath[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

static const char *getPathFromUri(char *dest, const char *basePath,
                                  const char *uri, size_t destSize)
{
    const size_t basePathLen = strlen(basePath);
    size_t pathLen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathLen = MIN(pathLen, quest - uri);
    }

    const char *hash = strchr(uri, '#');
    if (hash) {
        pathLen = MIN(pathLen, hash - uri);
    }

    if (basePathLen + pathLen + 1 > destSize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, basePath);
    strlcpy(dest + basePathLen, uri, pathLen + 1);

    /* Return pointer to path, skipping the base */
    return dest + basePathLen;
}

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

static esp_err_t setContentTypeFromFile(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        return httpd_resp_set_type(req, "application/pdf");
    } else if (IS_FILE_EXT(filename, ".html")) {
        return httpd_resp_set_type(req, "text/html");
    } else if (IS_FILE_EXT(filename, ".jpeg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    } else if (IS_FILE_EXT(filename, ".ico")) {
        return httpd_resp_set_type(req, "image/x-icon");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

esp_err_t downloadGetHandler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    const char *filename = getPathFromUri(
        filepath, ((struct fileServerData *)req->user_ctx)->basePath, req->uri,
        sizeof(filepath));

    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);

        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    fd = fopen(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);

        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                            "Failed to read existing file");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename,
             file_stat.st_size);
    setContentTypeFromFile(req, filename);

    /* Retrieve the pointer to scratch buffer for temporary storage */
    char *chunk = ((struct fileServerData *)req->user_ctx)->scratch;
    size_t chunksize;
    do {
        /* Read file in chunks into the scratch buffer */
        chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

        if (chunksize > 0) {
            /* Send the buffer contents as HTTP response chunk */
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(fd);
                ESP_LOGE(TAG, "File sending failed!");
                /* Abort sending file */
                httpd_resp_sendstr_chunk(req, NULL);

                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                                    "Failed to send file");
                return ESP_FAIL;
            }
        }
    } while (chunksize != 0);

    fclose(fd);
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);

    return ESP_OK;
}

esp_err_t getHandler(httpd_req_t *req)
{
    return downloadGetHandler(req);
}

esp_err_t postHandler(httpd_req_t *req)
{
    char content[100];

    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t startWebserver(httpd_handle_t *handle, const char *basePath)
{
    static struct fileServerData *server_data = NULL;

    if (server_data) {
        ESP_LOGE(TAG, "File server already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = (fileServerData *)calloc(1, sizeof(struct fileServerData));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->basePath, basePath, sizeof(server_data->basePath));

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

    static httpd_uri_t uriGet = {
        .uri = "/*",
        .method = HTTP_GET,
        .handler = getHandler,
        .user_ctx = server_data,
    };

    static httpd_uri_t uriPost = {
        .uri = "/*",
        .method = HTTP_POST,
        .handler = postHandler,
        .user_ctx = server_data,
    };

    if (httpd_start(handle, &config) == ESP_OK) {
        httpd_register_uri_handler(*handle, &uriGet);
        httpd_register_uri_handler(*handle, &uriPost);
    }

    return ESP_OK;
}

void stopWebserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}