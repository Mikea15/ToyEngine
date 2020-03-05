#include "BTree.h"

/*
template<typename T, int min, int max>
Node<T, min, max>::Node()
	: Count(0)
{
	Keys.resize(max);
	Branch.resize(max + 1);
}

template<typename T, int min, int max>
void Node<T, min, max>::MoveLeft(int k)
{
	Branch[k - 1].Count++;
	Branch[k - 1].Keys[Branch[k - 1].Count - 1] = Keys[k - 1];
	Branch[k - 1].Branch[Branch[k - 1].Count - 1] = Branch[k].Branch[0];

	Keys[k - 1] = Branch[k].Keys[0];
	Branch[k].Branch[0] = Branch[k].Branch[1];
	Branch[k].Count--;

	for (int c = 1; c <= Branch[k].Count; c++)
	{
		Branch[k].Keys[c - 1] = Branch[k].Keys[c];
		Branch[k].Branch[c] = Branch[k].Branch[c + 1];
	}
}

template<typename T, int min, int max>
void Node<T, min, max>::MoveRight(int k)
{
	for (int c = Branch[k].Count; c >= 1; c--)
	{
		Branch[k].Keys[c] = Branch[k].Keys[c - 1];
		Branch[k].Branch[c + 1] = Branch[k].Branch[c];
	}

	Branch[k].Branch[1] = Branch[k].Branch[0];
	Branch[k].Count++;
	Branch[k].Keys[0] = Keys[k - 1];

	Keys[k - 1] = Branch[k - 1].Keys[Branch[k - 1].Count - 1];
	Branch[k].Branch[0] = Branch[k - 1].Branch[Branch[k - 1].Count];
	Branch[k - 1].Count--;
}

template<typename T, int min, int max>
void Node<T, min, max>::Combine(int k)
{
	Node<T> q = Branch[k];

	Branch[k - 1].Count++;
	Branch[k - 1].Keys[Branch[k - 1].Count - 1] = Keys[k - 1];
	Branch[k - 1].Branch[Branch[k - 1].Count] = q.Branch[0];

	for (int c = 1; c <= q.Count; c++)
	{
		Branch[k - 1].Count++;
		Branch[k - 1].Keys[Branch[k - 1].Count - 1] = q.Keys[c - 1];
		Branch[k - 1].Branch[Branch[k - 1].Count] = q.Branch[c];
	}

	for (int c = k; c <= Count - 1; c++)
	{
		Keys[c - 1] = Keys[c];
		Branch[c] = Branch[c + 1];
	}
	Count--;
}

template<typename T, int min, int max>
void Node<T, min, max>::Successor(int k)
{
	Node<T> q = Branch[k];
	while (q.Branch[0] != nullptr) q = q.Branch[0];
	Keys[k - 1] = q.Keys[0];
}

template<typename T, int min, int max>
void Node<T, min, max>::Restore(int k)
{
	if (k == 0)
	{
		if (Branch[1].Count > min)
			MoveLeft(1);
		else
			Combine(1);
	}
	else if (k == Count)
	{
		if (Branch[k - 1].Count > min)
			MoveRight(k);
		else
			Combine(k);
	}
	else
	{
		if (Branch[k - 1].Count > min)
			MoveRight(k);
		else if (Branch[k + 1].Count > min)
			MoveLeft(k + 1);
		else
			Combine(k);
	}
}

template<typename T, int min, int max>
void Node<T, min, max>::Remove(int k)
{
	for (int i = k + 1; i <= Count; i++)
	{
		Keys[i - 2] = Keys[i - 1];
		Branch[i - 1] = Branch[i];
	}
	Count--;
}


template<typename T, int min, int max, class Compare>
BTree<T, min, max, Compare>::BTree()
{
	Root = nullptr;
}

template<typename T, int min, int max, class Compare>
bool BTree<T, min, max, Compare>::Exists(T target)
{
	Node<T> targetNode = nullptr;
	int targetPosition = 0;
	return Search(Target, Root, targetNode, targetPosition);
}

template<typename T, int min, int max, class Compare>
void BTree<T, min, max, Compare>::Add(T key)
{
	Insert(newKey, ref Root);
}

template<typename T, int min, int max, class Compare>
void BTree<T, min, max, Compare>::Remove(T key)
{
	Delete(newKey, ref Root);
}

template<typename T, int min, int max, class Compare>
bool BTree<T, min, max, Compare>::Search(T target, Node<T> root, Node<T>& outTargetNode, int& outTargetPosition)
{
	if (Root == nullptr)
		return false;

	if (SearchNode(Target, Root, targetPosition))
	{
		targetNode = Root;
		return true;
	}
	else
		return Search(Target, Root.Branch[targetPosition], targetNode, targetPosition);
}

template<typename T, int min, int max, class Compare>
bool BTree<T, min, max, Compare>::SearchNode(T target, Node<T> root, int& outPosition)
{
	int iCompare = Compare(Target, Root.Keys[0]);
	if (iCompare < 0)
	{
		Position = 0;
		return false;
	}
	else
	{
		Position = Root.Count;
		iCompare = Compare(Target, Root.Keys[Position - 1]);
		while (iCompare < 0 && Position > 1)
		{
			Position--;
			iCompare = Compare(Target, Root.Keys[Position - 1]);
		}
		return iCompare == 0;
	}
}

template<typename T, int min, int max, class Compare>
bool BTree<T, min, max, Compare>::Insert(T key, Node<T>& root)
{
	T x;
	Node<T> xr;

	if (PushDown(newKey, root, out x, out xr))
	{
		Node<T> p = new Node<T>();
		p.Count = 1;
		p.Keys[0] = x;
		p.Branch[0] = root;
		p.Branch[1] = xr;
		root = p;
	}
}

template<typename T, int min, int max, class Compare>
bool BTree<T, min, max, Compare>::PushDown(T key, Node<T> p, T& x, Node<T>& xr)
{
	bool pushUp = false;
	int k = 1;

	if (p == null)
	{
		pushUp = true;
		x = newKey;
		xr = null;
	}
	else
	{
		if (SearchNode(newKey, p, k)) {
			//LOG Entry already present
			return false;
		};

		if (PushDown(newKey, p.Branch[k], x, xr))
		{
			if (p.Count < max)
			{
				pushUp = false;
				PushIn(x, xr, ref p, k);
			}
			else
			{
				pushUp = true;
				Split(x, xr, p, k, ref x, ref xr);
			}
		}
	}

	return pushUp;
}

template<typename T, int min, int max, class Compare>
void BTree<T, min, max, Compare>::PushIn(T x, Node<T> xr, Node<T>& p, int k)
{
	for (int i = p.Count; i >= k + 1; i--)
	{
		p.Keys[i] = p.Keys[i - 1];
		p.Branch[i + 1] = p.Branch[i];
	}
	p.Keys[k] = x;
	p.Branch[k + 1] = xr;
	p.Count++;
}

template<typename T, int min, int max, class Compare>
bool BTree<T, min, max, Compare>::Split(T x, Node<T> xr, Node<T> p, int k, T& y, Node<T>& yr)
{
	int median = k <= min ? min : min + 1;

	yr = new Node<T>();

	for (int i = median + 1; i <= (int)4; i++)
	{
		yr.Keys[i - median - 1] = p.Keys[i - 1];
		yr.Branch[i - median] = p.Branch[i];
	}

	yr.Count = 2 - median;
	p.Count = median;

	if (k <= min)
		PushIn(x, xr, p, k);
	else
		PushIn(x, xr, yr, k - median);

	y = p.Keys[p.Count - 1];
	yr.Branch[0] = p.Branch[p.Count];

	p.Count--;

	return true;
}

template<typename T, int min, int max, class Compare>
void BTree<T, min, max, Compare>::Delete(T target, Node<T>& root)
{
	if (!RecDelete(Target, Root)) {
		// LOG Error, entry not found
		return;
	}
	else if (root.Count == 0)
	{
		root = root.Branch[0];
	}
}

template<typename T, int min, int max, class Compare>
bool BTree<T, min, max, Compare>::RecDelete(T target, Node<T> p)
{
	int k = 0;
	bool found = false;

	if (p == null)
		return false;
	else
	{
		found = SearchNode(Target, p, k);
		if (found)
		{
			if (p.Branch[k - 1] == null)
				p.Remove(k);
			else
			{
				p.Successor(k);
				if (!RecDelete(p.Keys[k - 1], p.Branch[k])) {
					// LOG Error: Entry not found
					return false;
				}
			}
		}
		else
			found = RecDelete(Target, p.Branch[k]);

		if (p.Branch[k] != null)
			if (p.Branch[k].Count < min)
				p.Restore(k);

		return found;
	}
}
*/
