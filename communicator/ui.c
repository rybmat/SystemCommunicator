#include "protocol.h"
#include "ui.h"

//ilość wyswietlanych wiadomosci posczegolnego typu
int all_msg_num = 0;
int priv_msg_num = 0;
int room_msg_num = 0;

//ilość wyświetatlanych kontaktow danego typu
int room_cnt_num = 0;
int ppl_cnt_num = 0;

//liczba lini mozliwych do zadrukowania i iterator po tablicy z trescia dla okienka wiadomosci
int recived_lines = RECIVED_HEIGHT - 2;
int recived_iter;

//liczba lini mozliwych do zadrukowania i iterator po tablicy z trescia dla okienka kontaktow
int contacts_iter = 0;
int contacts_lines = CONTACTS_HEIGHT - 2;

//tytuly okienek wiadomosci i kontaktow
 char* msg_title;
 char* cnt_title;
 
 //komunikat wyswietlany w oknie komend
 char command_message2[RESPONSE_LENGTH * 2];
 char* command_message;

 //licznik uzywany przy wysylaniu zapytan o liste kontaktow i pokoi
 int counter = 0; 
 
 //PID procesu przetwarzającego heartbeat
 int child_pid;

 int get_child_pid(){
     return child_pid;
 }
 
 void set_child_pid(int pid){
     child_pid = pid;
 }
 
