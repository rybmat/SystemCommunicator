#include <sys/shm.h>

#include "server_operations.h"


//warunek zakończenia głównej pętli
short run = 1;

//struktury używane przy komunikatach
MSG_LOGIN login;
MSG_RESPONSE response;
MSG_CHAT_MESSAGE chmsg;
MSG_ROOM room;
MSG_REQUEST req;
MSG_USERS_LIST ulist;
MSG_SERVER2SERVER s2s;


//identyfikatory semaforów
int server_ids_sem_id;
int user_server_sem_id;
int room_server_sem_id;
int logfile_sem_id;

//identyfikatory obszarów pamięci współdzielonej
int server_ids_shm_id;
int user_server_shm_id;
int room_server_shm_id;

//wskaźniki na obszary wspoldzielone
int *server_ids;
USER_SERVER *user_server;
ROOM_SERVER *room_server;

//identyfikator kolejki serwera
int que_id = 0;

//struktury odpowiadające operacjom P i V używane w semop
struct sembuf P, V;

//tablica userów tego serwera
LOGGED_USER local_users[MAX_USERS_NUMBER];

int init(){
    //"operacje" P i V
    P.sem_op = -1;
    P.sem_num = 0;
    P.sem_flg = 0;
    V.sem_op = 1;
    V.sem_num = 0;
    V.sem_flg = 0;    

//pobranie id/tworzenie semaforów
    server_ids_sem_id = semget(SEM_SERVER_IDS, 1, IPC_CREAT|IPC_EXCL);
    if(server_ids_sem_id == -1){
        server_ids_sem_id = semget(SEM_SERVER_IDS, 1, 0);
    }else{
        SEM_UNION su;
        su.val = 1;     
        semctl(server_ids_sem_id, 0, SETVAL, su);
    }
    
    user_server_sem_id = semget(SEM_USER_SERVER, 1, IPC_CREAT|IPC_EXCL);
    if(user_server_sem_id == -1){
        user_server_sem_id = semget(SEM_USER_SERVER, 1, 0);
    }else{
        SEM_UNION su;
        su.val = 1;     
        semctl(user_server_sem_id, 0, SETVAL, su);
    }
    
    room_server_sem_id = semget(SEM_ROOM_SERVER, 1, IPC_CREAT|IPC_EXCL);
    if(room_server_sem_id == -1){
        room_server_sem_id = semget(SEM_ROOM_SERVER, 1, 0);
    }else{
        SEM_UNION su;
        su.val = 1;     
        semctl(room_server_sem_id, 0, SETVAL, su);
    }
    
    //semafor pliku logu
    logfile_sem_id = semget(SEM_LOGFILE, 1, IPC_CREAT|IPC_EXCL);
    if(logfile_sem_id == -1){
        logfile_sem_id = semget(SEM_LOGFILE, 1, 0);
    }else{
        SEM_UNION su;
        su.val = 1;     
        semctl(logfile_sem_id, 0, SETVAL, su);
    }

//pobranie id/tworzenie obszarów pamieci współdzielonej
    server_ids_shm_id = shmget(SHM_SERVER_IDS, MAX_SERVERS_NUMBER * sizeof(int), IPC_CREAT|IPC_EXCL|0666);
    if(server_ids_shm_id == -1){
        server_ids_shm_id = shmget(SHM_SERVER_IDS, MAX_SERVERS_NUMBER * sizeof(int), IPC_CREAT|0666);
        server_ids = (int*)shmat(server_ids_shm_id, NULL, 0);        
    }else{
        server_ids = (int*)shmat(server_ids_shm_id, NULL, 0);
        int i;
        for(i = 0; i < MAX_SERVERS_NUMBER; ++i){
            server_ids[i] = -1;
        }
    }
      
    user_server_shm_id = shmget(SHM_USER_SERVER, MAX_USERS_NUMBER * MAX_SERVERS_NUMBER * sizeof(USER_SERVER), IPC_CREAT|IPC_EXCL|0666);
    if(user_server_shm_id == -1){
        user_server_shm_id = shmget(SHM_USER_SERVER, MAX_USERS_NUMBER * MAX_SERVERS_NUMBER * sizeof(USER_SERVER), IPC_CREAT|0666);
        user_server = (USER_SERVER*)shmat(user_server_shm_id, NULL, 0);
    }else{
        user_server = (USER_SERVER*)shmat(user_server_shm_id, NULL, 0);
        int i;
        for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
            user_server[i].server_id = -1;
        }
    }
    
    room_server_shm_id = shmget(SHM_ROOM_SERVER, MAX_USERS_NUMBER * MAX_SERVERS_NUMBER * sizeof(ROOM_SERVER), IPC_CREAT|IPC_EXCL|0666);
    if(room_server_shm_id == -1){
        room_server_shm_id = shmget(SHM_ROOM_SERVER, MAX_USERS_NUMBER * MAX_SERVERS_NUMBER * sizeof(ROOM_SERVER), IPC_CREAT|0666);
        room_server = (ROOM_SERVER*)shmat(room_server_shm_id, NULL, 0);
    }else{
        int i;
        room_server = (ROOM_SERVER*)shmat(room_server_shm_id, NULL, 0);
        for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
            room_server[i].server_id = -1;
        }
    }
    
    if(room_server_sem_id < 0 || user_server_sem_id < 0 || server_ids_sem_id < 0 || logfile_sem_id < 0){
        printf("blad przy odczycie semaforow\n");
        return 0;
    }
    
    if(room_server_shm_id < 0 || user_server_shm_id < 0 || server_ids_shm_id < 0){
        printf("blad przy dostepie do pamieci wspoldzielonej\n");
        return 0;
    }
    
