# Tree: A Standard Library Compatible N-ary Tree

The project contained in this repository represents a Standard Library compatible, templated tree data structure. Each node in this tree can have any number of sibling and child nodes, but only one parent. By providing the necessary functions and typedefs, this library aims to be fully compatible with all iterator based STL algorithms. In order to provide this compatible, there are several iterator classes available to traverse the tree: a pre-order iterator, a post-order iterator, a leaf iterator, and a sibling iterator. Please note that an in-order iterator has not been provided because there is no intuitive way to traverse an N-ary tree in an in-order fashion for any N greater than two.

# Usage Examples

The following is an example of how one might construct a simple, binary tree:

```C++
#include <Tree.hpp>
//...

Tree<std::string> tree{ "Head" };
tree.GetHead()->AppendChild("Left Child");
tree.GetHead()->AppendChild("Right Child");
tree.GetHead()->GetFirstChild()->AppendChild("First Grandchild of Left Child");
tree.GetHead()->GetFirstChild()->AppendChild("Second Grandchild of Left Child");
```

Iterating over the tree in a post-order fashion isn't very hard either. Continuing with the `Tree<std::string>` constructed above, and using a `std::for_each` to perform some unspecified task, one might write the following to iterate over the tree:

```C++
std::for_each(std::begin(tree), std::end(tree),
	[] (Tree<std::string>::const_reference node)
{
	const auto& data = node.GetData();
	std::cout << "Data: " << data << std::endl;
});
```

The `Tree<DataType>` class defines the `begin()` and `end()` functions to return a `Tree<DataType>::PostOrderIterator`. If, instead, you'd like to iterate over the tree in a pre-order fashion, the following should fit the bill:

```C++
std::for_each(tree.beginPreOrder(), tree.endPreOrder(),
	[] (Tree<std::string>::const_reference node)
{
	const auto& data = node.GetData();
	std::cout << "Data: " << data << std::endl;
});
```

Performing an iteration over the leaf nodes is very similar; just call `beginLeaf()` and `endLeaf()` on the `Tree<DataType>` object.

In some cases, you may not want to iterate over the whole tree, but only over a portion of the larger tree. In that case, the following technique can be used to iterate over a subtree:

```C++
const TreeNode<std::string>* someNode = FetchSomeRandomNode();

std::for_each(
   Tree<std::string>::LeafIterator{ someNode },
   Tree<std::string>::LeafIterator{ },
   [&] (Tree<std::string>::const_reference node)
{
	const auto& data = node.GetData();
	std::cout << "Data: " << data << std::endl;
});
```

In the above example, notice that you can construct any iterator from any `TreeNode<DataType>` object without having to go through an instance of `Tree<DataType>`. Also note that while the example above uses a `LeafIterator`, the use of any of the other iterator types is also perfectly valid.

For more examples, check out the benchmarks and the unit tests.

# Graphviz Support

Using the `TreeUtilities.hpp` header, you can now also generate DOT files for use with Graphviz. This means that you can now quickly and easily visualize the structure of the tree. In order to generate a DOT file, simply pass the Tree object to be visualized to `TreeUtilities::OutputToDotFile(...)`, along with the desired output path and filename. For example:

```C++
Tree<std::string> tree{ "F" };
tree.GetHead()->AppendChild("B")->AppendChild("A");
tree.GetHead()->GetFirstChild()->AppendChild("D")->AppendChild("C");
tree.GetHead()->GetFirstChild()->GetLastChild()->AppendChild("E");
tree.GetHead()->AppendChild("G")->AppendChild("I")->AppendChild("H");

TreeUtilities::OutputToDotFile(tree, "C:\\PathToFile\\TreeGraph.dot");
```

The code above will generate the following DOT file as output:

```
digraph {
   rankdir = TB;
   edge [arrowsize=0.4, fontsize=10]
   
   // Node Declarations:
   2035463130976 [label = "F"]
   2035463131456 [label = "B"]
   2035463131072 [label = "A"]
   2035463131264 [label = "D"]
   2035463131360 [label = "C"]
   2035463132224 [label = "E"]
   2035463131648 [label = "G"]
   2035463131744 [label = "I"]
   2035463214128 [label = "H"]
   
   // Edge Declarations:
   2035463130976 -> 2035463131456
   2035463131456 -> 2035463131072
   2035463131456 -> 2035463131264
   2035463131264 -> 2035463131360
   2035463131264 -> 2035463132224
   2035463130976 -> 2035463131648
   2035463131648 -> 2035463131744
   2035463131744 -> 2035463214128
}
```

In order to guarantee unique node names, the address of each node in memory is used as an identifier. Once the DOT file has been created, the visualization can be generated by running the following command from the command prompt:

```
$>dot -Tpng C:\PathToFile\TreeGraph.dot -O
```

And all this leaves us with the following image:

![Graphviz Example]
(https://github.com/TimSevereijns/Tree/blob/master/screenshots/TreeGraph.png)
