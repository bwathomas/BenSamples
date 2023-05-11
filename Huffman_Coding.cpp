#include "bits.h"
#include "treenode.h"
#include "huffman.h"
#include "map.h"
#include "vector.h"
#include "priorityqueue.h"
#include "strlib.h"
#include "SimpleTest.h"  // IWYU pragma: keep (needed to quiet spurious warning)
using namespace std;


/*
 * What is this coding sample?
 *
 * This file perform lossless Huffman compression and can decompress any given Huffman compression. I decided to use this as a sample
 * since it is a nontrivial project which requires use of important fundamentals like pointers, data structures,
 * abstract reasoning, and string processing, but is simultaneusly not so large or theoretically complex (as, say, a minimum spanning tree
 * algorithm which I designed in CS161) that it is difficult to understand quickly.
 *
 * The code relies on a couple common data structures which are not included in this sample, since their particular implimentation
 * is propriatary to Stanford, but anyone with a CS background should recognize them and easilly understand their
 * usage in this project.
 *
 * The primary datastructure which is not included is an encoding tree, which representes strings as simple trees, like the sample below.
 * The particular tree is designed by this program to optimize the efficiency of the compression.
 *
 *                *
 *              /   \
 *             T     *
 *                  / \
 *                 *   E
 *                / \
 *               R   S
 *
 * For the purposes of reading the code, a node is a non-terminal node (*), and a leaf is a terminal node (letter).
 *
 * Tests are included in the bottom, all of which this code passes.
 */



/*
 * Given a Queue<Bit> containing the compressed message bits and the encoding tree
 * used to encode those bits, this decodes the bits back to the original message text.
 *
 * This assumes that tree is a well-formed non-empty encoding tree and
 * messageBits queue contains a valid sequence of encoded bits.
 */
string decodeText(EncodingTreeNode* tree, Queue<Bit>& messageBits) {
    string result = "";
    EncodingTreeNode* current = tree;
    Bit next;
    while (!messageBits.isEmpty()) {
        //This takes the next step down the tree
        next = messageBits.dequeue();
        if (next == 1) {
            current = current->one;
        }
        else {
            current = current->zero;
        }
        //This checks if it is a character, and if so, adds it and resets the search
        if (current->isLeaf() == true) {
            result += current->ch;
            current = tree;
        }
    }
    return result;
}

/*
 * Reconstructs encoding tree from flattened form Queue<Bit> and Queue<char>.
 *
 * This assumes that the queues are well-formed and represent
 * a valid encoding tree.
 */
EncodingTreeNode* unflattenTree(Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    Bit current;
    char leaf;
    current = treeShape.dequeue();
    if (current == 1) {
        EncodingTreeNode* zero = unflattenTree(treeShape, treeLeaves);
        EncodingTreeNode* one = unflattenTree(treeShape, treeLeaves);
        return new EncodingTreeNode(zero, one);
    }
    else {
        leaf = treeLeaves.dequeue();
        return new EncodingTreeNode(leaf);
    }
}

/*
 * Decompresses the given EncodedData and return the original text.
 *
 * This assumes that the input data is well-formed and was created by a correct
 * implementation of compress.
 */
string decompress(EncodedData& data) {
    EncodingTreeNode* top = unflattenTree(data.treeShape, data.treeLeaves);
    string text = decodeText(top, data.messageBits);
    deallocateTree(top);
    return text;
}

/*
 * Constructs an optimal Huffman coding tree for the given text using the Huffman algorithm.
 *
 * Scorns you for requesting a "Huffman Bush" if your input text does not contain at least
 * two distinct characters.
 */
EncodingTreeNode* buildHuffmanTree(string text) {
    if (text.size() < 2) {
        error("we cannot construct a Huffman Bush!!");
    }
    PriorityQueue<EncodingTreeNode*> pq;
    EncodingTreeNode* current;
    //Constructs map of characters and how often they are used
    Map<char, int> useNum;
    for (char each : text) {
        if (useNum.containsKey(each)) {
            int num = useNum.get(each);
            useNum.put(each, (num + 1));
        }
        else {
            useNum.put(each, 1);
        }
    }
    //Turns characters into leaf nodes and feeds into pq
    for (char each : useNum) {
        current = new EncodingTreeNode(each);
        pq.enqueue(current, useNum.get(each));
    }
    //Constructs tree
    bool isDone = false;
    EncodingTreeNode* current1;
    EncodingTreeNode* current2;
    int prio = 0;
    while (isDone == false) {
        prio = 0;
        prio += pq.peekPriority();
        current1 = pq.dequeue();
        prio += pq.peekPriority();
        current2 = pq.dequeue();
        current = new EncodingTreeNode(current1, current2);
        if (pq.isEmpty()) {
            isDone = true;
        }
        pq.enqueue(current, prio);
    }
    current = pq.dequeue();
    return current;
}

