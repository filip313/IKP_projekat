#include "Funkcije.h"

CRITICAL_SECTION cs;
client_veza* povezani_igraci = NULL;
client_veza* vodja_igre = NULL;
opseg op;

poruka_igraca* red_poruka_igraca_in = NULL;
poruka_igraca* red_poruka_igraca_out = NULL;
odgovor_vodje* red_poruka_vodje_in = NULL;
odgovor_vodje* red_poruka_vodje_out = NULL;

bool pocela_igra = false;
int id_cnt = 0;
int broj_igraca = 0;
int broj_poslatih_poruka_igraca = 0;
HANDLE slanje_vodji_sem;
HANDLE slanje_igracima_sem;
HANDLE prihvat_konekcije_tr;

int pobednik_id = -1;

void oslobodi_klijenta(client_veza* client) {

	if (povezani_igraci != NULL) {

		client_veza* prethodni = povezani_igraci;
		client_veza* head = povezani_igraci;
		while (head != NULL && head->id != client->id) {
			prethodni = head;
			head = head->next;
		}

		prethodni->next = head->next;
		if (head == povezani_igraci) {
			povezani_igraci = head->next;
			printf("OSLOBADJAM KLIJENTA %d\n", head->id);
			free(head);
		}
		else {
			printf("OSLOBADJAM KLIJENTA %d\n", head->id);
			free(head);
		}
		broj_igraca--;
	}
}

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

