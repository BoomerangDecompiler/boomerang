/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#include <stdio.h>
#include <stdlib.h>


typedef struct {
    int     vendor;
    int     chipType;
    int     chipRev;
    int     subsysVendor;
    int     subsysCard;
    int     bus;
    int     device;
    int     func;
    int     class;
    int     subclass;
    int     interface;
    /* more, deleted */
} pciVideoRec;
typedef pciVideoRec *pciVideoPtr;

pciVideoPtr *xf86GetPciVideoInfo();


int getDevice(pciVideoPtr p)
{
    return p->device;
}


int main()
{
    pciVideoPtr *p1 = xf86GetPciVideoInfo();
    printf("%i\n", getDevice(*p1));
    return EXIT_SUCCESS;
}