//tworzenie kolejki
    que_id = msgget(IPC_PRIVATE, 0666);
    if(que_id <= 0){
        printf("blad tworzenia kolejki\n");
        return 0;
    }
    
//wpisanie sie do rejestru serwerów
    int i;
    semop(server_ids_sem_id, &P, 1);
        for(i = 0; i < MAX_SERVERS_NUMBER; ++i){
            if(server_ids[i] == -1){
                server_ids[i] = que_id;
                break;
            }
        }
    semop(server_ids_sem_id, &V, 1);
    
    add_log("serwer uruchomiony ");

    if(i == MAX_SERVERS_NUMBER){
        printf("Zbyt duża liczba serwerów\n");
        return 0;
    }
       
//zainicjowanie tablicy userów tego serwera
    int k;
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
            local_users[i].username[k] = '\0';
        }
        for(k = 0; k < ROOM_NAME_MAX_LENGTH; ++k){
            local_users[i].room[k] = '\0';
        }
        local_users[i].que_id = -1;
        local_users[i].heartbeat = 5;
    }
    
    printf("server id: %d\n",que_id);
    return 1;
}

void close_server(){   
//usuniecie wpisu z tablicy serwerów
    int i;
    semop(server_ids_sem_id, &P, 1);
        for(i = 0; i < MAX_SERVERS_NUMBER; ++i){
            if(server_ids[i] == que_id){
                server_ids[i] = -1;
                break;
            }
        }
    semop(server_ids_sem_id, &V, 1);
 
    //wylogowanie wszystkich uzytkownikow - unregister_user dba tez o to zeby usunac wpisy z room_serwer
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(local_users[i].que_id != -1){
            unregister_user(local_users[i].username);
        }
    }
    
//sprawdzenie czy był ostatnim serwerem - jesli tak to usuwa pamiec wpoldzielona    
    for(i = 0; i < MAX_SERVERS_NUMBER; ++i){
        if(server_ids[i] >= 0){
            break;
        }
    }
    
    if(i >= MAX_SERVERS_NUMBER){ 
        shmdt(server_ids);
        shmdt(user_server);
        shmdt(room_server);

        struct shmid_ds shm_desc;
        shmctl(server_ids_shm_id, IPC_RMID, &shm_desc);
        shmctl(user_server_shm_id, IPC_RMID, &shm_desc);
        shmctl(room_server_shm_id, IPC_RMID, &shm_desc);
        printf("usunieto pamiec\n");
    }else{
        shmdt(server_ids);
        shmdt(user_server);
        shmdt(room_server);
        printf("odlaczono pamiec\n");
    }
      
