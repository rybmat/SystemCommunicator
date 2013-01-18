#include "ui.h"

//ilość wyswietlanych wiadomosci posczegolnego typu
int all_msg_num = 0;
int priv_msg_num = 0;
int room_msg_num = 0;

//liczba lini mozliwych do zadrukowania i iterator po tablicy z trescia dla okienka wiadomosci
int recived_lines = RECIVED_HEIGHT - 2;
int recived_iter;

//liczba lini mozliwych do zadrukowania i iterator po tablicy z trescia dla okienka kontaktow
int contacts_iter = 0;
int contacts_lines = CONTACTS_HEIGHT - 2;

//tytuly okienek wiadomosci i kontaktow
 char* msg_title;
 char* cnt_title;
 
 char* command_message;

void ui_main(){	
    
        all_msg_num = 0;
        priv_msg_num = 0;
        room_msg_num = 0;
    
    //poczatek rysowania okienek
        int startx = 1;
        int starty = 1;
	
        //wcisniety klawisz klawiatury
        int c;
        
        //iteratory: aktywne okno, aktywne kontakty, aktywne wiadomosci
        int active_window = 0;
        int active_contacts = 0;
        int active_messages = 0;
        
        //zmienna wyjscia z glownej petli
        short exit = 0;
       
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
        //petla glowna
	while(1){
		keypad(active_win, TRUE);
                nodelay(active_win, TRUE);
                c = wgetch(active_win);
		switch(c){//sprawdza ktory klawisz wcisnieto
			case KEY_UP://w gore (przesuwanie tresci)
				switch(active_window){
                                    case RECIVED:
                                        if(recived_iter > 0){
                                            recived_iter--;
                                        }
                                        break;
                                    case CONTACTS:
                                        if(contacts_iter > 0){
                                            contacts_iter--;
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
                                        break;
                                    case CONTACTS:                                     
                                        switch(active_contacts){
                                            case CONTACTS_PEOPLE:
                                                if(contacts_iter + contacts_lines <= sizeof(people_contacts)/sizeof(char*)){
                                                        contacts_iter++;
                                                }
                                                break;
                                            case CONTACTS_ROOMS:
                                                if(contacts_iter + contacts_lines <= sizeof(rooms_contacts)/sizeof(char*)){
                                                        contacts_iter++;
                                                }
                                                break;
                                        }
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
                                                contacts_iter = sizeof(people_contacts)/sizeof(char*) - contacts_lines;
                                                break;
                                            case CONTACTS_ROOMS:                                              
                                                active_cnt = rooms_contacts;
                                                cnt_title = "Rooms";
                                                contacts_iter = sizeof(rooms_contacts)/sizeof(char*) - contacts_lines;
                                                break;
                                        }
                                        
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
                                                contacts_iter = sizeof(people_contacts)/sizeof(char*) - contacts_lines;
                                                break;
                                            case CONTACTS_ROOMS:
                                                
                                                active_cnt = rooms_contacts;
                                                cnt_title = "Rooms";
                                                contacts_iter = sizeof(rooms_contacts)/sizeof(char*) - contacts_lines;
                                                
                                                break;
                                        }
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
                                        break;
                                case CONTACTS: active_win = contacts_win;
                                        break;
                                case COMMANDS: active_win = command_win;
                                        break;
                            }
                            break;
		}
                print_content(recived_win, msg_title, active_msg, recived_iter, recived_lines);
                print_content(contacts_win, cnt_title, active_cnt, contacts_iter, contacts_lines);
                command_window_drawing(command_win, "Command");
                if(command_message != NULL){
                        print_command_message(command_message);
                        if(strcmp(command_message, "logged out") == 0){	
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
    wclear(win);
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
    command_message = parser(command);
    
    nodelay(active_win, TRUE);
    curs_set(0);
    cbreak();
    noecho();
    wclear(command_win);
}

void print_command_message(char* message){
    mvwprintw(command_win, 6, 1, "%s", message);
}

void add_message(char* message, int msg_type){      
    int i;
    
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
}