/**
 * Open Space Program
 * Copyright © 2019-2021 Open Space Program Project
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

#include <osp/Active/opengl/SysRenderGL.h>
#include <osp/Active/activetypes.h>

#include <Magnum/Shaders/MeshVisualizerGL.h>

namespace osp::shader
{

using MeshVisualizer = Magnum::Shaders::MeshVisualizerGL3D;

struct ACtxDrawMeshVisualizer
{
    DependRes<MeshVisualizer> m_shader;

    active::acomp_storage_t< active::ACompDrawTransform >  *m_pDrawTf{nullptr};
    active::acomp_storage_t< active::ACompMeshGL >         *m_pMeshGl{nullptr};

    bool m_wireframeOnly{false};
};

void draw_ent_visualizer(
        active::ActiveEnt ent,
        active::ACompCamera const& camera,
        active::EntityToDraw::UserData_t userData) noexcept;


void assign_visualizer(
        active::RenderGroup::ArrayView_t entities,
        active::RenderGroup::Storage_t& rStorage,
        ACtxDrawMeshVisualizer &rData);

} // namespace osp::shader
