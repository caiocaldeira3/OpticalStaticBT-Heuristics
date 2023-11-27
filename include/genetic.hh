#pragma once

#include "util.hh"
#include <set>
#include <map>
#include <chrono>
#include <string>
#include <random>
#include <fstream>
#include <algorithm>
#include <filesystem>
#include <assert.h>
#include <iostream>


void updateVertexParent (
    int genIdx, int oldPIdx, int newPIdx, std::vector<int>& gPred, std::set<int>& remV,
    std::vector<std::set<int>>& children, std::map<int,int>& leafes, std::mt19937 gen
) {
    gPred[genIdx] = newPIdx;
    if (isValidBinaryTree(gPred)) {
        children[oldPIdx].erase(genIdx);
        children[newPIdx].insert(genIdx);

        leafes[oldPIdx] = 2 - children[oldPIdx].size();

        if (children[newPIdx].size() < 2) {
            leafes[newPIdx] = 2 - children[newPIdx].size();

        } else {
            leafes.erase(newPIdx);

        }

    } else {
        std::set<int> posChildren;
        for (int cIdx: children[genIdx])
            posChildren.insert(cIdx);

        bool possible = false;
        while (!posChildren.empty()) {
            std::uniform_int_distribution<> cDis(0, posChildren.size() - 1);
            auto cIt = posChildren.begin();
            std::advance(cIt, cDis(gen));
            int cIdx = *cIt;

            gPred[cIdx] = oldPIdx;
            gPred[genIdx] = newPIdx;

            if (isValidBinaryTree(gPred)) {
                remV.erase(genIdx);
                remV.erase(cIdx);

                possible = true;
                children[newPIdx].insert(genIdx);
                children[oldPIdx].erase(genIdx);
                children[oldPIdx].insert(cIdx);
                children[genIdx].erase(cIdx);

                leafes[genIdx] = 2 - children[genIdx].size();

                if (children[newPIdx].size() < 2) {
                    leafes[newPIdx] = 2 - children[newPIdx].size();

                } else {
                    leafes.erase(newPIdx);

                }

                break;

            }

            gPred[genIdx] = oldPIdx;
            gPred[cIdx] = genIdx;
            posChildren.erase(cIt);
        }

        assert (possible);
    }
}


bool swapDownSubtrees (
    int genIdx, int oldPIdx, int newPIdx, std::vector<int>& gPred, std::set<int>& remV,
    std::vector<std::set<int>>& children, std::map<int,int>& leafes, std::mt19937 gen
) {
    std::set<int> posChildren;
    for (int cIdx: children[newPIdx])
        posChildren.insert(cIdx);

    while (!posChildren.empty()) {
        std::uniform_int_distribution<> cDis(0, posChildren.size() - 1);
        auto cIt = posChildren.begin();
        std::advance(cIt, cDis(gen));
        int cIdx = *cIt;

        gPred[cIdx] = oldPIdx;
        gPred[genIdx] = newPIdx;

        if (isValidBinaryTree(gPred)) {
            remV.erase(genIdx);
            remV.erase(cIdx);

            children[newPIdx].insert(genIdx);
            children[newPIdx].erase(cIdx);
            children[oldPIdx].erase(genIdx);
            children[oldPIdx].insert(cIdx);

            return true;
        }

        gPred[genIdx] = oldPIdx;
        gPred[cIdx] = newPIdx;
        posChildren.erase(cIt);
    }

    return false;
}

void swapUpSubtrees (
    int genIdx, int oldPIdx, int newPIdx, std::vector<int>& gPred, std::set<int>& remV,
    std::vector<std::set<int>>& children, std::map<int,int>& leafes, std::mt19937 gen
) {
    std::uniform_int_distribution<> newDis(0, children[newPIdx].size() - 1);
    auto newChildIt = children[newPIdx].begin();
    std::advance(newChildIt, newDis(gen));

    int preteredChild = *newChildIt;

    std::set<int> posChildren;
    for (int cIdx: children[genIdx])
        posChildren.insert(cIdx);

    bool possible = false;

    while (!posChildren.empty()) {
        std::uniform_int_distribution<> cDis(0, posChildren.size() - 1);
        auto cIt = posChildren.begin();
        std::advance(cIt, cDis(gen));
        int cIdx = *cIt;

        gPred[cIdx] = oldPIdx;
        gPred[genIdx] = newPIdx;
        gPred[preteredChild] = genIdx;

        if (isValidBinaryTree(gPred)) {
            remV.erase(genIdx);
            remV.erase(cIdx);
            remV.erase(preteredChild);

            possible = true;

            children[genIdx].insert(preteredChild);
            children[genIdx].erase(cIdx);
            children[oldPIdx].erase(genIdx);
            children[oldPIdx].insert(cIdx);
            children[newPIdx].erase(preteredChild);
            children[newPIdx].insert(genIdx);

            break;
        }

        gPred[preteredChild] = newPIdx;
        gPred[genIdx] = oldPIdx;
        gPred[cIdx] = genIdx;
        posChildren.erase(cIt);
    }

    assert (possible);
}


class Individual {
    std::vector<int> genome;
    int nVertices;
    int cost;
    double weight;

    public:
        Individual (int nVertices, std::vector<int> genome, int cost = INF)
            : nVertices(nVertices), genome(genome), cost(cost) { };

        void computeCost (const std::vector<query>& queries) {
            if (this->cost == INF) {
                this->cost = treeCost(this->genome, queries);

            }
        }

        std::vector<int> getGenome () {
            return this->genome;

        }

        void computeWeight (int bestCost) {
            this->weight = (bestCost / (double) this->cost);

        }

        int getCost () {
            return this->cost;
        }

