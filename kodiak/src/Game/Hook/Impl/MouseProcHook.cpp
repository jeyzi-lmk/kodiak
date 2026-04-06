static thread_local MouseProcHook* tlsInstance = nullptr;
static HHOOK sHook = nullptr;

LRESULT CALLBACK MouseProcHook::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && tlsInstance) {
        auto* p = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
        if (p && (p->flags & (LLMHF_INJECTED | 0x2)) == 0) {
            if (wParam == WM_LBUTTONDOWN) tlsInstance->mouseDown.store(true,  std::memory_order_release);
            else if (wParam == WM_LBUTTONUP) tlsInstance->mouseDown.store(false, std::memory_order_release);
        }
    }
    return CallNextHookEx(sHook, nCode, wParam, lParam);
}

DWORD WINAPI MouseProcHook::ThreadProc(LPVOID param) {
    auto* instance = static_cast<MouseProcHook*>(param);
    tlsInstance = instance;

    MSG bootstrapMsg;
    PeekMessageW(&bootstrapMsg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
    if (instance->readyEvent) {
        SetEvent(instance->readyEvent);
    }

    sHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, nullptr, 0);
    if (!sHook) return 0;

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(sHook);
    sHook = nullptr;
    tlsInstance = nullptr;
    return 0;
}

void MouseProcHook::enable() {
    if (hThread) return;

    readyEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!readyEvent) return;

    hThread = CreateThread(nullptr, 0, ThreadProc, this, 0, &threadId);
    if (!hThread) {
        CloseHandle(readyEvent);
        readyEvent = nullptr;
        return;
    }

    WaitForSingleObject(readyEvent, 3000);
}

void MouseProcHook::disable() {
    if (!hThread) return;

    if (threadId != 0) {
        PostThreadMessageW(threadId, WM_QUIT, 0, 0);
    }

    WaitForSingleObject(hThread, 3000);
    CloseHandle(hThread);
    hThread = nullptr;
    threadId = 0;

    if (readyEvent) {
        CloseHandle(readyEvent);
        readyEvent = nullptr;
    }
}
