Structura a fost implementată astfel încât să poată fi asigurate câteva dintre funcțiile din stdio.h.

Abordare generală:
Structura _so_file incearcă să o imite pe cea implementată în stdio.h, astfel că aceasta asigură si buffering. Fiecare câmp din structură a fost comentat pentru a oferi mai multe informații.

Funcția so_fopen() face inițializarea unui fișier și permite deschiderea sau crearea acestuia cu flagurile din cerință.
so_fclose() închide file descriptorii și face so_fflush() în cazul în care trebuie scris din buffer in fișier (pe disk).
După acestea au fost implementate funcțiile so_fgetc()/so_fread() + so_fputc()/so_fwrite() conform recomandărilor. Trebuie menționat că so_fflush() a fost făcută cu ajutorul funcției xwrite din laboratorul 2 pentru a asigura scrierea completă în fișier, indiferent de existența unor posibile întreruperi.
so_fseek() ține cont de ce funcționalitate a deservit buffer-ul ultima dată (dacă a fost folosit pentru a se citi din el sau pentru a se scrie în el).

Tema este foarte utilă deoarece crează o imagine de ansamblu a modului de funcționare a stdio.h.

Implementarea mea este destul de naivă, am lucrat fără a avea o idee generală despre cum trebuie implementată structura so_file și am adăugat multe câmpuri pe parcurs, care sunt necesare implementării mele, dar care deviază de la ideal (structura de file din stdio.h).

IMPLEMENTARE

Nu a fost implementat pe deplin.
Nu există funcționalități în plus.
Lipsesc so_ferror(), so_popen() și so_pclose(). Acestea se reflectă din teste.

Am avut dificultăți la so_fread() în special din cauza size_t nmemb, pentru că mi-a luat ceva să-mi dau seama că nu poate trece sub 0 și la so_fflush() pentru că nu aveam o implementare asemănătoare xwrite().

COMPILARE
Se compilează cu make, fiind inclus și un Makefile și se testează cu checkerul oferit.