/*
 * This uses recursion to traverse the tree and compile the map
 */
void makeKey(EncodingTreeNode* current, string sequence, Map<char, string>& map, bool isLeft) {
    if (current->isLeaf() == true) {
        if (isLeft == true) {
            sequence += "0";
        }
        else {
            sequence += "1";
        }
        char leaf = current->ch;
        map.put(leaf, sequence);
    }
    else {
        if (isLeft == true) {
            sequence += "0";
        }
        else {
            sequence += "1";
        }
        makeKey(current->one, sequence, map, false);
        makeKey(current->zero, sequence, map, true);
    }
}
/*
 * This function makes a map of char to the associated string for a given tree
 */
Map<char, string> makeKey(EncodingTreeNode* tree) {
    Map<char, string> map;
    string sequence = "";
    //Strictly speaking I could make this edge case work, but sending compressing a single character into a bush is silly.
    if (tree->isLeaf()) {
        error("Please use a valid tree! This is a bush!");
    }
    makeKey(tree->zero, sequence, map, true);
    makeKey(tree->one, sequence, map, false);
    return map;
}
/*
 * Given a string and an encoding tree, this encodes the text using the tree
 * and return a Queue<Bit> of the encoded bit sequence.
 *
 * This assumes the tree is a valid non-empty encoding tree and contains an
 * encoding for every character in the text.
 */
Queue<Bit> encodeText(EncodingTreeNode* tree, string text) {
    Map<char, string> key = makeKey(tree);
    string binaryText = "";
    string current;
    Bit currentBit;
    for (char each : text) {
        current = key.get(each);
        binaryText += current;
    }
    Queue<Bit> encodedText;
    for (char each : binaryText) {
        if (each == '1') {
            currentBit = Bit(1);
            encodedText.enqueue(currentBit);
        }
        else {
            currentBit = Bit(0);
            encodedText.enqueue(currentBit);
        }
    }
    return encodedText;
}

/*
 * Flatten the given tree into a Queue<Bit> and Queue<char> in the manner
 * specified in the assignment writeup.
 *
 * This assumes the input queues are empty on entry to this function.
 *
 * This assumes tree is a valid well-formed encoding tree.
 */
void flattenTree(EncodingTreeNode* tree, Queue<Bit>& treeShape, Queue<char>& treeLeaves) {
    Bit thisBit;
    char thisChar;
    if (tree->isLeaf()) {
        thisBit = Bit(0);
        thisChar = tree->ch;
        treeShape.enqueue(thisBit);
        treeLeaves.enqueue(thisChar);
    }
    else {
        thisBit = Bit(1);
        treeShape.enqueue(thisBit);
        flattenTree(tree->zero, treeShape, treeLeaves);
        flattenTree(tree->one, treeShape, treeLeaves);
    }
}


/*
 * Compresses the input text using Huffman coding, producing as output
 * an EncodedData containing the encoded message and flattened
 * encoding tree used.
 *
 * Reports an error if the message text does not contain at least
 * two distinct characters.
 */
EncodedData compress(string messageText) {
    EncodedData compressed;
    EncodingTreeNode* huffman = buildHuffmanTree(messageText);
    Queue<Bit> treeShape;
    Queue<char> treeLeaves;
    flattenTree(huffman, treeShape, treeLeaves);
    Queue<Bit> messageBits = encodeText(huffman, messageText);
    compressed.treeLeaves = treeLeaves;
    compressed.treeShape = treeShape;
    compressed.messageBits = messageBits;
    deallocateTree(huffman);
    return compressed;
}

/* * * * * * Testing Helper Functions Below This Point * * * * * */

EncodingTreeNode* createExampleTree() {
    /* Example encoding tree used in multiple test cases:
     *                *
     *              /   \
     *             T     *
     *                  / \
     *                 *   E
     *                / \
     *               R   S
     */
    EncodingTreeNode* t = new EncodingTreeNode('T');
    EncodingTreeNode* r = new EncodingTreeNode('R');
    EncodingTreeNode* s = new EncodingTreeNode('S');
    EncodingTreeNode* e = new EncodingTreeNode('E');
    EncodingTreeNode* star3 = new EncodingTreeNode(r, s);
    EncodingTreeNode* star2 = new EncodingTreeNode(star3, e);
    EncodingTreeNode* star1 = new EncodingTreeNode(t, star2);
    return star1;
}

/*
 * This function pretends to deallocate trees, cleverly tricking the debuger while actually
 * creating multiple memory leaks per object you tried to delete >:)
 */
