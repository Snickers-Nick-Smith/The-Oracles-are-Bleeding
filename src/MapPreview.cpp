// MapPreview.cpp (standalone helper if you want)
#include "Map.hpp"
int previewmain(){
    TempleMap m; m.populateFromDesign();
    m.printAscii();
    m.printAdjacency();
    return 0;
}
