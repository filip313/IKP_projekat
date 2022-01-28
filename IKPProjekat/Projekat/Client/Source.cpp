#include "ZajednickeStrukture.h"
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 27016

// Initializes WinSock2 library
// Returns true if succeeded, false otherwise.
bool InitializeWindowsSockets();
void GasenjeClienta(SOCKET socket);
bool gasenje = false;

int __cdecl main(int argc, char** argv)
{
	// socket used to communicate with server
	SOCKET connectSocket = INVALID_SOCKET;
	// variable used to store function return value
	int iResult;
	// message to send
	const char* messageToSend = "this is a test";

	// Validate the parameters
	if (argc != 2)
	{
		printf("usage: %s server-name\n", argv[0]);
		return 1;
	}

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// create and initialize address structure
	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
		return 0;
	}

	int izbor = 0;
	do {
		printf("Odaberite ulogu:\n\t1 -> vodja\n\t2 -> igrac\n");
		printf("-> ");
		scanf_s("%d", &izbor);
	} while ((izbor != 1 && izbor != 2));

	const char* uloga = (izbor == 1) ? "vodja" : "igrac";

	// Send an prepared message with null terminator included
	iResult = send(connectSocket, uloga, (int)strlen(uloga) + 1, 0);

	if (iResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}



	char buffer[512];
	opseg op;
	int pogadjanje;
	odgovor odg;
	poruka_igraca* poruke_igraca_lista = NULL;
	odgovor_vodje* odgovor_vodje_lista = NULL;

	iResult = recv(connectSocket, buffer, 512, 0);


	// logika za igraca
	if (strcmp(buffer, "igrac") == 0) {
		if (strcmp(buffer, uloga) == 0) {
			printf("Uspesno registrovani kao igrac!\n");
		}
		else {
			printf("Uloga vodje je zauzeta. Registrovani kao igrac!\n");
		}

		iResult = recv(connectSocket, (char*)&op, sizeof(op), 0);
		if (iResult > 0) {
			printf("Igra je pocela.Birajte broj u opsegu %d -- %d:\n", op.od, op.doo);
			printf("Odaberite nacin igre:\n");
			printf("1.Upotreba algoritma\n");
			printf("2.Rucno unosenje\n");
			int nacin = 0;
			int sredina = 0;
			int buf = -1;
			scanf_s("%d", &nacin);
			while (1)
			{

				if (nacin == 1)
				{
					if (buf == -1) {
						sredina = (op.od + op.doo) / 2;

					}
					else if (buf == VECE)
					{
						op.doo = sredina - 1;
						sredina = (op.od + op.doo) / 2;
					}
					else if (buf == MANJE) {
						op.od = sredina + 1;
						sredina = (op.od + op.doo) / 2;
					}
					pogadjanje = sredina;
					printf("Algoritam salje broj %d\n", pogadjanje);
				}
				else {

					printf("Unesite broj: ");
					scanf_s("%d", &pogadjanje);
				}

				iResult = send(connectSocket, (const char*)&pogadjanje, sizeof(int), 0);

				if (iResult == SOCKET_ERROR) {
					printf("Doslo je do greske prilikom slanja pogadjanja na server. Error %d\n", WSAGetLastError());
					GasenjeClienta(connectSocket);
				}

				iResult = recv(connectSocket, (char*)&buf, sizeof(int), 0);
				if (iResult > 0) {
					odg = (odgovor)buf;
					switch (odg) {

					case VECE:
						printf("Zamisljeni broj je veci od pogadjanog!\n");
						break;
					case MANJE:
						printf("Zamisljeni broj je manji od pogadjanog!\n");
						break;
					case TACNO:
						printf("Zamisljeni broj i broj koji ste poslali se podudaraju!\n");
						GasenjeClienta(connectSocket);
						break;
					case IZGUBIO:
						printf("Igra je zavrsena,izgubili ste!\n");
						GasenjeClienta(connectSocket);
						break;

					}
				}
				else if (iResult == 0) {
					printf("Konekcija sa serverom zatvorena\n");
					GasenjeClienta(connectSocket);
				}
				else {
					printf("Doslo je do greske prilikom prijema odgovora sa servera. Error %d.", WSAGetLastError());
					GasenjeClienta(connectSocket);
				}
			}

		}
		else if (iResult == 0) {
			printf("Konekcija sa serverom zatvorena\n");
			GasenjeClienta(connectSocket);
		}
		else {
			printf("Doslo je do greske prilikom prijema opsega sa servera. Error %d\n", WSAGetLastError());
			GasenjeClienta(connectSocket);
		}
	}
	//logika za vodju
	else {

		int zamisljen_broj = -1;
		printf("Uspesno registrovani kao vodja!\n");
		printf("Unesite zamisljen broj: ");
		scanf_s("%d", &zamisljen_broj);
		printf("Unesite opseg brojeva!\n");
		do {

			printf("Od: ");
			scanf_s("%d", &op.od);
		} while (zamisljen_broj < op.od);
		do {

			printf("Do: ");
			scanf_s("%d", &op.doo);
		} while (zamisljen_broj > op.doo);

		iResult = send(connectSocket, (const char*)&op, sizeof(op), 0);
		if (iResult == SOCKET_ERROR) {
			printf("Doslo je do greske prilikom slanja opsega na server. Error %d", WSAGetLastError());
			GasenjeClienta(connectSocket);
		}

		while (1) {

			poruka_igraca* tmp;
			do {
				tmp = (poruka_igraca*)malloc(sizeof(poruka_igraca));
				iResult = recv(connectSocket, (char*)tmp, sizeof(poruka_igraca), 0);
				if (iResult > 0) {

					tmp->next = NULL;
					if (poruke_igraca_lista == NULL) {
						poruke_igraca_lista = tmp;
					}
					else {
						tmp->next = poruke_igraca_lista;
						poruke_igraca_lista = tmp;
					}

				}
				else if (iResult == 0) {
					printf("Konekcija sa serverom prekinuta.\n");
					GasenjeClienta(connectSocket);
				}
				else {
					printf("Doslo je do greske prilikom prijema pogadjanja sa servera. Error %d\n", WSAGetLastError());
					GasenjeClienta(connectSocket);
				}
			} while (tmp->id != -1);

			poruka_igraca* head = poruke_igraca_lista;

			while (head != NULL) {
				printf("Poruka od igraca %d je %d\n", head->id, head->broj);
				head = head->next;
			}

			head = poruke_igraca_lista;

			odgovor_vodje* odg = NULL;
			while (head != NULL) {
				if (head->id != -1) {

					odgovor_vodje* tmp = (odgovor_vodje*)malloc(sizeof(odgovor_vodje));
					tmp->id = head->id;
					tmp->next = NULL;
					if (head->broj == zamisljen_broj) {

						tmp->odg = TACNO;
						printf("Od igraca id = %d primljen broj %d i on je TACAN\n", head->id, head->broj);
						gasenje = true;

					}
					else if (head->broj > zamisljen_broj) {

						tmp->odg = VECE;
						printf("Od igraca id = %d primljen broj %d i on je VECI\n", head->id, head->broj);
					}
					else {

						tmp->odg = MANJE;
						printf("Od igraca id = %d primljen broj %d i on je MANJI\n", head->id, head->broj);
					}

					if (odg == NULL) {

						odg = tmp;
					}
					else {

						tmp->next = odg;
						odg = tmp;
					}
				}

				head = head->next;
				free(poruke_igraca_lista);
				poruke_igraca_lista = head;
			}

			odgovor_vodje* prvi = odg;
			while (prvi != NULL) {

				iResult = send(connectSocket, (const char*)prvi, sizeof(odgovor_vodje), 0);
				if (iResult == SOCKET_ERROR) {

					printf("Doslo je do greske prilikom slanja odgovora serveru. Error %d", WSAGetLastError());
					GasenjeClienta(connectSocket);
				}
				prvi = prvi->next;
				free(odg);
				odg = prvi;
			}
			odgovor_vodje kraj;
			kraj.id = -1;

			iResult = send(connectSocket, (const char*)&kraj, sizeof(odgovor_vodje), 0);
			if (iResult == SOCKET_ERROR) {

				printf("Doslo je do greske prilikom slanja odgovora serveru. Error %d", WSAGetLastError());
				GasenjeClienta(connectSocket);
			}

			if (gasenje == true) {
				GasenjeClienta(connectSocket);
			}
		}
	}



	return 0;
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

void GasenjeClienta(SOCKET socket) {


	closesocket(socket);
	WSACleanup();
	printf("Pritisni dugme za gasenje programa !\n");
	_getch();
	exit(0);
}