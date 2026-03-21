#include <stdbool.h>
#include <stdio.h>
#include <ncurses.h>

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void mouse_mon(int height, int width, int starty, int startx, WINDOW *my_win);
bool check_coll(int height, int width, int starty, int startx, int m_x, int m_y);

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

	bool move = false;

	for(;;) {
		int c = getch();

		if (c == '\n')
			break;

		char buffer[512];
		size_t max_size = sizeof(buffer);

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

				mvprintw(0, 0, "x=%d y=%d dragging=%d", event.x, event.y, move);
			}
		}

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
	box(local_win, ' ', ' ');
	// wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(local_win);
	delwin(local_win);
}
