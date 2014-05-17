// Spataru Florina Gabriela 323 CA florina.spataru@gmail.com
// Tema 1 PC

Precizez ca pentru toate task-urile (pe care le-am rezolvat) folosesc
acelasi cod ca sa trimit si sa primesc filename, filesize si dimensiunea
ferestrei. Apoi pentru fiecare task am cate o functie de send si una de
receive.

* Task 0

La acest task, am folosit o implementare destul de simpla, bazata pe
rezolvarea laboratoarelor 2 si 4.
Am introdus o structura noua, my_pkt care va contine payload si un 
count, aka numarul pachetului trimis. Aceasta structura o voi pune 
in payload-ul structurii msg. 
In send.c trimit mesajele in doua for-uri. Intai trimit o fereastra 
intreaga si apoi alternez receive cu send. In recv.c am un while care
primeste pachete atata timp cat nu s-a primit date care insumate sunt
egale cu dimensiunea fisierului.

* Task 1

In send.c, intai trimit mesaje cat dimensiunea ferestrei. Am o variabila
sentSize care imi ofera conditia de la while. Atunci cand voi primi
un ack, voi incrementa sentSize cu dimensiunea lui my_pkt.
Cand primesc ack-ul pentru mesajul corect (am o variabila expectedAck
pentru verificare) mai citesc un mesaj din fisier si il trimit.
Altfel, trimit tot buffer-ul (vector circular in care am stocat mesajele 
trimise pentru care nu am primit ack). Receive va fi cu timeout in send.c.
In recv.c, daca nu primesc mesajul pe care il asteptam, pur si simplu il
ignor, si nu trimit nici un ack inapoi. Daca este bun, il scriu in fisierul
de iesire si trimit ack pentru el.

* Task 2

Ca la celelalte task-uri, intai trimit o fereastra in for. Apoi am un while
in care fac operatii atata timp cat mai exista bucati din fisier care nu au
fost primite de recv.c. 
Am un recv_with_timeout. Daca rezultatul lui e < 0 deci ori s-a terminat
timeout-ul, ori a fost eroare, trimit toate mesajele din buffer pt care am
ack = 0 (nu au fost primite de recv.c).
Daca recv_with_timeout intoarce ceva pozitiv, deci am primit un ack, fac 
ack = 1 in pentru mesajul respectiv din buffer. Apoi verific daca ack-ul
primit era cel pe care il asteptam. Daca este, voi verifica intai daca in
buffer mai am mesaje consecutive care au fost deja primite de recv.c
(ack = 1). Numarul acestor mesaje il stochez in variabila buffDiff. Daca 
nu se gaseste nici un mesaj pentru care sa am ack = 0 inseamna ca tot 
buffer-ul a fost deja trimis deci buffDiff = dimF (dimensiunea ferestrei), 
in caz constrar, voi determina valoare acestei variabile. In acest punct
maresc variabila ce imi retine cat din fisier a fost deja trimis cu succes
cu buffDiff*dimensiunea_unui_mesaj. Apoi, voi inlocui buffDiff mesaje
din buffer cu mesaje noi citite din fisier.

In recv.c am de asemenea un buffer de mesaje, dar acesta functioneaza diferit 
fata de cel din send.c. Il folosesc ca pe un fel de stiva. Daca nu primesc 
mesajul pe care il asteptam, adaug in vector mesajul. Spre exemplu, am in vector 
mesajele 4, 5, 7. Cand primesc mesajul 3, scot din vector 4 si 5 si le scriu in 
fisier, cu tot cu 3, iar vectorul il va contine acum doar pe 7. Am observat ca
exista cazul in care acest vector contine ceva gen 5, 8, 7. Pe acest caz, 
evident, algoritmul nu mai functiona cum trebuie. Solutia pe care am gasit-o 
a fost sortarea acestui vector prin metoda bubble sort.


* Task 3

La rezolvarea acestui task m-am bazat mult pe algoritmul precedent. Insa, in loc
de structura my_pkt am un my_sum care, pe langa campurile count si payload, mai
are si un checksum. Pentru a calcula aceasta valoare folosesc o functie care 
va primi ca parametrii payload, lungimea lui si count. Checksum va fi suma pe 
char-uri din payload si acel count. In send.c trimit pachete cu acest camp aditional
si in recv.c il verific reconstruind-ul. In cazul in care checksum calculat in recv.c
este diferit de checksum primit, inseamna ca mesajul este corupt si il tratez ca pe 
un loss, pur si simplu il ignor.

Feedback: Trebuie sa recunosc ca a fost o tema interesanta, cu ajutorul careia am
inteles foarte bine protocoalele Go-Back-N si Selective Repeat. 
Desi am trimis cu intarziere! :)

