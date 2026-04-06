#pragma once

class MouseProcHook final : public Hook {
private:
    HANDLE hThread = nullptr;
    HANDLE readyEvent = nullptr;
    DWORD  threadId = 0;
    std::atomic_bool& mouseDown;

    static DWORD WINAPI ThreadProc(LPVOID param);
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
public:
    explicit MouseProcHook(std::atomic_bool& sharedFlag)
        : Hook(), mouseDown(sharedFlag) {}

    void enable() override;
    void disable() override;
};
