#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sstream>

#include "Core/ISystemComponent.h"

class SystemComponentManager
{
public:
    SystemComponentManager();
    ~SystemComponentManager();

    void Initialize(Game* game);
    void HandleInput(SDL_Event* event);
    void PreUpdate(float frameTime);
    void Update(float deltaTime);
    void Render(float alpha);
    void RenderUI();
    void Cleanup();

    template< class ComponentType, typename... Args >
    void AddComponent(Args&&... params);

    template< class ComponentType >
    ComponentType& GetComponent();

    template< class ComponentType >
    bool RemoveComponent();

    template< class ComponentType >
    std::vector< ComponentType* > GetComponents();

    template< class ComponentType >
    int RemoveComponents();

private:
    std::vector<std::unique_ptr<ISystemComponent>> m_components;
    std::unordered_map<std::size_t, std::unique_ptr<ISystemComponent>> m_componentMap;
};

//***************
// SystemComponentManager::AddComponent
// perfect-forwards all params to the ComponentType constructor with the matching parameter list
// DEBUG: be sure to compare the arguments of this fn to the desired constructor to avoid perfect-forwarding failure cases
// EG: deduced initializer lists, decl-only static const int members, 0|NULL instead of nullptr, overloaded fn names, and bitfields
//***************
template< class ComponentType, typename... Args >
void SystemComponentManager::AddComponent(Args&&... params)
{
    m_components.emplace_back(std::make_unique< ComponentType >(std::forward< Args >(params)...));
}

//***************
// SystemComponentManager::GetComponent
// returns the first component that matches the template type
// or that is derived from the template type
// EG: if the template type is Component, and components[0] type is BoxCollider
// then components[0] will be returned because it derives from Component
//***************
template<class ComponentType>
inline ComponentType& SystemComponentManager::GetComponent()
{
    for (auto&& component : m_components)
    {
        if (component->IsClassType(ComponentType::Type))
        {
            return *static_cast<ComponentType*>(component.get());
        }
    }
    return *std::unique_ptr<ComponentType>(nullptr);
}

//***************
// SystemComponentManager::RemoveComponent
// returns true on successful removal
// returns false if components is empty, or no such component exists
//***************
template<class ComponentType>
inline bool SystemComponentManager::RemoveComponent()
{
    if (m_components.empty())
        return false;

    auto& index = std::find_if(m_components.begin(), m_components.end(),
        [classType = ComponentType::Type](auto& component)
    {
        return component->IsClassType(classType);
    });
    bool success = index != m_components.end();
    if (success)
    {
        m_components.erase(index);
    }

    return success;
}

//***************
// SystemComponentManager::GetComponents
// returns a vector of pointers to the the requested component template type following the same match criteria as GetComponent
// NOTE: the compiler has the option to copy-elide or move-construct componentsOfType into the return value here
// TODO: pass in the number of elements desired (eg: up to 7, or only the first 2) which would allow a std::array return value,
// except there'd need to be a separate fn for getting them *all* if the user doesn't know how many such Components the GameObject has
// TODO: define a GetComponentAt<ComponentType, int>() that can directly grab up to the the n-th component of the requested type
//***************
template<class ComponentType>
inline std::vector<ComponentType*> SystemComponentManager::GetComponents()
{
    std::vector<ComponentType> componentsOfType;
    for (auto&& component : m_components)
    {
        if (component->IsClassType(ComponentType::Type))
        {
            componentsOfType.emplace_back(static_cast<ComponentType*>(component.get()));
        }
    }
    return componentsOfType;
}

//***************
// SystemComponentManager::RemoveComponents
// returns the number of successful removals, or 0 if none are removed
//***************
template<class ComponentType>
inline int SystemComponentManager::RemoveComponents()
{
    if (m_components.empty())
        return 0;

    int numRemoved = 0;
    bool success = false;

    do {
        auto& index = std::find_if(m_components.begin(), m_components.end(),
            [classType = ComponentType::Type](auto& component)
        {
            return component->IsClassType(classType);
        });

        success = index != m_components.end();

        if (success)
        {
            m_components.erase(index);
            ++numRemoved;
        }
    } while (success);

    return numRemoved;
}
