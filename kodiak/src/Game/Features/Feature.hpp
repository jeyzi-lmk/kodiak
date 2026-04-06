#pragma once

class Feature {
private:
    std::string name{};
    std::string description{};
    bool enabled = false;
  public:
    virtual ~Feature() = default;
    explicit Feature(const std::string& name, const std::string& description) : name(name), description(description) {}

    [[nodiscard]] std::string getName() const {
        return name;
    }

    [[nodiscard]] std::string getDescription() const {
        return description;
    }

    bool isEnabled() const {
        return enabled;
    }

    void setEnabled(bool value) {
        enabled = value;
    }

    virtual void tick(int i) = 0;
};