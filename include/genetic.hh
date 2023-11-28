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


void cleanSwap (
    int genIdx, int oldPIdx, int newPIdx, std::vector<int>& gPred, std::set<int>& remV,
    std::vector<std::set<int>>& children, std::map<int,int>& leafes
) {
    auto leafNP = leafes.find(newPIdx);

    remV.erase(genIdx);
    gPred[genIdx] = newPIdx;

    children[oldPIdx].erase(genIdx);
    children[newPIdx].insert(genIdx);

    leafes[oldPIdx] = 2 - children[oldPIdx].size();
    if (leafNP->second == 2) {
        leafNP->second--;

    } else {
        leafes.erase(leafNP);

    }
}

void bothFullSwap (
    int genIdx, int oldPIdx, int newPIdx, int npChild, int gChild,
    std::vector<int>& gPred, std::set<int>& remV, std::vector<std::set<int>>& children
) {
    remV.erase(genIdx);
    remV.erase(npChild);
    remV.erase(gChild);

    assert (gPred[genIdx] == oldPIdx);
    assert (gPred[gChild] == genIdx);
    assert (gPred[npChild] == newPIdx);

    gPred[genIdx] = newPIdx;
    gPred[npChild] = genIdx;
    gPred[gChild] = oldPIdx;

    children[newPIdx].erase(npChild);
    children[newPIdx].insert(genIdx);
    children[genIdx].erase(gChild);
    children[genIdx].insert(npChild);
    children[oldPIdx].erase(genIdx);
    children[oldPIdx].insert(gChild);
}

void genAvailableSwap (
    int genIdx, int oldPIdx, int newPIdx, int npChild, std::vector<int>& gPred, std::set<int>& remV,
    std::vector<std::set<int>>& children, std::map<int,int>& leafes
) {
    remV.erase(genIdx);
    remV.erase(npChild);

    gPred[genIdx] = newPIdx;
    gPred[npChild] = genIdx;

    children[oldPIdx].erase(genIdx);
    children[newPIdx].erase(npChild);
    children[newPIdx].insert(genIdx);
    children[genIdx].insert(npChild);

    leafes[oldPIdx] = 2 - children[oldPIdx].size();
    if (children[genIdx].size() < 2) {
        leafes[genIdx] = 2 - children[genIdx].size();

    } else {
        leafes.erase(genIdx);

    }
}

void swapDiffSubtree (
    int genIdx, int oldPIdx, int newPIdx, std::vector<int>& gPred, std::set<int>& remV,
    std::vector<std::set<int>>& children, std::map<int,int>& leafes, std::mt19937 gen
) {
    auto leafNP = leafes.find(newPIdx);
    if (leafNP != leafes.end()) {
        cleanSwap(genIdx, oldPIdx, newPIdx, gPred, remV, children, leafes);

    } else {
        std::uniform_int_distribution<> fullDis(0, 1);
        auto npCIt = children[newPIdx].begin();
        std::advance(npCIt, fullDis(gen));
        int npChild = *npCIt;

        if (leafes.find(genIdx) != leafes.end()) {
            genAvailableSwap(
                genIdx, oldPIdx, newPIdx, npChild, gPred, remV, children, leafes
            );

        } else {
            auto gCIt = children[genIdx].begin();
            std::advance(gCIt, fullDis(gen));
            int gChild = *gCIt;

            bothFullSwap(
                genIdx, oldPIdx, newPIdx, npChild, gChild, gPred, remV, children
            );
        }
    }
}

void swapUpSubtree (
    int genIdx, int oldPIdx, int newPIdx, int sbtreeChild,
    std::vector<int>& gPred, std::set<int>& remV, std::vector<std::set<int>>& children,
    std::map<int,int>& leafes, std::mt19937 gen
) {
    auto leafNP = leafes.find(newPIdx);
    if (leafNP != leafes.end()) {
        remV.erase(genIdx);
        remV.erase(sbtreeChild);

        assert (gPred[genIdx] == oldPIdx);
        assert (gPred[sbtreeChild] == genIdx);

        gPred[genIdx] = newPIdx;
        gPred[sbtreeChild] = oldPIdx;

        children[genIdx].erase(sbtreeChild);
        children[oldPIdx].erase(genIdx);
        children[oldPIdx].insert(sbtreeChild);
        children[newPIdx].insert(genIdx);

        leafes[genIdx] = 2 - children[genIdx].size();
        if (leafNP->second == 2) {
            leafNP->second--;

        } else {
            leafes.erase(leafNP);

        }

    } else {
        std::uniform_int_distribution<> npDis(0, 1);
        auto npCIt = children[newPIdx].begin();
        std::advance(npCIt, npDis(gen));
        int npChild = *npCIt;

        bothFullSwap(
            genIdx, oldPIdx, newPIdx, npChild, sbtreeChild, gPred, remV, children
        );
    }
}

