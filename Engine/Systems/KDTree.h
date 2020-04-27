#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <vector>
#include <algorithm>

#include "Engine/Renderer/DebugDraw.h"

struct Vec3ComparerX {
    bool operator()(const glm::vec3& lhs, const glm::vec3& rhs) const {
        return lhs.x < rhs.x;
    }
};

struct Vec3ComparerY {
    bool operator()(const glm::vec3& lhs, const glm::vec3& rhs) const {
        return lhs.y < rhs.y;
    }
};

struct Vec3ComparerZ {
    bool operator()(const glm::vec3& lhs, const glm::vec3& rhs) const {
        return lhs.z < rhs.z;
    }
};

struct kdtree
{
    using NodeContent = std::pair<glm::vec3, size_t>;
    struct node
    {
        size_t payload = 0;
        glm::vec3 location;
        node* left = nullptr;
        node* right = nullptr;
        size_t axis = 0;
    };

    kdtree()
    {
        root = nullptr;
    }

    kdtree(std::vector<NodeContent> points)
    {
        root = build(points, 0);
    }

    glm::vec3 nearest(glm::vec3 p)
    {
        if (root == nullptr) return {};
        best = nullptr;
        visited = 0;
        bestDistance = FLT_MAX;
        nearest(root, p, 0);
        if (best != nullptr) return best->location;
        return {};
    }

    std::vector<NodeContent> nearest(glm::vec3 p, float range)
    {
        if (root == nullptr) return {};

        std::vector<NodeContent> results;
        best = nullptr;
        visited = 0;
        bestDistance = FLT_MAX;
        nearest(root, p, range, results, 0);

        return results;
    }

    void print()
    {
        print(root, 0);
    }

    void print(node* n, int depth)
    {
        if (n == nullptr) { return; }

        print(n->left, ++depth);
        print(n->right, ++depth);

        printf("(%.1f, %.1f)", n->location.x, n->location.y);
    }

    float distance() { return sqrt(bestDistance); }

    using Hyperplane = std::pair<glm::vec3, size_t>;

    void DebugDraw() 
    {
        const float opacity = 0.7f;
        // draw tree
        std::vector<Hyperplane> kdtreeVis;
        GetAllHyperplanes(root, kdtreeVis);
        const size_t size = kdtreeVis.size();
        for (size_t i = 0; i < size; ++i)
        {
            auto pos = kdtreeVis[i].first;
            auto axis = kdtreeVis[i].second;

            glm::vec4 color = { 1.0f, 0.0f, 0.0f, opacity };
            glm::vec3 dir = { 1.0f, 0.0f, 0.0f };
            if (axis == 1) 
            {
                color = { 0.0f, 1.0f, 0.0f, opacity };
                dir = { 0.0f, 1.0f, 0.0f };
            }
            else if (axis == 2) 
            {
                color = { 0.0f, 1.0f, 1.0f, opacity };
                dir = { 0.0f, 0.0f, 1.0f };
            }

            DebugDraw::AddRect(kdtreeVis[i].first, 25.0f, dir, color);
        }
    }

private:
    void GetAllHyperplanes(node* node, std::vector<Hyperplane>& outResult)
    {
        if (node == nullptr) { return; }

        outResult.push_back({ node->location, node->axis });

        GetAllHyperplanes(node->left, outResult);
        GetAllHyperplanes(node->right, outResult);
    }

    node* build(std::vector<NodeContent> points, int depth)
    {
        if (points.empty()) { return nullptr; }

        uint8_t axis = depth % 3;
        if (axis == 0)
        {
            std::sort(points.begin(), points.end(), 
                [](const NodeContent& lhs, const NodeContent& rhs) {
                return lhs.first.x < rhs.first.x;
            });
        }
        else if (axis == 1)
        {
            std::sort(points.begin(), points.end(),
                [](const NodeContent& lhs, const NodeContent& rhs) {
                    return lhs.first.y < rhs.first.y;
                });
        }
        else
        {
            std::sort(points.begin(), points.end(),
                [](const NodeContent& lhs, const NodeContent& rhs) {
                    return lhs.first.z < rhs.first.z;
                });
        }

        const size_t medianCount = points.size() / 2;
        std::vector<NodeContent> leftPoints(points.begin(), points.begin() + medianCount);
        std::vector<NodeContent> rightPoints(points.begin() + medianCount + 1, points.end());

        node* n = new node();
        n->location = (points.begin() + medianCount)->first;
        n->payload = (points.begin() + medianCount)->second;
        n->left = build(leftPoints, ++depth);
        n->right = build(rightPoints, ++depth);
        n->axis = axis;
        return n;
    }

    void printSorted(node* n, int depth)
    {
        if (n == nullptr) { return; }

        printSorted(n->left, ++depth);
        printf("(%.1f, %.1f)", n->location.x, n->location.y);
        printSorted(n->right, ++depth);
    }

    void nearest(node* root, glm::vec3 p, size_t index)
    {
        if (root == nullptr) return;

        ++visited;

        float d = glm::length2(root->location - p);
        if (best == nullptr || d < bestDistance)
        {
            bestDistance = d;
            best = root;
        }

        if (bestDistance < FLT_EPSILON) return;

        float dx = root->location[index] - p[index];
        index = ++index % 3; // 3 is xyz ( 3 dimensions )

        nearest(dx > 0 ? root->left : root->right, p, index);
        if (dx * dx >= bestDistance) return;
        nearest(dx > 0 ? root->right : root->left, p, index);
    }

    void nearest(node* root, glm::vec3 p, float range, std::vector<NodeContent>& results, size_t index)
    {
        if (root == nullptr) return;

        ++visited;

        float d = glm::length2(root->location - p);
        if (best == nullptr || d < bestDistance) {
            bestDistance = d;
            best = root;
        }

        if (d < range * range) { results.push_back({ root->location, root->payload }); }

        float dx = root->location[index] - p[index];
        index = ++index % 3; // 3 is xyz ( 3 dimensions )

        nearest(dx > 0 ? root->left : root->right, p, range, results, index);
        if (dx * dx >= bestDistance) return;
        nearest(dx > 0 ? root->right : root->left, p, range, results, index);
    }

    node* root = nullptr;

    node* best = nullptr;
    float bestDistance = FLT_MAX;
    size_t visited{};
};
