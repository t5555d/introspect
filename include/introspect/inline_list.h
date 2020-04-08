#pragma once

#include "fwd.h"

INTROSPECT_NS_OPEN;

//
// class for organizing linked list of a statically allocated nodes
// Item should be inherited from inline_list::node
//

template <typename Item>
class inline_list
{
public:

	class node
	{
	protected:
		// prev <-> this <-> next
		node(inline_list& list)
		{
			list.insert_before(this, &list.tail);
		}

	private:
		node() {
			prev = next = this;
		}

	private:
		node *prev, *next;
		friend class inline_list;
	};

	template<typename Node, typename Value>
	class basic_iterator
	{
	public:
		typedef Value value_type;
		typedef Value* pointer;
		typedef Value& reference;

		basic_iterator& operator++() {
			node = node->next;
			return *this;
		}

		basic_iterator& operator--() {
			node = node->prev;
			return *this;
		}

		basic_iterator operator++(int) { return{ node->next }; }
		basic_iterator operator--(int) { return{ node->prev }; }

		reference operator*() const { return static_cast<reference>(*node); }
		pointer operator->() const { return static_cast<pointer>(node); }

		bool operator==(const basic_iterator& that) const { return node == that.node; }
		bool operator!=(const basic_iterator& that) const { return node != that.node; }

		basic_iterator(const basic_iterator& node) = default;

	private:

		basic_iterator() = default;
		basic_iterator(Node *node) :
			node(node) {}

		friend class inline_list;

		Node *node = nullptr;
	};

	using iterator = basic_iterator<node, Item>;
	using const_iterator = basic_iterator<const node, const Item>;

	inline_list() = default;
	iterator begin() { return { tail.next }; }
	iterator end() { return { &tail }; }
	const_iterator cbegin() const { return { tail.next }; }
	const_iterator cend() const { return { &tail }; }
	const_iterator begin() const { return cbegin(); }
	const_iterator end() const { return cend(); }

protected:
	node tail;

	// prev <-> item <-> next
	void insert_before(node *item, node *next) {
		item->next = next;
		auto prev = item->prev = next->prev;
		prev->next = next->prev = item;
	}

	// prev <-> item <-> next
	void insert_after(node *item, node *prev) {
		item->prev = prev;
		auto next = item->next = prev->next;
		prev->next = next->prev = item;
	}

};

INTROSPECT_NS_CLOSE;