void ui_main(){	  
    signal(SIGUSR1, refr_recived);
    signal(SIGUSR2, refr_contacts);
    signal(SIGALRM, get_contacts_lists);
    
        all_msg_num = 0;
        priv_msg_num = 0;
        room_msg_num = 0;
    
    //poczatek rysowania okienek
        int startx = 1;
        int starty = 2;
	
        //wcisniety klawisz klawiatury
        int c;
        
        //iteratory: aktywne okno, aktywne kontakty, aktywne wiadomosci
        int active_window = 0;
        int active_contacts = 0;
        int active_messages = 0;
       
        init_strings();
        
     //inicjalizacja ncurses
	initscr();
	clear();
	noecho();
	cbreak();
    //tworzenie okienek
	recived_win = newwin(RECIVED_HEIGHT, RECIVED_WIDTH, starty, startx);
        contacts_win = newwin(CONTACTS_HEIGHT, CONTACTS_WIDTH, starty, startx + RECIVED_WIDTH);
        command_win = newwin(COMMAND_HEIGHT, COMMAND_WIDTH, starty + RECIVED_HEIGHT, startx);      
        
     //wybor aktywnych okien   
        active_win = recived_win;
        active_msg = all_messages;
        active_cnt = people_contacts;
        
     //ustawienie iteratora wiadomosci tak by byly wyswietlane ostatnie wiadomosci   
        recived_iter = 0;
        msg_title = "All messages";
        cnt_title = "People";
        
        print_content(recived_win, msg_title, active_msg, recived_iter, recived_lines);
        print_content(contacts_win, cnt_title, active_cnt, contacts_iter, contacts_lines);
        command_window_drawing(command_win, "Command");	      
       
        curs_set(0);
        
        alarm(1); //żeby pobralo pierwszy raz listy kontaktow
        //petla glowna
	while(1){
		keypad(active_win, TRUE);
                nodelay(active_win, TRUE);
                process_ipc_msgs();
                process_server_responses();
                c = wgetch(active_win);
		switch(c){//sprawdza ktory klawisz wcisnieto
			case KEY_UP://w gore (przesuwanie tresci)
				switch(active_window){
                                    case RECIVED:
                                        if(recived_iter > 0){
                                            recived_iter--;
                                            kill(getpid(), SIGUSR1);
                                        }
                                        break;
                                    case CONTACTS:
                                        if(contacts_iter > 0){
                                            contacts_iter--;
                                            kill(getpid(), SIGUSR2);
                                        }                                        
                                        break;
                                }
				break;
			case KEY_DOWN://w dol (przesuwanie tresci)
				switch(active_window){
                                    case RECIVED:
                                        switch(active_messages){
                                            case MESSAGES_ALL:
                                                if(recived_iter + recived_lines <= all_msg_num){
                                                        recived_iter++;
                                                }
                                                break;
                                            case MESSAGES_PRIV:
                                                if(recived_iter + recived_lines <= priv_msg_num){
                                                        recived_iter++;
                                                }
                                                break;
                                            case MESSAGES_ROOM:
                                                if(recived_iter + recived_lines <= room_msg_num){
                                                        recived_iter++;
                                                }
                                                break;
                                        }
                                        kill(getpid(), SIGUSR1);
                                        break;
                                    case CONTACTS:                                     
                                        switch(active_contacts){
                                            case CONTACTS_PEOPLE:
                                                if(contacts_iter + contacts_lines <= ppl_cnt_num){
                                                        contacts_iter++;
                                                }
                                                break;
                                            case CONTACTS_ROOMS:
                                                if(contacts_iter + contacts_lines <= room_cnt_num){
                                                        contacts_iter++;
                                                }
                                                break;
                                        }
                                        kill(getpid(), SIGUSR2);
                                        break;
                                }
				break;
                        case KEY_LEFT://w lewo (zmiana wyswietlanych wiadomosci lub kontaktow)
                                switch(active_window){
                                    case RECIVED:
                                        if(active_messages > 0){
                                          active_messages--;  
                                        }else{
                                            active_messages = 2;
                                        }
                                        switch(active_messages){
                                            case MESSAGES_ALL:                                            
                                                active_msg = all_messages;
                                                recived_iter = all_msg_num - recived_lines;
                                                if(recived_iter < 0){
                                                    recived_iter = 0;
                                                }
                                                msg_title = "All messages";
                                                break;
                                            case MESSAGES_PRIV:                                               
                                                active_msg = priv_messages;
                                                recived_iter = priv_msg_num - recived_lines;
                                                if(recived_iter < 0){
                                                    recived_iter = 0;
                                                }
                                                msg_title = "Private messages";
                                                break;
                                            case MESSAGES_ROOM:                                             
                                                active_msg = room_messages;
                                                recived_iter = room_msg_num - recived_lines;
                                                if(recived_iter < 0){
                                                    recived_iter = 0;
                                                }
                                                msg_title = "Room messages";
                                                break;
                                        }
                                        kill(getpid(), SIGUSR1);
                                        break;
                                    case CONTACTS:
                                        if(active_contacts == 0){
                                            active_contacts = 1;
                                        }else
                                            active_contacts = 0;
                                        switch(active_contacts){
                                            case CONTACTS_PEOPLE:                                               
                                                active_cnt = people_contacts;
                                                cnt_title = "People";
                                                contacts_iter = 0;
                                                break;
                                            case CONTACTS_ROOMS:                                              
                                                active_cnt = rooms_contacts;
                                                cnt_title = "Channels";
                                                contacts_iter = 0;
                                                break;
                                        }
                                        kill(getpid(), SIGUSR2);
                                        break;
                                }
                            break;
                        case KEY_RIGHT://w prawo (zmiana wyswietlanych wiadomosci lub kontaktow)
                                switch(active_window){
                                    case RECIVED:          
                                        if(active_messages < 2){
                                          active_messages++;  
                                        }else{
                                            active_messages = 0;
                                        }
                                        switch(active_messages){
                                            case MESSAGES_ALL:                                            
                                                active_msg = all_messages;
                                                recived_iter = all_msg_num - recived_lines;
                                                if(recived_iter < 0){
                                                    recived_iter = 0;
                                                }
                                                msg_title = "All messages";
                                                break;
                                            case MESSAGES_PRIV:                                               
                                                active_msg = priv_messages;
                                                recived_iter = priv_msg_num - recived_lines;
                                                if(recived_iter < 0){
                                                    recived_iter = 0;
                                                }
                                                msg_title = "Private messages";
                                                break;
                                            case MESSAGES_ROOM:                                             
                                                active_msg = room_messages;
                                                recived_iter = room_msg_num - recived_lines;
                                                if(recived_iter < 0){
                                                    recived_iter = 0;
                                                }
                                                msg_title = "Room messages";
                                                break;
                                        }
                                        kill(getpid(), SIGUSR1);
                                        break;
                                    case CONTACTS:
                                        if(active_contacts == 0){
                                            active_contacts = 1;
                                        }else
                                            active_contacts = 0;
                                        switch(active_contacts){
                                            case CONTACTS_PEOPLE:
                                                
                                                active_cnt = people_contacts;
                                                cnt_title = "People";
                                                contacts_iter = 0;
                                                break;
                                            case CONTACTS_ROOMS:
                                                
                                                active_cnt = rooms_contacts;
                                                cnt_title = "Channels";
                                                contacts_iter = 0;
                                                
                                                break;
                                        }
                                        kill(getpid(), SIGUSR2);
                                        break;
                                }
                            break;
			case 10://ENTER - wpisanie komendy (jezeli aktywne jest okno komend)
                            if(active_win == command_win){
                               get_command();
                            }
                            break;                            
                            
                        case '\t'://TAB - zmiana aktywnego okienka
                            active_window = (active_window + 1) % 3;
                            switch(active_window){
                                case RECIVED: active_win = recived_win;
                                        kill(getpid(), SIGUSR1);
                                        break;
                                case CONTACTS: active_win = contacts_win;
                                        kill(getpid(), SIGUSR2);
                                        break;
                                case COMMANDS: active_win = command_win;
                                        break;
                            }
                            break;
		}
                //wyswietlanie tresci
                if(get_channel() != NULL){
                    mvprintw(1,1,"                                                     ");
                    mvprintw(1,1,"Username: %s\tChannel: %s", get_nick(), get_channel());
                }else{
                    mvprintw(1,1,"Username: %s\tChannel: ", get_nick());
                }
                refresh();
                print_content(recived_win, msg_title, active_msg, recived_iter, recived_lines);
                print_content(contacts_win, cnt_title, active_cnt, contacts_iter, contacts_lines);
                command_window_drawing(command_win, "Command");
                //wyjscie z petli jezeli wpisano komende logout
                if(command_message != NULL){
                        print_command_message(command_message);
                        if(strcmp(command_message, "logged out") == 0){	
                            kill(child_pid, SIGTERM);    
                            break;
                        }
                }
	}
	clrtoeol();
	refresh();
//usuniecie okienek
        delwin(recived_win);
        delwin(contacts_win);
        delwin(command_win);
//zamkniecie ncurses
	endwin();
}

