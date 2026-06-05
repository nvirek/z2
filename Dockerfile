# syntax=docker/dockerfile:1
# Dyrektywa powyżej odblokowuje rozszerzony frontend BuildKit

# ETAP 1: Builder
FROM alpine:latest AS builder

# Instalacja niezbędnych narzędzi kompilacji i kompresji
# - gcc: kompilator języka C
# - musl-dev: niezbędne składniki, które pozwalają programowi działać bez systemu (kluczowe dla obrazów scratch)
# - upx: paker binarek, drastycznie redukujący końcowy rozmiar pliku wykonywalnego
# - --no-cache: flaga apk usuwająca indeksy pakietów zaraz po instalacji, co minimalizuje wagę warstwy builder
RUN apk add --no-cache gcc musl-dev upx

WORKDIR /app

# W środowisku GitHub Actions akcja 'checkout' pobiera kod automatycznie
COPY main.c .

# Zmienna BuildKit przechowująca informację o docelowej architekturze
ARG TARGETARCH

# Kompilacja krzyżowa 
# GCC generuje binarkę właściwą dla architektury docelowej (TARGETARCH)
# -static: „pakuje” wszystkie niezbędne narzędzia do jednego pliku, aby program działał w pustym obrazie
# -Os: optymalizacja pod kątem rozmiaru
# -s: stripping - usunięcie zbędnych tablic symboli
RUN gcc -static -Os -s -o app main.c

# Maksymalna kompresja binarki za pomocą algorytmu UPX
RUN upx --ultra-brute app

# ETAP 2: Produkcja
FROM scratch

# Implementacja adnotacji
LABEL org.opencontainers.image.authors="Weronika Małyska <s101040@pollub.edu.pl>"
LABEL org.opencontainers.image.description="Obraz Multi-arch"

# Kopiujemy TYLKO skompresowaną binarkę
COPY --from=builder /app/app /app

# Uruchomienie procesu jako użytkownik nieuprzywilejowany
USER 1001

# Dokumentacja portu nasłuchiwania aplikacji
EXPOSE 8080

# Healthcheck bezpośrednio na binarce
HEALTHCHECK --interval=30s CMD ["/app", "h"]

# Uruchomienie aplikacji jako głównego procesu kontenera
ENTRYPOINT ["/app"]