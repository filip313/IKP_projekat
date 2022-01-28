#include "Funkcije.h"



int  main(void)
{
	DWORD prihvat_konekcije_tr_id;
	HANDLE slanje_vodji_tr;
	DWORD slanje_vodji_tr_id;
	HANDLE slanje_igracima_tr;
	DWORD slanje_igracima_tr_id;

	slanje_vodji_sem = CreateSemaphore(0, 0, 1, NULL);
	slanje_igracima_sem = CreateSemaphore(0, 0, 1, NULL);


	SOCKET listenSocket = INVALID_SOCKET;

	int iResult;

	if (InitializeWindowsSockets() == false)
	{
		return 1;
	}

	addrinfo* resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 


	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	InitializeCriticalSection(&cs);

	slanje_vodji_tr = CreateThread(NULL, 0, &slanje_vodji, NULL, 0, &slanje_vodji_tr_id);
	slanje_igracima_tr = CreateThread(NULL, 0, &slanje_igracima, NULL, 0, &slanje_igracima_tr_id);
	char resenje = NULL;
	do
	{
		resenje = NULL;

		listenSocket = socket(AF_INET,      // IPv4 address famly
			SOCK_STREAM,  // stream socket
			IPPROTO_TCP); // TCP

		if (listenSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(resultingAddress);
			WSACleanup();
			return 1;
		}

		iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(resultingAddress);
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		// Set listenSocket in listening mode
		iResult = listen(listenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		printf("Server initialized, waiting for clients.\n");

		prihvat_konekcije_tr = CreateThread(NULL, 0, &prihvat_konekcije, &listenSocket, 0, &prihvat_konekcije_tr_id);
		WaitForSingleObject(prihvat_konekcije_tr, INFINITE);
		closesocket(listenSocket);
		while (broj_igraca != 0);
		printf("Da li zelite da nastavite sa novom igrom?y\\n\n");
		resenje=_getch();
		/*WSACleanup();
		if (InitializeWindowsSockets() == false)
		{
			return 1;
		}*/
	} while (resenje != 'n' && resenje !='N');

	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	DeleteCriticalSection(&cs);

	// cleanup
	closesocket(listenSocket);
	WSACleanup();
	TerminateThread(prihvat_konekcije_tr, 0);
	TerminateThread(slanje_igracima_tr, 0);
	TerminateThread(slanje_vodji_tr, 0);
	_getch();


	return 0;
}

