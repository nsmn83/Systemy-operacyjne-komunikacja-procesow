# Systemy-operacyjne-komunikacja-procesow

W poniższym pliku znajduje się treść polecenia z przedmiotu Systemy Operacyjne wraz z opisem zaimplementowanego rozwiązania.   

1. Zadanie projektowe:
Opracować zestaw programów typu producent - konsument realizujących następujący schemat synchronicznej komunikacji międzyprocesowej: 
•	Proces 1: po uruchomieniu wczytuje od użytkownika ścieżkę do dowolnego katalogu, a następnie przekazuje do procesu 2, poprzez mechanizm komunikacyjny K1, pełną ścieżkę dla każdego znalezionego wewnątrz zasobu (skrótu, pliku, katalogu). 
•	Proces 2: pobiera dane przesłane przez proces 1. Koduje otrzymane od procesu 1 dane do postaci heksadecymalnej, wypisuje postać źródłową i zakodowaną na ekranie w formacie: ścieżka_oryginalna -:- ścieżka_zakodowana, a następnie dane w postaci zakodowanej przekazuje do procesu 3 poprzez mechanizm komunikacyjny K2. 
•	Proces 3: pobiera dane wyprodukowane przez proces 2 i wypisuje je na ekranie. 

Ponadto należy zaimplementować oddzielną aplikację, która umożliwi odwrotną konwersję pozwalając tym samym na weryfikację poprawności przesyłania danych przez przygotowane aplikacje. W przypadku stwierdzenia błędów w komunikacji międzyprocesowej (np. przesłane dane zostaną uszkodzone lub są niekompletne), dalsza część projektu nie będzie sprawdzana). 
Wszystkie trzy procesy powinny być powoływane automatycznie z jednego procesu inicjującego (jeśli wykorzystane mechanizmy komunikacji to umożliwiają). Po powołaniu procesów potomnych proces inicjujący wstrzymuje swoją pracę. Proces inicjujący wznawia pracę w momencie kończenia pracy programu (o czym niżej) - jego zadaniem jest „posprzątać” po programie przed zakończeniem działania. 
Ponadto należy zaimplementować mechanizm asynchronicznego przekazywania informacji pomiędzy operatorem a procesami oraz pomiędzy procesami. Wykorzystać do tego dostępny mechanizm sygnałów. Operator może wysłać do dowolnego procesu sygnał zakończenia działania (S1), sygnał wstrzymania działania (S2), sygnał wznowienia działania (S3) i sygnał włączenia/wyłączenia funkcji kodującej (S4). Sygnał S2 powoduje wstrzymanie synchronicznej wymiany danych pomiędzy procesami. Sygnał S3 powoduje wznowienie tej wymiany. Sygnał S4 włącza/wyłącza kodowanie danych do postaci heksadecymalnej, w związku z czym między procesem 2 i 3 dane mogą być przesyłane w sposób niezmieniony. Sygnał S1 powoduje zakończenie działania aplikacji oraz zwolnienie wszelkich wykorzystywanych przez procesy zasobów (zasoby zwalnia proces macierzysty). Każdy z sygnałów może być przekazywany przez operatora do dowolnego procesu 1, 2 lub 3. O tym, do którego procesu wysłać sygnał, decyduje operator, a nie programista. Każdy z sygnałów operator może wysłać do innego procesu. Mimo, że operator kieruje sygnał do jednego procesu, to pożądane przez operatora działanie musi zostać zrealizowane przez wszystkie trzy procesy. W związku z tym, proces odbierający sygnał od operatora musi powiadomić o przyjętym żądaniu pozostałe dwa procesy. Powinien wobec tego przekazać do nich odpowiedni sygnał informując o tym jakiego działania wymaga operator. Procesy odbierające sygnał, powinny zachować się adekwatnie do otrzymanego sygnału. Wszystkie trzy procesy powinny zareagować zgodnie z żądaniem operatora.

Sygnały oznaczone w opisie zadania symbolami S1  S4 należy wybrać samodzielnie spośród sygnałów dostępnych w systemie.  
Parametr 	K1 	K2 
Mechanizm 	pamięć współdzielona 	kolejka komunikatów 

2.	Opis rozwiązania

Proces start.c jest procesem macierzystym, powołuje on procesy potomne (na ekran zostają wypisane ich PID), które z użyciem funkcji execl, która uruchamia programy p1, p2, p3. Proces macierzysty znajduje się w stanie wstrzymania – jego jedynym działaniem jest odbieranie sygnałów od procesów potomnych i wysyłanie odpowiednich sygnałów do każdego z procesów. Sygnały, których obsługe wykonuje proces macierzysty to kolejno: 
SIGHUP – po otrzymaniu tego sygnału proces macierzysty wysyła do P2 sygnał SIGUSR2 – P2 przestaje kodować otrzymywane ścieżki
SIGINT – po otrzymaniu tego sygnału proces macierzysty wstrzymuje wszysktie procesy wysyłając im sygnał SIGABRT
SIGQUIT – po otrzymaniu tego sygnału proces macierzysty wysyła do pozostałych procesów sygnał SIGBUS odpowiedzialny za wznowienie działania programu
SIGILL – po otrzymaniu tego sygnału proces wysyła do pozostałych procesów syganł SIGIO, który kończy ich działanie, następnie proces macierzysty zwalnia zasoby i wywołuje na sobie funkcje SIGKILL.

Proces P1 pobiera od użytkownika ścieżkę do katalogu, następnie zapisuje kolejne ścieżki do zasobów znalezionych w katalogu do pamięci współdzielonej, podnosi semafor o id równym 1 i oczekuje, aż proces P2 podniesie semafor o id = 0. W przypadku, gdy w katalgou nie ma kolejnych zasobów program czeka, aż P2 wypisze ostatnią ścieżkę, po czym sam podnosi semafor o id równym 0 i pyta użytkownika o ścieżkę do katalogu.

Proces P2, w  chwili, gdy możliwe jest opuszczenie semafora o id równym 1. Odczytuje z pamięci współdzielonej ścieżkę do zasobu, po czym, jeśli sygnał zmiany kodowania nie ustawił zmiennej flag na 0. Koduje ścieżkę do postaci heksadecymalnej. Wiadomość w postaci:
Ścieżka -:- zakodowana ścieżka
Jest następnie wysyłana do kolejki komunikatów, a proces P2 podnosi semafor o id równym 0, dzięki czemu P1 wysyła kolejną ścieżkę.

Proces P3 odczytuje dane zapisane w kolejce i wypisuje na ekran ściężkę do pliku, w takiej postaci, w jakiej zapisał ją P2.

Procesy  używają funkcji sleep, by zachować odstęp czasowy pomiędzy wykonywaniem kolejnych czynności, co pozwala użytkownikowi na wysłanie sygnału w trakcie działania programu. Program używa dwóch semaforów do komunikacji P1 i P2. Wstrzymanie kodowania zmienia zmienną globalną z 0 na 1 i na odwrót, co powoduje wstrzymanie bądź wznowienie operacji kodowania ścieżki przez P2.
Wstrzymanie działania całego programu jest realizowane poprzez zmiane kolejnej zmiennej globalnej. Każdy z procesów P1, P2, P3 po zmianie wartości tej zmiennej na 0, jeśli oczywiście jest ona ustawiona na 1, powoduje wejście w pętlę, która wykonuje się tak długo, aż sygnał wznowienia nie zmieni zmiennej globalnej z powrotem na 1. W implementacji obsługi semaforów, użyto pętli while, która pilnuje, by operacja atomowa, jaką jest podniesienia bądź opuszczenie sygnału nie została przerwana przez sygnał użytkownika.
