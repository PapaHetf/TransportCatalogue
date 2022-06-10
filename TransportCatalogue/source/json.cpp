#include "json.h"

using namespace std;

namespace json {

    namespace {
        Node LoadNode(istream& input);
        
        void CheckEscapeSequence(istream& input, char& c) {
            input.get(c);
            if (c == 'n') {
                c = '\n';
                return;
            }
            if (c == 't') {
                c = '\t';
                return;
            }
            if (c == 'r') {
                c = '\r';
                return;
            }
        }
        
        Node LoadString(istream& input) {
            string all_value;
            char c = 0;
            input.get(c);
            while (c != '\"' && !input.eof()) {
                if (c == '\\') {
                    CheckEscapeSequence(input, c);
                }
                all_value.push_back(move(c));
                input.get(c);
            }
            if (!(c == '\"')) {
                throw ParsingError("Failed to read parsing data"s);
            }
            return Node(move(all_value));
        }

        Node LoadArray(istream& input) {
            Array result;
            char c = 0;
            input >> c;
            while (c != ']' && !input.eof()) {
                if(c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));        
                input >> c;
            }
            if (!(c ==']')) {
                throw ParsingError("Failed to read parsing data"s);
            }
            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c = 0;
            input >> c;
            while (c != '}' && !input.eof()) {
                if (c == ',') {
                    input >> c;
                }
                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
                input >> c;
            }
            if (!(c == '}')) {
                throw ParsingError("Failed to read parsing data"s);
            }
            return Node(move(result));
        }
       
        Node LoadNull() {
            return Node(nullptr);
        }

        Node LoadTrue() {
            return Node(true);
        }

        Node LoadFalse() {
            return Node(false);
        }

        Node LoadInt(int value) {
            return  Node(value);
        }

