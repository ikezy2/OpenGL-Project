#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define WINDOW_X1 (1000)
#define WINDOW_Y1 (1000)
#define WINDOW_NAME1 "3DConnect4_Board"
#define TEXTURE_HEIGHT (128)
#define TEXTURE_WIDTH (128)

void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void glut_idle();

void draw_pole();
void draw_box();
void draw_thin_pole();
void switch_focus(char input);
void enter(int x, int z);
void reset_board(int option);
void check_for_win();
void check_for_reach();
void set_texture();
void displayUI();
void print_text(int x, int y, char* string);

// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 20.0;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;
int board[4][4][4];
int red_hint[4][4][4];
int yellow_hint[4][4][4];
int focus[2];
int turn_count;

GLdouble point1[] = {0.4, 0, 0.4};
GLdouble point2[] = {0.4, 0, -0.4};
GLdouble point3[] = {-0.4, 0, -0.4};
GLdouble point4[] = {-0.4, 0, 0.4};
GLdouble point5[] = {-2, 2, 0};
GLdouble point6[] = {-2, 6, 0};
GLdouble point7[] = {2, 2, 0};
GLdouble point8[] = {2, 6, 0};
GLdouble point9[] = {0, 2, -2};
GLdouble point10[] = {0, 6, -2};
GLdouble point11[] = {0, 2, 2};
GLdouble point12[] = {0, 6, 2};
GLuint g_TextureHandles[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int counter = 0;
int counter2 = 0;
int flag = 0;
int game_end = 0;
int red_win_count = 0;
int yellow_win_count = 0;
int show_hint = 0;

int main(int argc, char *argv[]){

	reset_board(0);

	/* OpenGLの初期化 */
	init_GL(argc, argv);

	/* このプログラム特有の初期化 */
	init();

	/* コールバック関数の登録 */
	set_callback_functions();

	/* メインループ */
	glutMainLoop();

	return 0;
}

void init_GL(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_X1,WINDOW_Y1);
	glutCreateWindow(WINDOW_NAME1);
}

void init(){
	glClearColor(0, 0, 0, 0);
	glGenTextures(14, g_TextureHandles);

	for(int i = 0; i < 14; i++){
		glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	set_texture();
}

void set_callback_functions(){
	glutDisplayFunc(glut_display);
	glutKeyboardFunc(glut_keyboard);
	glutMouseFunc(glut_mouse);
	glutMotionFunc(glut_motion);
	glutPassiveMotionFunc(glut_motion);
	glutIdleFunc(glut_idle);
}

void glut_keyboard(unsigned char key, int x, int y){
	switch(key){
	case 'q':
	case 'Q':
	case '\033':
		exit(0);
	
	case 'a':
		switch_focus('a');
	break;
	case 'd':
		switch_focus('d');
	break;
	case 's':
		switch_focus('s');
	break;
	case 13:
		if (game_end == 0) enter(focus[0], focus[1]);
		check_for_win();
		check_for_reach();
		//if (game_end == 1 && turn_count % 2 == 0) yellow_win_count += 1;
		//if (game_end == 1 && turn_count % 2 == 1) red_win_count += 1;
	break;
	case 'r':
		reset_board(0);
	break;
	case 'c':
		reset_board(1);
	break;
	case 'h':
		if (show_hint == 1) show_hint = 0;
		else if (show_hint == 0) show_hint = 1;
		check_for_reach();
	break;
	}

	glutPostRedisplay();
}

void glut_mouse(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON){
		if(state == GLUT_UP){
			g_isLeftButtonOn = false;
		}else if(state == GLUT_DOWN){
			g_isLeftButtonOn = true;
		}
	}

	if(button == GLUT_RIGHT_BUTTON){
		if(state == GLUT_UP){
			g_isRightButtonOn = false;
		}else if(state == GLUT_DOWN){
			g_isRightButtonOn = true;
		}
	}
}

void glut_motion(int x, int y){
	static int px = -1, py = -1;
	if(g_isLeftButtonOn == true){
		if(px >= 0 && py >= 0){
			g_angle1 += (double)-(x - px)/20;
		}
		px = x;
		py = y;
	}else if(g_isRightButtonOn == true){
		if(px >= 0 && py >= 0){
			g_distance += (double)(y - py)/20;
		}
		px = x;
		py = y;
	}else{
		px = -1;
		py = -1;
	}
	glutPostRedisplay();
}

