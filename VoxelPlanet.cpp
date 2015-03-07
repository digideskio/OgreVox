// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// Copyrighted under the MPLv2 and LGPLv3 by Jesse Johnson.

#include "VoxelPlanet.h"

#include <cmath>
#include <limits>
#include <vector>

anl::CMWC4096 rnd2;
anl::CMWC4096 rnd3;

anl::CImplicitFractal p_lowland_fractal(anl::FBM, anl::GRADIENT, anl::QUINTIC);
anl::CImplicitAutoCorrect p_lowland_autocorrect(0.7, 1);
anl::CImplicitNormalizeCoords p_lowland_normalize(1);
// anl::CImplicitScaleOffset p_lowland_scale(1, 0);
anl::CImplicitScaleOffset p_lowland_scale(150, 0);

// anl::CImplicitFractal p_highland_fractal(anl::RIDGEDMULTI, anl::GRADIENT, anl::QUINTIC);
// anl::CImplicitAutoCorrect p_highland_autocorrect(0.8, 1);
// anl::CImplicitNormalizeCoords p_highland_normalize(1);
// //anl::CImplicitScaleOffset p_highland_scale(45, 0);
// anl::CImplicitScaleOffset p_highland_scale(200, 0);

// anl::CImplicitFractal p_mountains_fractal(anl::BILLOW, anl::GRADIENT, anl::QUINTIC);
// anl::CImplicitAutoCorrect p_mountains_autocorrect(0.4, 1);
// anl::CImplicitNormalizeCoords p_mountains_normalize(1);
// //anl::CImplicitScaleOffset p_mountains_scale(50, 0);
// anl::CImplicitScaleOffset p_mountains_scale(220, 0);

// anl::CImplicitFractal p_terrain_type_fractal(anl::FBM, anl::GRADIENT, anl::QUINTIC);
// anl::CImplicitAutoCorrect p_terrain_autocorrect(0, 1);
// anl::CImplicitNormalizeCoords p_terrain_normalize(1);
// anl::CImplicitCache p_terrain_type_cache;
// anl::CImplicitSelect p_highland_mountain_select;
// anl::CImplicitSelect p_highland_lowland_select;
anl::CImplicitCache p_surface_terrain;

anl::CImplicitFractal p_cave_shape1(anl::RIDGEDMULTI, anl::GRADIENT, anl::QUINTIC);
anl::CImplicitFractal p_cave_shape2(anl::RIDGEDMULTI, anl::GRADIENT, anl::QUINTIC);
anl::CImplicitBias p_cave_attenuate_bias(0.45);
// anl::CImplicitBias p_cave_shape_bias(0.45);
// anl::CImplicitBias p_cave_shape_bias(0.01);
anl::CImplicitCombiner p_cave_shape_attenuate(anl::MULT);
// anl::CImplicitAutoCorrect p_cave_shape_autocorrect(0.8, 1);
anl::CImplicitAutoCorrect p_cave_shape_autocorrect(0.5, 1);
// anl::CImplicitScaleOffset p_cave_shape_scale(40, 0);
//anl::CImplicitNormalizeCoords p_cave_shape_normalize(1);
// anl::CImplicitFractal p_cave_perturb_fractal(anl::FBM, anl::GRADIENT, anl::QUINTIC);
// anl::CImplicitAutoCorrect p_cave_perturb_autocorrect(0.8, 1);
// anl::CImplicitScaleOffset p_cave_perturb_scale(40, 0);
//anl::CImplicitNormalizeCoords p_cave_perturb_normalize(1);
// anl::CImplicitTranslateDomain p_cave_perturb;
anl::CImplicitScaleDomain p_cave_scale_domain(0.01, 0.01, 0.01);
anl::CImplicitCache p_cave_terrain;

// anl::CImplicitCombiner p_surface_cave_multiply(anl::MULT);

anl::CImplicitSphere p_sphere_gradient;

anl::CImplicitCombiner p_sphere_cave_multiply(anl::MULT);


VoxelPlanet::VoxelPlanet(
    const Ogre::String& name, Ogre::SceneNode* cameraSceneNode,
    Ogre::SceneManager* sceneMgr)
    : mCamNode(cameraSceneNode)
    , mRegionDims(128,128,128)
      // , mRegionDimensions(65,65,65)
    , mPagingDistance(32.0)
    , shutdownThreads(false)
