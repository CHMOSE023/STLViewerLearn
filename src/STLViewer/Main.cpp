#include "STLWindows.h"
#include <iostream>
#include <Windows.h>
int main() {
    SetConsoleOutputCP(CP_UTF8);   // 控制台输出用 UTF-8
    SetConsoleCP(CP_UTF8);         // 控制台输入用 UTF-8
    try {
        
        STLWindows* pSTLWindows = new STLWindows;

        pSTLWindows->Initialize(1024, 768, L"STLViewer");

        pSTLWindows->Run();
		 
        delete pSTLWindows;
		pSTLWindows = nullptr;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return -1;
    }  
     
    return 0;
}