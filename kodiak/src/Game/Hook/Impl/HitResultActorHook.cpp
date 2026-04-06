#include "HitResultActorHook.hpp"

#include "src/Game/Game.hpp"

void HitResultActorHook::enable() {
    auto signature = Game::findPattern("48 8B 00 48 8B 4C 24 ? 48 33 CC E8 ? ? ? ? 48 83 C4 ? C3 33 C0 48 8B 4C 24 ? 48 33 CC E8 ? ? ? ? 48 83 C4 ? C3 CC CC CC 40 55");
    if(signature == nullptr) {
        Console::print("Failed to find HitResultActor signature.");
        return;
    }

#ifdef DEBUG
    Console::print_addr("Found HitResultActor signature at ", reinterpret_cast<uintptr_t>(signature));
#endif

    auto shellCode = Asm::mov_rax(reinterpret_cast<uintptr_t>(field));
    shellCode.insert(shellCode.end(), 5, 0x11); // placeholder for jmp back

    cave = Mem::create_code_cave(Game::handle, reinterpret_cast<uintptr_t>(Game::module), shellCode.size() + 5);
    Mem::write_bytes(Game::handle, cave, shellCode);

    detour = Game::createDetour(signature, cave, 8);
    detour->enable();
}

void HitResultActorHook::disable() {
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