DWORD WINAPI prihvat_konekcije(LPVOID param) {

	SOCKET listenSocket = *(SOCKET*)param;
	



	while (1) {

		SOCKET socket = accept(listenSocket, NULL, NULL);

		if (socket == INVALID_SOCKET)
		{
			printf("accept failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}
		client_veza* tmp = (client_veza*)malloc(sizeof(client_veza));
		tmp->socket = socket;
		tmp->id = id_cnt;
		id_cnt++;
		tmp->next = NULL;
		tmp->thread = CreateThread(NULL, 0, &prijem_poruke, tmp, 0, NULL);


	}
}

int registracija(client_veza* client) {

	char recvbuf[512];

	int iResult = recv(client->socket, recvbuf, 512, 0);
	if (iResult > 0)
	{
		printf("Message received from client %d : %s.\n", client->id, recvbuf);

		if (strcmp(recvbuf, "igrac") == 0) {
			const char* poruka = "igrac";

			send(client->socket, poruka, (int)strlen(poruka) + 1, 0);
			EnterCriticalSection(&cs);
			broj_igraca++;
			if (povezani_igraci == NULL) {
				povezani_igraci = client;
			}
			else {
				client_veza* head = povezani_igraci;
				while (head->next != NULL) {
					head = head->next;
				}
				head->next = client;
			}
			LeaveCriticalSection(&cs);

			return 1;
		}
		else {
			EnterCriticalSection(&cs);
			if (vodja_igre == NULL) {
				const char* poruka = "vodja";
				send(client->socket, poruka, (int)strlen(poruka) + 1, 0);
				vodja_igre = client;
				LeaveCriticalSection(&cs);
				return 2;
			}

			else {
				const char* poruka = "igrac";
				send(client->socket, poruka, (int)strlen(poruka) + 1, 0);
				broj_igraca++;

				if (povezani_igraci == NULL) {
					povezani_igraci = client;
				}
				else {
					client_veza* head = povezani_igraci;
					while (head->next != NULL) {
						head = head->next;
					}
					head->next = client;
				}
				LeaveCriticalSection(&cs);

				return 1;
			}

		}
	}
	else if (iResult == 0)
	{
		// connection was closed gracefully
		printf("Connection with client %d closed.\n", client->id);
		closesocket(client->socket);
		free(client);
	}
	else
	{
		// there was an error during recv
		printf("recv failed with error: %d\n", WSAGetLastError());
		closesocket(client->socket);
		free(client);
	}

	return 0;
}

void Prijem_igrac(client_veza client,int kraj) {
	int iResult;
	while (kraj) {

		poruka_igraca* tmp = (poruka_igraca*)malloc(sizeof(poruka_igraca));
		int broj = 0;
		iResult = recv(client.socket, (char*)&broj, sizeof(int), 0);

		if (iResult > 0) {

			EnterCriticalSection(&cs);
			tmp->broj = broj;
			tmp->id = client.id;
			tmp->next = NULL;
			broj_poslatih_poruka_igraca++;
			if (red_poruka_igraca_in == NULL)
			{
				red_poruka_igraca_in = tmp;
				red_poruka_igraca_out = tmp;

			}
			else
			{
				red_poruka_igraca_in->next = tmp;
				red_poruka_igraca_in = tmp;
			}
			LeaveCriticalSection(&cs);

			if (broj_igraca == broj_poslatih_poruka_igraca)
			{
				broj_poslatih_poruka_igraca = 0;
				ReleaseSemaphore(slanje_vodji_sem, 1, NULL);

			}


		}
		else if (iResult == 0)
		{
			// connection was closed gracefully
			printf("Connection with client %d closed.\n", client.id);
			closesocket(client.socket);
			free(tmp);
			oslobodi_klijenta(&client);
			kraj = 0;
		}
		else
		{
			// there was an error during recv
			printf("recv igrac failed with error: %d\n", WSAGetLastError());
			closesocket(client.socket);
			free(tmp);
			oslobodi_klijenta(&client);
			kraj = 0;
		}
	}
}

void Prijem_vodja(client_veza client, int kraj) {
	int iResult;
	int rcvCnt = 0;
	while (kraj) {

		if (!rcvCnt) {

			iResult = recv(client.socket, (char*)&op, sizeof(op), 0);
			if (iResult > 0) {

				printf("%d - %d\n", op.od, op.doo);
				PocetakIgre(op);

				TerminateThread(prihvat_konekcije_tr, 0);
				rcvCnt++;
			}
			else if (iResult == 0) {
				printf("Vodja prekinuo konekciju\n");
				closesocket(vodja_igre->socket);
				free(vodja_igre);
				vodja_igre = NULL;
				kraj = 0;
			}
			else {
				printf("Doslo je do greske prilikom prijema na vodji. Error %d\n", WSAGetLastError());
				closesocket(vodja_igre->socket);
				free(vodja_igre);
				vodja_igre = NULL;
				kraj = 0;
			}
		}
		else {
			odgovor_vodje* odg;
			do {

				odg = (odgovor_vodje*)malloc(sizeof(odgovor_vodje));
				iResult = recv(client.socket, (char*)odg, sizeof(odgovor_vodje), 0);

				if (iResult > 0) {
					if (odg->odg == TACNO) {

						if (pobednik_id == -1) {
							pobednik_id = odg->id;

						}
					}

					odg->next = NULL;
					if (odg->id != -1) {
						EnterCriticalSection(&cs);

						if (red_poruka_vodje_in == NULL) {

							red_poruka_vodje_in = odg;
							red_poruka_vodje_out = odg;
						}
						else {

							red_poruka_vodje_in->next = odg;
							red_poruka_vodje_in = odg;
						}
						LeaveCriticalSection(&cs);
					}
					else {
						free(odg);
						odg = NULL;
					}
				}
				else if (iResult == 0) {
					printf("Vodja prekinuo konekciju\n");
					closesocket(vodja_igre->socket);
					free(vodja_igre);
					free(odg);
					vodja_igre = NULL;
					kraj = 0;
					pobednik_id = -1;
				}
				else {
					printf("Doslo je do greske prilikom prijema na vodji. Error %d\n", WSAGetLastError());
					closesocket(vodja_igre->socket);
					free(vodja_igre);
					free(odg);
					vodja_igre = NULL;
					kraj = 0;
				}
			} while (odg != NULL && kraj);

			if (kraj) {

				ReleaseSemaphore(slanje_igracima_sem, 1, NULL);
			}
		}
	}
}

DWORD WINAPI prijem_poruke(LPVOID param) {

	client_veza client = *(client_veza*)param;

	int uloga = registracija((client_veza*)param);


	int kraj = 1;
	if (uloga == 1) {
		Prijem_igrac(client, kraj);

	}
	else if (uloga == 2) {
		//logika za vodju
		Prijem_vodja(client, kraj);
	}
	else {
		printf("Nesto ne valja\n");
	}

	return 0;
}

void PocetakIgre(opseg op) {
	client_veza* head = povezani_igraci;

	EnterCriticalSection(&cs);
	while (head != NULL)
	{
		int iResult = send(head->socket, (const char*)&op, sizeof(op), 0);

		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(head->socket);
			oslobodi_klijenta(head);
		}

		head = head->next;
	}
	LeaveCriticalSection(&cs);
}

DWORD WINAPI slanje_vodji(LPVOID param)
{
	while (1) {
		WaitForSingleObject(slanje_vodji_sem, INFINITE);
		EnterCriticalSection(&cs);
		do {
			poruka_igraca* poruka = red_poruka_igraca_out;
			red_poruka_igraca_out = poruka->next;
			int iResult = send(vodja_igre->socket, (const char*)poruka, sizeof(poruka_igraca), 0);
			if (iResult == SOCKET_ERROR)
			{
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(vodja_igre->socket);
				free(vodja_igre);
				vodja_igre = NULL;
			}

			free(poruka);

		} while (red_poruka_igraca_out != NULL);
		red_poruka_igraca_in = NULL;
		poruka_igraca poruka;
		poruka.id = -1;
		int iResult = send(vodja_igre->socket, (const char*)&poruka, sizeof(poruka_igraca), 0);
		if (iResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(vodja_igre->socket);
			free(vodja_igre);
			vodja_igre = NULL;
		}

		LeaveCriticalSection(&cs);
	}
}

DWORD WINAPI slanje_igracima(LPVOID param) {

	while (1) {

		WaitForSingleObject(slanje_igracima_sem, INFINITE);
		

		do {
			EnterCriticalSection(&cs);
			odgovor_vodje* odg = red_poruka_vodje_out;
			if (odg->id != -1) {

				client_veza* head = povezani_igraci;
				while (head->id != odg->id) {
					head = head->next;
				}
				int buf = (int)odg->odg;
				if (pobednik_id != -1) {
					if (pobednik_id != odg->id) {
						buf = IZGUBIO;
					}
				}
				int res = send(head->socket, (const char*)&buf, sizeof(int), 0);
				if (res == SOCKET_ERROR) {
					printf("GRESKA : %d\n", WSAGetLastError());
					closesocket(head->socket);
					oslobodi_klijenta(head);
				}
			}

			red_poruka_vodje_out = red_poruka_vodje_out->next;
			free(odg);

			LeaveCriticalSection(&cs);
		} while (red_poruka_vodje_out != NULL);


		red_poruka_vodje_in = NULL;

	}

}