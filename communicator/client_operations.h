/* 
 * File:   client_operations.h
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 20:31
 */

#ifndef CLIENT_OPERATIONS_H
#define	CLIENT_OPERATIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "protocol.h"
    
//parsuje komendy i wywoluje odpowiednie funkcje obslugi
//zwraca komunikat wyswietlany w oknie komend
//parametry: komenda, lista kontaktow
char* parser(char* command, char** ppl_cnt);

//tworzy kolejke klienta, pryzjmuje nazwe uzytkownika, loguje do wskazanego serwera
//zwraca 1 w przypadku powodzenia, 0 w przpadku niepowodzenia
int init();

//wylogowuje z serwera, usuwa kolejke komunikatow klienta
//zwraca komunikat wyswietlany w oknie komend
char* logout();

//wysyla wiadomosc
//zwraca komunikat wyswietlany w oknie komend
//parametry: odbiorca wiadomosci, wiadowosc, lista kontaktow (ludzi), lista kontatów (pokoje)
//funkcja okresla typ wiadomosci na podstawie listy uzytkownikow
char* msg_snd(char* receiver, char* msg, char** ppl_cnt);

//wysyła żądanie wejścia lub zmiany kanału, parametr: nazwa (nowego)kanału
char* enter_channel(char* channel_name);

//zwraca nazwe uzytkownika
char* get_nick();

//zwraca nazwe aktualnego kanalu
char* get_channel();

//zwraca id kolejki klienta
int get_my_que_id();

int get_serv_que_id();

#ifdef	__cplusplus
}
#endif

#endif	/* CLIENT_OPERATIONS_H */

