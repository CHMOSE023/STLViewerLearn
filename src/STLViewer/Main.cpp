#include "CadBase.h"
#include "Entity.h"
#include "OpenGLDC.h"
#include <stdio.h>
#include <iostream>

int main() {
    try {
        CadBase* cad = new CadBase;
        Entity* ent = new Entity;
        OpenGLDC* dc = new OpenGLDC;

        // TODO: 实际业务逻辑

        delete dc;
        delete ent;
        delete cad;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }

    getchar();
    return 0;
}