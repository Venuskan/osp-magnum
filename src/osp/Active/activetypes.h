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
#pragma once

// IWYU pragma: begin_exports

#include <entt/core/fwd.hpp>          // for entt::id_type
#include <entt/entity/view.hpp>       // for basic_view
#include <entt/entity/registry.hpp>   // for entt::basic_registry
#include <entt/entity/sparse_set.hpp> // for entt::sparse_set

// IWYU pragma: end_exports

namespace osp::active
{

class SysNewton;

class ActiveScene;

struct ACompCamera;

struct ACompTransform;

inline constexpr unsigned gc_heir_physics_level = 1;

enum class ActiveEnt: entt::id_type {};

using ActiveReg_t = entt::basic_registry<ActiveEnt>;

template<typename COMP_T>
using acomp_storage_t = entt::basic_storage<ActiveEnt, COMP_T>;

}
