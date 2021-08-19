#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <ncurses.h>
#include "lib/Actor/Actor.cpp"
#include "lib/Math/SimpleMath.cpp"

bool killgame = false;		// Exit if true
int MAX_FPS = 30;		// Fps
long long lastFrameTime, frameBeginTime, totalFrameTime = 0;
long long FRAME_DELAY = 20;
double FPS_SPEED_MULT = 2;	// multiplier for normalized speed
char msg[50];			// for storing data before printing

int h, w, o;			// h and w of window
const char *me = "@";		// player 'sprite'
const char *wall = "#";		// wall 'sprite'
const char *bord = "+";		// border 'sprite'
int jumps = 0;			// total registered jumps

// PLAYER VARS
Actor *a;

// DEBUG
double lastx, lasty, lastvx, lastvy = 0;

// MAP VARS
#define NUM_LEVELS 10
#define NUM_MAP_X 70
#define NUM_MAP_Y 40
std::vector < std::vector < std::vector < int >>>map(NUM_LEVELS, std::vector < std::vector < int >>(NUM_MAP_X, std::vector < int >(NUM_MAP_Y, 0)));

std::minstd_rand gen;

void setup()
{
	// NCURSES //
	//Init ncurses window
	initscr();
	//Character input, nonblocking
	cbreak();
	nodelay(stdscr, TRUE);
	//Don 't show input on screen
	noecho();
	//Don't show cursor
	curs_set(0);
	//disable scroll
	scrollok(stdscr, false);
	//Capture special characters
	keypad(stdscr, TRUE);
	//Capture mouse left clicks
	//mousemask(BUTTON1_PRESSED | BUTTON1_RELEASED | BUTTON1_CLICKED, NULL);
	mousemask(ALL_MOUSE_EVENTS, NULL);
	// Colors
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);

	// Wait for window to be right size:
	// while (true) {
	//      getmaxyx(stdscr, h, w);
	//      h--;
	//      w--;

	//      if (h < NUM_MAP_Y || w < NUM_MAP_X) {
	//              // cleanup();
	//              printf("Screen must be %d x %d\n", NUM_MAP_X, NUM_MAP_Y);
	//              std::this_thread::sleep_for(std::chrono::seconds(1));
	//              // exit(1);
	//      } else {
	//              break;
	//      }
	// }

	// MAPS //
	gen.seed(std::chrono::system_clock::now().time_since_epoch().count());

	// init map
	// already done when defined

	// add cells
	for (int lvl = 0; lvl < NUM_LEVELS; lvl++) {
		for (int i = 0; i < 12; i++) {
			int r = gen() % (NUM_MAP_X - 12);
			for (int j = 0; j < 10; j++) {
				map[lvl][r + j][(3 * i) + (r % 3) - 1] = 1;
			}
		}
	}

	//PLAYER
	a = new Actor(map);

	// PHYSICS CONSTANTS //
	a->setMultiplier(1);
	a->setMaxVelocity(1, 1);
	a->setGravity(0, -0.05);
	a->setFriction(0.7, 0);
	a->setBounceThreshold(0, 0.5);
	a->setBounceFriction(0.5, 0.7);
}

void cleanup()
{
	//Restore normal terminal functions
	endwin();
}

// Check for input from user
void getInput()
{
	MEVENT event;
	int ch;
	if ((ch = wgetch(stdscr)) != ERR) {
		switch (ch) {
		case KEY_MOUSE:
			if (getmouse(&event) == OK) {
				if (event.bstate & BUTTON1_CLICKED) {
					if (a->isOnGround()) {
						jumps++;
						int vxa = (double)((event.x - o) - a->xi()) / 5.0;
						printf("%d - %d = %f\n", event.x, a->xi(), a->vx);
						int vya = (double)((h - event.y) - a->yi()) / 5.0;
						printf("(%d - %d) - %d = %f\n", h, event.y, a->yi(), a->vy);
						a->addVelocity(vxa, vya);
						lastx = event.x;
						lasty = event.y;
						lastvx = a->vx;
						lastvy = a->vy;
					}
				}
			}
			break;
		case 'Q':
		case 'q':
		case KEY_BACKSPACE:
		case KEY_DOWN:
			killgame = true;
			break;
		}
	}
}

void showOutput()
{
	// Clear screen
	wclear(stdscr);

	// Draw stats
	wmove(stdscr, 0, 0);
	sprintf(msg, "%d %d | %4.4f %4.4f | %d %d | %d %d", jumps, a->isOnGround(), a->vx, a->vy, a->xi(), a->yi(), a->level(), NUM_LEVELS);
	waddstr(stdscr, msg);
	wmove(stdscr, 1, 0);
	sprintf(msg, "%4.4f %4.4f | %4.4f %4.4f", lastx, lasty, lastvx, lastvy);
	waddstr(stdscr, msg);
	totalFrameTime = frameBeginTime - lastFrameTime;
	FRAME_DELAY = (1000000 / MAX_FPS) - totalFrameTime;
	wmove(stdscr, 2, 0);
	sprintf(msg, "%lld | %lld | %4.4f FPS", totalFrameTime, FRAME_DELAY, (double)1000000 / (double)(FRAME_DELAY + totalFrameTime));
	waddstr(stdscr, msg);

	// Draw map
	for (int j = 0; j < NUM_MAP_Y; j++) {
		wmove(stdscr, h - j, o);
		waddstr(stdscr, bord);
		wmove(stdscr, h - j, o + NUM_MAP_X);
		waddstr(stdscr, bord);
	}
	for (int i = 0; i < NUM_MAP_X; i++) {
		wmove(stdscr, h - NUM_MAP_Y, o + i);
		waddstr(stdscr, bord);
		for (int j = 0; j < NUM_MAP_Y; j++) {
			if (map[a->level()][i][j]) {
				wmove(stdscr, h - j, o + i);
				waddstr(stdscr, wall);
			}
		}
	}

	// Draw player
	wmove(stdscr, h - a->yi(), o + a->xi());
	attron(COLOR_PAIR(1));
	waddstr(stdscr, me);
	attroff(COLOR_PAIR(1));

	// Refresh screen
	wrefresh(stdscr);
}

void update()
{
	lastFrameTime = frameBeginTime;
	frameBeginTime = std::chrono::system_clock::now().time_since_epoch().count();

	//Update height and width
	getmaxyx(stdscr, h, w);
	h--;
	w--;
	// offset for term width
	o = (w - NUM_MAP_X) / 2;

	// if (h < NUM_MAP_Y || w < NUM_MAP_X) {
	//      cleanup();
	//      printf("Screen must be %d x %d\n", NUM_MAP_X, NUM_MAP_Y);
	//      exit(1);
	// }
	// move char
	a->step();

	getInput();

	showOutput();

	//Move cursor to origin
	//wmove(stdscr, 0, 0);

	//long long margin = (MAX_FPS / 1000)

	//wait
	std::this_thread::sleep_for(std::chrono::microseconds(FRAME_DELAY));
}

int main()
{
	printf("Hello world\n");

	setup();

	while (!killgame) {
		update();
	}

	cleanup();
}
