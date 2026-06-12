#include "ota_esp.h"
#include "wifi_mqtt_esp.h"
#include "LogSistema.h"

#include <Update.h>
#include <Ethernet.h>
#include <WiFiClient.h>
#include <HTTPClient.h>

static void parseUrl(const char *url, char *host, uint16_t &port, char *path)
{
    port = 80;
    const char *p = strstr(url, "://");
    p = p ? p + 3 : url;
    const char *slash = strchr(p, '/');
    const char *colon = strchr(p, ':');

    if (colon && (!slash || colon < slash))
    {
        size_t len = (size_t)(colon - p);
        strncpy(host, p, len);
        host[len] = '\0';
        port = (uint16_t)atoi(colon + 1);
    }
    else
    {
        size_t len = slash ? (size_t)(slash - p) : strlen(p);
        strncpy(host, p, len);
        host[len] = '\0';
    }

    strcpy(path, slash ? slash : "/");
}

static void otaEthernet(const char *host, uint16_t port, const char *path)
{
    EthernetClient client;

    if (!client.connect(host, port))
    {
        LOG("\r\nOTA Ethernet: no se pudo conectar a " + String(host));
        return;
    }

    client.print(String("GET ") + path + " HTTP/1.0\r\nHost: " + host + "\r\nConnection: close\r\n\r\n");

    // Parsear headers HTTP: status code + Content-Length
    int statusCode = 0;
    long contentLength = 0;
    bool firstLine = true;
    while (client.connected())
    {
        String line = client.readStringUntil('\n');
        if (firstLine)
        {
            // "HTTP/1.0 200 OK"
            int sp = line.indexOf(' ');
            if (sp >= 0)
                statusCode = line.substring(sp + 1).toInt();
            firstLine = false;
        }
        line.trim();
        if (line.length() == 0)
            break; // fin de headers
        String lower = line;
        lower.toLowerCase();
        if (lower.startsWith("content-length:"))
            contentLength = line.substring(line.indexOf(':') + 1).toInt();
    }

    if (statusCode != 200)
    {
        LOG("\r\nOTA Ethernet: HTTP status " + String(statusCode));
        client.stop();
        return;
    }

    if (contentLength <= 0)
    {
        LOG("\r\nOTA Ethernet: Content-Length inválido (" + String(contentLength) + ").");
        client.stop();
        return;
    }

    if (!Update.begin(contentLength))
    {
        LOG("\r\nOTA Ethernet: Update.begin() falló. " + String(Update.errorString()));
        client.stop();
        return;
    }

    LOG("\r\nOTA Ethernet iniciando. tamano: " + String(contentLength) + " Path: " + String(path) + " Host: " + String(host) + " Port: " + String(port));

    uint8_t buf[1024];
    size_t written = 0;
    while ((client.connected() || client.available()) && written < (size_t)contentLength)
    {
        int n = client.read(buf, sizeof(buf));
        if (n > 0)
        {
            if (Update.write(buf, n) != (size_t)n)
            {
                LOG("\r\nOTA Ethernet: error en Update.write(). " + String(Update.errorString()));
                break;
            }
            written += (size_t)n;
        }
        yield();
    }

    client.stop();

    if (written == (size_t)contentLength && Update.end())
        LOG("\r\nOTA Ethernet: flash completado.");
    else
        LOG("\r\nOTA Ethernet: Update.end() falló. Escrito " + String(written) + "/" + String(contentLength) + ". " + String(Update.errorString()));
}

static void otaWiFi(const char *url)
{
    WiFiClient wClient;
    HTTPClient http;
    http.begin(wClient, url);

    int code = http.GET();
    if (code == HTTP_CODE_OK)
    {
        int size = http.getSize();
        if (Update.begin(size > 0 ? size : UPDATE_SIZE_UNKNOWN))
        {
            LOG("\r\nOTA WiFi iniciando. URL: " + String(url));
            Update.writeStream(*http.getStreamPtr());
            if (Update.end())
                LOG("\r\nOTA WiFi: flash completado.");
            else
                LOG("\r\nOTA WiFi: Update.end() falló.");
        }
        else
        {
            LOG("\r\nOTA WiFi: Update.begin() falló.");
        }
    }
    else
    {
        LOG("\r\nOTA WiFi: HTTP GET falló. Código: " + String(code));
    }

    http.end();
}

void ejecutarOTA(const char *url)
{
    LOG("\r\n\r\nOTA: iniciando. URL: " + String(url));

    char host[128] = {0};
    uint16_t port = 80;
    char path[128] = {0};
    parseUrl(url, host, port, path);

    LOG("\r\nOTA host: " + String(host) + " port: " + String(port) + " path: " + String(path));

    if (getUsingEthernet())
        otaEthernet(host, port, path);
    else
        otaWiFi(url);

    LOG("\r\nOTA: reiniciando...");
    delay(200);
    ESP.restart();
}
