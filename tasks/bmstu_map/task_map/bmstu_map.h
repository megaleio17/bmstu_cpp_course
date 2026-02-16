#include <algorithm>  //данная библиотека позволяет использовать функции по типу max, min и sort, понадобится потом при балансировке дерева
#include <cstddef>	  // для size_t, чтобы определять размер контейнера
#include <cstdint>	// для типов фиксированного размера, таких как uint8_t для экономии памяти
#include <iostream>
#include <iterator>
#include <stack>  // по нашему заданию надо использовать именно такую структуру хранения данных
#include <stdexcept>			// для исключений, например если ключ не найден
#include <utility>				// нужен нам для хранения пары ключа и значения
#include "abstract_iterator.h"	//наш собственнй заголовочный файл, в котором я буду прописывать работу итератора

namespace bmstu
{

template <typename K, typename V>  // объявляем шаблон, что данная структура
								   // будет работать с любыми типами K и V
struct tree_node
{
	tree_node(const K& k, const V& v)  // специальная функция, которая
									   // вызывается при создании объекта
		: key(k),
		  value(v),
		  left(nullptr),
		  right(nullptr),
		  height(1)	 // после : список инициализации
	{
	}

	K key;	// поле для хранения ключа, например если K=string, то key="apple"
	V value;
	uint8_t height;
	tree_node* left;
	tree_node* right;
};

template <typename K, typename V>
class avl_balanced_tree
{
   public:
	avl_balanced_tree() : root_(nullptr), size_(0) {}

	~avl_balanced_tree() { clear(root_); }

	void insert(const K& key, const V& value)
	{
		this->insert(key, value, root_);
	}

	void remove(const K& key) { this->remove(key, root_); }

	tree_node<K, V>* find(const K& key)
	{
		return this->find(key,
						  root_);  // ищет узел с ключём key и возращает
								   // указатель на него дляизменяемого объекта
	}

	const tree_node<K, V>* find(const K& key) const
	{
		return this->find(key, root_);
	}

	bool contains(const K& key) const { return find(key) != nullptr; }

	size_t size() const { return size_; }

	bool empty() const { return size_ == 0; }

	tree_node<K, V>* get_root() { return root_; }

	const tree_node<K, V>* get_root() const { return root_; }

	void print() { print_tree_(root_, 1); }

	void inorder_print()
	{
		inorder_print(root_);
		std::cout << "\n";
	}

   private:
	tree_node<K, V>* insert(const K& key,
							const V& value,
							tree_node<K, V>*& node)
	{  // tree_node<K, V>*& node, чтобы изменитьсам
	   // указатель, например если node == nullptr
		if (node == nullptr)
		{
			node = new tree_node<K, V>(key, value);
			size_++;
		}

		else if (key < node->key)
		{
			insert(key, value, node->left);
		}

		else if (key > node->key)
		{
			insert(key, value, node->right);
		}

		else
		{
			node->value = value;
		}

		balance(node);
		return node;
	}

	void remove(const K& key, tree_node<K, V>*& node)
	{
		if (node == nullptr)
		{
			return;
		}

		if (key < node->key)
		{
			remove(key, node->left);
		}

		else if (key > node->key)
		{
			remove(key, node->right);
		}

		else
		{
			if (node->left != nullptr && node->right != nullptr)
			{
				tree_node<K, V>* min_right = findMinPtr(node->right);
				node->key = min_right->key;
				node->value = min_right->value;
				remove(node->key, node->right);
			}

			else
			{
				tree_node<K, V>* old_node = node;
				node = (node->left != nullptr) ? node->left : node->right;
				delete old_node;
				size_--;
			}
		}

		balance(node);
	}

	tree_node<K, V>* find(const K& key, tree_node<K, V>* node) const
	{
		if (node == nullptr)
		{
			return nullptr;
		}

		if (key < node->key)
		{
			return find(key, node->left);
		}

		else if (key > node->key)
		{
			return find(key, node->right);
		}

		else
		{
			return node;
		}
	}

	tree_node<K, V>* findMinPtr(tree_node<K, V>* node)
	{
		if (node == nullptr)
		{
			return nullptr;
		}

		while (node->left != nullptr)
		{
			node = node->left;
		}

		return node;
	}

	uint8_t heightOfTree(tree_node<K, V>* t)
	{
		return t == nullptr ? 0 : t->height;
	}

	void rotateWithLeftChild(tree_node<K, V>*& k2)
	{
		tree_node<K, V>* k1 = k2->left;
		k2->left = k1->right;
		k1->right = k2;
		k2->height =
			std::max(heightOfTree(k2->left), heightOfTree(k2->right)) + 1;
		k1->height = std::max(heightOfTree(k1->left), k2->height) + 1;
		k2 = k1;
	}