//usuniecie kolejki
    msgctl(que_id, IPC_RMID, 0);   
    add_log("serwer zamkniety");
    exit(0);
}

void add_log(char* log){
    //if(!fork()){    
        char tmp[100];
        int i;
        for(i = 0; i < 100; ++i){
            tmp[i] = '\0';
        }

        time_t t = time(NULL);
        char *time_str = ctime(&t);

        char id[10];
        for(i = 0; i < 10; ++i){
            id[i] = '\0';
        }
        sprintf(id, "%d", que_id);

        strcat(tmp, id);
        strcat(tmp, "\t");
        strncat(tmp, time_str, 24);
        strcat(tmp, "\t");
        strcat(tmp, log);
        strcat(tmp, "\n");

        FILE *F;

        semop(logfile_sem_id, &P, 1);
        F = fopen("/tmp/czat.log", "a");
        fprintf(F, "%s", tmp);
        fclose(F);
        semop(logfile_sem_id, &V, 1);
        
   /*     exit(0);
    }else{
        return;
    }*/   
}

void server_main(){
    signal(SIGTERM, close_server);
    signal(SIGALRM, heartbeat);
    alarm(5);
    
    do{
     //rządanie zalogowania
        if(msgrcv(que_id, &login, sizeof(MSG_LOGIN) - sizeof(long), LOGIN, IPC_NOWAIT) != -1){
            register_user(login.username, login.ipc_num);
        }
     //rządanie wylogowania   
        if(msgrcv(que_id, &login, sizeof(MSG_LOGIN) - sizeof(long), LOGOUT, IPC_NOWAIT) != -1){
            unregister_user(login.username);
        }
     //wiadomości   
        if(msgrcv(que_id, &chmsg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), MESSAGE, IPC_NOWAIT) != -1){
            send_message(chmsg);
        }
     //wejście/wyjście/zmiana pokoju
        if(msgrcv(que_id, &room, sizeof(MSG_ROOM) - sizeof(long), ROOM, IPC_NOWAIT) != -1){
            if(room.operation_type == ENTER_ROOM){ //wejscie do pokoju
                enter_to_room(room.user_name, room.room_name);
                response.type = RESPONSE;
                response.response_type = ENTERED_ROOM_SUCCESS;
                int i;
                for(i = 0; i < RESPONSE_LENGTH; ++i){
                    response.content[i] = '\0';
                }
                strcpy(response.content, "Welcome in this room");
                
                msgsnd(get_client_queue_id(room.user_name), &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
                
            }else if(room.operation_type == CHANGE_ROOM){ //zmiana pokoju
                leave_the_room(room.user_name);
                enter_to_room(room.user_name, room.room_name);
                response.type = RESPONSE;
                response.response_type = CHANGE_ROOM_SUCCESS;
                int i;
                for(i = 0; i < RESPONSE_LENGTH; ++i){
                    response.content[i] = '\0';
                }
                strcpy(response.content, "Welcome in this room");
                
                msgsnd(get_client_queue_id(room.user_name), &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
                
            }else if(room.operation_type == LEAVE_ROOM){ //wyjscie z pokoju
                leave_the_room(room.user_name);
                response.type = RESPONSE;
                response.response_type = LEAVE_ROOM_SUCCESS;
                int i;
                for(i = 0; i < RESPONSE_LENGTH; ++i){
                    response.content[i] = '\0';
                }
                strcpy(response.content, "");
 
                msgsnd(get_client_queue_id(room.user_name), &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
                
            }
        }
     //różne requesty
        if(msgrcv(que_id, &req, sizeof(MSG_REQUEST) - sizeof(long), REQUEST, IPC_NOWAIT) != -1){
            if(req.request_type == USERS_LIST){ //lista uzytkownikow
                send_users_list(req.user_name);
            }else if(req.request_type == ROOMS_LIST){
                send_channels_list(req.user_name);
            }else if(req.request_type == PONG){ //otrzymanie odpowiedzi na heartbeat
                int i;
                for(i = 0; i < MAX_USERS_NUMBER; ++i){
                    if(strcmp(req.user_name, local_users[i].username) == 0){
                        local_users[i].heartbeat = 5;
                    }
                }
            }          
        }
     //odpowiedz na sprawdzanie dostepnosci
        if(msgrcv(que_id, &s2s, sizeof(MSG_SERVER2SERVER) - sizeof(long), SERVER2SERVER, IPC_NOWAIT) != -1){
            MSG_SERVER2SERVER serv2serv;
            serv2serv.server_ipc_num = que_id;
            serv2serv.type = SERVER2SERVER;
            msgsnd(s2s.server_ipc_num, &serv2serv, sizeof(MSG_SERVER2SERVER) - sizeof(long), 0);
        }
        
    }while(run);
}

void register_user(char *username, int user_queue){
    int i;
    int user_exists = 0, user_exists_in_repo = 0, too_many_users = 0;
 //dodanie do pamieci lokalnej    
    for( i = 0; i < MAX_USERS_NUMBER; ++i){
        if(strcmp(username, local_users[i].username) == 0){
            user_exists = 1;
            break;
        }
    }
    if(user_exists == 0){
        for(i = 0; i < MAX_USERS_NUMBER; ++i){     
            if(local_users[i].que_id == -1){
                local_users[i].que_id = user_queue;
                strcpy(local_users[i].username, username);
                local_users[i].heartbeat = 5;
                break;
            }
        }
    }
  
    if(i == MAX_USERS_NUMBER){
        too_many_users = 1;
    }
 //dodanie do repozytorium   
    if((user_exists == 0) && (too_many_users == 0)){
        semop(user_server_sem_id, &P, 1);
            for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
                if((strcmp(username, user_server[i].user_name) == 0) && (user_server[i].server_id >= 0) ){
                    user_exists_in_repo = 1;
                    break;
                }
            }
            if(user_exists_in_repo == 0){
                for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
                    if(user_server[i].server_id == -1){
                        user_server[i].server_id = que_id;
                        int k;
                        for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
                            user_server[i].user_name[k] = '\0';
                        }
                        strcpy(user_server[i].user_name, username);
                        user_server[i].user_name[USER_NAME_MAX_LENGTH - 1] = '\0';
                        break;
                    }
                }
            }
        semop(user_server_sem_id, &V, 1);
    }
    
    for(i = 0; i < RESPONSE_LENGTH; ++i){
            response.content[i] = '\0';
    }
 //logowanie powiodlo sie   
    if((user_exists == 0) && (user_exists_in_repo == 0) && (too_many_users == 0)){
        response.type = RESPONSE;
        response.response_type = LOGIN_SUCCESS;
        strcpy(response.content, "Have a lot of fun!");
        msgsnd(user_queue, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
        //wpis do logu
        char tmp[50];
        for(i = 0; i < 50; ++i){
            tmp[i] = '\0';
        }
        strcat(tmp, "zarejestrowano uzytkownika: ");
        strcat(tmp, username);
        add_log(tmp);        
    }else{//logowanie nie powiodlo sie
        response.type = RESPONSE;
        response.response_type = LOGIN_FAILED;
        
        if(too_many_users == 1){
            strcpy(response.content, "Too many users is using this server, sorry");
        }else if((user_exists == 1) || (user_exists_in_repo == 1)){
            strcpy(response.content, "There is somebody who uses the same nick, try another one");
        }
        msgsnd(user_queue, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
        
        //user zostal dodany do pamieci lokalnej, ale w repo był już ktoś o tym samym nicku - usuniecie z pamieci lokalnej
        if(user_exists_in_repo == 1){
            for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
                if(local_users[i].que_id == user_queue){
                    local_users[i].que_id = -1;
                    int k;
                    for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
                        local_users[i].username[k] = '\0';
                    }
                }
            }
        }       
    }  
}

