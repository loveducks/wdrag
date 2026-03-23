#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <menu.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof (a) / sizeof(a[0]))
#define CTRLD 4

typedef struct{
	char txt[CTRLD+1];
} AppState;

AppState app_state;

const char *choices[] = {
	"1. Write",
	"2. Cancel",
	"3. Exit",
};

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void mouse_mon(int height, int width, int starty, int startx, WINDOW *my_win);
bool check_coll(int height, int width, int starty, int startx, int m_x, int m_y);
void spawn_menu();
void handle_ip(int size);

int main(void) {
	WINDOW *my_win;
	int startx, starty, width, height;
	int ch;

	initscr();
	cbreak(); /*line buffering*/

	keypad(stdscr, TRUE);

	height = 3;
	width = 10;
	// center
	starty = (LINES - height) / 2;
	startx = (COLS - width) / 2;

	refresh();
	my_win = create_newwin(height, width, starty, startx);
	mouse_mon(height, width, starty, startx, my_win);

	// some lingering effects if you dont
	endwin();

	return 0;
}

void handle_ip(int size){
	char c;
	int i = 0;

	move(1, 1);

	while(i < size) {
		c = getch();
		app_state.txt[i++] = c;
	}

	app_state.txt[i] = '\0';
}

void spawn_menu(){
	ITEM **my_items;
	MENU *my_menu;
	int c;
	int n_choices, i;
	n_choices = ARRAY_SIZE(choices);
	my_items = (ITEM **) calloc((size_t) (n_choices + 1), sizeof(ITEM *));

	for (int i=0; i<n_choices; ++i)
	{
		my_items[i] = new_item(choices[i], "");
	}

	my_items[n_choices] = (ITEM *) NULL;

	my_menu = new_menu((ITEM **) my_items);
	set_menu_mark(my_menu, " * ");
	set_current_item(my_menu, my_items[0]);
	post_menu(my_menu);
	refresh();

	while (((c = getch()) != '\n')) {
		switch(c){
			case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
				break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
				break;
			default:
				break;
		}
	}
	ITEM* cur = current_item(my_menu);
	const char* name = item_name(cur);

	if (strcmp(name, "1. Write") == 0) {
		handle_ip(CTRLD);
	}
	mvprintw(0, 0, "");
	refresh();
	unpost_menu(my_menu);
	free_menu(my_menu);

	for (int i=0; i<n_choices; ++i)
	{
		free_item(my_items[i]);
	}

	free(my_items);
}

bool check_coll(int height, int width, int starty, int startx, int m_x, int m_y) {
	if (
		startx <= m_x && m_x <= startx + width &&
		starty <= m_y && m_y <= starty + height
	) {
		return true;
	}
	return false;
}

void mouse_mon(int height, int width, int starty, int startx, WINDOW *my_win) {
	MEVENT event;

	mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
	printf("\033[?1003h\n"); // Makes the terminal report mouse movement events
	int start_x_init = startx;
	int start_y_init = starty;

	bool move = false;

	for(;;) {
		int c = getch();

		if (c == '\n')
			break;

		if (c == KEY_MOUSE) {
			if (getmouse(&event) == OK) {
				if (event.bstate & BUTTON1_PRESSED) {
					int m_x = event.x;
					int m_y = event.y;
					bool is_it_in = check_coll(height, width, starty, startx, m_x, m_y);
					if (is_it_in)
						move = true;
				}
				if (event.bstate & BUTTON1_RELEASED) {
					move = false;
				}
				if (move) {
					startx = event.x;
					starty = event.y;
					mvwin(my_win, starty, startx);
				}

				//mvprintw(0, 0, "x=%d y=%d dragging=%d", event.x, event.y, move);
			}
		}

		if (c == 'M' || c == 'm') {
			move = false;
			spawn_menu();
			refresh();
			destroy_win(my_win);
			my_win = create_newwin(height, width, starty, startx);
		}

		mvprintw(starty + 1, startx + 1, "%s", app_state.txt);
		wrefresh(my_win);
		refresh();

	}

	printw("\033[?100312\n");
}

WINDOW *create_newwin(int height, int width, int starty, int startx) {
	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);

	wrefresh(local_win);

	return local_win;
}

void destroy_win(WINDOW *local_win) {
	/** This won't produce the desired
	 * result of erasing the window. It will leave its four corners
	 * and so an ugly remnant of window. (which is kind of what we want, but not needed anymore)
	*/
	// box(local_win, ' ', ' ');
	wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(local_win);
	delwin(local_win);
}
