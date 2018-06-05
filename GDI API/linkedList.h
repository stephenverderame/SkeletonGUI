#pragma once
#pragma once
#include <initializer_list>
template<typename T>
struct linkedNode {
	T data;
	linkedNode<T> * next;
	linkedNode(T data, linkedNode<T> * lnode) : data(data), next(lnode) {};
};
template<typename T>
class LinkedList {
	//similar to std::vector - good at appending elements to the rear of the list
private:
	linkedNode<T> * root;
	void pushBack(linkedNode<T> * node, T & value) {
		if (node->next == nullptr)
			node->next = new linkedNode<T>(value, nullptr);
		else
			pushBack(node->next, value);
	}
	bool test(linkedNode<T> * node, const T & testVal) {
		if (node == nullptr)
			return false;
		else if (node->data == testVal)
			return true;
		else
			test(node->next, testVal);
	}
	void deleteNode(linkedNode<T> * node) {
		if (node->next != nullptr)
			deleteNode(node->next);
		delete node;
	}
public:
	class iterator;
public:
	LinkedList() { root = nullptr; };
	LinkedList(std::initializer_list<T> list) {
		for (auto it = list.begin(); it != list.end(); it++) {
			pushBack(*it);
		}
	}
	~LinkedList() {
		if (root != nullptr)
			deleteNode(root);
	}
	void pushBack(T value) {
		if (root == nullptr)
			root = new linkedNode<T>(value, nullptr);
		else
			pushBack(root, value);
	}
	bool remove(T value) {
		linkedNode<T> * prev = nullptr;
		linkedNode<T> * x = root;
		while (x != nullptr) {
			if (x->data == value) {
				if(prev != nullptr)
					prev->next = x->next;
				delete x;
				return true;
			}
			prev = x;
			x = x->next;

		}
		return false;
	}
	bool find(const T & testVal) {
		if (root == nullptr)
			return false;
		else
			return test(root, testVal);
	}
	iterator begin() {
		return iterator(root);
	}
	iterator end() {
		return iterator(nullptr);
	}

};
template<typename T>
class LinkedList<T>::iterator {
private:
	linkedNode<T> * currentNode;
public:
	iterator(linkedNode<T> * n) : currentNode(n) {};
	iterator & operator++(int) {
		currentNode = currentNode->next;
		return *this;
	}
	iterator & operator++() {
		currentNode = currentNode->next;
		return *this;
	}
	T & operator*() {
		return currentNode->data;
	}
	bool operator!=(const iterator & other) {
		return currentNode != other.currentNode;
	}

};