        Node LoadDouble(double value) {
            return  Node(value);
        }

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        template <typename Val>
        void CheckParsingData(istream& input, Val value) {
            string line;
            for (char c; input >> c;) {
                if (c == ']') {
                    input.putback(c);
                    break;
                }
                if (c == '}') {
                    input.putback(c);
                    break;
                }
                if (c == ',') {
                    input.putback(c);
                    break;
                }
                line.push_back(c);
            }
            if (line != value) {
                throw ParsingError("Failed to read parsing data"s);
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            if ((c == ']') || (c == '}')) {
                throw ParsingError("Failed to read parsing data"s);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {    //nullptr
                input.putback(c);
                CheckParsingData(input, "null");
                return LoadNull();

            }
            else if (c == 't') {    //true
                input.putback(c);
                CheckParsingData(input, "true");
                return LoadTrue();
            }
            else if (c == 'f') {    //false
                input.putback(c);
                CheckParsingData(input, "false");
                return LoadFalse();
            }
            else {
                input.putback(c);
                Number value = LoadNumber(input);
                if (std::holds_alternative<int>(value)) {
                    return LoadInt(get<int>(value));
                }
                return LoadDouble(get<double>(value));
            }
        }

    }  // namespace

    //---------------------------- Node ----------------------------
    Node::Node(Value&& value) : Value(move(value)) {}

    const Array& Node::AsArray() const {
        if (!IsArray()) {
            throw logic_error("Data have another type, not array"s);
        }
        return get<Array>(*this);
    }

    bool Node::IsArray() const {
        if (std::holds_alternative<Array>(*this)) {
            return true;
        }
        return false;
    }

    const Dict& Node::AsDict() const {
        if (!IsDict()) {
            throw logic_error("Data have another type, not map"s);
        }
        return get<Dict>(*this);
    }

    bool Node::IsDict() const {
        if (std::holds_alternative<Dict>(*this)) {
            return true;
        }
        return false;
    }

    int Node::AsInt() const {
        if (!IsDouble()) {
            throw logic_error("Data have another type"s);
        }
        return get<int>(*this);
    }

    bool Node::IsInt() const {
        if (std::holds_alternative<int>(*this)) {
            return true;
        }
        return false;
    }

    double Node::AsDouble() const {
        if (!IsDouble()) {
            throw logic_error("Data have another type"s);
        }
        if (std::holds_alternative<int>(*this)) {
            return get<int>(*this);
        }
        return get<double>(*this);
    }

    bool Node::IsDouble() const {
        if (std::holds_alternative<double>(*this) || std::holds_alternative<int>(*this)) {
            return true;
        }
        return false;
    }

    const string& Node::AsString() const {
        if (!IsString()) {
            throw logic_error("Data have another type"s);
        }
        return get<string>(*this);
    }

    bool Node::IsString() const {
        if (std::holds_alternative<string>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::AsBool() const {
        if (!IsBool()) {
            throw logic_error("Data have another type"s);
        }
        return get<bool>(*this);
    }

    bool Node::IsBool() const {
        if (std::holds_alternative<bool>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsPureDouble() const {
        if (std::holds_alternative<double>(*this)) {
            return true;
        }
        return false;
    }

    bool Node::IsNull() const {
        if (std::holds_alternative<nullptr_t>(*this)) {
            return true;
        }
        return false;
    }

    const JsonType& Node::GetJsonType() const {
        return *this;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }
    //----------------- Print JSON formar ---------------------------------
    void PrintObj(ostream& output, nullptr_t /*null*/) {
        output << "null";
    }

    void PrintObj(ostream& output, const bool& value) {
        output << boolalpha << value;
    }

    void PrintObj(ostream& output, const double& value) {
        output << value;
    }

    void PrintObj(ostream& output, const int& value) {
        output << value;
    }

    void PrintObj(ostream& output, const string& line) {
        output << "\""s;
        for (char symbol : line) {
            if (symbol == '"') {
                output << "\\";
            }
            if (symbol == '\n') {
                output << "\\n";
                continue;
            }
            if (symbol == '\t') {
                output << "\\t";
                continue;
            }
            if (symbol == '\r') {
                output << "\\r";
                continue;
            }
            output << symbol;
        }
        output << "\"";
    }

    void PrintObj(ostream& output, const Dict& dict) {
        output << "{";
        output << endl;
        bool status = false;
        for (const auto& [key_str, json_type] : dict) {
            if (status) {
                output << ",";
                output << endl;
            }
            PrintObj(output, key_str);
            output << ":";
            std::visit([&output](const auto& value) {
                PrintObj(output, value);
                }, json_type.GetJsonType());
            status = true;
        }
        output << endl;
        output << "}";
    }

    void PrintObj(ostream& output, const Array& array) {
        output << "[";
        output << endl;
        bool status = false;
        for (auto& json_type : array) {
            if (status) {
                output << ",";
                output << endl;
            }
            std::visit([&output](const auto& value) {
                PrintObj(output, value);
                }, json_type.GetJsonType());
            status = true;
        }
        output << endl;
        output << "]";
    }

    void Print(const Document& doc, ostream& output) {
        auto json_type = doc.GetRoot().GetJsonType();
        std::visit([&output](const auto& value) {
            PrintObj(output, value);
            }, json_type);
    }

    bool operator==(const Array& lhs, const Array& rhs) {
        bool res = (lhs.size() == rhs.size());
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end()) && res;
    }

    bool operator==(const Node& node1, const Node& node2) {
        if (node1.GetJsonType() == node2.GetJsonType()) {
            return true;
        }
        return false;
    }

    bool operator!=(const Node& node1, const Node& node2) {
        if (node1.GetJsonType() == node2.GetJsonType()) {
            return false;
        }
        return true;
    }

    bool operator==(const Document& lhs, const Document& rhs) {
        if (lhs.GetRoot() == rhs.GetRoot()) {
            return true;
        }
        return false;
    }

    bool operator!=(const Document& lhs, const Document& rhs) {
        if (lhs.GetRoot() == rhs.GetRoot()) {
            return false;
        }
        return true;
    }
}  // namespace json
