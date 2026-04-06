#pragma once
namespace Commands {
    using Arg = std::variant<int64_t, double, bool, std::string>;

    struct Command {
        std::string name;
        std::vector<Arg> args;
        std::string raw;
    };

    template<typename T> const T* arg(const Commands::Command& c, size_t i) {
        if (i >= c.args.size()) return nullptr;
        return std::get_if<T>(&c.args[i]);
    }

    inline std::optional<double> number(const Command& c, size_t i) {
        if (i >= c.args.size()) return std::nullopt;
        if (auto d = std::get_if<double>(&c.args[i]))    return *d;
        if (auto i64 = std::get_if<int64_t>(&c.args[i])) return static_cast<double>(*i64);
        if (auto s = std::get_if<std::string>(&c.args[i])) {
            char* end=nullptr;
            double v = std::strtod(s->c_str(), &end);
            if (end && *end == '\0') return v;
        }
        return std::nullopt;
    }

    inline void toLowerInPlace(std::string& s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char ch){ return static_cast<char>(std::tolower(ch)); });
    }

    class Dispatcher {
    public:
        using Handler = std::function<void(const Command&)>;

        static bool init() {
            std::lock_guard lk(s_m);
            if (s_cmdEvent) return true;
            s_cmdEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
            return s_cmdEvent != nullptr;
        }

        static void shutdown() {
            std::lock_guard lk(s_m);
            s_handlers.clear();
            std::deque<Command>().swap(s_q);
            if (s_cmdEvent) { CloseHandle(s_cmdEvent); s_cmdEvent = nullptr; }
        }

        static HANDLE eventHandle() { return s_cmdEvent; }

        static void registerHandler(const std::string& name, Handler h) {
            std::lock_guard lk(s_m);
            s_handlers[name] = std::move(h);
        }

        static void clearHandlers() {
            std::lock_guard lk(s_m);
            s_handlers.clear();
        }

        static std::vector<std::string> listCommands() {
            std::lock_guard lk(s_m);
            std::vector<std::string> out;
            out.reserve(s_handlers.size());
            for (auto &kv : s_handlers) out.push_back(kv.first);
            return out;
        }

        static void post(Command cmd) {
            {
                std::lock_guard lk(s_m);
                s_q.emplace_back(std::move(cmd));
            }
            if (s_cmdEvent) SetEvent(s_cmdEvent);
        }

        static bool drain() {
            std::deque<Command> local;
            {
                std::lock_guard lk(s_m);
                local.swap(s_q);
            }

            bool allFound = true;
            for (const auto& cmd : local) {
                Handler h;
                {
                    std::lock_guard lk(s_m);
                    auto it = s_handlers.find(cmd.name);
                    if (it != s_handlers.end()) h = it->second; else allFound = false;
                }
                if (h) h(cmd);
            }

            {
                std::lock_guard lk(s_m);
                if (s_q.empty()) {
                    if (s_cmdEvent) ResetEvent(s_cmdEvent);
                } else {
                    if (s_cmdEvent) SetEvent(s_cmdEvent);
                }
            }
            return allFound;
        }

    private:
        static inline HANDLE s_cmdEvent = nullptr;
        static inline std::mutex s_m;
        static inline std::deque<Command> s_q;
        static inline std::unordered_map<std::string, Handler> s_handlers;
    };
} // namespace Commands