	void rotateWithRightChild(tree_node<K, V>*& k1)
	{
		tree_node<K, V>* k2 = k1->right;
		k1->right = k2->left;
		k2->left = k1;
		k1->height =
			std::max(heightOfTree(k1->left), heightOfTree(k1->right)) + 1;
		k2->height = std::max(heightOfTree(k2->right), k1->height) + 1;
		k1 = k2;
	}

	void doubleWithLeftChild(tree_node<K, V>*& k3)
	{
		rotateWithRightChild(k3->left);
		rotateWithLeftChild(k3);
	}

	void doubleWithRightChild(tree_node<K, V>*& k1)
	{
		rotateWithLeftChild(k1->right);
		rotateWithRightChild(k1);
	}

	void balance(tree_node<K, V>*& t)
	{
		if (t == nullptr)
		{
			return;
		}

		if (static_cast<int>(heightOfTree(t->left)) -
				static_cast<int>(heightOfTree(t->right)) >
			1)
		{  // защита от арифметики с беззнаковыми числами
			if (heightOfTree(t->left->left) >= heightOfTree(t->left->right))
			{
				rotateWithLeftChild(t);	 // малый правый поворот
			}

			else
			{
				doubleWithLeftChild(t);	 // двойной поворот
			}
		}

		else if (static_cast<int>(heightOfTree(t->right)) -
					 static_cast<int>(heightOfTree(t->left)) >
				 1)
		{
			if (heightOfTree(t->right->right) >= heightOfTree(t->right->left))
			{
				rotateWithRightChild(t);
			}

			else
			{
				doubleWithRightChild(t);
			}
		}

		t->height = std::max(heightOfTree(t->left), heightOfTree(t->right)) +
					1;	// обновление высоты текущего узла, чтобы родительские
						// узлы могли правильно балансироваться
	};

	void inorder_print(tree_node<K, V>* node)
	{
		if (node == nullptr)
		{
			return;
		}

		inorder_print(node->left);
		std::cout << "[" << node->key << ":" << node->value << "] ";
		inorder_print(node->right);
	}

	void clear(tree_node<K, V>* node)
	{
		if (node != nullptr)
		{
			clear(node->left);
			clear(node->right);
			delete node;
		}
	}

	void print_tree_(tree_node<K, V>* node, int space)
	{
		if (node == nullptr)
		{
			return;
		}

		space += 5;
		this->print_tree_(node->right, space);
		for (int i = 0; i < space; ++i)
		{
			std::cout << " ";
		}

		std::cout << node->key << ":" << node->value << "\n";
		this->print_tree_(node->left, space);
	}

	tree_node<K, V>* root_ = nullptr;
	size_t size_ = 0;
};

template <typename K, typename V>
class map
{
   public:
	using key_type = K;
	using mapped_type = V;
	using value_type = std::pair<const K, V>;
	struct iterator : public abstract_iterator<iterator,
											   std::pair<const K, V>,
											   std::bidirectional_iterator_tag>
	{
		tree_node<K, V>* current_;
		std::stack<tree_node<K, V>*> stack_;
		tree_node<K, V>* root_;	 // добавил для поддержки operator--
		mutable typename iterator::value_type pair_cache_;

		iterator() : current_(nullptr), root_(nullptr), pair_cache_(K{}, V{}) {}

		explicit iterator(tree_node<K, V>* root, bool is_end = false)
			: current_(nullptr), root_(root), pair_cache_(K{}, V{})
		{
			if (!is_end && root)
			{
				tree_node<K, V>* node = root;
				while (node)
				{
					stack_.push(node);
					node = node->left;
				}

				if (!stack_.empty())
				{
					current_ = stack_.top();
					stack_.pop();
				}
			}
		}

		typename iterator::reference operator*() const override
		{
			if (current_)
			{
				const_cast<K&>(pair_cache_.first) = current_->key;
				pair_cache_.second = current_->value;
			}

			return pair_cache_;
		}

		typename iterator::pointer operator->() const override
		{
			if (current_)
			{
				const_cast<K&>(pair_cache_.first) = current_->key;
				pair_cache_.second = current_->value;
			}

			return &pair_cache_;
		}

		iterator& operator++() override
		{
			if (current_ == nullptr)
				return *this;

			if (current_->right)
			{
				tree_node<K, V>* node = current_->right;
				while (node)
				{
					stack_.push(node);
					node = node->left;
				}
			}

			if (!stack_.empty())
			{
				current_ = stack_.top();
				stack_.pop();
			}

			else
			{
				current_ = nullptr;
			}

			return *this;
		}

		iterator operator++(int) override
		{
			iterator temp = *this;
			++(*this);
			return temp;
		}

		// реализация operator--
		iterator& operator--() override
		{
			if (current_ == nullptr)
			{  // если мы на самом левом элементе и решим вызвать
			   // operator--, то нас перекинет на самый последний узел
				if (!root_)
					return *this;
				std::stack<tree_node<K, V>*> temp_stack;
				tree_node<K, V>* node = root_;
				while (node)
				{
					temp_stack.push(node);
					node = node->right;
				}

				current_ = temp_stack.top();
				return *this;
			}

			if (current_->left)
			{
				tree_node<K, V>* node = current_->left;
				while (node)
				{
					stack_.push(node);
					node = node->right;
				}

				current_ = stack_.top();
				stack_.pop();
			}

			else
			{
				tree_node<K, V>* candidate = nullptr;
				tree_node<K, V>* node = root_;
				tree_node<K, V>* target = current_;

				while (node && node != target)
				{
					if (target->key > node->key)
					{
						candidate = node;
						node = node->right;
					}

					else
					{
						node = node->left;
					}
				}

				current_ = candidate;
			}

			return *this;
		}

		iterator operator--(int) override
		{
			iterator temp = *this;
			--(*this);
			return temp;
		}

		iterator& operator+=(
			const typename iterator::difference_type& n) override
		{
			return *this;
		}

		iterator& operator-=(
			const typename iterator::difference_type& n) override
		{
			return *this;
		}

		iterator operator+(
			const typename iterator::difference_type& n) const override
		{
			return *this;
		}

		iterator operator-(
			const typename iterator::difference_type& n) const override
		{
			return *this;
		}

		bool operator==(const iterator& other) const override
		{
			return current_ == other.current_;
		}

		bool operator!=(const iterator& other) const override
		{
			return current_ != other.current_;
		}

		explicit operator bool() const override { return current_ != nullptr; }

		typename iterator::difference_type operator-(
			const iterator& other) const override
		{
			return 0;
		}
	};

