#include <windows.h>
#include "AppWindow.h"

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    AppWindow app;
    if (!app.create(hInstance)) return -1;
    return app.run();
}

