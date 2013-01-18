#include "server_operations.h"


MSG_LOGIN log_in;
MSG_RESPONSE response;

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
    do{
        if(msgrcv(que_id, &log_in, sizeof(MSG_LOGIN) - sizeof(long), LOGIN, IPC_NOWAIT) != -1){
            printf("logged in:\nclient queue: %d\n client nick: %s\n", log_in.ipc_num, log_in.username);
    
            response.response_type = LOGIN_SUCCESS;
            response.type = RESPONSE;
            strcpy(response.content, "login success");

            msgsnd(log_in.ipc_num, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
        }
        if(msgrcv(que_id, &log_in, sizeof(MSG_LOGIN) - sizeof(long), LOGOUT, IPC_NOWAIT) != -1){
            printf("logged out client nick: %s\n", log_in.username);
    
            response.response_type = LOGOUT_SUCCESS;
            response.type = RESPONSE;
            strcpy(response.content, "logout success");

            msgsnd(log_in.ipc_num, &response, sizeof(MSG_RESPONSE) - sizeof(long), IPC_NOWAIT);
        }
    }while(1);
    
    
    
   
}