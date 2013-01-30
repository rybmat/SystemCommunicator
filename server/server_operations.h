/* 
 * File:   server_operations.h
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 20:28
 */

#ifndef SERVER_OPERATIONS_H
#define	SERVER_OPERATIONS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "protocol.h"    
    
    //unia używana w funkcji semctl
    typedef union{
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } SEM_UNION;
    
    //struktura reprezentująca jednego użytkownika zalogowanego na serwerze
    typedef struct{
        char username[USER_NAME_MAX_LENGTH];    //nazwa uzytkownika
        int que_id;                             //identyfikator kolejki klienta
        char room[ROOM_NAME_MAX_LENGTH];        //nazwa pokoju do ktorego nalezy uzytkownik
        int heartbeat;                          //licznik uzywany przy mechanizmie heartbeat
    } LOGGED_USER;
    
    //pobiera id semaforów, pamieci wspoldzielonych, dolacza je, tworzy kolejke komunikatow
    //wpisuje sie do rejestru serwerow, inicjuje lokalna tablice userow
    int init();
    
    //zamyka serwer - usuwa wpis z rejestru serwerow, wylogowuje wszystkich userow
    //odlacza(ew usuwa) pamieci wspoldzielone
    void close_server();
    
    //dodaje, podany jako parametr, wpis do pliku logu (data wpisu jest generowana przez funkcje)
    void add_log(char* log);
    
    //glowna petla, wywoluje funkcje obslugujace dany typ komunikatu
    void server_main();
    
    //rejestruje uzytkownika (w pamieci wspoldzielonej jak i w pamieci lokalnej)
    void register_user(char* username, int user_queue);
    
    //wylogowuje usera - usuwa wpisy z pamieci lokalnej, pamieci wspoldzielonych user_server i w razie potrzeby room_server
    void unregister_user(char* username);
    
    //wysyla PING-a do klientow - nadpisana obsluga SIGALRM
    void heartbeat();
    
    //wpisuje nazwe pokoju obok nazwy odpowiedniego usera w pamieci lokalnej
    //jezeli jest to pierwszy user tego pokoju na tym serwerze to dokonuje wpisu do pamieci wspoldzielonej
    void enter_to_room(char *username, char *room);

    //opuszczenie pokoju - jezeli jest to ostatni user tego pokoju na tym serwerze
    //to zostaje usuniety wpis z pamieci room_server
    void leave_the_room(char *username);

    //pobiera z repozytorium i wysyła do usera liste wszystkich uzytkownikow
    void send_users_list(char* username);
    
    //pobiera z repozytorium i wysyła do usera liste wszystkich kanalow
    void send_channels_list(char* username);
    
    //przetwarza i odpowiednio wysyła wiadomosci prywatne i publiczne
    void send_message(MSG_CHAT_MESSAGE msg);
    
    //zwraca id kolejki na ktorej nasluchuje klient o podanym nicku
    int get_client_queue_id(char* username);
    

#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_OPERATIONS_H */

