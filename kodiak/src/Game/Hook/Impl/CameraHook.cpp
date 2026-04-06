#include "CameraHook.hpp"

#include "src/Game/Game.hpp"

void CameraHook::enable() {
    auto signature = Game::findPattern("F3 0F 10 4B ? 0F 28 F8 F3 0F 59 0D ? ? ? ? 44 0F 28 C0 0F C6 F8");
    if(signature == nullptr) {
        Console::print("Failed to find Camera signature.");
        return;
    }
#ifdef DEBUG
    Console::print_addr("Found Camera signature at ", reinterpret_cast<uintptr_t>(signature));
#endif

    std::vector<uint8_t> shellCode = {0x48, 0x8D, 0x03}; // lea rax, [rbx]

    auto rax = Asm::mov_rax(reinterpret_cast<uintptr_t>(field));
    shellCode.insert(shellCode.end(), rax.begin(), rax.end()); // mov rax, field

    shellCode.insert(shellCode.end(), 5, 0x11); // placeholder for jmp back

    cave = Mem::create_code_cave(Game::handle, reinterpret_cast<uintptr_t>(Game::module), shellCode.size() + 5);
    Mem::write_bytes(Game::handle, cave, shellCode);

    detour = Game::createDetour(signature, cave, 5);
    detour->enable();
}

void CameraHook::disable() {
    if(detour != nullptr) {
        detour->disable();
        delete detour;
        detour = nullptr;
    }

    if(cave != nullptr) {
        Mem::unalloc(Game::handle, cave, 0);
        cave = nullptr;
    }

    std::cout << "CameraHook disabled." << std::endl;
}
