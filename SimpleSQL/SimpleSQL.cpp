#include <iostream>
#include <stdexcept>

#include "tokenizer.hpp"
#include "Parser.hpp"
#include "Table.hpp"
#include "DataBase.hpp"

/*

    - typy danych
    NULL - brak danych
    integer - liczba calkowita
    text - text o dowolnej wielkosci np. 'email.com'
    numeric - liczba zmiennoprzecinkowa

    loaddb 'Nazwa' - laduje baze danych o nazwie Nazwa
    Jest to tak naprawde sciezka do folder o nazwie Nazwa
    w srodku sa pliki .table oraz plik .rel, ktore odpowiednio sa zwiazane z zapisem tabel i relacji
    savedb 'Nazwa' - zapisuje baze danych, Nazwa jest opcjonalna

    alter table NazwaTabeli add NazwaKolumny typDanych - dodaje kolumne do tabeli
    - typ moze byc: integer

    alter table NazwaTabeli drop NazwaKolumny
    - usuwa kolumne z tabeli, kolumna do usuniecia nie moze byc kluczem glownym oraz nie moze byc relacji z nia zwiazanych

    alter table NazwaTabeli addkey NazwaTabeli2 on NazwaKolumny
    - dodaje klucz obcy do tabeli NazwaTabeli2 na wartosci z NazwaKolumny, warosc z NazwaKolumny musi odpowiadac jakiejs
    wartosci z NazwaTabli(z klucza glownego) lub music byc NULL

    alter table NazwaTabeli dropkey NazwaTabeli2 on NazwaKolumny
    - usuwa klucz obcy

    insert into NazwaTabeli NazwaKolumny = wartosc, ...
    - dodaje wiersz do tabeli NazwaTabeli, trzeba tylko dodac klucz glowny
    - w przypadku typu columny text wartosc powinna byc w '' np 'email@emai.com'
    ex: INSERT INTO User Id = 2, Email = 'text.pl'

    create table NazwaTabeli (NazwaKolumny typDanych [pk], ...)
    tworzy tabele o nazwie NazwaTabeli, pk (Klucz głowny) musi byc ustawione dla jednej kolumny
    ex: CREATE TABLE ex (Id integer pk, Email text)

    select NazwaKolumny, ... from NazwaTabeli [join NazwaTabeli2 on NazwaTabeli.k1 = NazwaTabeli2.k2 ] [where kolumna operacja [opcjonalna wartosc]] [orderby kolumna [asc, desc], ...] [limit n [offset z]]
    - wybiera wiersze z tabli NazwaTabeli (mozna wyswietlic wszystkie wiersze operatorem *)
    - opcjonalnie mozna dodac where i okreslic warunki
    - warunkow moze byc dowolnie wiele
    - podstawowe warunki =, !=, <, >, <=, >=, isnull, isnotnull, like(_ - dowolny jeden znak, % zero lub wiecej dowolnych znakow)
    - podstawowe warunki mozna laczy operatorami AND i OR
    - join (opcjonalne, INNER JOIN) Laczy dwie tabele w jedna na kluczu NazwaTabeli.k1 = NazwaTabeli2.k2, przed nazwa kolumn trzeba dodac nazwetabeli z kropka
    - orderby(opcjonalne) sortuje wyniki po podanych kolumnach asc - rosnaca, desc - malejaco
    - limit(opcjonalne) ogranicza wyswitlanie weirszy do n
    - offset(opcjonalne) przesuwa wyswietlane wiersze o z
    ex: select * from User join UserData on User.Id = UserData.UserId where User.Email like '%.p_' and UserData.Secret isnotnull orderby User.Id desc limit 1 offset 1

    delete from NazwaTabeli [where warunki...]
    - usuwa wiersze z tabeli, jesli nie ma where usunie wszystkie wiersze
    - warunkow moze byc dowolnie wiele i sa takie same jak w przypadku select
    ex: delete from User where Email like '%.com'

    update NazwaTabeli set columna = cos, ... where warunki
    - aktualizuje warotsci w kolumnach dla ktorych warunek jest spelniony
    - warunkow moze byc dowolnie wiele i sa takie same jak w przypadku select
    - nie mozna zmieniac kluczy glownych
    - jesli klucz obcy nie odwoluje sie odpowiednio do tabeli, update nie powiedzie sie
    ex: update User set password = 'admin123' where password isnull

    drop table NazwaTabli
    - usuwa cala tabele
    - jesli sa jakies referencje do tej tabeli operacja nie powiedzie sie

    quit
    - konczy dzialanie bazy i nie zapisuje danych

    create
*/

auto main() -> int
{
    kSQL::DataBase db;
    auto query = std::string("");
    while (std::getline(std::cin, query))
        db.Execute(query);
}