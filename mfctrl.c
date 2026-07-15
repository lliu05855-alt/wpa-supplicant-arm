#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUF 2048

const char *html =
"HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
"<!DOCTYPE html><html><head><meta charset='UTF-8'>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<title>MF922</title>"
"<style>*{margin:0;padding:0}body{font-family:Arial;padding:15px;background:#1a1a2e;color:#eee}"
"h1{color:#e94560;text-align:center;margin:0 0 20px}"
".card{background:#16213e;padding:15px;border-radius:10px;margin:10px 0;text-align:center;font-size:18px}"
".btn{display:block;width:100%;padding:25px;font-size:22px;border:0;border-radius:12px;"
"margin:12px 0;color:white;cursor:pointer;font-weight:bold}"
".ap{background:#e94560}.sta{background:#53d769}</style></head><body>"
"<h1>MF922 WiFi</h1>"
"<div class='card' id='s'>检测中...</div>"
"<a href='/ap'><button class='btn ap'>切换 AP 热点</button></a>"
"<a href='/sta'><button class='btn sta'>切换 Station 客户端</button></a>"
"<script>fetch('/status').then(r=>r.text()).then(t=>document.getElementById('s').innerText=t)</script>"
"</body></html>";

int main() {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr = {AF_INET, htons(PORT), {0}, {0}};
    bind(srv, (struct sockaddr*)&addr, sizeof(addr));
    listen(srv, 5);
    
    while (1) {
        int cli = accept(srv, NULL, NULL);
        char buf[BUF] = {0};
        read(cli, buf, BUF-1);
        
        char resp[512];
        if (strstr(buf, "GET /ap ")) {
            system("sh /mnt/userdata/wifi/ap.sh &");
            sprintf(resp, "HTTP/1.0 200 OK\r\n\r\nAP mode starting...");
        } else if (strstr(buf, "GET /sta ")) {
            system("sh /mnt/userdata/wifi/station.sh &");
            sprintf(resp, "HTTP/1.0 200 OK\r\n\r\nStation mode starting...");
        } else if (strstr(buf, "GET /status")) {
            FILE *fp = popen("ps|grep -q hostapd&&echo 'AP 热点'||(ps|grep -q wpa_supplicant&&echo 'Station 客户端'||echo '未知')", "r");
            char st[64] = "未知";
            if (fp) { fgets(st, 63, fp); pclose(fp); }
            sprintf(resp, "HTTP/1.0 200 OK\r\n\r\n%s", st);
        } else {
            write(cli, html, strlen(html));
            close(cli);
            continue;
        }
        write(cli, resp, strlen(resp));
        close(cli);
    }
}