void glut_display(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, 1.0, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0, 0.5*g_distance, 0.75*g_distance, 0, 0, 0, 0, 1, 0);

	glRotatef(-g_distance * g_angle1, 0, 1, 0); //rotate on y axis
	glRotatef(g_distance * g_angle2, 1, 0, 0); //rotate on x axis <=change this

	GLfloat lightpos1[] = {0, 10, 0};
	GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat ambient[] = {0.4, 0.4, 0.4, 1.0};

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	glPushMatrix();
	glTranslatef(0, -1, 0);
	draw_box();
	glPopMatrix();

	for (int x = -2; x <= 2; ++x) {
		for (int z = -2; z <= 2; ++z) {
			if (abs(x) + abs(z) == 4) {
				glPushMatrix();
				glTranslatef(x, 0, z);
				draw_pole();
				glPopMatrix();
			}
		}
	}

	for (int x = -2; x <= 2; ++x) {
		for (int y = -2; y <= 2; ++y) {
			if (abs(x) + abs(y) == 4) {
				glPushMatrix();
				glTranslatef(x, y, 0);
				glRotatef(90, 1, 0, 0);
				draw_pole();
				glPopMatrix();
			}
		}
	}

	for (int y = -2; y <= 2; ++y) {
		for (int z = -2; z <= 2; ++z) {
			if (abs(y) + abs(z) == 4) {
				glPushMatrix();
				glTranslatef(0, y, z);
				glRotatef(90, 0, 0, 1);
				draw_pole();
				glPopMatrix();
			}
		}
	}

	for (int x = -1; x <= 1; ++x) {
		for (int z = -1; z <= 1; ++z) {
			if (abs(x) + abs(z) == 2) {
				glPushMatrix();
				glTranslatef(x, -0.5, z);
				glScalef(1, 0.75, 1);
				draw_pole();
				glPopMatrix();
			}
		}
	}

	for (int x = -1; x <= 1; ++x) {
		for (int y = -2; y <= 1; ++y) {
			if ((y == -2 || y == 1) && (x == -1 || x == 1)) {
				glPushMatrix();
				glTranslatef(x, y, 0);
				glScalef(1, 1, 0.5);
				glRotatef(90, 1, 0, 0);
				draw_pole();
				glPopMatrix();
			}
		}
	}

	for (int y = -2; y <= 1; ++y) {
		for (int z = -1; z <= 1; ++z) {
			if ((y == -2 || y == 1) && (z == -1 || z == 1)) {
				glPushMatrix();
				glTranslatef(0, y, z);
				glScalef(0.5, 1, 1);
				glRotatef(90, 0, 0, 1);
				draw_pole();
				glPopMatrix();
			}
		}
	}

	glPushMatrix();
	if (turn_count % 2 == 0) glColor3d(1, 0, 0);
	if (turn_count % 2 == 1) glColor3d(1, 1, 0);
	glTranslatef(focus[1] - 1.5, 2.01, focus[0] - 1.5);
    glBegin(GL_POLYGON);
    glVertex3dv(point1);
    glVertex3dv(point2);
    glVertex3dv(point3);
    glVertex3dv(point4);
    glEnd();
	glPopMatrix();

	glPushMatrix();
	if (turn_count % 2 == 0) glColor3d(1, 0, 0);
	if (turn_count % 2 == 1) glColor3d(1, 1, 0);
	glTranslatef(focus[1] - 1.5, 0, focus[0] - 1.5);
	draw_thin_pole();
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, lightpos1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	GLfloat facecolor1[] = {1.0f, 0.0f, 0.0f, 1.0f};
	GLfloat facecolor2[] = {1.0f, 1.0f, 0.0f, 1.0f};
	GLfloat facecolor3[] = {1.0f, 0.3f, 0.3f, 1.0f};
	GLfloat facecolor4[] = {1.0f, 1.0f, 0.3f, 1.0f};

	for (int x = 0; x < 4; ++x) {
		for (int y = 0; y < 4; ++y) {
			for (int z = 0; z < 4; ++z) {
				if (board[x][y][z] == 1 || board[x][y][z] == 2 || board[x][y][z] == 4 || board[x][y][z] == 5) {
					glPushMatrix();
					glTranslatef(z - 1.5, y - 1.5, x - 1.5);
					if (board[x][y][z] == 1) {
						glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor1);
						glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor1);
						//glColor3d(1, 0, 0);
					}
					if (board[x][y][z] == 2) {
						glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor2);
						glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor2);
						//glColor3d(1, 1, 0);
					}
					if (board[x][y][z] == 4) {
						glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor3);
						glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor3);
						//glColor3d(1, 0.5, 0.5);
					}
					if (board[x][y][z] == 5) {
						glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor4);
						glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor4);
						//glColor3d(1, 1, 0.5);
					}
					glutSolidSphere(0.5, 50, 50);
					glPopMatrix();
				}
				if (red_hint[x][y][z] == 1 || yellow_hint[x][y][z] == 1) {
					glPushMatrix();
					glTranslatef(z - 1.5, y - 1.5, x - 1.5);
					if (red_hint[x][y][z] == 1 && yellow_hint[x][y][z] == 1) {
						if (counter2 < 50) {
							glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor1);
							glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor1);
						}
						else {
							glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor2);
							glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor2);
						}
					}
					else if (red_hint[x][y][z] == 1) {
						glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor1);
						glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor1);
					}
					else if (yellow_hint[x][y][z] == 1) {
						glMaterialfv(GL_FRONT, GL_DIFFUSE, facecolor2);
						glMaterialfv(GL_FRONT, GL_AMBIENT, facecolor2);
					}
					glutSolidSphere(0.1, 50, 50);
					glPopMatrix();
				}

			}
		}
	}

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);
	glDisable(GL_LIGHT3);
	glDisable(GL_LIGHTING);

	displayUI();

	glFlush();

	glDisable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

