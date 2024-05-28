#include "grbl.h"

void firmware_update(char *line) {
    // mc_reset();
    if(0 == strcmp(line, "@STCISP#")) {
	    IAP_CONTR = 0x60;
    }
}