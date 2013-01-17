#include "ui.h"



void ui_main(){	
    //poczatek rysowania okienek
        int startx = 1;
        int starty = 1;
	
        //wcisniety klawisz klawiatury
        int c;
        
       //tytuly okienek wiadomosci i kontaktow
        char* msg_title;
        char* cnt_title;
        
        //iteratory: aktywne okno, aktywne kontakty, aktywne wiadomosci
        int active_window = 0;
        int active_contacts = 0;
        int active_messages = 0;
        
        //liczba lini mozliwych do zadrukowania i iterator po tablicy z trescia dla okienka wiadomosci
        int recived_lines = RECIVED_HEIGHT - 2;
        int recived_iter;
        //liczba lini mozliwych do zadrukowania i iterator po tablicy z trescia dla okienka kontaktow
        int contacts_iter = 0;
        int contacts_lines = CONTACTS_HEIGHT - 2;
        
        //zmienna wyjscia z glownej petli
        short exit = 0;
       
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
        recived_iter = sizeof(all_messages)/sizeof(char*) - recived_lines;
        msg_title = "All messages";
        cnt_title = "People";
        
        print_content(recived_win, msg_title, active_msg, recived_iter, recived_lines);
        print_content(contacts_win, cnt_title, active_cnt, contacts_iter, contacts_lines);
        command_window_drawing(command_win, "Command");	      
        
        //petla glowna
	while(1){
		keypad(active_win, TRUE);
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
                                                if(recived_iter + recived_lines <= sizeof(all_messages)/sizeof(char*)){
                                                        recived_iter++;
                                                }
                                                break;
                                            case MESSAGES_PRIV:
                                                if(recived_iter + recived_lines <= sizeof(priv_messages)/sizeof(char*)){
                                                        recived_iter++;
                                                }
                                                break;
                                            case MESSAGES_ROOM:
                                                if(recived_iter + recived_lines <= sizeof(room_messages)/sizeof(char*)){
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
                                                recived_iter = sizeof(all_messages)/sizeof(char*) - recived_lines;
                                                msg_title = "All messages";
                                                break;
                                            case MESSAGES_PRIV:
                                                
                                                active_msg = priv_messages;
                                                recived_iter = sizeof(priv_messages)/sizeof(char*) - recived_lines;
                                                msg_title = "Private messages";
                                                break;
                                            case MESSAGES_ROOM:
                                                
                                                active_msg = room_messages;
                                                recived_iter = sizeof(room_messages)/sizeof(char*) - recived_lines;
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
                                                recived_iter = sizeof(all_messages)/sizeof(char*) - recived_lines;
                                                msg_title = "All messages";
                                                break;
                                            case MESSAGES_PRIV:
                                                
                                                active_msg = priv_messages;
                                                recived_iter = sizeof(priv_messages)/sizeof(char*) - recived_lines;
                                                msg_title = "Private messages";
                                                break;
                                            case MESSAGES_ROOM:
                                                
                                                active_msg = room_messages;
                                                recived_iter = sizeof(room_messages)/sizeof(char*) - recived_lines;
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
                        case 27://ESC - wyjscie z petli
                            exit = 1;
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

                if(exit){	
			break;
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
    while((content[begin + i] != NULL) && (i < nlines)){
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
    mvwgetstr(command_win, 1, 1, command);
    cbreak();
    noecho();
    wclear(command_win);
}

void print_command_message(char* message){
    mvwprintw(command_win, 5, 1, "%s", message);
}
