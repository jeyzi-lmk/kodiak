#pragma once

class CameraHook final : public Hook {
private:
    Detour* detour = nullptr;
    void* cave = nullptr;
    void* field = nullptr;
public:
    explicit CameraHook(void* field) : Hook() {
        this->field = field;
    }

    void enable() override;
    void disable() override;
};