void glut_idle(){

	counter++;
	counter2++;

    if (counter == 1) flag = 0;
    if (counter == 3) flag = 1;
    if (counter == 5) flag = 2;
    if (counter == 7) flag = 3;
    if (counter == 9) flag = 4;
    if (counter == 11) flag = 5;
    if (counter == 13) flag = 6;
    if (counter == 15) flag = 7;
    if (counter == 17) flag = 8;
    if (counter == 19) flag = 9;
	if (counter > 20) counter = 0;

	if (counter2 > 100) counter2 = 0;

	glutPostRedisplay();
}

void draw_pole() {
	GLdouble pointA[] = {0.05, 2.05, 0.05};
	GLdouble pointB[] = {0.05, 2.05, -0.05};
	GLdouble pointC[] = {0.05, -2.05, 0.05};
	GLdouble pointD[] = {0.05, -2.05, -0.05};
	GLdouble pointE[] = {-0.05, 2.05, 0.05};
	GLdouble pointF[] = {-0.05, 2.05, -0.05};
	GLdouble pointG[] = {-0.05, -2.05, 0.05};
	GLdouble pointH[] = {-0.05, -2.05, -0.05};

    glColor3d(0.8, 0.8, 0.8);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointD);
    glVertex3dv(pointC);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointF);
    glVertex3dv(pointE);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointC);
    glVertex3dv(pointG);
    glVertex3dv(pointE); 
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointE);
    glVertex3dv(pointF);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointB);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointF);
    glEnd();
}

void draw_thin_pole() {
	GLdouble pointA[] = {0.01, 2, 0.01};
	GLdouble pointB[] = {0.01, 2, -0.01};
	GLdouble pointC[] = {0.01, -2, 0.01};
	GLdouble pointD[] = {0.01, -2, -0.01};
	GLdouble pointE[] = {-0.01, 2, 0.01};
	GLdouble pointF[] = {-0.01, 2, -0.01};
	GLdouble pointG[] = {-0.01, -2, 0.01};
	GLdouble pointH[] = {-0.01, -2, -0.01};

    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointD);
    glVertex3dv(pointC);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointF);
    glVertex3dv(pointE);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointC);
    glVertex3dv(pointG);
    glVertex3dv(pointE); 
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointE);
    glVertex3dv(pointF);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3dv(pointB);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointF);
    glEnd();
}

