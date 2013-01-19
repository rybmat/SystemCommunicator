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
        return 0;
    }
    
    printf("server id: %d\n",que_id);
    return 1;
}

void register_client(){
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
            printf("message:\n Nadawca: %s\n czas: %s\n tresc: %s\n", chmsg.sender, chmsg.send_time, chmsg.message);
        }
        //operacje z pokojem
        if(msgrcv(que_id, &room, sizeof(MSG_ROOM) - sizeof(long), ROOM, IPC_NOWAIT) != -1){
            if(room.operation_type == ENTER_ROOM){
                printf("ENTER_ROOM: %s -> %s\n", room.user_name, room.room_name);
            }else if(room.operation_type == CHANGE_ROOM){
                printf("CHANGE_ROOM: %s -> %s\n", room.user_name, room.room_name);
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
            }
            if(req.request_type == ROOMS_LIST){
                printf("rooms list request\n");
                int i,k;
                ulist.type = ROOMS_LIST_STR;
                for(i = 0; i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER; ++i){
                    for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
                        ulist.users[i][k] = '\0';
                    }
                    ulist.users[i][0] = 'r'%10;
                    ulist.users[i][1] = '0'%10;
                    ulist.users[i][2] = '0'%10;
                    ulist.users[i][3] = 'm'%10;
                }
            }
            msgsnd(log_in.ipc_num, &ulist, sizeof(MSG_USERS_LIST) - sizeof(long), 0);
            printf("wyslane\n");
        }
                
                
    }while(1);
    
    
    
   
}