/**
 * Open Space Program
 * Copyright © 2019-2020 Open Space Program Project
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <iostream>

#include <osp/Active/ActiveScene.h>
#include <osp/Active/SysDebugRender.h>
#include <osp/Active/SysVehicle.h>
#include <osp/Active/physics.h>

#include "Rocket.h"
#include "osp/Resource/AssetImporter.h"
#include "adera/Shaders/Phong.h"
#include "adera/Shaders/PlumeShader.h"
#include "osp/Resource/blueprints.h"
#include "osp/PhysicsConstants.h"
#include "adera/SysExhaustPlume.h"
#include "adera/Plume.h"
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

using namespace adera::active::machines;
using namespace osp::active;
using namespace osp;

void MachineRocket::propagate_output(WireOutput* output)
{

}

WireInput* MachineRocket::request_input(WireInPort port)
{
    return existing_inputs()[port];
}

WireOutput* MachineRocket::request_output(WireOutPort port)
{
    return existing_outputs()[port];
}

std::vector<WireInput*> MachineRocket::existing_inputs()
{
    std::vector<WireInput*> inputs;
    inputs.reserve(3 + m_resourceLines.size());

    inputs.insert(inputs.begin(), {&m_wiGimbal, &m_wiIgnition, &m_wiThrottle});
    for (auto& resource : m_resourceLines)
    {
        inputs.push_back(&resource.m_source);
    }
    return inputs;
}

std::vector<WireOutput*> MachineRocket::existing_outputs()
{
    return {};
}

void SysMachineRocket::add_functions(ActiveScene &rScene)
{
    rScene.debug_update_add(rScene.get_update_order(), "mach_rocket", "controls", "physics",
                            &SysMachineRocket::update_physics);
    rScene.debug_update_add(rScene.get_update_order(), "mach_rocket_construct", "vehicle_activate", "vehicle_modification",
                            &SysMachineRocket::update_construct);
}

//void SysMachineRocket::update_sensor()
//{
//}

void SysMachineRocket::update_construct(ActiveScene& rScene)
{
    auto view = rScene.get_registry()
            .view<osp::active::ACompVehicle,
                  osp::active::ACompVehicleInConstruction>();

    machine_id_t const id = mach_id<SysMachineRocket>();

    for (auto [vehEnt, rVeh, rVehConstr] : view.each())
    {
        // Check if the vehicle blueprint might store UserControls
        if (rVehConstr.m_blueprint->m_machines.size() <= id)
        {
            continue;
        }

        BlueprintVehicle const& vehBp = *rVehConstr.m_blueprint;

        // Initialize all MachineRockets in the vehicle
        for (BlueprintMachine const &mach : vehBp.m_machines[id])
        {
            // Get part
            ActiveEnt partEnt = rVeh.m_parts[mach.m_blueprintIndex];

            // Get machine entity previously reserved by SysVehicle
            auto& machines = rScene.reg_get<ACompMachines>(partEnt);
            ActiveEnt machEnt = machines.m_machines[machines.m_numInit];
            machines.m_numInit ++;

            BlueprintPart const& partBp = vehBp.m_blueprints[mach.m_blueprintIndex];
            instantiate(rScene, machEnt,
                        vehBp.m_prototypes[partBp.m_protoIndex]
                                ->m_protoMachines[mach.m_protoMachineIndex],
                        mach);
        }
    }
}


void SysMachineRocket::update_physics(ActiveScene& rScene)
{
    auto view = rScene.get_registry().view<MachineRocket, ACompTransform>();

    for (ActiveEnt ent : view)
    {
        auto &machine = view.get<MachineRocket>(ent);
        if (!machine.m_enable)
        {
            continue;
        }

        machine.m_powerOutput = 0.0f;  // Will be set later if engine is on

        // Check for nonzero throttle, continue otherwise
        WireData *pThrottle = machine.m_wiThrottle.connected_value();
        wiretype::Percent* pThrotPercent = nullptr;
        if (pThrottle != nullptr)
        {
            using wiretype::Percent;
            pThrotPercent = std::get_if<Percent>(pThrottle);
            if ((pThrotPercent == nullptr) || !(pThrotPercent->m_value > 0.0f))
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        // Check for adequate resource inputs
        bool fail = false;
        for (auto& resource : machine.m_resourceLines)
        {
            if (auto const* pipe = resource.m_source.get_if<wiretype::Pipe>();
                pipe != nullptr)
            {
                
                if (auto const* src = rScene.reg_try_get<MachineContainer>(pipe->m_source);
                    src != nullptr)
                {
                    uint64_t required = resource_units_required(rScene, machine,
                        pThrotPercent->m_value, resource);
                    if (src->check_contents().m_quantity > required)
                    {
                        continue;
                    }
                }
            }
            fail = true;
            break;
        }
        if (fail)
        {
            continue;
        }

        // Perform physics calculation

        // Get rigidbody ancestor and its transformation component
        auto const* pRbAncestor =
            SysPhysics_t::try_get_or_find_rigidbody_ancestor(rScene, ent);
        auto& rCompRb = rScene.reg_get<ACompRigidBody_t>(pRbAncestor->m_ancestor);
        auto const& rCompTf = rScene.reg_get<ACompTransform>(pRbAncestor->m_ancestor);

        if (WireData *ignition = machine.m_wiIgnition.connected_value())
        {

        }

        Matrix4 relTransform = pRbAncestor->m_relTransform;

        /* Compute thrust force
         * Thrust force is defined to be along +Z by convention.
         * Obtains thrust vector in rigidbody space
         */
        Vector3 thrustDir = relTransform.transformVector(Vector3{0.0f, 0.0f, 1.0f});
        float thrustMag = machine.m_params.m_maxThrust * pThrotPercent->m_value;
        // Take thrust in rigidbody space and apply to RB in world space
        Vector3 thrust = thrustMag * thrustDir;
        Vector3 worldThrust = rCompTf.m_transform.transformVector(thrust);
        SysPhysics_t::body_apply_force(rCompRb, worldThrust);

        // Obtain point where thrust is applied relative to RB CoM
        Vector3 location = relTransform.translation();
        // Compute worldspace torque from engine location, thrust vector
        Vector3 torque = Magnum::Math::cross(location, thrust);
        Vector3 worldTorque = rCompTf.m_transform.transformVector(torque);
        SysPhysics_t::body_apply_torque(rCompRb, worldTorque);
        
        rCompRb.m_inertiaDirty = true;

        // Perform resource consumption calculation
        for (MachineRocket::ResourceInput const& resource : machine.m_resourceLines)
        {
            // Pipe must be non-null since we checked earlier
            const auto& pipe = *resource.m_source.get_if<wiretype::Pipe>();
            auto& src = rScene.reg_get<MachineContainer>(pipe.m_source);

            uint64_t required = resource_units_required(rScene, machine,
                pThrotPercent->m_value, resource);
            uint64_t consumed = src.request_contents(required);

            SPDLOG_LOGGER_TRACE(m_scene.get_application().get_logger(),
                                "Consumed {} units of fuel, {} remaining",
                consumed, src.check_contents().m_quantity);
        }

        // Set output power level (for plume effect)
        // TODO: later, take into account low fuel pressure, bad mixture, etc.
        machine.m_powerOutput = pThrotPercent->m_value;
    }
}

