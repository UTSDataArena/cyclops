

#ifndef __CY_CAMERA_MANIPULATOR__
#define __CY_CAMERA_MANIPULATOR__


#include <osg/Node>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osgGA/NodeTrackerManipulator>
// flags defined here
#include <osgGA/StandardManipulator>

#include "cyclopsConfig.h"
#include "Entity.h"

#include <queue>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaOsg/omegaOsg.h>

namespace cyclops {
    using namespace omega;
    using namespace omegaOsg;


    class NodeTrackerManipulator;

    ///////////////////////////////////////////////////////////////////////////
    class CY_API CameraManipulator: public ReferenceType
    {
    public:
    	static CameraManipulator* create();


		CameraManipulator();

		void setTrackedNode(Entity* entity);
		void onEvent(Event* event);
		void setHomeEye(const Vector3f& eye);





	protected:
		osg::ref_ptr<NodeTrackerManipulator> myManipulator;
		osg::ref_ptr<osg::Node> myTrackedNode;
    };



	class NodeTrackerManipulator : public osgGA::NodeTrackerManipulator
	{
		    typedef osgGA::NodeTrackerManipulator inherited;
	public:
			NodeTrackerManipulator(Camera* omegaCam, int flags = DEFAULT_SETTINGS);

	        bool handle(Event* event);
	        void handleMouseMove(Event *event);
	        void handleMouseDrag(Event* event);
			bool handleMouse();
			bool handleMouseRelease(Event *event);
			bool handleMousePush(Event *event);
			bool handleMouseWheel(Event *event);
	
			void addMouseEvent( Event *newEvent );
			void flushMouseEventStack();


	protected:
		Event* currentEvent = NULL;
		Event* lastEvent = NULL;
		Camera* _omegaCam;
	};
}

#endif
