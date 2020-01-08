int main(int argc, char *argv[]);
void proc_0x0804a550();


/** address: 0x0804b320 */
int main(int argc, char *argv[])
{
    int *local0; 		// m[esp - 28]

    glutInit(&argc, argv);
    glutInitDisplayMode(16);
    local0 = *argv;
    glutCreateWindow(local0);
    proc_0x0804a550();
    glutDisplayFunc(0x804ac50);
    glutKeyboardFunc(0x804aeec);
    glutSpecialFunc(0x804b098);
    glutReshapeFunc(0x804ad04);
    glutMainLoop();
    return 0;
}

/** address: 0x0804a550 */
void proc_0x0804a550()
{
    glClearColor(0, 0x3f000000, 0x3f333333, 0x3f800000);
    glEnable(0xb71);
    memset(0x804c280, 0, 500);
    memset(0x804ca50, 0, 4);
    return;
}

