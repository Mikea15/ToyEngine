#pragma once

#include <vector>
#include <functional>

/*

template<typename T, int min = 2, int max = 4>
class Node
{
public:
	Node();
	void MoveLeft(int k);
	void MoveRight(int k);
	void Combine(int k);
	void Successor(int k);
	void Restore(int k);
	void Remove(int k);

private:
	int Count;
	std::vector<T> Keys;
	std::vector<Node<T>> Branch;
};

template<typename T, int min = 2, int max = 4, class Compare = std::less<T>>
class BTree
{
public:
	BTree();
	bool Exists(T target);

	void Add(T key);
	void Remove(T key);

private:
	bool Search(T target, Node<T> root, Node<T>& outTargetNode, int& outTargetPosition);
	bool SearchNode(T target, Node<T> root, int& outPosition);
	bool Insert(T key, Node<T>& root);
	bool PushDown(T key, Node<T> p, T& x, Node<T>& xr);
	void PushIn(T x, Node<T> xr, Node<T>& p, int k);
	bool Split(T x, Node<T> xr, Node<T> p, int k, T& y, Node<T>& yr);
	void Delete(T target, Node<T>& root);
	bool RecDelete(T target, Node<T> p);

private:
	Node<T> Root;

};
*/
/*
// BTrees in C#

namespace BTrees
{
    enum Limits { Maximum = 4, Minimum = 2 }
    enum Test { Maximum = 100000 };

    class Program
    {
        static void Main(string[] args)
        {
            BTree<string> bTree = new BTree<string>();

            for (int i = 0; i < (int)Test.Maximum; i++)
                bTree.Add("String" + i);

            DateTime dtBTreeStart = DateTime.Now;

            for (int i = 0; i < (int)Test.Maximum; i++)
                if (!bTree.Exists("String" + i))
                    Console.WriteLine("String" + i + " doesn't ecsist.");

            DateTime dtBTreeEnd = DateTime.Now;

            Console.WriteLine("BTree searches took: {0}", dtBTreeEnd - dtBTreeStart);

            for (int i = 0; i < (int)Test.Maximum; i++)
                bTree.Remove("String" + i);
        }
    }
}
*/


