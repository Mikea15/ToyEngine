
#pragma once

#include <iostream>
#include <functional>
#include <queue>

namespace container 
{
    template<typename T>
    struct Node
    {
        Node(T key)
            : Key(key)
        { }

        T Key = 0;
        Node<T>* Left = nullptr;
        Node<T>* Right = nullptr;
    };

    template<typename T>
    class BST
    {
    public:
        typedef T           value_type;
        typedef T*          pointer;
        typedef const T*    const_pointer;
        typedef T&          reference;
        typedef const T&    const_reference;

        BST(T rootItem);
        ~BST();

        void insert(T item);
        void erase(T item);
        Node<T>* find(T item);

        void traverseBreadthFirst() const;
        void traverseInOrder() const;
        void traversePreOrder() const;
        void traversePostOrder() const;

    private:
        Node<T>* insert(Node<T>* node, T item);
        Node<T>* erase(Node<T>* node, T item);
        Node<T>* find(Node<T>* node, T item);

        void traverseBreadthFirst(Node<T>* node) const;
        void traverseInOrder(Node<T>* node) const;
        void traversePreOrder(Node<T>* node) const;
        void traversePostOrder(Node<T>* node) const;

        Node<T>* minValueNode(Node<T>* node);
        
    private:
        Node<T>* m_root = nullptr;
    };

    template<typename T>
    inline BST<T>::BST(T rootItem)
    {
        insert(rootItem);
    }

    template<typename T>
    inline BST<T>::~BST()
    {
    }

    template<typename T>
    inline void BST<T>::insert(T item)
    {
        m_root = insert(m_root, item);
    }

    template<typename T>
    inline void BST<T>::erase(T item)
    {
        m_root = erase(m_root, item);
    }

    template<typename T>
    inline Node<T>* BST<T>::find(T item)
    {
        return NULL;
    }

    template<typename T>
    inline void BST<T>::traverseBreadthFirst() const
    {
        traverseBreadthFirst(m_root);
    }

    template<typename T>
    inline void BST<T>::traverseInOrder() const
    {
        traverseInOrder(m_root);
    }

    template<typename T>
    inline void BST<T>::traversePreOrder() const
    {
        traversePreOrder(m_root);
    }

    template<typename T>
    inline void BST<T>::traversePostOrder() const
    {
        traversePostOrder(m_root);
    }

    template<typename T>
    inline Node<T>* BST<T>::insert(Node<T>* node, T item)
    {
        if (node == nullptr)
        {
            return new Node<T>(item);
        }
        if (item < node->Key)
        {
            node->Left = insert(node->Left, item);
        }
        else
        {
            node->Right = insert(node->Right, item);
        }
        return node;
    }

    template<typename T>
    inline Node<T>* BST<T>::erase(Node<T>* node, T item)
    {
        if (node == nullptr) 
        {
            return node;
        }

        if (node->Key > item) 
        {
            node->Left = erase(node->Left, item);
            return node;
        }
        
        if (node->Key < item) 
        {
            node->Right = erase(node->Right, item);
            return node;
        }

        if (node->Left == nullptr)
        {
            Node<T>* temp = node->Right;
            delete node;
            return temp;
        }

        if (node->Right == nullptr)
        {
            Node<T>* temp = node->Left;
            delete node;
            return temp;
        }

        Node<T>* succParent = node->Right;

        Node<T>* succ = node->Right;
        while (succ->Left != nullptr) 
        {
            succParent = succ;
            succ = succ->Left;
        }

        succParent->Left = succ->Right;
        node->Key = succ->Key;
        delete succ;

        return node;
    }

    template<typename T>
    inline Node<T>* BST<T>::find(Node<T>* node, T item)
    {
        if (node == NULL || node->Key == item) 
        {
            return node;
        }

        if (node->Key < item) 
        {
            return find(node->Right, item);
        }

        return find(node->Left, item);
    }

    template<typename T>
    inline void BST<T>::traverseBreadthFirst(Node<T>* node) const
    {
        std::queue<Node<T>*> queue;
        std::vector<Node<T>*> visited;
        queue.push(node);
        while (!queue.empty()) 
        {
            Node<T>* current = queue.front();
            queue.pop();
            visited.push_back(current);

            std::cout << current->Key << " ";

            if (current->Right && (std::find(visited.begin(), visited.end(), current->Right) == visited.end()))
            {
                queue.push(current->Right);
            }

            if (current->Left && (std::find(visited.begin(), visited.end(), current->Left) == visited.end()))
            {
                queue.push(current->Left);
            }
        }
    }

    template<typename T>
    inline void BST<T>::traverseInOrder(Node<T>* node) const
    {
        if (node != NULL)
        {
            traverseInOrder(node->Left);
            std::cout << " " << node->Key;
            traverseInOrder(node->Right);
        }
    }

    template<typename T>
    inline void BST<T>::traversePreOrder(Node<T>* node) const
    {
        if (node != NULL)
        {
            std::cout << " " << node->Key;
            traversePreOrder(node->Left);
            traversePreOrder(node->Right);
        }
    }

    template<typename T>
    inline void BST<T>::traversePostOrder(Node<T>* node) const
    {
        if (node != NULL)
        {
            traversePostOrder(node->Left);
            traversePostOrder(node->Right);
            std::cout << " " << node->Key;
        }
    }

    template<typename T>
    inline Node<T>* BST<T>::minValueNode(Node<T>* node)
    {
        Node<T>* current = node;
        while (current && current->Left != nullptr) 
        {
            current = current->Left;
        }
        return current;
    }
}