void switch_focus(char input) {
	if (input == 'a') {
		if ((focus[1] == 1 && focus[0] == 1) || (focus[0] == 0 && (focus[1] == 0 || focus[1] == 1 || focus[1] == 2))) {
			focus[1] += 1;
		}
		else if ((focus[1] == 2 && focus[0] == 1) || (focus[1] == 3 && (focus[0] == 0 || focus[0] == 1 || focus[0] == 2))) {
			focus[0] += 1;
		}
		else if ((focus[1] == 2 && focus[0] == 2) || (focus[0] == 3 && (focus[1] == 1 || focus[1] == 2 || focus[1] == 3))) {
			focus[1] -= 1;
		}
		else if ((focus[1] == 1 && focus[0] == 2) || (focus[1] == 0 && (focus[0] == 1 || focus[0] == 2 || focus[0] == 3))) {
			focus[0] -= 1;
		}
	}
	if (input == 'd') {
		if ((focus[1] == 1 && focus[0] == 1) || (focus[1] == 0 && (focus[0] == 0 || focus[0] == 1 || focus[0] == 2))) {
			focus[0] += 1;
		}
		else if ((focus[1] == 1 && focus[0] == 2) || (focus[0] == 3 && (focus[1] == 0 || focus[1] == 1 || focus[1] == 2))) {
			focus[1] += 1;
		}
		else if ((focus[1] == 2 && focus[0] == 2) || (focus[1] == 3 && (focus[0] == 1 || focus[0] == 2 || focus[0] == 3))) {
			focus[0] -= 1;
		}
		else if ((focus[1] == 2 && focus[0] == 1) || (focus[0] == 0 && (focus[1] == 1 || focus[1] == 2 || focus[1] == 3))) {
			focus[1] -= 1;
		}
	}
	if (input == 's') {
		if (focus[1] == 0 && focus[0] == 0 || focus[1] == 0 && focus[0] == 1 || focus[1] == 1 && focus[0] == 0) {
			focus[1] = 1;
			focus[0] = 1;
		}
		else if (focus[1] == 0 && focus[0] == 2 || focus[1] == 0 && focus[0] == 3 || focus[1] == 1 && focus[0] == 3) {
			focus[1] = 1;
			focus[0] = 2;
		}
		else if (focus[1] == 2 && focus[0] == 3 || focus[1] == 3 && focus[0] == 3 || focus[1] == 3 && focus[0] == 2) {
			focus[1] = 2;
			focus[0] = 2;
		}
		else if (focus[1] == 3 && focus[0] == 1 || focus[1] == 3 && focus[0] == 0 || focus[1] == 2 && focus[0] == 0) {
			focus[1] = 2;
			focus[0] = 1;
		}
		else if (focus[1] == 1 && focus[0] == 1) {
			focus[1] = 0;
			focus[0] = 0;
		}
		else if (focus[1] == 1 && focus[0] == 2) {
			focus[1] = 0;
			focus[0] = 3;
		}
		else if (focus[1] == 2 && focus[0] == 2) {
			focus[1] = 3;
			focus[0] = 3;
		}
		else if (focus[1] == 2 && focus[0] == 1) {
			focus[1] = 3;
			focus[0] = 0;
		}
	}
	//printf("called with input as %c\n", input);
	//printf("%d, %d\n", focus[0], focus[1]);
}

