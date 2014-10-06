// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// Copyrighted under the MPLv2 and LGPLv3 by Jesse Johnson.

#ifndef __OgreVox_h_
#define __OgreVox_h_

#include "BareOgre.h"
#include "VoxelSphere.h"
#include "LargeVoxelSphere.h"
#include "VoxelTerrain.h"
#include "VoxelPlanet.h"

#include "PolyVoxCore/SurfaceMesh.h"

class OgreVox : public BareOgre
{
public:
    OgreVox(void);
    virtual ~OgreVox(void);

protected:
    virtual void createScene(void);
    bool frameRenderingQueued(const Ogre::FrameEvent& evt);

    void mouseMoved(const SDL_MouseMotionEvent& event);
    void mousePressed(const SDL_MouseButtonEvent& event);
    void mouseReleased(const SDL_MouseButtonEvent& event);

private:
    VoxelSphere* mVoxelSphere;
    Ogre::SceneNode* mVoxelSphereNode;
    LargeVoxelSphere* mLargeVoxelSphere;
    Ogre::SceneNode* mLargeVoxelSphereNode;
    VoxelTerrain* mVoxelTerrain;
    Ogre::SceneNode* mVoxelTerrainNode;
    VoxelPlanet* mVoxelPlanet;
    Ogre::SceneNode* mVoxelPlanetNode;

    Ogre::SceneNode* mMarkerNode;

    PolyVox::Vector3DInt32* mChunk;
    static const int mViewRadius = 3;
};

#endif // #ifndef __OgreVox_h_