//Ogre::ManualObject(name)
{
    // newMesh = false;
    // setDynamic(false);

    mSceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode("VoxelPlanet");

    int infinity = std::numeric_limits<int>::max() / 10;
    mChunk = PolyVox::Vector3DInt32(infinity, infinity, infinity);
}

VoxelPlanet::~VoxelPlanet()
{
    shutdownThreads = true;
    for (auto& thread : mThreads) {
        thread.join();
    }
}

// void VoxelPlanet::generateNoise()
// {
//     std::cout << "Begin configuring noise." << std::endl;

//     rnd2.setSeedTime();

//     // Lowland
//     p_lowland_fractal.setNumOctaves(2);
//     p_lowland_fractal.setFrequency(1);
//     p_lowland_fractal.setSeed(rnd2.get());

//     p_lowland_autocorrect.setSource(&p_lowland_fractal);

//     p_lowland_normalize.setSource(&p_lowland_autocorrect);

//     p_lowland_scale.setSource(&p_lowland_normalize);

//     // Highland
//     p_highland_fractal.setNumOctaves(2);
//     p_highland_fractal.setFrequency(1);
//     p_highland_fractal.setSeed(rnd2.get());

//     p_highland_autocorrect.setSource(&p_highland_fractal);

//     p_highland_normalize.setSource(&p_highland_autocorrect);

//     p_highland_scale.setSource(&p_highland_normalize);

//     // Mountains
//     p_mountains_fractal.setNumOctaves(4);
//     p_mountains_fractal.setFrequency(1.8);
//     p_mountains_fractal.setSeed(rnd2.get());

//     p_mountains_autocorrect.setSource(&p_mountains_fractal);

//     p_mountains_normalize.setSource(&p_mountains_autocorrect);

//     p_mountains_scale.setSource(&p_mountains_normalize);


//     // Surface Terrain Mix
//     p_terrain_type_fractal.setNumOctaves(3);
//     p_terrain_type_fractal.setFrequency(0.5);
//     p_terrain_type_fractal.setSeed(rnd2.get()); //FIXME use multiple seeds!

//     p_terrain_autocorrect.setSource(&p_terrain_type_fractal);

//     p_terrain_normalize.setSource(&p_terrain_autocorrect);

//     p_terrain_type_cache.setSource(&p_terrain_normalize);

//     p_highland_mountain_select.setLowSource(&p_highland_scale);
//     p_highland_mountain_select.setHighSource(&p_mountains_scale);
//     p_highland_mountain_select.setControlSource(&p_terrain_type_cache);
//     p_highland_mountain_select.setThreshold(0.85);
//     p_highland_mountain_select.setFalloff(0.15);

//     p_highland_lowland_select.setLowSource(&p_lowland_scale);
//     p_highland_lowland_select.setHighSource(&p_highland_mountain_select);
//     p_highland_lowland_select.setControlSource(&p_terrain_type_cache);
//     p_highland_lowland_select.setThreshold(0.70);
//     p_highland_lowland_select.setFalloff(0.15);

//     p_surface_terrain.setSource(&p_highland_lowland_select);

//     // Caves
//     rnd3.setSeedTime();

//     // p_cave_attenuate_bias.setSource(&p_surface_terrain);

//     p_cave_shape1.setNumOctaves(1);
//     p_cave_shape1.setFrequency(4);
//     p_cave_shape1.setSeed(rnd2.get());
//     p_cave_shape2.setNumOctaves(1);
//     p_cave_shape2.setFrequency(4);
//     p_cave_shape2.setSeed(rnd3.get());

//     p_cave_shape_attenuate.setSource(0, &p_cave_shape1);
//     p_cave_shape_attenuate.setSource(1, &p_cave_attenuate_bias);
//     p_cave_shape_attenuate.setSource(2, &p_cave_shape2);

//     p_cave_shape_autocorrect.setSource(&p_cave_shape_attenuate);

//     p_cave_shape_scale.setSource(&p_cave_shape_autocorrect);

//     //p_cave_shape_normalize.setSource(&p_cave_shape_scale);

//     p_cave_perturb_fractal.setNumOctaves(6);
//     p_cave_perturb_fractal.setFrequency(3);
//     p_cave_perturb_fractal.setSeed(rnd2.get());