void init_strings(){
    int i;
    for(i = 0; i < MSG_BUF_SIZE; ++i){
        priv_messages[i] = '\0';
        room_messages[i] = '\0';
        all_messages[i] = '\0';
    }
    for(i = 0; i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER; ++i){
        people_contacts[i] = '\0';
        rooms_contacts[i] = '\0';
    }
}

void print_content(WINDOW *win, char* title, char** content, int begin, int nlines){
    int x = 1, y = 1, i = 0;
    //wclear(win);
    box(win,0,0);
    
    //jezeli okno jest aktywne to wyroznia tytul
    if(win == active_win){
        wattron(win, A_REVERSE);
        mvwprintw(win, 0, 2, "%s", title);
        wattroff(win, A_REVERSE);
    }else{
        mvwprintw(win, 0, 2, "%s", title);
    }
    
    //drukowanie tresci
    while((content[begin + i] != '\0') && (i < nlines)){
        mvwprintw(win, y + i, x, "%s", content[begin + i] );
        ++i;
    }
    
    wrefresh(win);
}

void command_window_drawing(WINDOW *win, char* title){
    box(win,0,0);
    
    //jezeli okno jest aktywne to wyroznia tytul
    if(win == active_win){
        wattron(win, A_REVERSE);
        mvwprintw(win, 0, 2, "%s", title);
        wattroff(win, A_REVERSE);
    }else{
        mvwprintw(win, 0, 2, "%s", title);
    }
    wrefresh(win);
}

void get_command(){
    kill(child_pid, SIGCONT);
    
    echo();
    nocbreak();
    curs_set(1);
    nodelay(active_win, FALSE);
    int i;
    for(i = 0; i<299; ++i){
        command[i] = '\0';
    }
    
    mvwgetstr(command_win, 1, 1, command);
    command[299] = '\0';
    command_message = parser(command, people_contacts);
    
    nodelay(active_win, TRUE);
    curs_set(0);
    cbreak();
    noecho();
    wclear(command_win);
    
    kill(child_pid, SIGSTOP);
}

void print_command_message(char* message){
    mvwprintw(command_win, 6, 1, "%s", message);
}

