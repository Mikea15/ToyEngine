#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <sstream>

#define TO_STRING( x ) #x
#define TO_STRING2( x ) TO_STRING(x)

//****************
// CLASS_DECLARATION
//
// This macro must be included in the declaration of any subclass of Component.
// It declares variables used in type checking.
//****************
#define CLASS_DECLARATION( classname )                                         \
public:                                                                        \
    static const std::size_t Type;                                             \
    virtual bool IsClassType( const std::size_t classType ) const override;    \

//****************
// CLASS_DEFINITION
// 
// This macro must be included in the class definition to properly initialize 
// variables used in type checking. Take special care to ensure that the 
// proper parentclass is indicated or the run-time type information will be
// incorrect. Only works on single-inheritance RTTI.
//****************
#define CLASS_DEFINITION( parentclass, childclass )                                         \
const std::size_t childclass::Type = std::hash< std::string >()( TO_STRING( childclass ) ); \
bool childclass::IsClassType( const std::size_t classType ) const {                         \
        if ( classType == childclass::Type )                                                \
            return true;                                                                    \
        return parentclass::IsClassType( classType );                                       \
}                                                                                           \

union SDL_Event;
class Game;

class ISystemComponent
{
public:
    static const std::size_t Type;
    virtual bool IsClassType(const std::size_t classType) const
    {
        return classType == Type;
    }

    ISystemComponent() {};
    ISystemComponent(std::string&& move)
        : m_value(move)
    { }

    virtual ~ISystemComponent() = default;

    virtual void Initialize(Game* game) = 0;
    virtual void HandleInput(SDL_Event* event) = 0;
    virtual void PreUpdate(float frameTime) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(float alpha) = 0;
    virtual void RenderUI() = 0;
    virtual void Cleanup() = 0;

public:
    std::string m_value = "uninitialized";
};

