void glutInit(int *argcp, char **argv);
int glutCreateWindow(const char *title);
void glutInitDisplayMode(unsigned int mode);
void glutMainLoop(void);
void glutSpecialFunc(void (*func)(int key, int x, int y));
void glutDisplayFunc(void (*func)(void));
void glutKeyboardFunc(void (*func)(char key, int x, int y));
void glutReshapeFunc(void (*func)(int width, int height));
void glutBitmapCharacter(void *font, int character);
