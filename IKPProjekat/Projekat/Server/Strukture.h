#pragma once
#include "ZajednickeStrukture.h"
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <conio.h>


#define DEFAULT_PORT "27016"


typedef struct client_veza_st {
	int id;
	SOCKET socket;
	HANDLE thread;
	client_veza_st* next;
}client_veza;

extern CRITICAL_SECTION cs;
extern client_veza* povezani_igraci;
extern client_veza* vodja_igre;
extern opseg op;

extern poruka_igraca* red_poruka_igraca_in;
extern poruka_igraca* red_poruka_igraca_out;
extern odgovor_vodje* red_poruka_vodje_in;
extern odgovor_vodje* red_poruka_vodje_out;

extern bool pocela_igra;
extern int id_cnt;
extern int broj_igraca;
extern int broj_poslatih_poruka_igraca;
extern HANDLE slanje_vodji_sem;
extern HANDLE slanje_igracima_sem;
extern HANDLE prihvat_konekcije_tr;

extern int pobednik_id;



