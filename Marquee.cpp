#include "headers/Marquee.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

Marquee::Marquee() {}

void Marquee::startMarquee() {
    const std::string marqueeText = "Hello world in marquee!";
    const int textLength = marqueeText.size();
    const int screenWidth = 80;  
    const int screenHeight = 20; 

    int x = 0, y = 0; 
    int dx = 1, dy = 1;         

    while (true) {
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif

        std::cout << "************************************************************\n";
        std::cout << "*               Displaying a marquee console!              *\n";
        std::cout << "************************************************************\n\n";

        for (int i = 0; i < y; ++i) {
            std::cout << '\n';
        }

        std::cout << std::string(x, ' ') << marqueeText << std::flush;

        x += dx;
        y += dy;

        if (x <= 0 || x + textLength >= screenWidth) {
            dx = -dx; 
            x = std::max(0, x); 
        }
        if (y <= 0 || y >= screenHeight) {
            dy = -dy;  
            y = std::max(0, y); 
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50)); 
    }
}