void add_message(char* message, int msg_type){      
    int i;
    //wszystkie wiadomosci
    if(all_msg_num < MSG_BUF_SIZE){
        all_messages[all_msg_num] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
        strcpy(all_messages[all_msg_num], message);
        all_msg_num++;
    }else{
        for(i = 0; i < MSG_BUF_SIZE - 1; ++i){
            free(all_messages[i]);
            all_messages[i] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
            strcpy(all_messages[i], all_messages[i+1]);
        }
        free(all_messages[MSG_BUF_SIZE - 1]);
        all_messages[MSG_BUF_SIZE - 1] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
        strcpy(all_messages[MSG_BUF_SIZE - 1], message);
    }
    recived_iter = all_msg_num - recived_lines;
    if(recived_iter < 0){
        recived_iter = 0;
    }
   
    //wiadomosc pokoju
    if(msg_type == PUBLIC){
        FILE *f = fopen(get_channel_messages_file_name(),"a");
        fprintf(f,"%s\n",message);
        fclose(f);
        
        if(room_msg_num < MSG_BUF_SIZE){
            room_messages[room_msg_num] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
            strcpy(room_messages[room_msg_num], message);
            room_msg_num++;
        }else{
            for(i = 0; i < MSG_BUF_SIZE - 1; ++i){
                    free(room_messages[i]);
                    room_messages[i] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
                    strcpy(room_messages[i], room_messages[i+1]);
            }
            free(room_messages[MSG_BUF_SIZE - 1]);
            room_messages[MSG_BUF_SIZE - 1] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
            strcpy(room_messages[MSG_BUF_SIZE - 1], message);
        }
        recived_iter = room_msg_num - recived_lines;
        if(recived_iter < 0){
            recived_iter = 0;
        }
    }
    
    //wiadomosc prywatna
    if(msg_type == PRIVATE){
        FILE *f = fopen(get_private_messages_file_name(),"a");
        fprintf(f,"%s\n",message);
        fclose(f);
        
        if(priv_msg_num < MSG_BUF_SIZE){
            priv_messages[priv_msg_num] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
            strcpy(priv_messages[priv_msg_num], message);
            priv_msg_num++;
        }else{
           for(i = 0; i < MSG_BUF_SIZE - 1; ++i){
                    free(priv_messages[i]);
                    priv_messages[i] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
                    strcpy(priv_messages[i], priv_messages[i+1]);
            }
            free(priv_messages[MSG_BUF_SIZE - 1]);
            priv_messages[MSG_BUF_SIZE - 1] = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
            strcpy(priv_messages[MSG_BUF_SIZE - 1], message); 
        }
        recived_iter = priv_msg_num - recived_lines;
        if(recived_iter < 0){
            recived_iter = 0;
        }
    }
    kill(getpid(), SIGUSR1);
}

void process_ipc_msgs(){
    MSG_CHAT_MESSAGE chmsg;
    MSG_USERS_LIST ulist;
    
    int i, k;
    //odbiór nowej wiadomosci
    if(msgrcv(get_my_que_id(), &chmsg, sizeof(MSG_CHAT_MESSAGE) - sizeof(long), MESSAGE, IPC_NOWAIT) != -1){
        char *m = (char*)malloc(sizeof(char) * (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10));
        for(i = 0; i < (MAX_MSG_LENGTH + USER_NAME_MAX_LENGTH +10); ++i){
            m[i] = '\0';
        }
        strcat(m, chmsg.send_time);
        strcat(m, " <");
        strcat(m, chmsg.sender);
        strcat(m, "> ");
        strcat(m, chmsg.message);
        add_message(m, chmsg.msg_type);
    }
    
    //odbior listy userow
    if(msgrcv(get_my_que_id(), &ulist, sizeof(MSG_USERS_LIST) - sizeof(long), USERS_LIST_STR, IPC_NOWAIT) != -1){ 
        for(i = 0; i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER; ++i){
            people_contacts[i] = NULL;
        }
        ppl_cnt_num = 0;
        i=0;
        while(ulist.users[i][0] != '\0' && i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER){
            people_contacts[i] = (char*)malloc(USER_NAME_MAX_LENGTH * sizeof(char));
            k = 0;
            while(ulist.users[i][k] != '\0'){
               people_contacts[i][k] = ulist.users[i][k];
               k++;
            }
            ppl_cnt_num++;
            i++;
        }
        kill(getpid(), SIGUSR2);
    }
    
    //lista pokoi
    if(msgrcv(get_my_que_id(), &ulist, sizeof(MSG_USERS_LIST) - sizeof(long), ROOMS_LIST_STR, IPC_NOWAIT) != -1){ 
        for(i = 0; i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER; ++i){
            rooms_contacts[i] = NULL;
        }
        room_cnt_num = 0;
        i=0;
        while(ulist.users[i][0] != '\0' && i < MAX_SERVERS_NUMBER * MAX_USERS_NUMBER){
            rooms_contacts[i] = (char*)malloc(USER_NAME_MAX_LENGTH * sizeof(char));
            k = 0;
            while(ulist.users[i][k] != '\0'){
                rooms_contacts[i][k] = ulist.users[i][k];
                k++;
            }
            room_cnt_num++;
            i++;
        }
        kill(getpid(), SIGUSR2);
    }
}

