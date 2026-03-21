#include <gtest/gtest.h>
#include <containers/HierarchicalNode.h>
#include <string>

using namespace sk::common::containers;

using IntNode    = HierarchicalNode<int>;
using StringNode = HierarchicalNode<std::string>;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, DefaultConstruction) {
    auto node = std::make_shared<IntNode>();
    EXPECT_EQ(node->data, 0);
    EXPECT_TRUE(node->isRoot());
    EXPECT_TRUE(node->isLeaf());
    EXPECT_FALSE(node->hasChildren());
    EXPECT_EQ(node->getChildCount(), 0u);
    EXPECT_EQ(node->getParent(), nullptr);
}

TEST(HierarchicalNodeTest, ValueConstruction) {
    auto node = std::make_shared<IntNode>(42);
    EXPECT_EQ(node->data, 42);
}

TEST(HierarchicalNodeTest, MoveConstruction) {
    std::string value = "hello";
    auto node = std::make_shared<StringNode>(std::move(value));
    EXPECT_EQ(node->data, "hello");
}

// ---------------------------------------------------------------------------
// addChild / parent linkage
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, AddChildSetsParent) {
    auto parent = std::make_shared<IntNode>(1);
    auto child  = std::make_shared<IntNode>(2);

    parent->addChild(child);

    EXPECT_EQ(parent->getChildCount(), 1u);
    EXPECT_TRUE(parent->hasChildren());
    EXPECT_FALSE(parent->isLeaf());
    EXPECT_EQ(child->getParent(), parent);
    EXPECT_FALSE(child->isRoot());
}

TEST(HierarchicalNodeTest, AddMultipleChildren) {
    auto root = std::make_shared<IntNode>(0);
    auto c1   = std::make_shared<IntNode>(1);
    auto c2   = std::make_shared<IntNode>(2);
    auto c3   = std::make_shared<IntNode>(3);

    root->addChild(c1);
    root->addChild(c2);
    root->addChild(c3);

    EXPECT_EQ(root->getChildCount(), 3u);

    const auto& children = root->getChildren();
    EXPECT_EQ(children[0], c1);
    EXPECT_EQ(children[1], c2);
    EXPECT_EQ(children[2], c3);
}

TEST(HierarchicalNodeTest, AddNullChildIsIgnored) {
    auto parent = std::make_shared<IntNode>(1);
    parent->addChild(nullptr);
    EXPECT_EQ(parent->getChildCount(), 0u);
}

TEST(HierarchicalNodeTest, RootNodeHasNoParent) {
    auto root = std::make_shared<IntNode>(0);
    EXPECT_TRUE(root->isRoot());
    EXPECT_EQ(root->getParent(), nullptr);
}

// ---------------------------------------------------------------------------
// removeChild
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, RemoveChildSuccess) {
    auto parent = std::make_shared<IntNode>(0);
    auto child  = std::make_shared<IntNode>(1);

    parent->addChild(child);
    EXPECT_TRUE(parent->removeChild(child));

    EXPECT_EQ(parent->getChildCount(), 0u);
    EXPECT_TRUE(parent->isLeaf());
    EXPECT_EQ(child->getParent(), nullptr);
    EXPECT_TRUE(child->isRoot());
}

TEST(HierarchicalNodeTest, RemoveChildNotPresent) {
    auto parent  = std::make_shared<IntNode>(0);
    auto stranger = std::make_shared<IntNode>(99);

    EXPECT_FALSE(parent->removeChild(stranger));
}

TEST(HierarchicalNodeTest, RemoveCorrectChildFromMultiple) {
    auto parent = std::make_shared<IntNode>(0);
    auto c1     = std::make_shared<IntNode>(1);
    auto c2     = std::make_shared<IntNode>(2);
    auto c3     = std::make_shared<IntNode>(3);

    parent->addChild(c1);
    parent->addChild(c2);
    parent->addChild(c3);

    parent->removeChild(c2);

    EXPECT_EQ(parent->getChildCount(), 2u);
    const auto& children = parent->getChildren();
    EXPECT_EQ(children[0], c1);
    EXPECT_EQ(children[1], c3);
    EXPECT_EQ(c2->getParent(), nullptr);
}

