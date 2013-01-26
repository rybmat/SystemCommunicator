#include "server_operations.h"


MSG_LOGIN log_in;
MSG_RESPONSE response;
MSG_CHAT_MESSAGE chmsg;
MSG_ROOM room;
MSG_REQUEST req;
MSG_USERS_LIST ulist;

int que_id = 0;

int init(){
    que_id = msgget(IPC_PRIVATE, 0666);
    if(que_id <= 0){
        printf("blad\n");
        return 0;
    }
    
    log_in.ipc_num = -1;
    
    signal(SIGALRM, heartbeat);
    
    printf("server id: %d\n",que_id);
    return 1;
}

void register_client(){
    
    alarm(3);
    do{ //rzadanie zalogowania
        if(msgrcv(que_id, &log_in, sizeof(MSG_LOGIN) - sizeof(long), LOGIN, IPC_NOWAIT) != -1){
            printf("logged in:\nclient queue: %d\n client nick: %s\n", log_in.ipc_num, log_in.username);
    
            response.response_type = LOGIN_SUCCESS;
            response.type = RESPONSE;
            strcpy(response.content, "login success");

            msgsnd(log_in.ipc_num, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
        }
        //wylogowanie
        if(msgrcv(que_id, &log_in, sizeof(MSG_LOGIN) - sizeof(long), LOGOUT, IPC_NOWAIT) != -1){
            printf("logged out client nick: %s\n", log_in.username);
    
            response.response_type = LOGOUT_SUCCESS;
            response.type = RESPONSE;
            strcpy(response.content, "logout success");

            msgsnd(log_in.ipc_num, &response, sizeof(MSG_RESPONSE) - sizeof(long), IPC_NOWAIT);
        }
        //wiadomosc
        if(msgrcv(que_id, &chmsg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), MESSAGE, IPC_NOWAIT) != -1){
            response.type = RESPONSE;
            response.response_type = MSG_SEND;
            strcpy(response.content, "Message Send");
            printf("message:\n Nadawca: %s\n czas: %s\n tresc: %s\n", chmsg.sender, chmsg.send_time, chmsg.message);
            msgsnd(log_in.ipc_num, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
        }
        //operacje z pokojem
        if(msgrcv(que_id, &room, sizeof(MSG_ROOM) - sizeof(long), ROOM, IPC_NOWAIT) != -1){
            if(room.operation_type == ENTER_ROOM){
                printf("ENTER_ROOM: %s -> %s\n", room.user_name, room.room_name);
                response.type = RESPONSE;
                response.response_type = ENTERED_ROOM_SUCCESS;
                strcpy(response.content, "Entered room");
                msgsnd(log_in.ipc_num, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
            }else if(room.operation_type == CHANGE_ROOM){
                printf("CHANGE_ROOM: %s -> %s\n", room.user_name, room.room_name);
                response.type = RESPONSE;
                response.response_type = CHANGE_ROOM_SUCCESS;
                strcpy(response.content, "Changed room");
                msgsnd(log_in.ipc_num, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
            }
        }
        //request o liste userow
        if(msgrcv(que_id, &req, sizeof(MSG_REQUEST) - sizeof(long), REQUEST, IPC_NOWAIT) != -1){
            if(req.request_type == USERS_LIST){
                printf("users list request\n");
                int i,k;
                ulist.type = USERS_LIST_STR;
                for(i = 0; i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER; ++i){
                    for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
                        ulist.users[i][k] = '\0';
                    }
                    ulist.users[i][0] = 'r'+i%10;
                    ulist.users[i][1] = 'y'+i%10;
                    ulist.users[i][2] = 'b'+i%10;
                    ulist.users[i][3] = 'a'+i%10;
                }
                msgsnd(log_in.ipc_num, &ulist, sizeof(MSG_USERS_LIST) - sizeof(long), 0);
                printf("wyslane %d\n", ulist.type);
            }else if(req.request_type == ROOMS_LIST){
                printf("rooms list request\n");
                int i,k;
                ulist.type = ROOMS_LIST_STR;
                for(i = 0; i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER; ++i){
                    for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
                        ulist.users[i][k] = '\0';
                    }
                    ulist.users[i][0] = 'r'+i%10;
                    ulist.users[i][1] = '0'+i%10;
                    ulist.users[i][2] = '0'+i%10;
                    ulist.users[i][3] = 'm'+i%10;
                }
                msgsnd(log_in.ipc_num, &ulist, sizeof(MSG_USERS_LIST) - sizeof(long), 0);
                printf("wyslane %d\n", ulist.type);
            }else if(req.request_type == PONG){
                printf("PONG\n");
            }
            
        }
                
                
    }while(1);    
   
}

void heartbeat(){
    MSG_RESPONSE rs;
    rs.response_type = PING;
    rs.type = RESPONSE;
    
    
    if(log_in.ipc_num != -1){
        if(msgsnd(log_in.ipc_num, &rs, sizeof(MSG_RESPONSE) - sizeof(long), 0) != -1){
                printf("PING\n");
        }
    }
    
    alarm(3);
}