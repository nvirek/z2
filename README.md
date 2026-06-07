# Zadanie 2
## Inicjalizacja lokalnego repozytorium i przesłanie plików na GitHuba
<img width="945" height="848" alt="image" src="https://github.com/user-attachments/assets/e99147c4-e001-4c5b-9b9b-69c24e49109e" />

## Ustawienie tokena i loginu w GitHub Secrets/Variables
<img width="920" height="394" alt="image" src="https://github.com/user-attachments/assets/b7377d0c-6e29-4356-8d5d-8235ff5660c4" />

## Zatwierdzenie zmian i wysłanie pliku pipeline.yml na GitHuba
<img width="945" height="489" alt="image" src="https://github.com/user-attachments/assets/7da7ed76-83ca-4a52-b41f-e163c8a03d38" />

## Dowód poprawnego wykonania wszystkich kroków łańcucha
<img width="945" height="580" alt="image" src="https://github.com/user-attachments/assets/2a044de1-d8aa-4e19-9b58-3d19505d5cc2" />

# a. Wsparcie dla architektur linux/arm64 oraz linux/amd64

Budowanie obrazu dla wielu architektur zrealizowano za pomocą środowiska Docker Buildx. W łańcuchu GitHub Actions wykorzystano wtyczkę `docker/setup-qemu-action` do uruchomienia emulatora architektur sprzętowych (QEMU) oraz wtyczkę `docker/setup-buildx-action`. W ostatecznym kroku budowania (`docker/build-push-action`) zdefiniowano docelowe platformy za pomocą parametru `platforms: linux/amd64,linux/arm64`, co pozwoliło na utworzenie jednego, gotowego obrazu działającego na obu architekturach.

# b. Wykorzystanie cache w trybie max na DockerHub

W celu optymalizacji czasu budowania zaimplementowano mechanizm pamięci podręcznej typu _registry_. Gotowy obraz aplikacji jest wysyłany do rejestru na GitHubie (`ghcr.io`), natomiast dane cache zapisywane są w osobnym repozytorium na DockerHub (`z2_cache`). W konfiguracji użyto parametrów `cache-froM` oraz `cache-to`, ustawiając tryb na `mode=max`. Tryb `max` sprawia, że w cacheu zapisywane są wszystkie warstwy obrazu - zapamiętywany jest każdy pojedynczy krok budowy z pliku Dockerfile, a nie tylko gotowy wynik. To maksymalnie przyspiesza proces budowania przy kolejnych uruchomieniach.

# c. Test CVE przed wysłaniem obrazu (Trivy)

Do analizy bezpieczeństwa (test CVE) wybrano skaner Trivy (`aquasecurity/trivy-action`). Narzędzie to zostało wybrane, ponieważ działa bardzo szybko i łatwo je dodać do potoku w GitHub Actions.
Aby spełnić warunek wysłania obrazu do rejestru tylko po udanym teście bezpieczeństwa, proces podzielono na dwa etapy wewnątrz tego samego potoku:

* W pierwszej kolejności zbudowano tymczasowy lokalny obraz (parametr `load: true`), bez wysyłania go jeszcze do publicznego rejestru obrazów.
* Następnie skaner Trivy przeanalizował ten obraz, z warunkiem przerwania łańcucha (`exit-code: '1'`) w przypadku znalezienia luk oznaczonych jako `CRITICAL` lub `HIGH`.
* Zastosowanie w _Dockerfile_ pustego obrazu bazowego _scratch_ zagwarantowało całkowity brak podatności na poziomie systemu operacyjnego. Skaner potwierdził bezpieczeństwo obrazu, co pozwoliło potokowi przejść do ostatecznego kroku: ponownego zbudowania obrazu dla obu architektur i opublikowania go na GitHubie. W przypadku wykrycia luk, łańcuch zakończyłby się błędem, skutecznie blokując publikację niebezpiecznego kontenera.
