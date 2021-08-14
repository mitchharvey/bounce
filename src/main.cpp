#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <ncurses.h>

bool killgame = false;		// Exit if true
int MAX_FPS = 30;		// Fps
char msg[50];			// for storing data before printing

int h, w;			// h and w of window
const char *me = "@";		// player 'sprite'
const char *wall = "#";		// wall 'sprite'
int clicks = 0;			// total registered clicks

// PLAYER VARS
int xi, yi = 0;			// integer values of player coords
double x, y = 0;		// player coords
double vx, vy = 0;		// velocity
double mvx, mvy = 0;		// max velocity
double rx, ry = 0;		// velocity friction (only use x currently)
double gx, gy = 0;		// gravity acceleration (only use y currently)
double bvx, bvy = 0;		// bounce threshold
double bdvx, bdvy = 0;		// bounce dampening
bool onGround = true;		// if on the ground (ie not falling) or not

// DEBUG
double lastx, lasty, lastvx, lastvy = 0;

// MAP VARS
#define levels 10
#define mapx 70
#define mapy 40
int map[levels][mapx][mapy] = { 0 };

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

	// PHYSICS CONSTANTS //
	gy = 0.05;		// gravity
	rx = 0.5;		// friction on ground
	bvy = 0.25;
	bdvy = 0.7;
	bdvx = 0.3;
	mvx = 1.5; //1
	mvy = 1.5; //1

	// MAPS //
	gen.seed(std::chrono::system_clock::now().time_since_epoch().count());

	for (int lvl = 0; lvl < levels; lvl++) {
		for (int i = 0; i < 12; i++) {
			int r = gen() % (mapx - 12);
			for (int j = 0; j < 10; j++) {
				map[lvl][r + j][(3 * i) + (r % 3) - 1] = 1;
			}
		}
	}
}

void cleanup()
{
	//Restore normal terminal functions
	endwin();
}

/*double abs(double a) {
  return a < 0 ? -a : a;
}*/

double min(double a, double b)
{
	return a < b ? a : b;
}

double max(double a, double b)
{
	return a > b ? a : b;
}

double clamp(double min, double max, double in)
{
	if (in >= max) {
		return max;
	} else if (in <= min) {
		return min;
	} else {
		return in;
	}
}

double approach(double tgt, double amt, double in)
{
	return in > tgt ? max(tgt, in - amt) : min(tgt, in + amt);
}

int level()
{
	return ((int)y / mapy);
}

bool isTouchingWall() {
  int l = map[level()][xi+1][yi];
  int r = map[level()][xi+1][yi];
  return (l && !r) || (!l && r) || xi == mapx-1|| xi == 1;
}

bool isOnGround()
{
	/*if (vx != 0) {
	   return false;
	   } */

	// Only "on ground" when falling onto it
	if (vy > 0) {
		return false;
	}

	// Check for platform or bottom of world
	if ((int)y <= 1 || map[level()][xi][yi - 1]) {
		return true;
	}

        return false;
}

bool canJump()
{
	return isOnGround() && vx == 0;
}

void updatePos()
{
	// Clamp velocity to max
	vx = clamp(-mvx, mvx, vx);
	vy = clamp(-mvy, mvy, vy);

	// Move character and Clamp character to limits
	x = clamp(1, mapx - 1, x + vx);
	y = clamp(1, (mapy * levels) - 1, y + vy);

	// adjust to integer
	xi = (int)x;
	yi = (int)y % mapy;

	// Gravity
	vy -= gy;

        // Bounce off walls and platforms
        if (isTouchingWall())
          vx = -vx;

	// Do ground check AFTER move
	onGround = isOnGround();
	if (onGround) {
		// check for bounce
		if (abs(vy) > bvy) {
			vy = approach(0, bdvy, -vy);
			vx = approach(0, rx, vx);
		} else {
			vy = 0;
			vx = approach(0, rx, vx);
		}
	}

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
					clicks++;
					if (onGround) {
						vx += (double)(event.x - xi) / 20.0;
						printf("%d - %d = %f\n", event.x, xi, vx);
						vy += (double)((h - event.y) - yi) / 10.0;
						printf("(%d - %d) - %d = %f\n", h, event.y, yi, vy);
						lastx = event.x;
						lasty = event.y;
						lastvx = vx;
						lastvy = vy;
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
	wclear(stdscr);

	// Draw stats
	wmove(stdscr, 0, 0);
	sprintf(msg, "%d %d | %d %d | %4.4f %4.4f", clicks, onGround, xi, yi, vx, vy);
	waddstr(stdscr, msg);
	wmove(stdscr, 1, 0);
	sprintf(msg, "%4.4f %4.4f | %4.4f %4.4f", lastx, lasty, lastvx, lastvy);
	waddstr(stdscr, msg);

	// Draw player
	wmove(stdscr, h - yi, xi);
	waddstr(stdscr, me);

	// Draw map
	for (int i = 0; i < mapx; i++) {
		for (int j = 0; j < mapy; j++) {
			if (map[level()][i][j]) {
				wmove(stdscr, h - j, i);
				waddstr(stdscr, wall);
			}
		}
	}
}

void update()
{
	//Update height and width
	getmaxyx(stdscr, h, w);
        h--;
        w--;

	if (h < mapy || w < mapx) {
		cleanup();
		printf("Screen must be %d x %d\n", mapx, mapy);
		exit(1);
	}
	// Clear screen

	// move char
	updatePos();

	getInput();
	showOutput();

	//Update screen
	wrefresh(stdscr);

	//Move cursor to origin
	wmove(stdscr, 0, 0);

	//wait
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