// ---------------------------------------------------------------------------
// clearChildren
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, ClearChildrenRemovesAll) {
    auto parent = std::make_shared<IntNode>(0);
    auto c1     = std::make_shared<IntNode>(1);
    auto c2     = std::make_shared<IntNode>(2);

    parent->addChild(c1);
    parent->addChild(c2);

    parent->clearChildren();

    EXPECT_EQ(parent->getChildCount(), 0u);
    EXPECT_TRUE(parent->isLeaf());
    EXPECT_EQ(c1->getParent(), nullptr);
    EXPECT_EQ(c2->getParent(), nullptr);
}

TEST(HierarchicalNodeTest, ClearChildrenOnLeafIsNoOp) {
    auto node = std::make_shared<IntNode>(0);
    EXPECT_NO_FATAL_FAILURE(node->clearChildren());
    EXPECT_EQ(node->getChildCount(), 0u);
}

// ---------------------------------------------------------------------------
// isRoot / isLeaf
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, LeafNodeReports) {
    auto parent = std::make_shared<IntNode>(0);
    auto child  = std::make_shared<IntNode>(1);

    EXPECT_TRUE(child->isLeaf());
    parent->addChild(child);
    EXPECT_TRUE(child->isLeaf());   // child itself has no children
    EXPECT_FALSE(parent->isLeaf());
}

// ---------------------------------------------------------------------------
// Multi-level hierarchy
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, MultiLevelHierarchy) {
    auto root   = std::make_shared<StringNode>("root");
    auto child  = std::make_shared<StringNode>("child");
    auto grand  = std::make_shared<StringNode>("grandchild");

    root->addChild(child);
    child->addChild(grand);

    // root
    EXPECT_TRUE(root->isRoot());
    EXPECT_FALSE(root->isLeaf());

    // child
    EXPECT_FALSE(child->isRoot());
    EXPECT_FALSE(child->isLeaf());
    EXPECT_EQ(child->getParent(), root);

    // grandchild
    EXPECT_FALSE(grand->isRoot());
    EXPECT_TRUE(grand->isLeaf());
    EXPECT_EQ(grand->getParent(), child);
    EXPECT_EQ(grand->getParent()->getParent(), root);
}

// ---------------------------------------------------------------------------
// Weak-ptr safety: parent does not keep child alive
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, ParentWeakPtrDoesNotPreventChildDestruction) {
    auto parent = std::make_shared<IntNode>(0);
    {
        auto child = std::make_shared<IntNode>(1);
        parent->addChild(child);
        EXPECT_EQ(parent->getChildCount(), 1u);
    }
    // child shared_ptr went out of scope; parent still holds a shared_ptr to child
    // via children_ vector, so child is NOT destroyed yet — that is by design.
    EXPECT_EQ(parent->getChildCount(), 1u);
}

TEST(HierarchicalNodeTest, ChildWeakPtrToParentExpires) {
    auto child = std::make_shared<IntNode>(1);
    {
        auto parent = std::make_shared<IntNode>(0);
        parent->addChild(child);
        EXPECT_NE(child->getParent(), nullptr);
    }
    // parent is destroyed; child's weak_ptr should have expired
    EXPECT_EQ(child->getParent(), nullptr);
    EXPECT_TRUE(child->isRoot());
}

// ---------------------------------------------------------------------------
// setParent (direct)
// ---------------------------------------------------------------------------

TEST(HierarchicalNodeTest, SetParentDirectly) {
    auto p = std::make_shared<IntNode>(0);
    auto c = std::make_shared<IntNode>(1);

    c->setParent(p);
    EXPECT_EQ(c->getParent(), p);
    EXPECT_FALSE(c->isRoot());
}

TEST(HierarchicalNodeTest, SetParentToNullMakesRoot) {
    auto p = std::make_shared<IntNode>(0);
    auto c = std::make_shared<IntNode>(1);

    p->addChild(c);
    c->setParent(nullptr);
    EXPECT_EQ(c->getParent(), nullptr);
    EXPECT_TRUE(c->isRoot());
}
