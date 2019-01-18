// first OpenVG program
// Anthony Starks (ajstarks@gmail.com)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"

int main() {
	int width, height;
	InitShapes(&width, &height);				   // Graphics initialization
	printf("%d %d\n", width, height);
	FinishShapes();					   // Graphics cleanup
	exit(0);
}
