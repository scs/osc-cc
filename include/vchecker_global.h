
#ifndef VCHECKER_GLOBAL_H_
#define VCHECKER_GLOBAL_H_

#include <oscar.h>

struct G_BOUNDINGBOX {
    uint16 label;
    uint16 x;
    uint16 y;
    uint16 width;
    uint16 height;
};

/* Anzahl Fourierkoeffizienten Amplitudenspektrum fuer Kontourbeschreibung*/
#define FOURIER_M                       4

#define G_MAX_NO_OF_BOUNDING_BOXES      10

#endif /* VCHECKER_GLOBAL_H_ */
