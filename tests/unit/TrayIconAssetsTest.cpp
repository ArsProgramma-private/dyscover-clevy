// TrayIconAssetsTest.cpp - Verify icon bundle indices used by TrayIcon mapping exist
#include "ResourceLoader.h"
#include <iostream>

int main() {
    auto icons = LoadDyscoverIcons();
    if(icons.GetCount() < 6) {
        std::cerr << "Expected at least 6 icons, got " << icons.GetCount() << std::endl;
        return 1;
    }
    // Validate specific indices used by TrayIcon mapping
    if(!icons[0].IsOk()) { std::cerr << "Icon[0] not loaded" << std::endl; return 2; }
    if(!icons[4].IsOk()) { std::cerr << "Icon[4] not loaded" << std::endl; return 3; }
    if(!icons[5].IsOk()) { std::cerr << "Icon[5] not loaded" << std::endl; return 4; }
    std::cout << "TrayIcon assets present and loadable (0,4,5)." << std::endl;
    return 0;
}