void SysMachineRocket::attach_plume_effect(ActiveScene &rScene, ActiveEnt ent)
{
    ActiveEnt plumeNode = entt::null;

    auto findPlumeHandle = [&rScene, &plumeNode](ActiveEnt ent)
    {
        auto const& node = rScene.reg_get<ACompHierarchy>(ent);
        static constexpr std::string_view nodePrefix = "fx_plume_";
        if (0 == node.m_name.compare(0, nodePrefix.size(), nodePrefix))
        {
            plumeNode = ent;
            return EHierarchyTraverseStatus::Stop;  // terminate search
        }
        return EHierarchyTraverseStatus::Continue;
    };

    rScene.hierarchy_traverse(ent, findPlumeHandle);

    if (plumeNode == entt::null)
    {
        SPDLOG_LOGGER_ERROR(m_scene.get_application().get_logger(),
                          "ERROR: could not find plume anchor for MachineRocket {}", ent);
        return;
    }

    SPDLOG_LOGGER_INFO(m_scene.get_application().get_logger(), "MachineRocket {}\'s associated plume: {}",
        ent, plumeNode);
   

    // Get plume effect
    Package& pkg = rScene.get_application().debug_find_package("lzdb");
    std::string_view plumeAnchorName = rScene.reg_get<ACompHierarchy>(plumeNode).m_name;
    std::string_view effectName = plumeAnchorName.substr(3, plumeAnchorName.length() - 3);
    DependRes<PlumeEffectData> plumeEffect = pkg.get<PlumeEffectData>(effectName);
    if (plumeEffect.empty())
    {
      SPDLOG_LOGGER_ERROR(m_scene.get_application().get_logger(),
                          "ERROR: couldn't find plume effect  {}", effectName);
        return;
    }

    rScene.reg_emplace<ACompExhaustPlume>(plumeNode, ent, plumeEffect);
}

