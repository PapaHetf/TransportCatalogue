#pragma once

#include <sstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using JsonType = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : private  std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:

        using variant::variant;
        using Value = variant;

        Node(Value&& value);

        const Array& AsArray() const;
        bool IsArray() const;

        const Dict& AsDict() const; 
        bool IsDict() const;

        int AsInt() const;
        bool IsInt() const;

        double AsDouble() const;
        bool IsDouble() const;

        const std::string& AsString() const;
        bool IsString() const;

        bool AsBool() const;
        bool IsBool() const;

        bool IsPureDouble() const;

        bool IsNull() const;

        const JsonType& GetJsonType() const;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    bool operator==(const Array& lhs, const Array& rhs);

    bool operator==(const Node& node1, const Node& node2);

    bool operator!=(const Node& node1, const Node& node2);

    bool operator==(const Document& lhs, const Document& rhs);

    bool operator!=(const Document& lhs, const Document& rhs);

    void PrintObj(std::ostream& output, nullptr_t /*null*/);

    void PrintObj(std::ostream& output, const bool& value);

    void PrintObj(std::ostream& output, const double& value);

    void PrintObj(std::ostream& output, const int& value);

    void PrintObj(std::ostream& output, const std::string& line);

    void PrintObj(std::ostream& output, const Dict& dict);

    void PrintObj(std::ostream& output, const Array& array);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