        double getWeight () {
            return this->weight;

        }

        friend bool operator< (Individual ind, Individual oth) {
            return ind.cost < oth.cost;

        }

        friend Individual cross (Individual ind, Individual oth, double mutProb, double crossProb) {
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            std::mt19937 gen(seed);
            std::uniform_real_distribution<> mDis(0.0, 1.0);

            std::vector<int> cPred = ind.getGenome();
            std::vector<std::set<int>> children(ind.nVertices, std::set<int>());
            std::map<int, int> leafes;
            std::set<int> remV;

            for (int vIdx = 0; vIdx < ind.nVertices; vIdx++) {
                if (cPred[vIdx] == -1)
                    continue;

                remV.insert(vIdx);
                children[cPred[vIdx]].insert(vIdx);
            }

            for (int vIdx = 0; vIdx < ind.nVertices; vIdx++) {
                if (children[vIdx].size() < 2) {
                    leafes[vIdx] = 2 - children[vIdx].size();

                }
            }

            while (!remV.empty()) {
                double p = mDis(gen);
                std::uniform_int_distribution<> rDis(0, remV.size() - 1);
                auto remIt = remV.begin();
                std::advance(remIt, rDis(gen));
                int genIdx = *remIt;
                int pIdx = cPred[genIdx];
                int othPIdx = oth.genome[genIdx];

                if (p < mutProb) {
                    std::uniform_int_distribution<> lDis(0, leafes.size() - 1);
                    auto leafIt = leafes.begin();
                    std::advance(leafIt, lDis(gen));

                    if (leafIt->first == genIdx || leafIt->first == pIdx) {
                        continue;

                    } else {
                        updateVertexParent(
                            genIdx, pIdx, leafIt->first, cPred, remV, children, leafes, gen
                        );
                    }

                } else if (p > 1 - crossProb) {
                    auto pIt = leafes.find(othPIdx);
                    auto gIt = leafes.find(genIdx);

                    if (othPIdx == pIdx || othPIdx == -1) {
                        remV.erase(remIt);

                    } else if (pIt != leafes.end()) {
                        updateVertexParent(
                            genIdx, pIdx, othPIdx, cPred, remV, children, leafes, gen
                        );

                    } else if (children[genIdx].empty()) {
                        assert (swapDownSubtrees(
                            genIdx, pIdx, othPIdx, cPred, remV, children, leafes, gen
                        ));

                    } else {
                        if (!swapDownSubtrees(
                            genIdx, pIdx, othPIdx, cPred, remV, children, leafes, gen
                        )) {
                            swapUpSubtrees(
                                genIdx, pIdx, othPIdx, cPred, remV, children, leafes, gen
                            );

                        }
                    }

                } else {
                    remV.erase(genIdx);

                }

            }

            assert (isValidBinaryTree(cPred));

            return Individual(ind.nVertices, cPred);
        }

};

std::vector<int> geneticAlgorithm (
    int popSize, int stoppingGen, int nVertices, int& totalCost, int& numGen,
    std::vector<query> queries, std::vector<std::vector<int>> othResponses = std::vector<std::vector<int>>(),
    double elitePct = 0.1, double crossFitPct = 0.4, double mutProb = 0.02, double crossProb = 0.2
) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 gen(seed);
    std::vector<Individual> pop;

    for (std::vector<int>& resp: othResponses) {
        Individual ind(nVertices, resp, treeCost(resp, queries));
        pop.push_back(ind);

    }

    for (int popIdx = 0; popIdx < popSize - othResponses.size(); popIdx++) {
        std::vector<int> pred(nVertices);
        std::ifstream popFile("pop/" + std::to_string(nVertices) + "/" + std::to_string(popIdx) + ".txt");

        for (auto& p: pred) {
            popFile >> p;

        }

        Individual ind(nVertices, pred, treeCost(pred, queries));
        pop.push_back(ind);
    }

    int crossFit = popSize * crossFitPct;
    int eliteSize = popSize * elitePct;
    int crossSize = popSize - eliteSize;
    int lastChanged = 0;
    std::vector<double> weights(crossFit);

    for (int genIdx = 0; lastChanged + stoppingGen >= genIdx; genIdx++) {
        std::sort(pop.begin(), pop.end());
        std::cout << "gen idx: " << genIdx << " - mnCost so far: " << pop[0].getCost() << std::endl;

        std::vector<Individual> newGen;

        for (int idx = 0; idx < eliteSize; idx++) {
            newGen.push_back(pop[idx]);

        }

        for (int genIdx = 0; genIdx < crossFit; genIdx++) {
            pop[genIdx].computeWeight(pop[0].getCost());
            weights[genIdx] = pop[genIdx].getWeight();
        }

        std::discrete_distribution<int> pDis(std::begin(weights), std::end(weights));

        for (int idx = 0; idx < crossSize; idx++) {
            int indIdx = pDis(gen);
            int othIdx = pDis(gen);
            while (othIdx == indIdx)
                othIdx = pDis(gen);

            Individual ind = pop[indIdx];
            Individual oth = pop[othIdx];
            Individual offspring = cross(ind, oth, mutProb, crossProb);
            offspring.computeCost(queries);

            if (offspring.getCost() < pop[0].getCost()) {
                lastChanged = genIdx;

            }

            newGen.push_back(offspring);
        }

        pop = newGen;
    }

    numGen = lastChanged;

    int mnIdx = 0;
    for (int idx = 1; idx < popSize; idx++) {
        if (pop[idx] < pop[mnIdx]) {
            mnIdx = idx;

        }
    }

    totalCost = pop[mnIdx].getCost();
    return pop[mnIdx].getGenome();

}