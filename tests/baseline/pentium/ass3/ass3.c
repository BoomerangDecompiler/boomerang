__size32 global64;// 4 bytes
__size32 global70;// 4 bytes

void proc2();
void proc3();
void proc4(char param1[]);
void proc5();

// address: 0x804dd8c
int main(int argc, char *argv[], char *envp[]) {
    int *local0; 		// m[esp - 28]

    glutInit(&argc, argv);
    glutInitDisplayMode(18);
    local0 = *argv;
    glutCreateWindow(local0);
    proc2();
    glutDisplayFunc(0x804d640);
    glutKeyboardFunc(0x804d884);
    glutSpecialFunc(0x804db14);
    glutReshapeFunc(0x804d758);
    glutIdleFunc();
    glutFullScreen();
    glutMainLoop();
    return 0;
}

// address: 0x804ce8c
void proc2() {
    __size32 eax; 		// r24
    char *local0; 		// m[esp - 28]
    int local1; 		// m[esp - 24]
    union { GLclampf x9; __size32 x10; } local2; 		// m[esp - 16]
    union { GLclampf x7; __size32 x8; } local3; 		// m[esp - 20]

    local2 = *0x807a5ec;
    local3 = *0x807a5e8;
    local1 = *0x807a5e4;
    local0 = *0x807a5e0;
    glClearColor(local0, local1, local3, local2);
    glEnable(0xb71);
    glEnable(0xb60);
    glFogfv();
    glFogi();
    glFogf();
    glHint();
    glHint();
    glBlendFunc();
    proc3();
    glHint();
    glCullFace();
    glEnable(0xb44);
    gluNewQuadric();
    global64 = eax;
    gluQuadricCallback();
    gluQuadricNormals();
    memset(0x807a980, 0, 2000);
    global70 = 0;
    *(__size8*)0x807c8c4 = 0;
    glGenTextures();
    glBindTexture();
    gluBuild2DMipmaps();
    if (eax != 0) {
        printf("Error in gluBuild2DMipmaps: %i\n", eax);
        proc4(0x8059156);
    }
    glTexParameteri();
    glTexParameteri();
    glBindTexture();
    proc5();
    gluBuild2DMipmaps();
    if (eax != 0) {
        printf("Error in gluBuild2DMipmaps: %i\n", eax);
        proc4(0x8059156);
    }
    glTexParameteri();
    glTexParameteri();
    glTexEnvf();
    return;
}

// address: 0x804cd34
void proc3() {
    __size32 ecx; 		// r25
    __size32 *edi; 		// r31
    __size32 *esi; 		// r30
    int esp; 		// r28

    edi = (esp - 28);
    esi = 0x807a60c;
    ecx = 4;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    edi = (esp - 44);
    glLightModelfv();
    esi = 0x807a61c;
    ecx = 4;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    edi = (esp - 60);
    esi = 0x807a62c;
    ecx = 4;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    edi = (esp - 76);
    esi = 0x807a63c;
    ecx = 4;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    glEnable(0x4000);
    glLightfv();
    glLightfv();
    edi = (esp - 92);
    glLightfv();
    esi = 0x807a64c;
    ecx = 4;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    edi = (esp - 108);
    esi = 0x807a65c;
    ecx = 4;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    edi = (esp - 124);
    esi = 0x807a66c;
    ecx = 4;
    while (ecx != 0) {
        *(__size32*)edi = *esi;
        esi +=  (DF == 0) ? 4 : -4;
        edi +=  (DF == 0) ? 4 : -4;
        ecx = ecx - 1;
    }
    glEnable(0x4001);
    glLightfv();
    glLightfv();
    glLightfv();
    glLightf();
    glEnable(0xb50);
    glLightModeli();
    return;
}

// address: 0x804cb98
void proc4(char param1[]) {
    union { GLenum x1; __size32 x2; } eax; 		// r24

    for(;;) {
        glGetError();
        if (eax == 0) {
            break;
        }
        gluErrorString(eax);
        printf("%s: %s\n", param1, eax);
    }
    return;
}

// address: 0x804cb20
void proc5() {
    memcpy(0x80625cd, 0x804dfa0, 0x8000);
    memcpy(0x806a5cd, 0x80725e0, 0x7fff);
    return;
}

