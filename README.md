# Proiect Lunch01

Cerinta:
Fie un server TCP concurent la care se pot conecta maxim N clienti. Din T in T secunde (T < 120) serverul va primi un mesaj de la o parte din clienti ce va identifica printr-un numar un fel de mancare (de la 1 la 5) ales random de respectivul client. Serverul va contoriza cererile iar pentru felul cel mai preferat (avand cele mai multe cereri) va trimite la clientii solicitanti ca raspuns: "Masa e servita". Celorlalti clienti (care au ales alt fel), le va raspunde cu "Indisponibil". Un client ce a fost servit cu pranzul va afisa un mesaj "Satul!" si isi va incheia executia. Un client refuzat va mai incerca o cerere aleasa tot random dupa un interval ales de asemeni aleator (pana in 60 de secunde). La trei cereri refuzate, un client va afisa "Schimb cantina! Aici mor de foame!" si isi va incheia executia.

Implementare:

Serverul deserveste clientii printr-un mecanism de prethread-ing, cu blocarea mutex de protectie pentru accept() si modificarea vectorului de comenzi.

Inainte de pornirea serverului si a clientilor trebuie rulat fisierul Makefile prin comanda make.

Severul porneste in urma comenzii ./server.bin N T, unde N - nr maxim de clienti, iar T - intervalul de secunde intre 2 "serviri" ale clientilor. N trebuie sa fie un nr pozitiv, iar T un nr intre 0 si 120. 

Un exemplu de astfel de comanda este ./server.bin 10 20

Un clientul porneste in urma comenzii ./client.bin 127.0.0.1 2910 (serverul foloseste portul 2910).

Un client mai poate fi pornit si prin comanda make client, definita in fisierul makefile.