void unregister_user(char* username){
    int i, user_queue = -1;
    char user_room[ROOM_NAME_MAX_LENGTH];
    for(i = 0; i < ROOM_NAME_MAX_LENGTH; ++i){
        user_room[i] = '\0';
    }
    
    //usuniecie wpisu z lokalnej pamieci
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(strcmp(local_users[i].username, username) == 0){
            user_queue = local_users[i].que_id;
            local_users[i].que_id = -1;
            strcpy(user_room, local_users[i].room);
            int k;
            for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
                local_users[i].username[k] = '\0';
            }
            for(k = 0; k < ROOM_NAME_MAX_LENGTH; ++k){
                local_users[i].room[k] = '\0';
            }
            break;
        }
    }
    
    char tmp_uname[USER_NAME_MAX_LENGTH];
    for(i = 0; i < USER_NAME_MAX_LENGTH; ++i){
        tmp_uname[i] = '\0';
    }
    strcpy(tmp_uname, username);
    
    //if(!fork()){
        //wyrejestrowanie usera z repo
        semop(user_server_sem_id, &P, 1);
            for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
                if(strcmp(user_server[i].user_name,tmp_uname) == 0){
                    user_server[i].server_id = -1;
                    int k;
                    for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
                        user_server[i].user_name[k] = '\0';
                    }
                    break;
                }
            }      
        semop(user_server_sem_id, &V, 1);
        
        response.type = RESPONSE;
        response.response_type = LOGOUT_SUCCESS;
        for(i = 0; i < RESPONSE_LENGTH; ++i){
            response.content[i] = '\0';
        }
        strcpy(response.content, "Logout success");
        msgsnd(user_queue, &response, sizeof(MSG_RESPONSE) - sizeof(long), 0);
       //wpis do logu        
        char log[50];
        for(i = 0; i < 50; ++i){
            log[i] = '\0';
        }
        strcat(log, "wyrejestrowano uzytkownika: ");
        strcat(log, tmp_uname);
        add_log(log);
 //gdy usuniety uzytkownik byl ostatnim uzytkownikiem swojego pokoju na tym serwerze to usuwany jest wpis z pamieci room_serwer
        for(i = 0; i < MAX_USERS_NUMBER; ++i){
            if(strcmp(local_users[i].room, user_room) == 0){
                break;
            }
        }
        if(i == MAX_USERS_NUMBER){
            semop(room_server_sem_id, &P, 1);
                for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
                    if((strcmp(room_server[i].room_name, user_room) == 0) && (room_server[i].server_id == que_id)){
                        room_server[i].server_id = -1;
                        int k;
                        for(k = 0; k < ROOM_NAME_MAX_LENGTH; ++k){
                            room_server[i].room_name[k] = '\0';
                        }
                        break;
                    }
                }      
            semop(room_server_sem_id, &V, 1);
        }
        
  /*      exit(0);
    }else{
        ;
        return;
    }*/
}

