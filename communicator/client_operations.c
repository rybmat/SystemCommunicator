#include "client_operations.h"

//id kolejek swojej i servera
int my_que_id = 0;
int serv_que_id = 0;

//nazwa usera
char nick[USER_NAME_MAX_LENGTH];

//nazwa kanalu w ktorym jest user i nazwa kanalu do ktorego user chce wejsc
char *channel = NULL;
char *temp_channel = NULL;

//nazwy plikow z historiami wiadomosci
char private_messages_file_name[35];
char channel_messages_file_name[35];

//struktury uzywane przy kolejkach komunikatow
MSG_CHAT_MESSAGE chmsg;
MSG_LOGIN log_in;
MSG_RESPONSE response;

char* get_nick(){
    return nick;
}
char* get_channel(){
    return channel;
}
char* get_temp_channel(){
    return temp_channel;
}
int get_my_que_id(){
    return my_que_id;
}
int get_serv_que_id(){
    return serv_que_id;
}
char* get_private_messages_file_name(){
    return private_messages_file_name;
}
char* get_channel_messages_file_name(){
    return channel_messages_file_name;
}

int init(){
    int i;
   
    for(i = 0; i < USER_NAME_MAX_LENGTH; ++i){
        nick[i] = '\0';
    }
  
    //tworzenie kolejki
    my_que_id = msgget(IPC_PRIVATE, 0666);
    if(my_que_id <= 0){
        return 0;
    }
    printf("my id: %d\n", my_que_id);
    
    //przyjecie nazwy uzytkownika, id serwera, próba zalogowania
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
        
        //jezeli nie mozna wyslac komunikatu
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
        
        //serwer nie mogl przyjac usera
        if(response.response_type == LOGIN_FAILED){
            printf("Login failed: %s\n", response.content);
            continue;
        }
        
    }while(response.response_type != LOGIN_SUCCESS);
    printf("have a lot of fun..\n");
  
  //tworzenie plikow do wiadomosci;
    time_t t = time(NULL);
    char *time_str = ctime(&t);
    
    for(i = 0; i < 35; ++i){
        private_messages_file_name[i] = '\0';
        channel_messages_file_name[i] = '\0';
    }
    private_messages_file_name[24] = '\0';
    channel_messages_file_name[24] = '\0';
    
    strcat(private_messages_file_name, time_str);
    strcat(private_messages_file_name, " PRV.txt");
    
    strcat(channel_messages_file_name, time_str);
    strcat(channel_messages_file_name, " CHN.txt");
    
    FILE *f=NULL;
    f = fopen(private_messages_file_name, "a");
    fclose(f);
    
    FILE *f2=NULL;
    f2 = fopen(channel_messages_file_name, "a");
    fclose(f2);
    
return 1;
}

