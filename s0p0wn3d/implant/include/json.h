#pragma once
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Echappe une chaine pour l'inserer dans un corps JSON.
std::string json_escape(const std::string& value);

// Retourne la valeur de `key` si c'est une chaine, sinon std::nullopt.
std::optional<std::string> json_get_string(const std::string& json,
                                           const std::string& key);

// Constructeur de corps JSON : chaque valeur est echappee par dump(), donc
// il est impossible d'oublier l'echappement en ajoutant un champ.
class JsonObject {
public:
    JsonObject& add(const std::string& key, const std::string& value) {
        fields_.emplace_back(key, value);
        return *this;
    }

    std::string dump() const {
        std::string out = "{";
        for (size_t i = 0; i < fields_.size(); ++i) {
            if (i > 0) out += ',';
            out += "\"" + json_escape(fields_[i].first) + "\":\"" +
                json_escape(fields_[i].second) + "\"";
        }
        return out + "}";
    }

private:
    std::vector<std::pair<std::string, std::string>> fields_;
};