void swapDownSubtree (
    int genIdx, int oldPIdx, int newPIdx, int sbtreeChild,
    std::vector<int>& gPred, std::set<int>& remV, std::vector<std::set<int>>& children,
    std::map<int,int>& leafes, std::mt19937 gen
) {
    auto leafNP = leafes.find(newPIdx);
    if (leafNP != leafes.end()) {
        cleanSwap(genIdx, oldPIdx, newPIdx, gPred, remV, children, leafes);

    } else {
        int npChild;
        for (int child: children[newPIdx]) {
            if (child != sbtreeChild) {
                npChild = sbtreeChild;
                break;

            }
        }

        if (leafes.find(genIdx) != leafes.end()) {
            genAvailableSwap(
                genIdx, oldPIdx, newPIdx, npChild, gPred, remV, children, leafes
            );

        } else {
            std::uniform_int_distribution<> gDis(0, 1);
            auto gCIt = children[genIdx].begin();
            std::advance(gCIt, gDis(gen));
            int gChild = *gCIt;

            bothFullSwap(
                genIdx, oldPIdx, newPIdx, sbtreeChild, gChild, gPred, remV, children
            );
        }
    }
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
                int newPIdx;

                if (p < mutProb) {
                    std::uniform_int_distribution<> lDis(0, leafes.size() - 1);
                    auto leafIt = leafes.begin();
                    std::advance(leafIt, lDis(gen));

                    newPIdx = leafIt->first;

                } else if (p > 1 - crossProb) {
                    newPIdx = oth.genome[genIdx];

                } else {
                    newPIdx = pIdx;

                }

                if (newPIdx == pIdx || newPIdx == -1 || newPIdx == genIdx) {
                    remV.erase(remIt);
                    continue;

                }

                int preLCA = getPreLCA(genIdx, newPIdx, cPred);
                if (preLCA == -1) {
                    swapDiffSubtree(
                        genIdx, pIdx, newPIdx, cPred, remV, children, leafes, gen
                    );

                } else if (cPred[preLCA] == genIdx) {
                    swapUpSubtree(
                        genIdx, pIdx, newPIdx, preLCA, cPred, remV, children, leafes, gen
                    );

                } else {
                    swapDownSubtree(
                        genIdx, pIdx, newPIdx, preLCA, cPred, remV, children, leafes, gen
                    );

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
    int sumSolution = 0;
    double mean;

    for (std::vector<int>& resp: othResponses) {
        Individual ind(nVertices, resp, treeCost(resp, queries));
        pop.push_back(ind);
        sumSolution += ind.getCost();

    }

    for (int popIdx = 0; popIdx < popSize - othResponses.size(); popIdx++) {
        std::vector<int> pred(nVertices);
        std::ifstream popFile("pop/" + std::to_string(nVertices) + "/" + std::to_string(popIdx) + ".txt");

        for (auto& p: pred) {
            popFile >> p;

        }

        Individual ind(nVertices, pred, treeCost(pred, queries));
        pop.push_back(ind);
        sumSolution += ind.getCost();

    }

    int crossFit = popSize * crossFitPct;
    int eliteSize = popSize * elitePct;
    int crossSize = popSize - eliteSize;
    int lastChanged = 0;
    std::vector<double> weights(crossFit);

    for (int genIdx = 0; lastChanged + stoppingGen >= genIdx; genIdx++) {
        std::sort(pop.begin(), pop.end());

        mean = sumSolution / double(popSize);
        sumSolution = 0;
        std::cout << "gen idx: " << genIdx << " - mnCost so far: ";
        std::cout << pop[0].getCost() << " mean: " << mean << std::endl;

        std::vector<Individual> newGen;
        int sum = 0;

        for (int idx = 0; idx < eliteSize; idx++) {
            newGen.push_back(pop[idx]);
            sumSolution += newGen[idx].getCost();

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
            sumSolution += offspring.getCost();
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