char* parser(char* command, char** ppl_cnt){
    char uname[USER_NAME_MAX_LENGTH];   
    char msg[MAX_MSG_LENGTH]; 
    char rname[ROOM_NAME_MAX_LENGTH];
    
    int i;
    
    for(i = 0; i < USER_NAME_MAX_LENGTH; ++i){
        uname[i] = '\0';
    }
    for(i = 0; i < MAX_MSG_LENGTH; ++i){
        msg[i] = '\0';
    }
    
    if(strncmp(command, "msg ", 4) == 0){       //wysylanie wiadomosci
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
        
        return msg_snd(uname, msg, ppl_cnt);
    }else if(strncmp(command, "logout", 6) == 0){       //wylogowanie z systemu
        logout();    
        return "logged out";
    }else if(strncmp(command, "channel ",8) == 0){      //wejscie do kanalu
        for(i = 8; command[i]!=' ' && i < ROOM_NAME_MAX_LENGTH + 7; ++i){
            rname[i-8] = command[i];
        }
        rname[ROOM_NAME_MAX_LENGTH - 1] = '\0';
        
        return enter_channel(rname);
    }
    
    if(strcmp(command, "") == 0){
        return NULL;
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

char* msg_snd(char* receiver, char* msg, char** ppl_cnt){
    chmsg.type = MESSAGE;
    chmsg.msg_type = -1;
    
    int i;
    for(i = 0; i < USER_NAME_MAX_LENGTH; ++i){
        chmsg.receiver[i] = '\0';
        chmsg.sender[i] = '\0';
    }
    strcpy(chmsg.receiver, receiver);
    strcpy(chmsg.sender, nick);
    chmsg.receiver[USER_NAME_MAX_LENGTH - 1] = '\0';
    chmsg.sender[USER_NAME_MAX_LENGTH - 1] = '\0';
    
    for(i = 0; i < MAX_MSG_LENGTH; ++i){
        chmsg.message[i] = '\0';
    }
    strcpy(chmsg.message, msg);
 
    //przygotowanie czasu wyslania
    time_t t = time(NULL);
    char *time_str = ctime(&t);
    
    for(i = 0; i < 5; ++i){
        chmsg.send_time[i] = time_str[i+11];
    }
    chmsg.send_time[5] = '\0';
    
    //sprawdzanie czy podany adresat jest na liscie kontaktow lub czy jest  to wiadomosc do pokoju i przypisanie odpowiedniego
    //typu wiadomosci

    if(strcmp(receiver, "channel") == 0){
        if(channel == NULL){
            return "You haven't entered to any channel yet";
        }
        chmsg.msg_type = PUBLIC;
    }else{
        for(i = 0; i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER; ++i){
            if(ppl_cnt[i] != NULL){
                if(strcmp(receiver, ppl_cnt[i]) == 0){
                    chmsg.msg_type = PRIVATE;
                    break;
                }
            }
        }
    }
    
    //jesli nie znaleziono adresata na liscie
    if(chmsg.msg_type == -1){
        return "There is no such person";
    }
    
    //wyslanie wiadomosci do serwera i do samego siebie (zeby pojawiala sie w oknie wiadomosci)
    msgsnd(serv_que_id, &chmsg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), 0);
    msgsnd(my_que_id, &chmsg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), 0);
    
    return NULL;
}

char* enter_channel(char* channel_name){
    MSG_ROOM msgroom;
    msgroom.type = ROOM;
    strcpy(msgroom.room_name, channel_name);
    strcpy(msgroom.user_name, nick);
    
    int i; 
      
    if(channel == NULL){  //jezeli jeszcze nie wszedl do zadnego pokoju to wysyla zadanie wejscia do danego pokoju
        channel = (char*)malloc(ROOM_NAME_MAX_LENGTH * sizeof(char));
        for(i = 0; i < ROOM_NAME_MAX_LENGTH; ++i){
            channel[i] = '\0';
        }
        temp_channel = (char*)malloc(ROOM_NAME_MAX_LENGTH * sizeof(char));
        for(i = 0; i < ROOM_NAME_MAX_LENGTH; ++i){
            channel[i] = '\0';
        }
        strcpy(temp_channel, channel_name);
        msgroom.operation_type = ENTER_ROOM;
        msgsnd(serv_que_id, &msgroom, sizeof(MSG_ROOM) - sizeof(long), 0);
    }else if(strcmp(channel_name, channel) == 0){ //jezeli juz jest w tym pokoju 
        return "You have already entered this channel";
    }else{ //zmiana pokoju
        channel = (char*)malloc(ROOM_NAME_MAX_LENGTH * sizeof(char));
        for(i = 0; i < ROOM_NAME_MAX_LENGTH; ++i){
            temp_channel[i] = '\0';
        }
        strcpy(temp_channel, channel_name);
        msgroom.operation_type = CHANGE_ROOM;
        msgsnd(serv_que_id, &msgroom, sizeof(MSG_ROOM) - sizeof(long), 0);
    }
      
 return NULL;   
}