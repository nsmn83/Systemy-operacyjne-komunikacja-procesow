# Systemy-operacyjne-komunikacja-procesow

## Opis rozwiązania

### Proces start.c
Proces start.c jest procesem macierzystym, powołuje on procesy potomne (na ekran zostają wypisane ich PID), które z użyciem funkcji execl, która uruchamia programy p1, p2, p3. Proces macierzysty znajduje się w stanie wstrzymania – jego jedynym działaniem jest odbieranie sygnałów od procesów potomnych i wysyłanie odpowiednich sygnałów do każdego z procesów. Sygnały, których obsługe wykonuje proces macierzysty to kolejno: 
SIGHUP – po otrzymaniu tego sygnału proces macierzysty wysyła do P2 sygnał SIGUSR2 – P2 przestaje kodować otrzymywane ścieżki
SIGINT – po otrzymaniu tego sygnału proces macierzysty wstrzymuje wszysktie procesy wysyłając im sygnał SIGABRT
SIGQUIT – po otrzymaniu tego sygnału proces macierzysty wysyła do pozostałych procesów sygnał SIGBUS odpowiedzialny za wznowienie działania programu
SIGILL – po otrzymaniu tego sygnału proces wysyła do pozostałych procesów syganł SIGIO, który kończy ich działanie, następnie proces macierzysty zwalnia zasoby i wywołuje na sobie funkcje SIGKILL.

### Proces P1

Proces P1 pobiera od użytkownika ścieżkę do katalogu, następnie zapisuje kolejne ścieżki do zasobów znalezionych w katalogu do pamięci współdzielonej, podnosi semafor o id równym 1 i oczekuje, aż proces P2 podniesie semafor o id = 0. W przypadku, gdy w katalgou nie ma kolejnych zasobów program czeka, aż P2 wypisze ostatnią ścieżkę, po czym sam podnosi semafor o id równym 0 i pyta użytkownika o ścieżkę do katalogu.

### Proces P2

Proces P2, w  chwili, gdy możliwe jest opuszczenie semafora o id równym 1. Odczytuje z pamięci współdzielonej ścieżkę do zasobu, po czym, jeśli sygnał zmiany kodowania nie ustawił zmiennej flag na 0. Koduje ścieżkę do postaci heksadecymalnej. Wiadomość w postaci:
Ścieżka -:- zakodowana ścieżka
Jest następnie wysyłana do kolejki komunikatów, a proces P2 podnosi semafor o id równym 0, dzięki czemu P1 wysyła kolejną ścieżkę.

### Proces P3

Proces P3 odczytuje dane zapisane w kolejce i wypisuje na ekran ściężkę do pliku, w takiej postaci, w jakiej zapisał ją P2.

Procesy  używają funkcji sleep, by zachować odstęp czasowy pomiędzy wykonywaniem kolejnych czynności, co pozwala użytkownikowi na wysłanie sygnału w trakcie działania programu. Program używa dwóch semaforów do komunikacji P1 i P2. Wstrzymanie kodowania zmienia zmienną globalną z 0 na 1 i na odwrót, co powoduje wstrzymanie bądź wznowienie operacji kodowania ścieżki przez P2.
Wstrzymanie działania całego programu jest realizowane poprzez zmiane kolejnej zmiennej globalnej. Każdy z procesów P1, P2, P3 po zmianie wartości tej zmiennej na 0, jeśli oczywiście jest ona ustawiona na 1, powoduje wejście w pętlę, która wykonuje się tak długo, aż sygnał wznowienia nie zmieni zmiennej globalnej z powrotem na 1. W implementacji obsługi semaforów, użyto pętli while, która pilnuje, by operacja atomowa, jaką jest podniesienia bądź opuszczenie sygnału nie została przerwana przez sygnał użytkownika.
