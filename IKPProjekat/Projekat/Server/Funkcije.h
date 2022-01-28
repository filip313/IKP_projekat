#pragma once
#include "Strukture.h"
/*
	Funkcija: oslobodi_klijenta
	---------------------------
	Funkcionalnost: Izbacujemo klijenta iz liste povezanih igraca i oslobadjamo memoriju.

	Parametri:
	client -> struktura klijenta sa podacima kojeg treba izbaciti iz liste.

	Povratana vrednost: Nema povratnu vrednost.
*/
void oslobodi_klijenta(client_veza* client);

/*
	Funkcija: Pocetak igre
	---------------------------
	Funkcionalnost: Prolazi kroz listu povezanih igraca i salje im strukturu Opseg koja sadrzi opseg pogadjanja

	Parametri:
	op-> struktura opseg koja sadrzi interval u kom se nalazi zamisljeni broj.

	Povratana vrednost: Nema povratnu vrednost.
*/
void PocetakIgre(opseg op);
/*
	Funkcija: InitializeWindowsSockets
	---------------------------
	Funkcionalnost:Funkcija za inicijalizovanje socketa.

	Parametri:
	Nema

	Povratana vrednost: indikator da li je podesavanje bilo uspesno
*/
bool InitializeWindowsSockets();
/*
	Funkcija: prihvat_konekcije
	---------------------------
	Funkcionalnost: Pokrece se u threadu i sluzi da prihvati klijentski zahtev za konekciju i napravi thread za klijenta

	Parametri:
	param-> listensocket

	Povratana vrednost: Nema povratnu vrednost
*/
DWORD WINAPI prihvat_konekcije(LPVOID param);
/*
	Funkcija: registracija
	---------------------------
	Funkcionalnost: Prihvata poruku sa klijenta i u zavisnosti da li je klijent poslao igrac ili vodja uvezuje ga u listu igraca ili popunjava strukturu za vodju 

	Parametri:
	client->klijentska struktura koja predstavlja odgovarajuceg klijenta

	Povratana vrednost: Da li je registrovan kao vodja(vraca 2) ako je registrovan kao igrac vraca(1)
*/
int registracija(client_veza* client);
/*
	Funkcija: Prijem_igrac
	---------------------------
	Funkcionalnost:Primi pogadjanje od igraca i smesta u red poruka.Ako je detektovano da su primljene sve poruke da obavesti thread za slanje vodji da krene sa slanjem 

	Parametri:
	client -> klijentska struktura koja predstavlja odgovarajuceg klijenta
	kraj -> oznacava da li je doslo do prekida konekcije

	Povratana vrednost: Nema povratnu vrednost.
*/
void Prijem_igrac(client_veza client, int kraj);

/*
	Funkcija: Prijem_vodja
	---------------------------
	Funkcionalnost:Primi opseg od vodje i pozove PocetakIgre,ugasi thread za prihvat konekcije,prima odgovore od vodje i smesta ih u red odgovora nakon prijema svih 
	poruka obavestava thread za slanje igracima.

	Parametri:
	client -> klijentska struktura koja predstavlja odgovarajuceg klijenta
	kraj -> oznacava da li je doslo do prekida konekcije

	Povratana vrednost: Nema povratnu vrednost.
*/
void Prijem_vodja(client_veza client, int kraj);
/*
	Funkcija: prijem_poruke
	---------------------------
	Funkcionalnost: Poziva registraciju igraca i na osnovu povratne vrednosti poziva prijem_igraca ili prijem_vodje
	Parametri:
	client -> klijentska struktura koja predstavlja odgovarajuceg klijenta
	

	Povratana vrednost: Nema povratnu vrednost.
*/
DWORD WINAPI prijem_poruke(LPVOID param);
/*
	Funkcija: Slanje_vodji
	---------------------------
	Funkcionalnost:Ceka da se red napuni tj da stignu sve poruke od igraca nakon cega thread zaduzen za slanje vodji krece sa prosledjivanjem poruka i brisanjem iz reda(nakon prosledjivanja)

	Parametri:
	

	Povratana vrednost: Nema povratnu vrednost.
*/
DWORD WINAPI slanje_vodji(LPVOID param);
/*
	Funkcija: slanje_igracima
	---------------------------
	Funkcionalnost:Ceka da se red napuni nakon cega thread zaduzen za slanje igracima skida poruke sa reda i prosledjuje ih odgovarajucim klijentima i nakon prosledjivanja 
	brise poruke sa reda

	Parametri:
	

	Povratana vrednost: Nema povratnu vrednost.
*/
DWORD WINAPI slanje_igracima(LPVOID param);