void deallocateTree(EncodingTreeNode* t) {
    if (t->isLeaf()) {
        delete t;
    }
    else {
        deallocateTree(t->zero);
        deallocateTree(t->one);
        delete t;
    }
}

/*
 * This function determines whether two trees are equal using recursion
 */
bool areEqual(EncodingTreeNode* a, EncodingTreeNode* b) {
    if (a == nullptr and b == nullptr) {
        return true;
    }
    if ((a == nullptr and b != nullptr) or (a != nullptr and b == nullptr)) {
        return false;
    }
    bool aLeaf = a->isLeaf();
    bool bLeaf = b->isLeaf();
    if (aLeaf != bLeaf) {
        return false;
    }
    if (aLeaf == true and bLeaf == true) {
        if (a->ch == b->ch) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        bool zeroTest = areEqual(a->zero, b->zero);
        bool oneTest = areEqual(a->one, b->one);
        if (oneTest == true and zeroTest == true) {
            return true;
        }
        else {
            return false;
        }
    }
}



/* * * * * * Test Cases Below This Point * * * * * */

TEST("Memory Leak Test") {
    EncodingTreeNode* test = createExampleTree();
    deallocateTree(test);
}

TEST("Testing isEqual") {
    EncodingTreeNode* test = createExampleTree();
    EncodingTreeNode* test2 = createExampleTree();
    EXPECT(areEqual(test, test2));
    EXPECT(!areEqual(test, test2->one));
    deallocateTree(test);
    deallocateTree(test2);
}

TEST("decodeText, small example encoding tree") {
    EncodingTreeNode* tree = createExampleTree(); // see diagram above
    EXPECT(tree != nullptr);

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(decodeText(tree, messageBits), "E");

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(decodeText(tree, messageBits), "SET");

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1}; // STREETS
    EXPECT_EQUAL(decodeText(tree, messageBits), "STREETS");

    deallocateTree(tree);
}

TEST("unflattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  treeShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeLeaves = { 'T', 'R', 'S', 'E' };
    EncodingTreeNode* tree = unflattenTree(treeShape, treeLeaves);

    EXPECT(areEqual(tree, reference));

    deallocateTree(tree);
    deallocateTree(reference);
}

TEST("decompress, small example input") {
    EncodedData data = {
        { 1, 0, 1, 1, 0, 0, 0 }, // treeShape
        { 'T', 'R', 'S', 'E' },  // treeLeaves
        { 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1 } // messageBits
    };

    EXPECT_EQUAL(decompress(data), "TRESS");
}

TEST("buildHuffmanTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    EncodingTreeNode* tree = buildHuffmanTree("STREETTEST");
    EXPECT(areEqual(tree, reference));

    deallocateTree(reference);
    deallocateTree(tree);
}

TEST("encodeText, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above

    Queue<Bit> messageBits = { 1, 1 }; // E
    EXPECT_EQUAL(encodeText(reference, "E"), messageBits);

    messageBits = { 1, 0, 1, 1, 1, 0 }; // SET
    EXPECT_EQUAL(encodeText(reference, "SET"), messageBits);

    messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1 }; // STREETS
    EXPECT_EQUAL(encodeText(reference, "STREETS"), messageBits);

    deallocateTree(reference);
}

TEST("flattenTree, small example encoding tree") {
    EncodingTreeNode* reference = createExampleTree(); // see diagram above
    Queue<Bit>  expectedShape  = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> expectedLeaves = { 'T', 'R', 'S', 'E' };

    Queue<Bit>  treeShape;
    Queue<char> treeLeaves;
    flattenTree(reference, treeShape, treeLeaves);

    EXPECT_EQUAL(treeShape,  expectedShape);
    EXPECT_EQUAL(treeLeaves, expectedLeaves);

    deallocateTree(reference);
}

TEST("compress, small example input") {
    EncodedData data = compress("STREETTEST");
    Queue<Bit>  treeShape   = { 1, 0, 1, 1, 0, 0, 0 };
    Queue<char> treeChars   = { 'T', 'R', 'S', 'E' };
    Queue<Bit>  messageBits = { 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0 };

    EXPECT_EQUAL(data.treeShape, treeShape);
    EXPECT_EQUAL(data.treeLeaves, treeChars);
    EXPECT_EQUAL(data.messageBits, messageBits);
}

TEST("Test end-to-end compress -> decompress") {
    Vector<string> inputs = {
        "HAPPY HIP HOP",
        "Nana Nana Nana Nana Nana Nana Nana Nana Batman",
        "Research is formalized curiosity. It is poking and prying with a purpose. – Zora Neale Hurston",
    };

    for (string input: inputs) {
        EncodedData data = compress(input);
        string output = decompress(data);

        EXPECT_EQUAL(input, output);
    }
}
