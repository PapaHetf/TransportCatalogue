#pragma once
#include "json.h"

namespace json {
	using namespace std::literals;

	class DictItemContext;
	class ArrayItemContext;
	class ValueItemContext;
	class KeyItemContext;
	class ArrValueItemContext;

	class Builder {
	public:
		Builder();
		DictItemContext StartDict();
		Builder& EndDict();
		KeyItemContext Key(std::string key);
		ArrayItemContext StartArray();
		Builder& EndArray();
		Builder& Value(Node::Value value);
		Node Build();
	private:
		Node root_;
		std::vector<Node*> nodes_stack_ = {};
		Node* value_ = nullptr;
	};

	class DictItemContext : private Builder {
	public:
		DictItemContext(Builder& builder);
		KeyItemContext Key(std::string key);
		Builder& EndDict();
	private:
		Builder& builder_;
	};

	class ArrayItemContext : private Builder {
	public:
		ArrayItemContext(Builder& builder);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();
		ArrValueItemContext Value(Node::Value value);

	private:
		Builder& builder_;
	};

	class KeyItemContext : private Builder {
	public:
		KeyItemContext(Builder& builder);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		ValueItemContext Value(Node::Value value);
	private:
		Builder& builder_;
	};

	class ValueItemContext : private Builder {
	public:
		ValueItemContext(Builder& builder);
		KeyItemContext Key(std::string key);
		Builder& EndDict();

		ArrayItemContext StartArray() = delete;
	private:
		Builder& builder_;
	};

	class ArrValueItemContext : private Builder {
	public:
		ArrValueItemContext(Builder& builder);
		ArrValueItemContext Value(Node::Value value);
		DictItemContext StartDict();
		ArrayItemContext StartArray();
		Builder& EndArray();

	private:
		Builder& builder_;
	};
}