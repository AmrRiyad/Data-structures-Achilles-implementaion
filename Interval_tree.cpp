#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace std;

/// The node of the Interval tree
struct Node {
    int l, r; /// the left end and the right end of the interval
    int max_r; /// maximum right end in the subtree of this node (this node is included)
    Node *left_child;
    Node *right_child;
    Node *parent;

    Node(int l, int r)
    {
        this->left_child = nullptr;
        this->right_child = nullptr;
        this->parent = nullptr;
        this->l = l;
        this->r = r;
        this->max_r = r;
    }
};

/// The node of the Linked list
/// is used to store the intersected intervals
struct LinkedListNode {
    int l, r;
    LinkedListNode *next;

    LinkedListNode(Node *node)
    {
        this->l = node->l;
        this->r = node->r;
        this->next = nullptr;
    }
};


class Interval_tree {
private:
    Node *root;
    LinkedListNode *head = nullptr;
    LinkedListNode *tail = nullptr;
    int interval_tree_size;

    /// check if two intervals are intersected
    bool check(int l1, int r1, int l2, int r2)
    {
        int mn = r1 < r2 ? r1 : r2; // min of r1 and r2
        int mx = l1 > l2 ? l1 : l2; // min of l1 and l2
        return mx <= mn;
    }

    /// Traverse the tree to get all intersected intervals
    Node *dfs(Node *current_node, int l, int r)
    {
        if (current_node == nullptr) return nullptr;
        /// check if two intervals are intersected
        if (check(l, r, current_node->l, current_node->r))
            return current_node;

        /// go to the left child if my l is smaller than the left child max right
        if (current_node->left_child && l <= current_node->left_child->max_r)
            return dfs(current_node->left_child, l, r);
        else /// go to the right child
            return dfs(current_node->right_child, l, r);
    }

    /// Traverse the tree to get the first occurrence of the interval and return it's node
    Node *dfs_del(Node *current_node, int l, int r)
    {
        if (current_node == nullptr) return nullptr;
        if (current_node->l == l && current_node->r == r) return current_node;

        Node *res = nullptr;
        /// go to the right child
        if (current_node->right_child && l <= current_node->right_child->max_r)
            res = dfs_del(current_node->right_child, l, r);
        if (res) return res;
        /// go to the left child
        if (current_node->left_child && l <= current_node->left_child->max_r)
            res = dfs_del(current_node->left_child, l, r);
        if (res) return res;
        return nullptr;
    }

    bool DeleteNode(Node *node_to_delete)
    {
        Node *current_node = node_to_delete;

        if (!node_to_delete->left_child && !node_to_delete->right_child) /// A leaf or the root
        {
            if (!node_to_delete->parent) /// The root
            {
                delete node_to_delete;
                root = nullptr;
                return true;
            }
            /// A leaf
            if (node_to_delete->parent->left_child == node_to_delete) /// if node_to_delete if the left child
                node_to_delete->parent->left_child = nullptr;
            if (node_to_delete->parent->right_child == node_to_delete) /// if node_to_delete if the right child
                node_to_delete->parent->right_child = nullptr;
            updateParents(node_to_delete->parent); /// update
            delete node_to_delete;
            return true;
        }

        /// Internal node
        if (current_node->left_child) { /// node_to_delete has a left child
            current_node = current_node->left_child;
            bool f = false;
            while (current_node->right_child) { /// get the most right node
                current_node = current_node->right_child;
                f = true;
            }
            /// copy the values
            node_to_delete->l = current_node->l;
            node_to_delete->r = current_node->r;
            /// Redirecting the pointers of the parent and the child
            if (f) {
                current_node->parent->right_child = nullptr;
            } else {
                current_node->parent->left_child = current_node->left_child;
                if (current_node->left_child)
                    current_node->left_child->parent = current_node->parent;
            }
            updateParents(current_node->parent);
            delete current_node;
            return true;
        } else { /// node_to_delete does not have a left child
            if (current_node->parent) { /// if it is not the root
                if (current_node->parent->right_child == current_node) { /// if current_node if the right child
                    current_node->parent->right_child = current_node->right_child;
                } else { /// if current_node if the left child
                    current_node->parent->left_child = current_node->right_child;
                }
                current_node->right_child->parent = current_node->parent;
                updateParents(current_node->parent);
            } else { /// if it is the root
                current_node->right_child->parent = nullptr;
                root = current_node->right_child;
            }
            delete current_node;
        }
        return true;
    }

    void updateParents(Node *current_node)
    {
        while (current_node)
        {
            if (current_node->left_child && current_node->right_child)
                current_node->max_r = max(current_node->left_child->r, current_node->right_child->r);
            else if (current_node->left_child)
                current_node->max_r = current_node->left_child->max_r;
            else if (current_node->right_child)
                current_node->max_r = current_node->right_child->max_r;
            else
                current_node->max_r = current_node->r;

            current_node->max_r = max(current_node->max_r, current_node->r);
            current_node = current_node->parent;
        }
    }

    /// overridden function
    void InsertInterval(LinkedListNode *node_to_insert)
    {
        InsertInterval(node_to_insert->l, node_to_insert->r);
    }

    void InsertIntervals()
    {
        LinkedListNode *temp = head;
        while (temp)
        {
            InsertInterval(temp);
            temp = temp->next;
        }
    }

