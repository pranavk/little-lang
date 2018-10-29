#pragma once

#include <unordered_set>

namespace Lilang {
    class Settings {
        std::unordered_set<std::string> _settings;
        public:
        void add(const std::string& setting) {
            _settings.insert(setting);
        }

        bool isOn(const std::string &opt)
        {
            // only compare the part before "="
            for (auto &arg : _settings)
            {
                if (opt == arg.substr(0, arg.find("=")))
                    return true;
            }
            return false;
        }

        std::string getOptionValue(const std::string &key)
        {
            for (auto &arg : _settings)
            {
                if (arg.find("=") != std::string::npos && key == arg.substr(0, arg.find("=")))
                    return arg.substr(arg.find("=") + 1);
            }
            return ""; // no value set in option
        }

        static Settings& get() {
            static Settings SettingsInstance;
            return SettingsInstance;
        }
    };
}