void draw_box() {
	GLdouble pointA[] = {1, 2, 1};
	GLdouble pointB[] = {1, 2, -1};
	GLdouble pointC[] = {1, -1, 1};
	GLdouble pointD[] = {1, -1, -1};
	GLdouble pointE[] = {-1, 2, 1};
	GLdouble pointF[] = {-1, 2, -1};
	GLdouble pointG[] = {-1, -1, 1};
	GLdouble pointH[] = {-1, -1, -1};

    glColor3d(0.5, 0.5, 0.5);
	glNormal3d(0, 0, 1);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointD);
    glVertex3dv(pointC);
    glEnd();

	glNormal3d(0, 1, 0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointB);
    glVertex3dv(pointF);
    glVertex3dv(pointE);
    glEnd();

	glNormal3d(0, 0, 1);
    glBegin(GL_POLYGON);
    glVertex3dv(pointA);
    glVertex3dv(pointC);
    glVertex3dv(pointG);
    glVertex3dv(pointE); 
    glEnd();

	glNormal3d(-1, 0, 0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointE);
    glVertex3dv(pointF);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

	glNormal3d(0, -1, 0);
    glBegin(GL_POLYGON);
    glVertex3dv(pointC);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointG);
    glEnd();

	glNormal3d(0, 0, -1);
    glBegin(GL_POLYGON);
    glVertex3dv(pointB);
    glVertex3dv(pointD);
    glVertex3dv(pointH);
    glVertex3dv(pointF);
    glEnd();
}

void enter(int x, int z) {
	int y = 0;
	while (board[x][y][z] >= 1 && game_end == 0) y += 1;
	if (y < 4) {
		board[x][y][z] = (turn_count % 2) + 1;
		turn_count += 1;
	}
	//printf("executed with input x = %d, y = %d, z = %d", x, y, z);
}

void check_for_win() {
	for (int x = 0; x < 4; ++x) {
		for (int z = 0; z < 4; ++z) {
			if (board[x][0][z] == board[x][1][z] && board[x][0][z] == board[x][2][z] && 
			board[x][0][z] == board[x][3][z] && (board[x][0][z] == 1 || board[x][0][z] == 2)) {
				game_end = 1;
				if (board[x][0][z] == 1) red_win_count += 1;
				if (board[x][0][z] == 2) yellow_win_count += 1;
				for (int y = 0; y < 4; ++y) board[x][y][z] += 3;
			}
		} 
	} //vertical
	for (int x = 0; x < 4; ++x) {
		for (int y = 0; y < 4; ++y) {
			if (board[x][y][0] == board[x][y][1] && board[x][y][0] == board[x][y][2] && 
			board[x][y][0] == board[x][y][3] && (board[x][y][0] == 1 || board[x][y][0] == 2)) {
				game_end = 1;
				if (board[x][y][0] == 1) red_win_count += 1;
				if (board[x][y][0] == 2) yellow_win_count += 1;
				for (int z = 0; z < 4; ++z) board[x][y][z] += 3;
			};
		}
	} //horizontal (z axis)
	for (int y = 0; y < 4; ++y) {
		for (int z = 0; z < 4; ++z) {
			if (board[0][y][z] == board[1][y][z] && board[0][y][z] == board[2][y][z] && 
			board[0][y][z] == board[3][y][z] && (board[0][y][z] == 1 || board[0][y][z] == 2)) {
				game_end = 1;
				if (board[0][y][z] == 1) red_win_count += 1;
				if (board[0][y][z] == 2) yellow_win_count += 1;
				for (int x = 0; x < 4; ++x) board[x][y][z] += 3;
			};
		}
	} //horizontal (y axis)
	for (int x = 0; x < 4; ++x) {
		if (board[x][0][0] == board[x][1][1] && board[x][0][0] == board[x][2][2] && 
		board[x][0][0] == board[x][3][3] && (board[x][0][0] == 1 || board[x][0][0] == 2)) {
			game_end = 1;
			if (board[x][0][0] == 1) red_win_count += 1;
			if (board[x][0][0] == 2) yellow_win_count += 1;
			for (int yz = 0; yz < 4; ++yz) board[x][yz][yz] += 3;
		};
		if (board[x][3][0] == board[x][2][1] && board[x][3][0] == board[x][1][2] && 
		board[x][3][0] == board[x][0][3] && (board[x][3][0] == 1 || board[x][3][0] == 2)) {
			game_end = 1;
			if (board[x][3][0] == 1) red_win_count += 1;
			if (board[x][3][0] == 2) yellow_win_count += 1;
			for (int yz = 0; yz < 4; ++yz) board[x][yz][3-yz] += 3;
		};
	}
	for (int y = 0; y < 4; ++y) {
		if (board[0][y][0] == board[1][y][1] && board[0][y][0] == board[2][y][2] && 
		board[0][y][0] == board[3][y][3] && (board[0][y][0] == 1 || board[0][y][0] == 2)) {
			game_end = 1;
			if (board[0][y][0] == 1) red_win_count += 1;
			if (board[0][y][0] == 2) yellow_win_count += 1;
			for (int xz = 0; xz < 4; ++xz) board[xz][y][xz] += 3;
		};
		if (board[0][y][3] == board[1][y][2] && board[0][y][3] == board[2][y][1] && 
		board[0][y][3] == board[3][y][0] && (board[0][y][3] == 1 || board[0][y][3] == 2)) {
			game_end = 1;
			if (board[0][y][3] == 1) red_win_count += 1;
			if (board[0][y][3] == 2) yellow_win_count += 1;
			for (int xz = 0; xz < 4; ++xz) board[xz][y][3-xz] += 3;
		};
	}
	for (int z = 0; z < 4; ++z) {
		if (board[0][0][z] == board[1][1][z] && board[0][0][z] == board[2][2][z] && 
		board[0][0][z] == board[3][3][z] && (board[0][0][z] == 1 || board[0][0][z] == 2)) {
			game_end = 1;
			if (board[0][0][z] == 1) red_win_count += 1;
			if (board[0][0][z] == 2) yellow_win_count += 1;
			for (int xy = 0; xy < 4; ++xy) board[xy][xy][z] += 3;
		};
		if (board[3][0][z] == board[2][1][z] && board[3][0][z] == board[1][2][z] && 
		board[3][0][z] == board[0][3][z] && (board[3][0][z] == 1 || board[3][0][z] == 2)) {
			game_end = 1;
			if (board[3][0][z] == 1) red_win_count += 1;
			if (board[3][0][z] == 2) yellow_win_count += 1;
			for (int xy = 0; xy < 4; ++xy) board[xy][3-xy][z] += 3;
		};
	}
}

