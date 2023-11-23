#pragma once

#include <map>
#include <memory>
#include <cstdlib>
#include <assert.h>
#include "util.hh"


typedef std::pair<int, int> rankAndTB;
typedef std::pair<int, int> leafsAndNode;


class HuffmanNode {
    int value;
    int occ;
    std::shared_ptr<HuffmanNode> lChild;
    std::shared_ptr<HuffmanNode> rChild;

    public:
        HuffmanNode (int occ) : occ(occ), value(-1) {}
        HuffmanNode (int occ, int value) : occ(occ), value(value) {}

        void setLChild (std::shared_ptr<HuffmanNode> newChild) {
            this->lChild = newChild;
        }

        void setRChild (std::shared_ptr<HuffmanNode> newChild) {
            this->rChild = newChild;
        }

        std::shared_ptr<HuffmanNode> getLChild () {
            return this->lChild;
        }

        std::shared_ptr<HuffmanNode> getRChild () {
            return this->rChild;
        }

        int getOcc () {
            return this->occ;

        }

        int getValue () {
            return this->value;

        }

};

class CompareHuffmanNode {
    public:

    bool operator() (
        const std::shared_ptr<HuffmanNode> x, const std::shared_ptr<HuffmanNode> y
    ) const {
        assert (x != nullptr);
        assert (y != nullptr);

        return x->getOcc() > y->getOcc();

    }
};

class CompareHuffmanRank {
    public:

    bool operator() (
        const std::pair<rankAndTB, leafsAndNode> x, const std::pair<rankAndTB, leafsAndNode>  y
    ) const {
        return (
            x.first.first == y.first.first ?
            x.first.second > y.first.second :
            x.first.first > y.first.first
        );
    }
};

std::vector<int> huffmanEncoding (int nVertices, const std::vector<query>& queries) {
    std::priority_queue<
        std::shared_ptr<HuffmanNode>, std::vector<std::shared_ptr<HuffmanNode>>, CompareHuffmanNode
    > alphabet;
    std::vector<int> occ(nVertices, 0);
    std::vector<int> hRank(nVertices);

    for (query qry: queries) {
        occ[qry.first]++;
        occ[qry.second]++;

    }

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        alphabet.push(std::make_shared<HuffmanNode>(occ[vIdx], vIdx));

    }

    while (alphabet.size() != 1) {
        std::shared_ptr<HuffmanNode> lChild = alphabet.top(); alphabet.pop();
        std::shared_ptr<HuffmanNode> rChild = alphabet.top(); alphabet.pop();

        std::shared_ptr<HuffmanNode> newNode = std::make_shared<HuffmanNode>(
            lChild->getOcc() + rChild->getOcc()
        );

        newNode->setLChild(lChild);
        newNode->setRChild(rChild);

        alphabet.push(newNode);
    }

    std::queue<std::pair<int, std::shared_ptr<HuffmanNode>>> huffmanRank;
    huffmanRank.push({ 0, alphabet.top() });

    while (!huffmanRank.empty()) {
        auto [rank, cNode] = huffmanRank.front(); huffmanRank.pop();

        if (cNode->getValue() != -1) {
            hRank[cNode->getValue()] = rank + 1;

        } else {
            huffmanRank.push({ rank + 1, cNode->getLChild() });
            huffmanRank.push({ rank + 1, cNode->getRChild() });

        }
    }

    return hRank;
}

void insertVertex (
    int vIdx, int& totalCost, std::vector<std::vector<int>>& distances,
    const std::vector<std::vector<int>>& occs, std::vector<int>& pred,
    std::priority_queue<
        std::pair<rankAndTB, leafsAndNode>,
        std::vector<std::pair<rankAndTB, leafsAndNode>>,
        CompareHuffmanRank
    >& leafes
) {
    if (pred[vIdx] != INF)
        return;

    else if (leafes.size() == 0) {
        pred[vIdx] = -1;
        leafes.push({{ 0, rand() }, { vIdx, 2 } });

        return;

    }

    auto [ rankTb, leafNode ] = leafes.top(); leafes.pop();
    if (leafNode.second > 1) {
        leafes.push({ rankTb, { leafNode.first, leafNode.second - 1 }});

    }


    leafes.push({{ rankTb.first + 1, rand() }, { vIdx, 2 }});
    pred[vIdx] = leafNode.first;

    for (int dst = 0; dst < occs[vIdx].size(); dst++) {
        if (pred[dst] == INF || vIdx == dst)
            continue;

        distances[vIdx][dst] = distances[leafNode.first][dst] + 1;
        distances[dst][vIdx] = distances[dst][leafNode.first] + 1;

        totalCost += distances[vIdx][dst] * occs[vIdx][dst];
    }
}

std::vector<int> huffmanHeuristic (
    const std::vector<query>& queries, int nVertices, int& totalCost
) {
    std::vector<std::vector<int>> occs(nVertices, std::vector<int>(nVertices, 0));
    std::vector<std::vector<int>> distances(nVertices, std::vector<int>(nVertices, 0));
    std::priority_queue<
        std::pair<rankAndTB, leafsAndNode>,
        std::vector<std::pair<rankAndTB, leafsAndNode>>,
        CompareHuffmanRank
    > leafes, nodeRanks;
    std::vector<int> hRank = huffmanEncoding(nVertices, queries);

    std::vector<int> pred(nVertices, INF);
    totalCost = 0;

    for (const auto [src, dst]: queries) {
        occs[src][dst]++;
        occs[dst][src]++;
    }

    for (int vIdx = 0; vIdx < nVertices; vIdx++) {
        nodeRanks.push({{ hRank[vIdx], rand() }, { vIdx, 0 }});

    }

    while (!nodeRanks.empty()) {
        auto [ rankTB, nodeLeaf ] = nodeRanks.top(); nodeRanks.pop();
        insertVertex(nodeLeaf.first, totalCost, distances, occs, pred, leafes);

    }

    assert (isValidBinaryTree(pred));

    return pred;
}