void heartbeat(){
    MSG_RESPONSE rs;
    rs.response_type = PING;
    rs.type = RESPONSE;
 
    int i;
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(local_users[i].que_id != -1){
            if(local_users[i].heartbeat < 0){
                unregister_user(local_users[i].username);
                add_log("usuniety przez heartbeat");
            }else{
                local_users[i].heartbeat--;   
                msgsnd(local_users[i].que_id, &rs, sizeof(MSG_RESPONSE) - sizeof(long), 0);
            }
        }
    }
    alarm(5);
}

void enter_to_room(char *username, char *room){
    int i;
//wpis do lokalnej pamieci
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(strcmp(local_users[i].username, username) == 0){
            if(local_users[i].room[0] != '\0'){
                leave_the_room(username);
            }
            strcpy(local_users[i].room, room);
            break;
        }
    }

//sprawdzenie czy wczesniej byl już jakis uzytkownik tego pokoju, jesli nie to wpisuje sie do room_serwer
    int count = 0;
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(strcmp(local_users[i].room, room) == 0){
            ++count;
            if(count > 1){
                break;
            }
        }
    }
    
    if(count > 1){
        return;
    }else{
        char tmp_rname[ROOM_NAME_MAX_LENGTH];
        for(i = 0; i < ROOM_NAME_MAX_LENGTH; ++i){
            tmp_rname[i] = '\0';
        }
        strcpy(tmp_rname, room);
        
        semop(room_server_sem_id, &P, 1);
            for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
                if(room_server[i].server_id == -1){
                    room_server[i].server_id = que_id;
                    int k;
                    for(k = 0; k < ROOM_NAME_MAX_LENGTH; ++k){
                        room_server[i].room_name[k] = '\0';
                    }
                    strcpy(room_server[i].room_name, tmp_rname);
                    break;
                }
            }
        semop(room_server_sem_id, &V, 1);
    }
}

