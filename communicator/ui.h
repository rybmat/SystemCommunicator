/**
 * File:   ui.h
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 11:23
 */

#ifndef UI_H
#define	UI_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>
#include "protocol.h"
#include "client_operations.h"

//wymiary poszczegolnych "okienek" recived - okno wiadomosci
#define RECIVED_WIDTH 90
#define RECIVED_HEIGHT 25
#define CONTACTS_WIDTH 30
#define CONTACTS_HEIGHT 35
#define COMMAND_WIDTH  90
#define COMMAND_HEIGHT 10

//stale identyfikujace poszczegolne okienka
#define RECIVED 0       //wiadomosci
#define CONTACTS 1      //kontakty
#define COMMANDS 2      //komendy

//stale definiujace typy kontaktow i wiadomosci
#define CONTACTS_PEOPLE 0
#define CONTACTS_ROOMS 1
#define MESSAGES_ALL 0
#define MESSAGES_PRIV 1
#define MESSAGES_ROOM 2

//ilosc wiadomosci pamietanych w programie    
#define MSG_BUF_SIZE 200
    
//odstep czasowy (sec) pomiedzy odswiezaniem kontaktow
#define GETTING_CONTACTS_TIME_INTERVAL 2    

//tablice ostatnich 100 wiadomosci (prywatnych, publicznych, wszystkich)
char *priv_messages[MSG_BUF_SIZE];
char *room_messages[MSG_BUF_SIZE];
char *all_messages[MSG_BUF_SIZE];

//tablice kontaktow (ludzi i pokojow)
char *people_contacts[MAX_SERVERS_NUMBER * MAX_USERS_NUMBER];
char *rooms_contacts[MAX_SERVERS_NUMBER * MAX_USERS_NUMBER];

//wskazniki na aktualnie wyswietlane wiadomosci i kontakty
char **active_msg;
char **active_cnt;

//tresc komendy
char command[300];

//wskazniki na poszczegolne okna - wiadomosci, kontaktow, okno polecen i aktualnie uzywane okno
WINDOW *recived_win;
WINDOW *contacts_win;
WINDOW *command_win;
WINDOW *active_win;

//glowna metoda UI - odpowiada za wyswietlanie okienek, poruszanie sie pomiedzy nimi itp
void ui_main();

//inicjuje tablice stringów wartściami NULL
void init_strings();

//"drukuje" zawartosc w oknie: win-okno do zadrukowania, title-wyswietlany tytul okna, content-zawartosc do wydruku,
//begin-indeks pierwszego do wydruku elementu z tablicy, nlines-ilosc lini ktore mozna zadrukowac
void print_content(WINDOW *win, char* title, char** content, int begin, int nlines);

//rysowanie okna komend: win-wskaznik na okienko, title-tytul okienka
void command_window_drawing(WINDOW *win, char* title);

//pobranie tresci komendy z okna komend (zapisuje ja w zmiennej command
void get_command();

//wyswietlanie komunikatow w oknie komend: message-tresc komunikatu
void print_command_message(char* message);

//dodanie nowej wiadomosci do bufora wyswietlanych wiadomosci
void add_message(char* message, int msg_type);

//odbiera i przetwarza komunikaty odebrane z kolejki zwiazane z odbiorem wiadomości, listami userów i kanałów,
//okresowo wysyła rządanie o listy userół i kanałów
void process_ipc_msgs();

//przetwarza odpowiedzi otrzymane od serwera
void process_server_responses();

//procedura obslugi sygnalu SIGUSR1 - na ten sygnal odswiezane jest okienko wiadomosci
void refr_recived();

//procedura obslugi sygnalu SIGUSR2 - na ten sygnal odswiezane jest okienko kontaktow
void refr_contacts();

//procedura obslugi sygnalu SIGALRM - na ten sygnal wysylane jest zadanie do serwera o listy kontaktow
//sygnal ten wysylany jest co GETTING_CONTACTS_TIME_INTERVAL sekund
void get_contacts_lists();
#ifdef	__cplusplus
}
#endif

#endif	/* UI_H */