//     p_cave_perturb_autocorrect.setSource(&p_cave_perturb_fractal);

//     p_cave_perturb_scale.setSource(&p_cave_perturb_autocorrect);

//     //p_cave_perturb_normalize.setSource(&p_cave_perturb_scale);

//     p_cave_perturb.setSource(&p_cave_shape_scale);
//     p_cave_perturb.setXAxisSource(&p_cave_perturb_scale);

//     p_cave_terrain.setSource(&p_cave_perturb);

//     // Combine surface and caves.
//     p_surface_cave_multiply.setSource(0, &p_cave_terrain);
//     p_surface_cave_multiply.setSource(1, &p_surface_terrain);

//     // Sphere Gradient
//     p_sphere_gradient.setCenter(0,0,0);
//     p_sphere_gradient.setRadius(&p_surface_terrain);

//     std::cout << "Done configuring noise." << std::endl;
// }


void VoxelPlanet::generateNoise()
{
    std::cout << "Begin configuring noise." << std::endl;

    rnd2.setSeedTime();

    // Lowland
    p_lowland_fractal.setNumOctaves(2);
    p_lowland_fractal.setFrequency(1);
    p_lowland_fractal.setSeed(rnd2.get());

    p_lowland_autocorrect.setSource(&p_lowland_fractal);

    p_lowland_normalize.setSource(&p_lowland_autocorrect);

    p_lowland_scale.setSource(&p_lowland_normalize);

    p_surface_terrain.setSource(&p_lowland_scale);

    // Caves
    rnd3.setSeedTime();

    p_cave_shape1.setNumOctaves(1);
    p_cave_shape1.setFrequency(4);
    p_cave_shape1.setSeed(rnd2.get());
    p_cave_shape2.setNumOctaves(1);
    p_cave_shape2.setFrequency(4);
    p_cave_shape2.setSeed(rnd3.get());

    // p_cave_shape_bias.setSource(&p_cave_shape1);

    p_cave_attenuate_bias.setSource(&p_sphere_gradient);
    
    p_cave_shape_attenuate.setSource(0, &p_cave_shape1);
    p_cave_shape_attenuate.setSource(1, &p_cave_shape2);
    p_cave_shape_attenuate.setSource(2, &p_cave_attenuate_bias);
    
    // p_cave_shape_autocorrect.setSource(&p_cave_shape_bias);
    p_cave_shape_autocorrect.setSource(&p_cave_shape_attenuate);

    // p_cave_shape_scale.setSource(&p_cave_shape_autocorrect);

    p_cave_scale_domain.setSource(&p_cave_shape_autocorrect);
 
    p_cave_terrain.setSource(&p_cave_scale_domain);
    // p_cave_terrain.setSource(&p_cave_select);

    // Sphere Gradient
    p_sphere_gradient.setCenter(0,0,0);
    p_sphere_gradient.setRadius(&p_surface_terrain);

    p_sphere_cave_multiply.setSource(0, &p_sphere_gradient);
    p_sphere_cave_multiply.setSource(1, &p_cave_terrain);

    std::cout << "Done configuring noise." << std::endl;
}


void VoxelPlanet::requestVolume(
    const PolyVox::ConstVolumeProxy<BYTE>& volume,
    const PolyVox::Region& region)
{
    //std::cout << "Begin filling VoxelPlanet." << std::endl;
    for (int z = region.getLowerCorner().getZ();
         z <= region.getUpperCorner().getZ(); z++) {
        for (int y = region.getLowerCorner().getY();
             y <= region.getUpperCorner().getY(); y++) {
            for (int x = region.getLowerCorner().getX();
                 x <= region.getUpperCorner().getX(); x++) {
                // BYTE voxel_value = p_sphere_gradient.get(x,y,z) * 255;
                BYTE voxel_value = p_sphere_cave_multiply.get(x,y,z) * 255;

                // if (static_cast<double>(voxel_value) != 0)
                //   std::cout << static_cast<double>(voxel_value) << std::endl;

                volume.setVoxelAt(x, y, z, voxel_value);
            }
        }
    }
    //std::cout << "Done filling VoxelPlanet." << std::endl;
}