void check_for_reach() {
	if (show_hint == 1) {
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				for (int z = 0; z < 4; ++z) {
					if (board[x][y][z] == 0) {
						if (board[(x+1)%4][y][z] == 1 && board[(x+2)%4][y][z] == 1 && board[(x+3)%4][y][z] == 1) red_hint[x][y][z] = 1;
						if (board[x][(y+1)%4][z] == 1 && board[x][(y+2)%4][z] == 1 && board[x][(y+3)%4][z] == 1) red_hint[x][y][z] = 1;
						if (board[x][y][(z+1)%4] == 1 && board[x][y][(z+2)%4] == 1 && board[x][y][(z+3)%4] == 1) red_hint[x][y][z] = 1;
						if ((x == 0 || x == 3) && y + z == 3 && board[x][(y+1)%4][(z-1)%4] == 1 && board[x][(y+2)%4][(z-2)%4] == 1 && board[x][(y+3)%4][(z-3)%4] == 1) red_hint[x][y][z] = 1;
						if ((x == 0 || x == 3) && y - z == 0 && board[x][(y+1)%4][(z+1)%4] == 1 && board[x][(y+2)%4][(z+2)%4] == 1 && board[x][(y+3)%4][(z+3)%4] == 1) red_hint[x][y][z] = 1;
						if ((y == 0 || y == 3) && x + z == 3 && board[(x+1)%4][y][(z-1)%4] == 1 && board[(x+2)%4][y][(z-2)%4] == 1 && board[(x+3)%4][y][(z-3)%4] == 1) red_hint[x][y][z] = 1;
						if ((y == 0 || y == 3) && x - z == 0 && board[(x+1)%4][y][(z+1)%4] == 1 && board[(x+2)%4][y][(z+2)%4] == 1 && board[(x+3)%4][y][(z+3)%4] == 1) red_hint[x][y][z] = 1;
						if ((z == 0 || z == 3) && x + y == 3 && board[(x+1)%4][(y-1)%4][z] == 1 && board[(x+2)%4][(y-2)%4][z] == 1 && board[(x+3)%4][(y-3)%4][z] == 1) red_hint[x][y][z] = 1;
						if ((z == 0 || z == 3) && x - y == 0 && board[(x+1)%4][(y+1)%4][z] == 1 && board[(x+2)%4][(y+2)%4][z] == 1 && board[(x+3)%4][(y+3)%4][z] == 1) red_hint[x][y][z] = 1;

						if (board[(x+1)%4][y][z] == 2 && board[(x+2)%4][y][z] == 2 && board[(x+3)%4][y][z] == 2) yellow_hint[x][y][z] = 1;
						if (board[x][(y+1)%4][z] == 2 && board[x][(y+2)%4][z] == 2 && board[x][(y+3)%4][z] == 2) yellow_hint[x][y][z] = 1;
						if (board[x][y][(z+1)%4] == 2 && board[x][y][(z+2)%4] == 2 && board[x][y][(z+3)%4] == 2) yellow_hint[x][y][z] = 1;
						if ((x == 0 || x == 3) && y + z == 3 && board[x][(y+1)%4][(z-1)%4] == 2 && board[x][(y+2)%4][(z-2)%4] == 2 && board[x][(y+3)%4][(z-3)%4] == 2) yellow_hint[x][y][z] = 1;
						if ((x == 0 || x == 3) && y - z == 0 && board[x][(y+1)%4][(z+1)%4] == 2 && board[x][(y+2)%4][(z+2)%4] == 2 && board[x][(y+3)%4][(z+3)%4] == 2) yellow_hint[x][y][z] = 1;
						if ((y == 0 || y == 3) && x + z == 3 && board[(x+1)%4][y][(z-1)%4] == 2 && board[(x+2)%4][y][(z-2)%4] == 2 && board[(x+3)%4][y][(z-3)%4] == 2) yellow_hint[x][y][z] = 1;
						if ((y == 0 || y == 3) && x - z == 0 && board[(x+1)%4][y][(z+1)%4] == 2 && board[(x+2)%4][y][(z+2)%4] == 2 && board[(x+3)%4][y][(z+3)%4] == 2) yellow_hint[x][y][z] = 1;
						if ((z == 0 || z == 3) && x + y == 3 && board[(x+1)%4][(y-1)%4][z] == 2 && board[(x+2)%4][(y-2)%4][z] == 2 && board[(x+3)%4][(y-3)%4][z] == 2) yellow_hint[x][y][z] = 1;
						if ((z == 0 || z == 3) && x - y == 0 && board[(x+1)%4][(y+1)%4][z] == 2 && board[(x+2)%4][(y+2)%4][z] == 2 && board[(x+3)%4][(y+3)%4][z] == 2) yellow_hint[x][y][z] = 1;
					}
				}
			}
		}
	}
	if (show_hint == 0) {
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				for (int z = 0; z < 4; ++z) {
					red_hint[x][y][z] = 0;
					yellow_hint[x][y][z] = 0;
				}
			}
		}
	}
}