void leave_the_room(char *username){
    int i;
    char tmp_rname[ROOM_NAME_MAX_LENGTH];
    for(i = 0; i < ROOM_NAME_MAX_LENGTH; ++i){
        tmp_rname[i] = '\0';
    }
//usuniecie wpisu z lokalnej pamieci
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(strcmp(local_users[i].username, username) == 0){
            strcpy(tmp_rname, local_users[i].room);
            int k;
            for(k = 0; k < ROOM_NAME_MAX_LENGTH; ++k){
                local_users[i].room[k] = '\0';
            }
            break;
        }
    }
    
//gdy uzytkownik byl ostatnim uzytkownikiem tego pokoju na tym serwerze to usuwany jest wpis z pamieci room_serwer
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(strcmp(local_users[i].room, tmp_rname) == 0){
            break;
        }
    }
    if(i == MAX_USERS_NUMBER){
        semop(room_server_sem_id, &P, 1);
            for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
                if((room_server[i].server_id == que_id) && (strcmp(room_server[i].room_name, tmp_rname) == 0)){
                    room_server[i].server_id = -1;
                    int k;
                    for(k = 0; k < ROOM_NAME_MAX_LENGTH; ++k){
                        room_server[i].room_name[k] = '\0';
                    }
                    break;
                }
            }      
        semop(room_server_sem_id, &V, 1);
    }   
}

void send_users_list(char *username){
    ulist.type =  USERS_LIST_STR;
    
    int i,k, line_counter = 0;
    for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
        for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
            ulist.users[i][k] = '\0';
        }
    }
    
    semop(user_server_sem_id, &P, 1);
        for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
            if(user_server[i].server_id != -1){
                k=0;
                while(user_server[i].user_name[k] != '\0'){
                    ulist.users[line_counter][k] = user_server[i].user_name[k];
                    k++;
                }
                line_counter++;
            }
        }      
    semop(user_server_sem_id, &V, 1);
    
    msgsnd(get_client_queue_id(username), &ulist, sizeof(MSG_USERS_LIST) - sizeof(long), IPC_NOWAIT);
}

void send_channels_list(char *username){
    ulist.type =  ROOMS_LIST_STR;
    
    int i,j,k, line_counter = 0;
    for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
        for(k = 0; k < USER_NAME_MAX_LENGTH; ++k){
            ulist.users[i][k] = '\0';
        }
    }
    
    semop(room_server_sem_id, &P, 1);
        for(i = 0; i < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++i){
            if(room_server[i].server_id != -1){
                for(j = 0; j < line_counter; ++j){
                    if(strcmp(room_server[i].room_name, ulist.users[j]) == 0){
                        break;
                    }
                }
                if(j == line_counter){
                    k = 0;
                    while(room_server[i].room_name[k] != '\0'){
                        ulist.users[line_counter][k] = room_server[i].room_name[k];
                        k++;
                    }
                    line_counter++;
                }
            }
        }
    semop(user_server_sem_id, &V, 1);

    msgsnd(get_client_queue_id(username), &ulist, sizeof(MSG_USERS_LIST) - sizeof(long), IPC_NOWAIT);
}