    void clearLinkedList()
    {
        LinkedListNode *temp;
        while (head)
        {
            temp = head;
            head = head->next;
            delete temp;
        }
        head = tail = nullptr;
    }

    void printIntervals()
    {
        LinkedListNode *temp = head;
        bool f = false;
        while (temp)
        {
            f = true;
            cout << temp->l << ' ' << temp->r << '\n';
            temp = temp->next;
        }
        if (!f) cout << "Not Found!\n";
    }

public:
    Interval_tree()
    {
        root = nullptr;
        interval_tree_size = 0;
    }

    /// Insert a new interval
    void InsertInterval(int l, int r)
    {
        Node *node_to_insert = new Node(l, r);
        Node *current_node = root;
        /// Special case if the tree is empty
        if (root == nullptr)
        {
            root = node_to_insert;
            interval_tree_size++;
            return;
        }
        /// While current_node not a leaf
        while (true)
        {
            if (node_to_insert->l < current_node->l) /// smaller ? then go left
            {
                if (current_node->left_child == nullptr)
                {
                    node_to_insert->parent = current_node;
                    current_node->left_child = node_to_insert;
                    break;
                }
                current_node = current_node->left_child;
            } else if (node_to_insert->l > current_node->l) /// bigger ? then go right
            {
                if (current_node->right_child == nullptr)
                {
                    node_to_insert->parent = current_node;
                    current_node->right_child = node_to_insert;
                    break;
                }
                current_node = current_node->right_child;
            } else { /// if both left ends of the two intervals are equal then check the right ends
                if (node_to_insert->r < current_node->r) /// smaller ? then go left
                {
                    if (current_node->left_child == nullptr)
                    {
                        node_to_insert->parent = current_node;
                        current_node->left_child = node_to_insert;
                        break;
                    }
                    current_node = current_node->left_child;
                } else /// bigger ? then go right
                {
                    if (current_node->right_child == nullptr)
                    {
                        node_to_insert->parent = current_node;
                        current_node->right_child = node_to_insert;
                        break;
                    }
                    current_node = current_node->right_child;
                }
            }
        }
        /// update max right end for all parents
        updateParents(current_node);
        interval_tree_size++;
    }

    /// auxiliary function
    LinkedListNode *SearchInterval(int l, int r)
    {
        clearLinkedList();
        Node *curr = dfs(root, l, r);
        while (curr)
        {
            if (head == nullptr)
            {
                head = new LinkedListNode(curr);
                tail = head;
            } else
            {
                tail->next = new LinkedListNode(curr);
                tail = tail->next;
            }
            DeleteNode(curr);
            curr = dfs(root, l, r);
        }
        printIntervals();
        InsertIntervals();
        return head;
    }

    /// Delete interval which has left end = l & right end = r
    /// return true if found and deleted, false otherwise.
    bool DeleteInterval(int l, int r)
    {
        /// search for that interval
        Node *node_to_delete = dfs_del(root, l, r);
        if (node_to_delete == nullptr) return false; /// not found
        interval_tree_size--;
        return DeleteNode(node_to_delete);
    }

    Node *get_root()
    {
        return root;
    }
};

int main() {
    Interval_tree t;
    t.InsertInterval(5, 5);
    t.InsertInterval(3, 7);
    t.InsertInterval(1, 2);
    t.InsertInterval(3, 8);
    t.InsertInterval(6, 8);

    //             [5,5]
    //            /     \
    //       [3,7]       [6,8]
    //      /     \
    // [1,2]       [3,8]


    cout << "test1.1:\n";
    t.SearchInterval(1, 4);
    cout << "\ntest1.2:\n";
    t.SearchInterval(1, 10);
    cout << "\ntest1.3:\n";
    t.SearchInterval(0, 0);

    // Delete all the tree
    t.DeleteInterval(3, 7);
    t.DeleteInterval(6, 8);
    t.DeleteInterval(3, 8);
    t.DeleteInterval(5, 5);
    t.DeleteInterval(1, 2);
    t.DeleteInterval(1, 1); // really nega !!!
    t.DeleteInterval(6, 8); // really nega !!!

    t.InsertInterval(1, 2);
    t.InsertInterval(2, 3);
    t.InsertInterval(3, 4);
    t.InsertInterval(4, 5);
    t.InsertInterval(5, 6);
    t.InsertInterval(6, 7);

    //    [1,2]
    //         \
    //          [2,3]
    //               \
    //                [3,4]
    //                     \
    //                      [4,5]
    //                           \
    //                            [5,6]
    //                                 \
    //                                  [6,7]

    cout << "\ntest2.1:\n";
    t.SearchInterval(1, 4);
    cout << "\ntest2.2:\n";
    t.SearchInterval(1, 10);
    cout << "\ntest2.3:\n";
    t.SearchInterval(6, 6);

    t.DeleteInterval(6, 7);

    cout << "\ntest2.4:\n";
    t.SearchInterval(6, 6);
    cout << "\ntest2.5:\n";
    t.SearchInterval(0, 10);

    t.DeleteInterval(1, 2);
    t.DeleteInterval(4, 5);

    cout << "\ntest2.6:\n";
    t.SearchInterval(6, 6);
    cout << "\ntest2.7:\n";
    t.SearchInterval(0, 10);
}