void reset_board(int option) {
	for (int x = 0; x < 4; ++x) {
		for (int y = 0; y < 4; ++y) {
			for (int z = 0; z < 4; ++z) {
				board[x][y][z] = 0;
				red_hint[x][y][z] = 0;
				yellow_hint[x][y][z] = 0;
			}
		}
	}
	for (int x = 1; x < 3; ++x) {
		for (int y = 0; y < 3; ++y) {
			for (int z = 1; z < 3; ++z) {
				board[x][y][z] = 3;
			}
		}
	}
	focus[0] = 0;
	focus[1] = 0;
	turn_count = 0;
	game_end = 0;
	if (option == 1) {
		red_win_count = 0;
		yellow_win_count = 0;
	}
}

void set_texture(){
    const char* inputFileNames[14] = {"parrot1.jpg", "parrot2.jpg", "parrot3.jpg", "parrot4.jpg", "parrot5.jpg", 
									"parrot6.jpg", "parrot7.jpg", "parrot8.jpg", "parrot9.jpg", "parrot10.jpg", 
									"red_win.png", "yellow_win.png", "red_parrot.jpg", "yellow_parrot.jpg"};
	for(int i = 0; i < 14; i++){
		cv::Mat input = cv::imread(inputFileNames[i], 1);
		// BGR -> RGBの変換
		cv::cvtColor(input, input, cv::COLOR_BGR2RGB);

		glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
		glTexSubImage2D(GL_TEXTURE_2D, 0,
			(TEXTURE_WIDTH - input.cols)/2,
			(TEXTURE_HEIGHT - input.rows)/2,
			input.cols, input.rows,
			GL_RGB, GL_UNSIGNED_BYTE, input.data);
	}
}

