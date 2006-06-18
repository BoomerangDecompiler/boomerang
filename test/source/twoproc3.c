
#include <stdio.h>

typedef struct {
	int                 vendor;
	int                 chipType;
	int                 chipRev;
	int                 subsysVendor;
	int                 subsysCard;
	int                 bus;
	int                 device;
	int                 func;
	int                 class;
	int                 subclass;
	int                 interface;
	// more, deleted
} pciVideoRec;
typedef pciVideoRec *pciVideoPtr;

pciVideoPtr *xf86GetPciVideoInfo(void);

int getDevice(pciVideoPtr p) {
	return p->device;
}

int main()
{
	pciVideoPtr *p1 = xf86GetPciVideoInfo();
    printf("%i\n", getDevice(*p1));	
	return 0;
}