	map() = default;
	~map() = default;

	void insert(const K& key, const V& value) { tree_.insert(key, value); }

	void insert(const value_type& pair)
	{
		tree_.insert(pair.first, pair.second);
	}

	V& operator[](const K& key)
	{
		auto node = tree_.find(key);
		if (node == nullptr)
		{
			tree_.insert(key, V());
			node = tree_.find(key);
		}
		return node->value;
	}

	V* find(const K& key)
	{
		auto node = tree_.find(key);
		return node ? &node->value : nullptr;
	}

	const V* find(const K& key) const
	{
		auto node = tree_.find(key);
		return node ? &node->value : nullptr;
	}

	V& at(const K& key)
	{
		auto node = tree_.find(key);
		if (node == nullptr)
		{
			throw std::out_of_range("Key not found in map");
		}

		return node->value;
	}

	const V& at(const K& key) const
	{
		auto node = tree_.find(key);
		if (node == nullptr)
		{
			throw std::out_of_range("Key not found in map");
		}

		return node->value;
	}

	void erase(const K& key) { tree_.remove(key); }

	bool contains(const K& key) const { return tree_.contains(key); }

	size_t size() const { return tree_.size(); }

	bool empty() const { return tree_.empty(); }

	void clear()
	{
		tree_.~avl_balanced_tree();
		new (&tree_) avl_balanced_tree<K, V>();
	}

	void print() { tree_.print(); }

	void inorder_print() { tree_.inorder_print(); }

	iterator begin() { return iterator(tree_.get_root(), false); }

	iterator end() { return iterator(tree_.get_root(), true); }

   private:
	avl_balanced_tree<K, V> tree_;
};

}  // namespace bmstu

// 20 test на подсчёт символов
bmstu::map<char, int> getCountOfLetters(const std::string& str)
{
	bmstu::map<char, int> freq_map;
	for (char c : str)
		freq_map[c]++;
	return freq_map;
}

// 21 test на подсчёт слов
bmstu::map<std::string, int> getCountOfWords(const std::string& input_text)
{
	bmstu::map<std::string, int> word_count;

	std::string current_word = "";

	for (int i = 0; i < static_cast<int>(input_text.size()); ++i)
	{
		char symbol = input_text[i];
		if ((symbol >= 'a' && symbol <= 'z') ||
			(symbol >= 'A' && symbol <= 'Z'))
		{
			if (symbol >= 'A' && symbol <= 'Z')
			{
				symbol = symbol - 'A' + 'a';
			}

			current_word += symbol;
		}

		else
		{
			if (current_word != "")
			{
				word_count[current_word] = word_count[current_word] + 1;
				current_word = "";
			}
		}
	}

	if (current_word != "")
	{
		word_count[current_word] = word_count[current_word] + 1;
	}

	return word_count;
}