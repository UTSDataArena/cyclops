/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided 
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	A scene layer is an abstract class that groups entities together for a
 *  variety of purposes: lighting, clipping, LOD and so on. SceneLayers can form
 *	a hyerarchy similar to the scene node tree, but the scene layer tree is used
 *	to represent properties of the scene different than spatial transformations.
 ******************************************************************************/
#include "cyclops/SceneLayer.h"

using namespace omega;
using namespace cyclops;

///////////////////////////////////////////////////////////////////////////////
SceneLayer::SceneLayer(SceneManager* scene):
	mySceneManager(scene)
{
	myRoot = new osg::Group();
	mySceneManager->getOsgRoot()->addChild(myRoot);
}

///////////////////////////////////////////////////////////////////////////////
SceneLayer::~SceneLayer()
{
	mySceneManager->getOsgRoot()->removeChild(myRoot);
}

///////////////////////////////////////////////////////////////////////////////
void SceneLayer::addEntity(Entity* e)
{
	// Add an entity to the clip plane: we are now in charge of the entity
	// scene change events, so we set ourselves as the entity listeners
	oassert(e != NULL);
	e->removeListener(mySceneManager);
	e->addListener(this);

	// Now detach the entity osg node from the scene and make it a child of the
	// clipnode
	mySceneManager->getOsgRoot()->removeChild(e->getOsgNode());
	myRoot->addChild(e->getOsgNode());
}

///////////////////////////////////////////////////////////////////////////////
void SceneLayer::removeEntity(Entity* e)
{
	// Entity removed from the clip plane: reset the scene manager as the entity
	// listener for scene change events.
	oassert(e != NULL);
	e->removeListener(this);
	e->addListener(mySceneManager);

	// Now detach the entity osg node from the scene and make it a child of the
	// scene
	myRoot->removeChild(e->getOsgNode());
	mySceneManager->getOsgRoot()->addChild(e->getOsgNode());
}

///////////////////////////////////////////////////////////////////////////////
void SceneLayer::onAttachedToScene(SceneNode* source)
{
	// Called by entities when their parent node changes. Update the osg parent node
	// accordingly.
	Entity* e = dynamic_cast<Entity*>(source);
	if(e != NULL)
	{
		myRoot->addChild(e->getOsgNode());
	}
}

///////////////////////////////////////////////////////////////////////////////
void SceneLayer::onDetachedFromScene(SceneNode* source)
{
	// Called by entities when their parent node changes. Update the osg parent node
	// accordingly.
	Entity* e = dynamic_cast<Entity*>(source);
	if(e != NULL)
	{
		myRoot->addChild(e->getOsgNode());
	}
}
