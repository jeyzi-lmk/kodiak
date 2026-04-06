#include "LocalPlayerHook.hpp"

#include "src/Game/Game.hpp"

void LocalPlayerHook::enable() {
    auto signature = Game::findPattern("45 8B 85 ? ? ? ? B8");
    if(signature == nullptr) {
        Console::print("Failed to find LocalPlayer signature.");
        return;
    }

#ifdef DEBUG
    Console::print_addr("Found LocalPlayer signature at ", reinterpret_cast<uintptr_t>(signature));
#endif

    auto shellCode = Asm::lea();
    auto rax = Asm::mov_rax(reinterpret_cast<uintptr_t>(field));
    shellCode.insert(shellCode.end(), rax.begin(), rax.end());
    shellCode.insert(shellCode.end(), 5, 0x11); // placeholder for jmp back

    cave = Mem::create_code_cave(Game::handle, reinterpret_cast<uintptr_t>(Game::module), shellCode.size() + 5);
    Mem::write_bytes(Game::handle, cave, shellCode);

    detour = Game::createDetour(signature, cave, 7);
    detour->enable();
}

void LocalPlayerHook::disable() {
    if(detour != nullptr) {
        detour->disable();
        delete detour;
        detour = nullptr;
    }

    if(cave != nullptr) {
        Mem::unalloc(Game::handle, cave, 0);
        cave = nullptr;
    }
}