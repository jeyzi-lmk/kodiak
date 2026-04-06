#pragma once

class Hook {
public:
    virtual ~Hook() = default;
    Hook() = default;

    virtual void enable() = 0;
    virtual void disable() = 0;
};