void VoxelPlanet::storeVolume(
    const PolyVox::ConstVolumeProxy<BYTE>& volume,
    const PolyVox::Region& region)
{
    std::cout << "Deleting VoxelPlanet volume." << std::endl;
}


void VoxelPlanet::update(Ogre::SceneManager* sceneMgr)
{
    render(sceneMgr);
    page();
}


void VoxelPlanet::generate(float radius, Ogre::Vector3 origin)
{
    //FIXME do something with the radius and origin!
    mVolumeOrigin = origin;
    mRadius = radius;

    generateNoise();

    mVolumeData = new PolyVox::LargeVolume<BYTE>(
        &VoxelPlanet::requestVolume,
        &VoxelPlanet::storeVolume);
    mVolumeData->setMaxNumberOfBlocksInMemory(8192);
    mVolumeData->setCompressionEnabled(false);
    mVolumeData->setMaxNumberOfUncompressedBlocks(8192);

    // PolyVox::Region region(PolyVox::Vector3DInt32(-65,-65,-65),
    //                        PolyVox::Vector3DInt32(65,65,65));
    // extractSurface(region);

    mThreads.push_back(
        std::thread(&VoxelPlanet::extractSurface, this)
    );
}

// Producer.
void VoxelPlanet::enqueueSurfaceExtraction(PolyVox::Region region)
{
    mSurfaceExtractionQueue.push(region);

    // Wake consumer.
    extractionQueueNotEmpty.notify_one();
}

// Consumer.
void VoxelPlanet::extractSurface()
{
    while (not shutdownThreads) {
        std::unique_lock<std::mutex> s(mSurfaceExtractionQueueMutex);
        while (not mSurfaceExtractionQueue.empty()) {
            //std::lock_guard<std::recursive_mutex> guard(mutex);
            mSurfaceExtractionMutex.lock();
            PolyVox::Region region = mSurfaceExtractionQueue.front();
            Ogre::ManualObject* man_obj = new Ogre::ManualObject("VoxelPlanet");
            _extractSurface(region, man_obj);
            mSurfaceExtractionQueue.pop();
            mSurfaceRenderQueue.push(man_obj);
            mSurfaceExtractionMutex.unlock();
        }
        // Sleep until woken by producer.
        extractionQueueNotEmpty.wait(
            s,
            [this](){
                return not mSurfaceExtractionQueue.empty();
            }
        );
    }
}


void VoxelPlanet::_extractSurface(
    PolyVox::Region region, Ogre::ManualObject* man_obj)
{
    PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal> mesh;

    if (DEBUG) { std::cout << "Begin extracting surface." << std::endl; }
    PolyVox::MarchingCubesSurfaceExtractor<PolyVox::LargeVolume<BYTE> >
        //PolyVox::CubicSurfaceExtractorWithNormals<PolyVox::LargeVolume<BYTE> >
        surface_extractor(mVolumeData, region, &mesh);
    surface_extractor.execute();
    if (DEBUG) { std::cout << "Done extracting surface." << std::endl; }

    man_obj->setDynamic(false);
    man_obj->clear();
    if (DEBUG) { std::cout << "Begin building manual object." << std::endl; }
    // man_obj->begin("SimpleTexture",
    //                Ogre::RenderOperation::OT_TRIANGLE_LIST);
    man_obj->begin("Terrain",
                   Ogre::RenderOperation::OT_TRIANGLE_LIST);
    //man_obj->begin("BareWhite",
    //               Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        const std::vector<PolyVox::PositionMaterialNormal>& vecVertices =
            mesh.getVertices();
        const std::vector<uint32_t>& vecIndices = mesh.getIndices();
        unsigned int uLodLevel = 0;
        int beginIndex = mesh.m_vecLodRecords[uLodLevel].beginIndex;
        int endIndex = mesh.m_vecLodRecords[uLodLevel].endIndex;

        for (int index = beginIndex; index < endIndex; ++index) {
            const PolyVox::PositionMaterialNormal& vertex =
                vecVertices[vecIndices[index]];
            const PolyVox::Vector3DFloat& v3dVertexPos = vertex.getPosition();
            const PolyVox::Vector3DFloat& v3dVertexNormal = vertex.getNormal();

            const PolyVox::Vector3DFloat v3dFinalVertexPos = v3dVertexPos +
                static_cast<PolyVox::Vector3DFloat>(mesh.m_Region.getLowerCorner());
            man_obj->position(v3dFinalVertexPos.getX(),
                              v3dFinalVertexPos.getY(),
                              v3dFinalVertexPos.getZ());
            man_obj->normal(v3dVertexNormal.getX(),
                            v3dVertexNormal.getY(),
                            v3dVertexNormal.getZ());
            man_obj->colour(1, 0, 0, 0.5);
            // Repeat mesh every 4 units.
            man_obj->textureCoord(v3dFinalVertexPos.getX()/4.0f,
                                  v3dFinalVertexPos.getY()/4.0f,
                                  v3dFinalVertexPos.getZ()/4.0f);
        }
    }
    if (DEBUG) { std::cout << "Done building manual object." << std::endl; }
    //end();
    //newMesh = true;
}


