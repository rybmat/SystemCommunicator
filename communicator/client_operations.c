#include "client_operations.h"


int my_que_id = 0;
int serv_que_id = 0;
char nick[USER_NAME_MAX_LENGTH];

MSG_LOGIN log_in;
MSG_RESPONSE response;

int init(){
    int i;
    for(i = 0; i < USER_NAME_MAX_LENGTH; ++i){
        nick[i] = '\0';
    }
  
    my_que_id = msgget(IPC_PRIVATE, 0666);
    if(my_que_id <= 0){
        return 0;
    }
    printf("my id: %d\n", my_que_id);
    
    do{
        serv_que_id = 0;
        do{
            printf("Put server id: ");
            scanf("%d",&serv_que_id);
        }while(serv_que_id <= 0);
        printf("Put your nick: ");
        scanf("%s",nick);
        nick[USER_NAME_MAX_LENGTH - 1]='\0';
        
        log_in.type = LOGIN;
        log_in.ipc_num = my_que_id;
        strcpy(log_in.username, nick);
        
        response.type = -1;
        response.response_type = -1;
        
        if(msgsnd(serv_que_id, &log_in, sizeof(MSG_LOGIN) - sizeof(long), 0) == -1){
            printf("Server is busy or you putted wrong server id, try again\n");
            continue;
        }
        
        //jakaś pauza by sie przydała
        
        if(msgrcv(my_que_id, &response, sizeof(MSG_RESPONSE) - sizeof(long), RESPONSE, 0) == -1){
            printf("Server doesn't respond, maybe you should try another one...\n");
            continue;
        }else{
            printf("%s\n", response.content);
        }
        
        if(response.response_type == LOGIN_FAILED){
            printf("Logging in failed: %s\n", response.content);
            continue;
        }
        
    }while(response.response_type != LOGIN_SUCCESS);
    printf("have a lot of fun..\n");
    
return 1;
}

char* parser(char* command){
    char uname[USER_NAME_MAX_LENGTH];
    char msg[MAX_MSG_LENGTH]; 
    
    int i;
    
    for(i = 0; i < USER_NAME_MAX_LENGTH; ++i){
        uname[i] = '\0';
    }
    for(i = 0; i < MAX_MSG_LENGTH; ++i){
        msg[i] = '\0';
    }
    
    if(strncmp(command, "msg ", 4) == 0){       //wyslanie wiadomosci
        for(i = 4; command[i]!=' ' && i < USER_NAME_MAX_LENGTH + 3; ++i){
            uname[i-4] = command[i];
        }
        uname[USER_NAME_MAX_LENGTH - 1] = '\0';
             
        if(command[i] != ' '){
            while(command[i]!=' '){
                ++i;
            }
        }
        int j;

        for(j=0; j < 255 && command[i+j+1]!='\0'; ++j){
            msg[j] = command[i+j+1];
        }
        msg[i+j] = '\0';
        //wywolanie funkcji wysylajacej wiadomosc (musi ona sprawdzac po username czy jest to msg priv czy room
        //dodać w funkcji parser argument będący wskaźnikiem na listy kontaktow
        //printf("uname:%s\nmsg:%s\n", uname, msg);
        
        return "Message send";
    }else if(strncmp(command, "logout", 6) == 0){       //wylogowanie z systemu
        logout();    
        return "logged out";
    }else if(strncmp(command, "channel ",8) == 0){      //wejscie do kanalu
        for(i = 8; command[i]!=' ' && i < USER_NAME_MAX_LENGTH + 7; ++i){
            uname[i-8] = command[i];
        }
        uname[USER_NAME_MAX_LENGTH - 1] = '\0';
        
        //wywolanie metody logujacej do pokoju
        //printf("%s\n", uname);
        
        return "Room changed";
    }
    

return "Wrong command";
}

char* logout(){
    log_in.type = LOGOUT;
    strcpy(log_in.username, nick);
    
    msgsnd(serv_que_id, &log_in, sizeof(MSG_LOGIN) - sizeof(long), 0);
    
    msgctl(my_que_id, IPC_RMID, 0);

 return NULL;
}