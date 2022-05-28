#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include <regex>

std::string ToLower(std::string str) {
    for (auto &item: str) {
        item = (char)std::tolower(item);
    }
    return str;
}

bool IsMatch(std::string key, std::string question) {
    key = ToLower(key);
    question = ToLower(question);
    std::regex pattern(".*" + question + ".*");     // Где-то внутри есть фраза
    return std::regex_match(key, pattern);
}

std::string GetUserQuestion() {
    std::string question;
    std::cout << "[USER]: ";
    std::getline(std::cin, question);
    return ToLower(question);
}

void ShowBotReply(const std::string& value) {
    std::cout << value << '\n';
}

int main() {
    std::map<std::string, std::string> answers_data_base = {
            {"Hi",                                 "Hello, my friend!"},
            {"What's.*the.*weather", "It's rainy and cool, i.e. wonderful!"},
            {"Hello",                              "Hi human!"},
            {"How.*are.*you",                      "I'm doing good"},
            {"What.*is.*your.*name",               "FAX"},
            {"Lets.*write.*some.*code.*tonight",   "Sure! I'll be waiting for you in discord at 8 pm"},
            {"See.*you.*later!", "Bye!!!"}
    };

    std::string question;
    bool isFoundAnswer = false;
    const auto& kNotFound = "Sorry, i can't tell you anything :(";

    while (question != "exit") {
        question = GetUserQuestion();
        isFoundAnswer = false;
        for (const auto &[key, value]: answers_data_base) {
            if (IsMatch(question, key)) {
                ShowBotReply(value);
                isFoundAnswer = true;
            }
        }
        if (!isFoundAnswer) {
            std::cout << kNotFound << '\n';
        }

    }

    return 0;
}