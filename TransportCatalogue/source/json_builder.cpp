#include "json_builder.h"

namespace json {
	Builder::Builder() {}

	DictItemContext Builder::StartDict() {
		if (nodes_stack_.empty()) {
			nodes_stack_.push_back(new Node(Dict{}));
		}
		else if (nodes_stack_.back()->IsArray()) {
			auto& new_node = const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Node(Dict{}));
			nodes_stack_.push_back(&new_node);
		}
		else if (nodes_stack_.back()->IsDict() && (value_ != nullptr)) {
			*value_ = Node(Dict{});
			nodes_stack_.push_back(value_);
			value_ = nullptr;
		}
		else {
			throw std::logic_error("Method StartDict can't called"s);
		}
		return { *this };
	}

	Builder& Builder::EndDict() {
		if (nodes_stack_.empty() || nodes_stack_.back()->IsArray()) {
			throw std::logic_error("Method EndDict can't called"s);
		}
		else if (nodes_stack_.back()->IsDict() && (value_ != nullptr)) {
			throw std::logic_error("Method EndDict can't called"s);
		}
		else if (nodes_stack_.size() == 1) {
			if (!nodes_stack_.back()->IsDict()) {
				throw std::logic_error("Method EndDict can't called"s);
			}
			root_ = move(*nodes_stack_[0]);
			return *this;
		}
		nodes_stack_.pop_back();
		return *this;
	}

	KeyItemContext Builder::Key(std::string key) {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && (value_ == nullptr)) {
			value_ = &const_cast<Dict&>(nodes_stack_.back()->AsDict()).insert({ move(key), Node() }).first->second;
		}
		else {
			throw std::logic_error("Method Key can't called"s);
		}
		return { *this };
	}

	ArrayItemContext Builder::StartArray() {
		if (nodes_stack_.empty()) {
			nodes_stack_.push_back(new Node(Array{}));
		}
		else if (nodes_stack_.back()->IsArray()) {
			auto& new_node = const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(Node(Array{}));
			nodes_stack_.push_back(&new_node);
		}
		else if (nodes_stack_.back()->IsDict() && (value_ != nullptr)) {
			*value_ = Node(Array{});
			nodes_stack_.push_back(value_);
			value_ = nullptr;
		}
		else {
			throw std::logic_error("Method StartArray can't called"s);
		}
		return { *this };
	}

	Builder& Builder::EndArray() {
		if (nodes_stack_.empty() || nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Method EndArray can't called"s);
		}
		else if (nodes_stack_.back()->IsDict() && (value_ != nullptr)) {
			throw std::logic_error("Method EndArray can't called"s);
		}
		else if (nodes_stack_.size() == 1) {
			if (!nodes_stack_.back()->IsArray()) {
				throw std::logic_error("Method EndArray can't called"s);
			}
			root_ = move(*nodes_stack_[0]);
			return *this;
		}
		nodes_stack_.pop_back();
		return *this;
	}

	Builder& Builder::Value(Node::Value value) {
		if (nodes_stack_.empty()) {
			nodes_stack_.push_back(new Node(move(value)));
			root_ = move(*nodes_stack_[0]);
			return *this;
		}
		else if (nodes_stack_.back()->IsDict() && (value_ != nullptr)) {
			*value_ = std::move(value);
			value_ = nullptr;
		}
		else if (nodes_stack_.back()->IsArray()) {
			const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(move(value));
		}
		else {
			throw std::logic_error("Method Value can't called"s);
		}
		return *this;
	}

	Node Builder::Build() {
		if (nodes_stack_.size() > 1 || (nodes_stack_.size() == 0)) {
			throw std::logic_error("JSON document was not completed"s);
		}
		else {
			nodes_stack_.pop_back();
		}
		return root_;
	}

	//---------------- DictItemContext ----------------
	DictItemContext::DictItemContext(Builder& builder): builder_(builder) {}

	Builder& DictItemContext::EndDict() {
		return builder_.EndDict();
	}

	KeyItemContext DictItemContext::Key(std::string key) {
		return builder_.Key(move(key));
	}
	//---------------- ArrayItemContext ----------------
	ArrayItemContext::ArrayItemContext(Builder& builder): builder_(builder) {}

	DictItemContext ArrayItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ArrayItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ArrayItemContext::EndArray() {
		return builder_.EndArray();
	}

	ArrValueItemContext ArrayItemContext::Value(Node::Value value) {
		return builder_.Value(move(value));
	}
	//---------------- KeyItemContext ----------------
	KeyItemContext::KeyItemContext(Builder& builder): builder_(builder) {}

	DictItemContext KeyItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext KeyItemContext::StartArray() {
		return builder_.StartArray();
	}

	ValueItemContext KeyItemContext::Value(Node::Value value) {
		return builder_.Value(move(value));
	}
	//---------------- ValueItemContext ----------------
	ValueItemContext::ValueItemContext(Builder& builder): builder_(builder) {}

	KeyItemContext ValueItemContext::Key(std::string key) {
		return builder_.Key(move(key));
	}

	Builder& ValueItemContext::EndDict() {
		return builder_.EndDict();
	}
	//---------------- ArrValueItemContext ----------------
	ArrValueItemContext::ArrValueItemContext(Builder& builder): builder_(builder) {}

	ArrValueItemContext ArrValueItemContext::Value(Node::Value value) {
		return builder_.Value(move(value));
	}

	DictItemContext ArrValueItemContext::StartDict() {
		return builder_.StartDict();
	}

	ArrayItemContext ArrValueItemContext::StartArray() {
		return builder_.StartArray();
	}

	Builder& ArrValueItemContext::EndArray() {
		return builder_.EndArray();
	}
}