/** Raycast from a given position and direction for a specific
    distance.
 **/
Raycast VoxelPlanet::raycast(
    Ogre::Vector3 origin, Ogre::Vector3 direction, float distance)
{
    Raycast raycast;
    //origin = origin / 90;
    PolyVox::Vector3DFloat orig(origin.x,origin.y,origin.z);
    PolyVox::Vector3DFloat dir(direction.x,direction.y,direction.z);
    dir.normalise();
    dir *= distance;

    PolyVox::RaycastResult raycast_result = PolyVox::raycastWithDirection(
        mVolumeData, orig, dir, raycast
    );
    raycast.result = raycast_result;

    return raycast;
}


void VoxelPlanet::render(Ogre::SceneManager* sceneMgr)
{
    // OpenGL state machine requires all OpenGL calls to
    // occur in main thread.  Thus Ogre::ManualObject must
    // end() here to fill OpenGL buffers.
    //TODO: When switching to OpenGL 3+, the main object
    // can be ended by other threads than main.
    if (not mSurfaceRenderQueue.empty()) {
        //std::cout << "new mesh!" << std::endl;
        mSurfaceRenderMutex.lock();
        Ogre::ManualObject* man_obj = mSurfaceRenderQueue.front();
        //Ogre::SceneNode* man_obj_node;
        man_obj->end();
        mSurfaceRenderQueue.pop();
        //TODO destroy previous manual object
        if (sceneMgr->hasSceneNode(man_obj->getName()))
        {
            sceneMgr->destroyManualObject(man_obj->getName());
            //mManualObjectNode = sceneMgr->getSceneNode(man->getName());
        }
        // convertToMesh("VoxelPlanetMesh");
        // Ogre::Entity * voxelPlanetEntity =
        //   mSceneMgr->createEntity("VoxelPlanetEntity","VoxelPlanetMesh");
        // mSceneNode->attachObject(voxelPlanetEntity);
        mSceneNode->detachAllObjects();
        mSceneNode->attachObject(man_obj);
        //mSceneNode->setVisible(true);
        mSurfaceRenderMutex.unlock();
    }
}


void VoxelPlanet::page()
{
    PolyVox::Vector3DInt32 chunk = getCurrentChunk();
    
    if (chunk == mChunk
        || abs((chunk - mChunk).length()) < mPagingDistance) {
        if (DEBUG)
            std::cout << "distance: " << (chunk - mChunk).length() << std::endl;
        return;
    }

    if (DEBUG) std::cout << "new chunk!" << std::endl;
    
    mChunk = chunk;    

    PolyVox::Region region(mChunk - mRegionDims,
                           mChunk + mRegionDims);

    enqueueSurfaceExtraction(region);
}


PolyVox::Vector3DInt32 VoxelPlanet::getCurrentChunk()
{
    // Position of camera relative to planet origin.
    Ogre::Vector3 pos = mCamNode->getPosition() - mSceneNode->getPosition();

    PolyVox::Vector3DInt32 chunk(
        floor(pos.x / mChunkSize / 10),
        floor(pos.y / mChunkSize / 10),
        floor(pos.z / mChunkSize / 10)
    );

    if (DEBUG)
    {
        std::cout << "current pos: " << pos << std::endl;
        std::cout << "current chunk: " << chunk << std::endl;
        std::cout << "mChunk: " << mChunk << std::endl;
    }

    return chunk;
}
