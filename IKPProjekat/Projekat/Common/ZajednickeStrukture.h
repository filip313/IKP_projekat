#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>



enum odgovor { VECE = 1, MANJE, TACNO, IZGUBIO };

typedef struct poruka_igraca_st {
	int id;
	int broj;
	poruka_igraca_st* next;
}poruka_igraca;

typedef struct odgovor_vodje_st {
	int id;
	odgovor odg;
	odgovor_vodje_st* next;
}odgovor_vodje;

typedef struct opseg_st {
	int od;
	int doo;
}opseg;