// TODO: come up with a better solution to this, and also replace config maps
//       and variants with something else entirely
template<typename TYPE_T>
TYPE_T const& config_get_if(
        std::map<std::string, config_node_t, std::less<>> const& nodeMap,
        std::string_view field, TYPE_T&& defaultValue)
{
    auto found = nodeMap.find(field);
    if (found == nodeMap.end())
    {
        return defaultValue;
    }
    return std::get<TYPE_T>(found->second);
//    if (TYPE_T const* value = std::get_if<TYPE_T>(&found->second); value != nullptr)
//    {
//        return *value;
//    }
//    return defaultValue;
}

MachineRocket& SysMachineRocket::instantiate(
        osp::active::ActiveScene& rScene,
        osp::active::ActiveEnt ent,
        osp::PrototypeMachine const& config,
        osp::BlueprintMachine const& settings)
{
    // Read engine config
    MachineRocket::Parameters params;
    params.m_maxThrust = config_get_if<double>(config.m_config, "thrust", 42.0);
    params.m_specImpulse = config_get_if<double>(config.m_config, "Isp", 42.0);

    std::string const& fuelIdent = config_get_if<std::string>(config.m_config, "fueltype", "");
    Path resPath = decompose_path(fuelIdent);
    Package& pkg = rScene.get_application().debug_find_package(resPath.prefix);
    DependRes<ShipResourceType> fuel = pkg.get<ShipResourceType>(resPath.identifier);

    std::vector<MachineRocket::input_t> inputs;
    if (!fuel.empty())
    {
        inputs.push_back({std::move(fuel), 1.0f});
    }

    attach_plume_effect(rScene, ent);
    return rScene.reg_emplace<MachineRocket>(ent, std::move(params), std::move(inputs));
}

uint64_t SysMachineRocket::resource_units_required(
    osp::active::ActiveScene const& scene,
    MachineRocket const& machine, float throttle,
    MachineRocket::ResourceInput const& resource)
{
    float massFlowRate = resource_mass_flow_rate(machine,
        throttle, resource);
    float massFlow = massFlowRate * scene.get_time_delta_fixed();
    return resource.m_type->resource_quantity(massFlow);
}

constexpr float SysMachineRocket::resource_mass_flow_rate(MachineRocket const& machine,
    float throttle, MachineRocket::ResourceInput const& resource)
{
    float thrustMag = machine.m_params.m_maxThrust * throttle;
    float massFlowRateTot = thrustMag /
        (phys::constants::g_0 * machine.m_params.m_specImpulse);

    return massFlowRateTot * resource.m_massRateFraction;
}
