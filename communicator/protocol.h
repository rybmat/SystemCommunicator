/* 
 * File:   protocol.h
 * Author: mateusz
 *
 * Created on 17 styczeń 2013, 12:04
 */

#ifndef PROTOCOL_H
#define	PROTOCOL_H

#ifdef	__cplusplus
extern "C" {
#endif


#define USER_NAME_MAX_LENGTH 10
#define RESPONSE_LENGTH 50
#define MAX_SERVERS_NUMBER 15
#define MAX_USERS_NUMBER 20      //na jednym serwerze
#define ROOM_NAME_MAX_LENGTH 10
#define MAX_MSG_LENGTH 256

int server_ids[15]; //pamiec wspoldzielona Obszar 1: przechowuje listę idktórych nasłuchują serwery.

//Obszar 2: przechowuje powiązania nazw użytkowników z identyfiktablicy struktur:
typedef struct {
        char user_name[USER_NAME_MAX_LENGTH];
        int server_id;
}USER_SERVER;

//Obszar 3: przechowuje powiązanie kanałów z identyfikatorami serwerów w postaci tablicy struktur:
typedef struct {
        char room_name[ROOM_NAME_MAX_LENGTH];
        int server_id;
}ROOM_SERVER;

//Kolejki komunikatów
enum MSG_TYPE {LOGIN=1, RESPONSE, LOGOUT, REQUEST, MESSAGE, ROOM, SERVER2SERVER,
USERS_LIST, ROOMS_LIST, ROOM_USERS_LIST};
typedef struct {
        long type;
        char username[USER_NAME_MAX_LENGTH];
        key_t ipc_num; //nr kolejki na której będzie nasłuchiwał klient
}MSG_LOGIN;

enum RESPONSE_TYPE {LOGIN_SUCCESS, LOGIN_FAILED, LOGOUT_SUCCESS, LOGOUT_FAILED,
        MSG_SEND, MSG_NOT_SEND, ENTERED_ROOM_SUCCESS, ENTERED_ROOM_FAILED,
        CHANGE_ROOM_SUCCESS, CHANGE_ROOM_FAILED, LEAVE_ROOM_SUCCESS,
        LEAVE_ROOM_FAILED, PING};
typedef struct {
        long type;
        int response_type;
        char content[RESPONSE_LENGTH]; //można tu wpisad jakiś komunikat wyświetlany klientowi
}MSG_RESPONSE;

enum REQUEST_TYPE{USERS_LIST, ROOMS_LIST, ROOM_USERS_LIST, PONG};
typedef struct{
        long type;
        int request_type;
        char user_name[USER_NAME_MAX_LENGTH];
}MSG_REQUEST;

typedef struct{
        long type;
        char users[MAX_SERVERS_NUMBER * MAX_USERS_NUMBER][USER_NAME_MAX_LENGTH];
}MSG_USERS_LIST;

enum CHAT_MESSAGE_TYPE {PUBLIC, PRIVATE};
typedef struct{
        long type;
        int msg_type;
        char send_time[6];      //czas wysłania hh:mm
        char sender[USER_NAME_MAX_LENGTH];      //nazwa nadawcy
        char receiver[USER_NAME_MAX_LENGTH];    //nazwa odbiorcy
        char message[MAX_MSG_LENGTH];   //treśd wiadomości
}MSG_CHAT_MESSAGE;

enum ROOM_OPERATION_TYPE {ENTER_ROOM, LEAVE_ROOM, CHANGE_ROOM};
typedef struct{
        long type;
        int operation_type;
        char user_name[USER_NAME_MAX_LENGTH];
        char room_name[ROOM_NAME_MAX_LENGTH];
}MSG_ROOM;

typedef struct{
        long type;
        int server_ipc_num;     //nr kolejki serwera, który wysyła tę strukturę
}MSG_SERVER2SERVER;


#ifdef	__cplusplus
}
#endif

#endif	/* PROTOCOL_H */

