#include "ISystemComponent.h"

const std::size_t ISystemComponent::Type = std::hash<std::string>()(TO_STRING(ISystemComponent));