void process_server_responses(){
    MSG_RESPONSE resp;
    if(msgrcv(get_my_que_id(), &resp, sizeof(MSG_RESPONSE) - sizeof(long), RESPONSE, IPC_NOWAIT) != -1){
        if(resp.response_type == PING){
            MSG_REQUEST req;
            req.type = REQUEST;
            req.request_type = PONG;
            strcpy(req.user_name, get_nick());
            msgsnd(get_serv_que_id(), &req, sizeof(MSG_REQUEST) - sizeof(long), 0);
        }
        if(resp.response_type == MSG_SEND){
            
        }
        if(resp.response_type == MSG_NOT_SEND){
            int i;
            for (i = 0; i < RESPONSE_LENGTH * 2; ++i){
                command_message2[i] = '\0';
            }
            strcpy(command_message2, "Message not send: ");
            strcat(command_message2, resp.content);
            print_command_message(command_message2);
            
        }
        if((resp.response_type == ENTERED_ROOM_SUCCESS) || (resp.response_type == CHANGE_ROOM_SUCCESS)){
            int i;
            for(i = 0; i < RESPONSE_LENGTH * 2; ++i){
                command_message2[i] = '\0';
            }
            strcpy(get_channel(), get_temp_channel());
            strcpy(command_message2, "You've just entered to channel ");
            strcat(command_message2, get_temp_channel());
            print_command_message(command_message2);
        }
        if((resp.response_type == ENTERED_ROOM_FAILED) || (resp.response_type == CHANGE_ROOM_FAILED)){
            int i;
            for (i = 0; i < RESPONSE_LENGTH * 2; ++i){
                command_message2[i] = '\0';
            }
            strcpy(command_message2, "Entering to channel failed: ");
            strcat(command_message2, resp.content);
            print_command_message(command_message2);
        } 
        if(resp.response_type == LEAVE_ROOM_SUCCESS){
            
        }
        if(resp.response_type == LEAVE_ROOM_FAILED){
            
        }
    }
}

void process_heartbeat(){
    MSG_RESPONSE resp;
    if(msgrcv(get_my_que_id(), &resp, sizeof(MSG_RESPONSE) - sizeof(long), RESPONSE, IPC_NOWAIT) != -1){
        if(resp.response_type == PING){
            MSG_REQUEST req;
            req.type = REQUEST;
            req.request_type = PONG;
            strcpy(req.user_name, get_nick());
            msgsnd(get_serv_que_id(), &req, sizeof(MSG_REQUEST) - sizeof(long), 0);
        }else{
            msgsnd(get_my_que_id(), &resp, sizeof(MSG_RESPONSE) - sizeof(long), 0);
        }        
    }
}

void refr_recived(){
    wclear(recived_win);
    box(recived_win,0,0);
    wrefresh(recived_win);
}

void refr_contacts(){
    wclear(contacts_win);
    box(contacts_win,0,0);
    wrefresh(contacts_win);
}

void get_contacts_lists(){
        MSG_REQUEST req;
        req.type = REQUEST;
        req.request_type = USERS_LIST;
        strcpy(req.user_name, get_nick());
        msgsnd(get_serv_que_id(), &req, sizeof(MSG_REQUEST) - sizeof(long), 0);
        
        req.request_type = ROOMS_LIST;
        msgsnd(get_serv_que_id(), &req, sizeof(MSG_REQUEST) - sizeof(long), 0);
        
        alarm(GETTING_CONTACTS_TIME_INTERVAL);
}