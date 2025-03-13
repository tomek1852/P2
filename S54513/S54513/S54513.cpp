#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

using namespace std;

// Struktura przechowująca stan gry
struct StanGry {
    char kolorGracza;  // 'b' dla białych, 'c' dla czarnych
    vector<string> sterty;  // Wektor przechowujący stan każdej sterty jako string
};

StanGry analizujOpisGry(const string& opis) {
    StanGry stan;
    stan.kolorGracza = opis[0];  // Pierwszy znak określa kolor gracza ('b' lub 'c')
    string opisSterty = opis.substr(2);  // Pobieramy opis stert, pomijając kolor i spację

    string aktualnaSterta = "";  // Tymczasowy string do budowania sterty
    for (int i = 0; i < opisSterty.length(); i++) {
        if (opisSterty[i] == '|') {  // Jeśli napotkamy separator
            stan.sterty.push_back(aktualnaSterta);  // Dodajemy zbudowaną stertę do wektora
            aktualnaSterta = "";  // Resetujemy tymczasowy string
        }
        else {
            aktualnaSterta += opisSterty[i];  // Dodajemy znak do aktualnej sterty
        }
    }
    if (!aktualnaSterta.empty()) {  // Dodajemy ostatnią stertę, jeśli coś zostało
        stan.sterty.push_back(aktualnaSterta);
    }

    return stan;  // Zwracamy gotowy stan gry
}

bool czySaRuchy(const StanGry& stan, char kolor) {
    for (const string& sterta : stan.sterty) {
        if (sterta.empty()) continue;  // Pomijamy puste sterty
        for (char krążek : sterta) {
            if (krążek == kolor) return true;  // Znaleźliśmy nasz kolor
        }
    }
    return false;  // Brak ruchów
}

pair<int, string> znajdzRuch(StanGry& stan) {
    char mojKolor = stan.kolorGracza;  // Nasz kolor
    char kolorPrzeciwnika = (mojKolor == 'b') ? 'c' : 'b';  // Kolor przeciwnika

    for (int i = 0; i < stan.sterty.size(); i++) {
        string& sterta = stan.sterty[i];
        if (sterta.empty()) continue;  // Pomijamy puste sterty

        for (int j = 0; j < sterta.length(); j++) {
            if (sterta[j] == mojKolor) {  // Znaleźliśmy nasz kolor
                string nowyStanSterty = (j == 0) ? "" : sterta.substr(0, j);  // Nowy stan sterty
                string staryStan = sterta;  // Zapamiętujemy obecny stan
                sterta = nowyStanSterty;  

                // Sprawdzamy, czy przeciwnik ma ruchy po naszym ruchu
                if (!czySaRuchy(stan, kolorPrzeciwnika)) {
                    sterta = staryStan;  
                    return { i, nowyStanSterty };  // Zwracamy zwycięski ruch
                }
                sterta = staryStan;  // Przywracamy stan sterty
            }
        }
    }

    // Jeśli nie ma zwycięskiego ruchu, bierzemy pierwszy dostępny
    for (int i = 0; i < stan.sterty.size(); i++) {
        string& sterta = stan.sterty[i];
        if (sterta.empty()) continue;
        for (int j = 0; j < sterta.length(); j++) {
            if (sterta[j] == mojKolor) {
                return { i, (j == 0) ? "" : sterta.substr(0, j) };
            }
        }
    }

    return { -1, "" };
}

// Funkcja generująca polecenie ruchu w formacie protokołu
string generujPolecenieRuchu(const pair<int, string>& ruch) {
    string polecenie = "210 " + to_string(ruch.first);  // Tworzymy komendę z numerem sterty
    if (!ruch.second.empty()) {  // Jeśli sterta nie jest pusta po ruchu
        polecenie += " " + ruch.second;  // Dodajemy nowy stan sterty
    }
    return polecenie;  // Zwracamy gotowe polecenie jako string
}

int main() {
    string linia; 
    StanGry stan;

    
    while (getline(cin, linia)) {
        int polecenie = 0;  // Numer polecenia z protokołu
        int i = 0;
        
        while (i < linia.length() && linia[i] >= '0' && linia[i] <= '9') {
            polecenie = polecenie * 10 + (linia[i] - '0');
            i++;
        }

        if (polecenie == 200) {  // Rozpoczęcie gry - otrzymujemy stan początkowy
            string opis = linia.substr(i);  // Pobieramy resztę linii (opis gry)
            if (!opis.empty() && opis[0] == ' ') opis = opis.substr(1);  // Usuwamy spację
            stan = analizujOpisGry(opis);  // Inicjalizujemy stan gry
            auto ruch = znajdzRuch(stan);  // Znajdujemy nasz ruch
            if (ruch.first != -1) {  // Jeśli mamy ruch, aktualizujemy stan
                stan.sterty[ruch.first] = ruch.second;
            }
            cout << generujPolecenieRuchu(ruch) << endl;  // Wysyłamy ruch do serwera

        }
        else if (polecenie == 220) {  // Ruch przeciwnika - aktualizujemy stan i odpowiadamy
            while (i < linia.length() && linia[i] == ' ') i++;  // Pomijamy spacje
            int numerSterty = 0;
            // Ręczne wczytanie numeru sterty
            while (i < linia.length() && linia[i] >= '0' && linia[i] <= '9') {
                numerSterty = numerSterty * 10 + (linia[i] - '0');
                i++;
            }
            string nowyStan = "";
            if (i < linia.length() && linia[i] == ' ') {
                nowyStan = linia.substr(i + 1);  // Pobieramy resztę linii jako nowy stan
            }

            // Aktualizujemy stan gry po ruchu przeciwnika
            stan.sterty[numerSterty] = nowyStan;

            auto ruch = znajdzRuch(stan);  // Znajdujemy nasz ruch
            if (ruch.first != -1) {  // Jeśli mamy ruch, aktualizujemy stan
                stan.sterty[ruch.first] = ruch.second;
            }
            cout << generujPolecenieRuchu(ruch) << endl;  // Wysyłamy ruch

        }
        else if (polecenie >= 230) {  // Koniec gry (wygrana lub przegrana)
            break;  // Kończymy działanie programu

        }
        else if (polecenie == 999) {  // Błąd w komunikacji z serwerem
            cerr << "Błąd: " << linia << endl;  // Wyświetlamy komunikat błędu
            break;  // Kończymy program
        }
    }

    return 0;
}