void displayUI() {
	glViewport(0, 0, WINDOW_X1, WINDOW_Y1);
	glMatrixMode(GL_PROJECTION);

	glPushMatrix();
	glLoadIdentity();

	gluOrtho2D(-WINDOW_X1/2, WINDOW_X1/2, WINDOW_Y1/2, -WINDOW_Y1/2);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	//glutBitmapCharacter;

	glColor3d(1, 1, 1);
	if (game_end == 1 && turn_count % 2 == 1) glBindTexture(GL_TEXTURE_2D, g_TextureHandles[flag]);
	else glBindTexture(GL_TEXTURE_2D, g_TextureHandles[12]);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	glVertex2d(-500, -500);
	glTexCoord2d(1, 0);
	glVertex2d(-300, -500);
	glTexCoord2d(1, 1);
	glVertex2d(-300, -300);
	glTexCoord2d(0, 1);
	glVertex2d(-500, -300);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glColor3d(1, 1, 1);
	if (game_end == 1 && turn_count % 2 == 0) glBindTexture(GL_TEXTURE_2D, g_TextureHandles[flag]);
	else glBindTexture(GL_TEXTURE_2D, g_TextureHandles[13]);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glTexCoord2d(0, 0);
	glVertex2d(500, -500);
	glTexCoord2d(1, 0);
	glVertex2d(300, -500);
	glTexCoord2d(1, 1);
	glVertex2d(300, -300);
	glTexCoord2d(0, 1);
	glVertex2d(500, -300);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	if (game_end == 0 && turn_count % 2 == 0 && turn_count < 52) print_text(-475, -250, (char*)"YOUR TURN");
	if (game_end == 0 && turn_count % 2 == 1 && turn_count < 52) print_text(325, -250, (char*)"YOUR TURN");
	if (game_end == 1 && turn_count % 2 == 0) print_text(-200, 400, (char*)"YELLOW WIN! PRESS 'R' TO PLAY AGAIN");
	if (game_end == 1 && turn_count % 2 == 1) print_text(-200, 400, (char*)"RED♪ WIN! PRESS 'R' TO PLAY AGAIN");
	if (game_end == 0 && turn_count == 52) print_text(-200, 400, (char*)"IT'S A TIE! PRESS 'R' TO PLAY AGAIN");
	if (game_end == 0 && turn_count < 52) {
		print_text(-350, 400, (char*)"A: move pointer clockwise");
		print_text(-350, 430, (char*)"D: move pointer counterclockwise");
		print_text(-350, 460, (char*)"S: move pointer inward/outward");
		print_text(50, 400, (char*)"ENTER: make your move");
		if (show_hint == 0) print_text(50, 430, (char*)"H: hint mode (currently OFF)");
		if (show_hint == 1) print_text(50, 430, (char*)"H: hint mode (currently ON)");
		print_text(50, 460, (char*)"R: reset board");
	}

//	const char* red_score = std::to_string(red_win_count).c_str();
//	const char* yellow_score = std::to_string(yellow_win_count).c_str();

    std::string red_str = std::to_string(red_win_count);
    char* red_score = new char[red_str.length() + 1];
    std::strcpy(red_score, red_str.c_str());

    std::string yellow_str = std::to_string(yellow_win_count);
    char* yellow_score = new char[yellow_str.length() + 1];
    std::strcpy(yellow_score, yellow_str.c_str());

	print_text(-400, -200, red_score);
	print_text(400, -200, yellow_score);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

}

void print_text(int x, int y, char* string) {
	int len;

	glRasterPos2f(x, y);
	len = (int) strlen(string);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

//added kakapo textures and winning animation
//added win counter
//added lighting to distinguish spheres

//add tie checker
//add score reset