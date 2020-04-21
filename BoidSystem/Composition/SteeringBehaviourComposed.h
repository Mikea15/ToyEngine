#pragma once

#include "Definitions.h"
#include "AgentComposition.h"

#include "Behaviours/Seek.h"
#include "Behaviours/Flee.h"
#include "Behaviours/Wander.h"
#include "Behaviours/Arrive.h"
#include "Behaviours/WorldBounds.h"
#include "Behaviours/Alignment.h"
#include "Behaviours/Separation.h"
#include "Behaviours/Cohesion.h"

class SteeringBehaviourComposed
{
public:
    SteeringBehaviourComposed(AgentComposition* agent)
        : agent(agent)
        , wander(agent)
        , seek(agent)
        , flee(agent)
        , arrive(agent)
        , alignment(agent)
        , cohesion(agent)
        , separation(agent)
        , worldBounds(agent)
    {}

    glm::vec3 CalculateWeighedSum()
    {
        // Steering Bit
        glm::vec3 force = {};
        const Properties& properties = *agent->m_properties;

        if (agent->HasFeature(eWallLimits)) 
        { 
            force += properties.m_weightWallLimits * worldBounds.Calculate();
        }
        if (agent->HasFeature(eWander)) 
        {
            force += properties.m_weightWander * wander.Calculate();
        }
        if (agent->HasFeature(eSeek)) 
        { 
            force += properties.m_weightSeek * seek.Calculate();
        }
        if (agent->HasFeature(eArrive)) 
        { 
            force += properties.m_weightArrive * arrive.Calculate();
        }
        if (agent->HasFeature(eFlee)) 
        { 
            force += properties.m_weightFlee * flee.Calculate();
        }
#if 0
        if (agent->HasFeature(eFleeRanged)) 
        { 
            force += properties.m_weightFlee * FleeRanged(m_fleePos);
        }
#endif

        if (agent->HasFeature(eSeparation)) 
        { 
            force += properties.m_weightSeparation * separation.Calculate();
        }
        if (agent->HasFeature(eCohesion)) 
        { 
            force += properties.m_weightCohesion * cohesion.Calculate();
        }
        if (agent->HasFeature(eAlignment)) 
        { 
            force += properties.m_weightAlignment * alignment.Calculate();
        }

        return glm::clamp(force, -properties.m_maxForce, properties.m_maxForce);
    }

private:
    AgentComposition* agent;

    WanderBehaviour wander;
    SeekBehaviour seek;
    FleeBehaviour flee;
    ArriveBehaviour arrive;

    AlignmentBehaviour alignment;
    CohesionBehaviour cohesion;
    SeparationBehaviour separation;

    WorldBoundsBehaviour worldBounds;
};