void send_message(MSG_CHAT_MESSAGE msg){
    int i, j, k, counter = 0;
    
    int servers[MAX_SERVERS_NUMBER];
    for(i = 0; i < MAX_SERVERS_NUMBER; ++i){
        servers[i] = -1;
    }
   
    //sprawdzanie czy nadawca jest z tego serwera
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if(strcmp(local_users[i].username, msg.sender) == 0){
            break;
        }
    }  
    if(i < MAX_USERS_NUMBER){ //jesli jest 
        switch(msg.msg_type){
            case PUBLIC: 
                //pobranie listy serwerow majacych dany kanal
                semop(room_server_sem_id, &P, 1);
                    for(j = 0; j < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++j){
                        if((room_server[j].server_id != -1) && (strcmp(room_server[j].room_name, msg.receiver) == 0)){
                            for(k = 0; k < counter; ++k){
                                if(room_server[j].server_id == servers[j]){
                                    break;
                                }
                            }
                            if(k == counter){
                                servers[counter] = room_server[j].server_id;
                                counter++;
                            }
                        }
                    }
                semop(user_server_sem_id, &V, 1);
                //wysylanie do wszystkich znalezionych serwerow
                for(j = 0; j < MAX_SERVERS_NUMBER; ++j){
                    if(servers[j] != que_id){
                        msgsnd(servers[j], &msg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), IPC_NOWAIT);
                    }
                }
                //wysylanie do wszystkich userow tego serwera bedacych w pokoju
                for(j = 0; j < MAX_USERS_NUMBER; ++j){
                    if((strcmp(local_users[j].room, msg.receiver) == 0) && strcmp(local_users[j].username, msg.sender) != 0 ){
                       msgsnd(local_users[j].que_id, &msg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), IPC_NOWAIT); 
                    }
                }
                break;
                
            case PRIVATE:
                //sprawdzanie czy odbiorca jest na tym serwerze
                for(j = 0; j < MAX_USERS_NUMBER; ++j){
                    if(strcmp(local_users[j].username, msg.receiver) == 0){
                        break;
                    }
                }
                if(j < MAX_USERS_NUMBER){ //jesli jest to wysyla do niego
                    msgsnd(local_users[j].que_id, &msg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), IPC_NOWAIT);
                }else{ //jesli nie to szuka w repo i wysyla do odpowiedniego serwera
                    int id;
                    semop(user_server_sem_id, &P, 1);
                        for(j = 0; j < MAX_USERS_NUMBER * MAX_SERVERS_NUMBER; ++j){
                            if((user_server[j].server_id != -1) && (strcmp(user_server[j].user_name, msg.receiver) == 0)){
                                id = user_server[j].server_id;
                            }
                        }      
                    semop(user_server_sem_id, &V, 1);
                    
                    msgsnd(id, &msg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), IPC_NOWAIT);                   
                }
                break;
        }
        response.type = RESPONSE;
        response.response_type = MSG_SEND;
        strcpy(response.content, "");
        msgsnd(get_client_queue_id(msg.sender), &response, sizeof(MSG_RESPONSE) - sizeof(long), IPC_NOWAIT); 
        
    }else{ //jesli nadawca nie jest z tego serwera
        switch(msg.msg_type){
            case PUBLIC: 
                //wysylanie do wszystkich userow tego serwera bedacych w pokoju
                for(j = 0; j < MAX_USERS_NUMBER; ++j){
                    if(strcmp(local_users[j].room, msg.receiver) == 0){
                       msgsnd(local_users[j].que_id, &msg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), IPC_NOWAIT); 
                    }
                }
                break;
            case PRIVATE:
                msgsnd(get_client_queue_id(msg.receiver), &msg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), IPC_NOWAIT); 
                break;
        }   
    }        
}

int get_client_queue_id(char* username){
    int i;
    int id = -1;
    for(i = 0; i < MAX_USERS_NUMBER; ++i){
        if((local_users[i].que_id != -1) && (strcmp(local_users[i].username, username) == 0)){
            id = local_users[i].que_id;
        }
    }
    return id;
}