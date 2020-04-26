#pragma once

#include <vector>

struct Agent;
struct AgentWorld;
struct Boid;

struct NeighborSearch
{
    static void Search(Agent* agent, AgentWorld& world, size_t maxNeighbors = 0);
    static void Search(Boid* agent, std::vector<Boid>& neighbors, std::vector<size_t>& result, size_t& outResultCount, size_t maxNeighbors = 0);
};