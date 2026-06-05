#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
const char* AUTHOR = "Weronika Malyska";

// Logi startowe
void log_info() {
    time_t now = time(0);
    printf("Start: %sAutor: %s\nPort: %d\n", ctime(&now), AUTHOR, PORT);
    fflush(stdout);
}

// Pobieranie pogody z wttr.in
void weather(int client_sock, const char* city) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
     struct sockaddr_in addr = {AF_INET, htons(80), {inet_addr("5.9.243.187")}};

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
        char req[512];
        sprintf(req, "GET /%s?format=%%l:++%%c+%%t++|++Wiatr:++%%w++|++Opady:++%%p++|++Wilgotnosc:++%%h HTTP/1.1\r\nHost: wttr.in\r\nUser-Agent: curl\r\nConnection: close\r\n\r\n", city);
        write(sock, req, strlen(req));
        
        char buf[1024];
        int n;
        write(client_sock, "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\n\r\n", 58);
        while ((n = read(sock, buf, 1024)) > 0) write(client_sock, buf, n);
    }
    close(sock);
}

int main(int argc, char *argv[]) {
    // Healthcheck sprawdzanie portu
    if (argc > 1) {
        int s = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in a = {AF_INET, htons(PORT), {inet_addr("127.0.0.1")}};
        return connect(s, (struct sockaddr *)&a, sizeof(a)) != 0;
    }

    log_info();
    int server = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in addr = {AF_INET, htons(PORT), {INADDR_ANY}};
    bind(server, (struct sockaddr *)&addr, sizeof(addr));
    listen(server, 5);

    while (1) {
        int client_sock = accept(server, NULL, NULL);
        char buffer[1024] = {0};
        read(client_sock, buffer, 1024);

        if (strstr(buffer, "loc=L")) weather(client_sock, "Lublin");
        else if (strstr(buffer, "loc=W")) weather(client_sock, "Warszawa");
        else if (strstr(buffer, "loc=G")) weather(client_sock, "Gdansk");
        else {
            const char* html = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\n\r\n"
                   "<html><head><style>"
                   "body { font-family: sans-serif; background: #f0f2f5; text-align: center; padding-top: 100px; }"
                   "h1 { color: #1c1e21; }"
                   "form { background: white; padding: 30px; border-radius: 12px; display: inline-block; box-shadow: 0 4px 12px rgba(0,0,0,0.1); }"
                   "select { padding: 10px; border-radius: 6px; border: 1px solid #ddd; margin-right: 10px; font-size: 16px; }"
                   "input { padding: 10px 20px; background: #007bff; color: white; border: none; border-radius: 6px; cursor: pointer; font-size: 16px; transition: 0.3s; }"
                   "input:hover { background: #0056b3; }"
                   "</style></head><body>"
                   "<h1>Pogoda</h1>"
                   "<form action='/' method='GET'>"
                   "<select name='loc'>"
                   "<option value='L'>Lublin</option>"
                   "<option value='W'>Warszawa</option>"
                   "<option value='G'>Gdańsk</option>"
                   "</select>"
                   "<input type='submit' value='Sprawdź'>"
                   "</form></body></html>";
            write(client_sock, html, strlen(html));
        }
        close(client_sock);
    }
    return 0;
}