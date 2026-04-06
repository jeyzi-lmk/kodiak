#pragma once

class LocalPlayerHook final : public Hook {
private:
    Detour* detour = nullptr;
    void* cave = nullptr;
    void* field = nullptr;
public:
    explicit LocalPlayerHook(void* field) : Hook() {
        this->field = field;
    }

    void enable() override;
    void disable() override;
};
