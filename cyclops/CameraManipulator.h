

#ifndef __CY_CAMERA_MANIPULATOR__
#define __CY_CAMERA_MANIPULATOR__
#include "ManipulatorHandler.h"


#include <osg/Node>
#include <osg/Camera>
#include <osg/MatrixTransform>
#include <osgGA/NodeTrackerManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/FirstPersonManipulator>

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
    class OrbitManipulator;
    class AbstractOmegaManipulator;

    ///////////////////////////////////////////////////////////////////////////
    class CY_API CameraManipulator: public CameraController
    {
    public:
    	static CameraManipulator* create();


		CameraManipulator();

		void onEvent(Event* event);
		void setManipulator(AbstractOmegaManipulator* manipulator);
		void setEventAdapter(EventAdapter *eventAdapter);
		// void setManipulator(NodeTrackerManipulator* manipulator);

		// events should be only handled by callng onEvent (explicit callback)
		// this enables the python subclasses to process events
		virtual void handleEvent(const Event& evt) {}
		virtual void update(const UpdateContext& context);

	protected:
		AbstractOmegaManipulator* myManipulator;
		Camera *myCamera;
    };

  	//=====================================================================

	class AbstractOmegaManipulator  {
	public:
		
		virtual void updateOmegaCamera(Camera *cam);
		virtual void setHomeEye(const Vector3f& eye);
		virtual void setHome(const Vector3f& eye, const Vector3f& center, const Vector3f& up = Vector3f(0,0,1));
		// virtual void setSceneNode(osg::Node* node) { osgCam.setChild(0, node);}
		
		// following functions must be implemented by all descendants
		virtual bool handle(Event* event){
			omsg("calling abstract method handle(event)");
		}; 

		virtual void _getTransformation(osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up) { 
			omsg("calling abstract method _getTransformation");
		}
		virtual void _setHomePosition(osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up) {
			omsg("calling abstract method _setHomePosition" );
		}
		virtual void _getHomePosition(osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up) {
			omsg("calling abstract method _getHomePosition");
		}

		virtual void _home(double time){
			omsg("calling abstract method _home");
		}

		virtual void _setNode( osg::Node* node ){
			omsg("calling abstract method _setNode");			
		}

		virtual void setEventAdapter(EventAdapter *eventAdapter){
			omsg("calling abstract methid setEventAdapter");
		}

		virtual void setNewOptions(){
			omsg("calling abstrect setNewOptions");
		}


	protected:
		// a non-rendering camera, used for intersections
		// osg::Camera osgCam;
	};

    
  	//=====================================================================


#define STANDARDHANDLERS() bool handle(Event* event){ return handler.handleEvent(event); } \
			bool handleMouseDrag(Event* event){ return handler.handleMouseDrag(event); } \
			bool handleMousePush(Event * event){ return handler.handleMousePush(event); }\
			bool handleMouseRelease(Event *event){ return handler.handleMouseRelease(event); } \
			bool handleMouseMove(Event *event){ return false; } \
			bool handleMouseWheel(Event *event){ return handler.handleMouseWheel(event); } \
			bool handleKeyDown(Event *event) { return handler.handleKeyDown(event);} \
			bool handleKeyUp(Event *event){ return handler.handleKeyUp(event);} \
			void addMouseEvent( Event *event ){ handler.addMouseEvent(event); } \
			/**/



#define STANDARDWRAPPERS() \
			void _getTransformation(osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up){ getTransformation(eye, center, up); } \
			void _setHomePosition(osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up) { setHomePosition(eye, center, up); } \
			void _getHomePosition(osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up) { getHomePosition(eye, center, up);	} \
			void _home(double time){ home(time);} \
			void _setNode(osg::Node* node) {setNode(node);} \
			void setEventAdapter(EventAdapter *eventAdapter) {handler.setAdapter(eventAdapter);}\
			void setNewOptions() {handler.setNewOptions();} \
			/**/

	


	class NodeTrackerManipulator : public osgGA::NodeTrackerManipulator, public AbstractOmegaManipulator
	{
		    typedef osgGA::NodeTrackerManipulator inherited;
		    friend ManipulatorHandler<NodeTrackerManipulator>;
	public:
			NodeTrackerManipulator(int flags = DEFAULT_SETTINGS);
			static NodeTrackerManipulator *create() { return new NodeTrackerManipulator();}

			STANDARDHANDLERS()

			STANDARDWRAPPERS()

			void setTrackedNode(Entity* entity);
			void setModes(std::string trackerMode, std::string rotationMode);

	protected:
		ManipulatorHandler<NodeTrackerManipulator> handler;
	};


	class OrbitManipulator : public osgGA::OrbitManipulator, public AbstractOmegaManipulator
	{
		    typedef osgGA::OrbitManipulator inherited;
		    friend ManipulatorHandler<OrbitManipulator>;
	public:
			OrbitManipulator(int flags = DEFAULT_SETTINGS);
			static OrbitManipulator *create() { return new OrbitManipulator();}

			STANDARDHANDLERS()

			STANDARDWRAPPERS()

	protected:
		ManipulatorHandler<OrbitManipulator> handler;
	};



	class TerrainManipulator : public osgGA::TerrainManipulator, public AbstractOmegaManipulator
	{
		    typedef osgGA::TerrainManipulator inherited;
		    friend ManipulatorHandler<TerrainManipulator>;
	public:
			TerrainManipulator(int flags = DEFAULT_SETTINGS);
			static TerrainManipulator *create() { return new TerrainManipulator();}

			STANDARDHANDLERS()

			STANDARDWRAPPERS()

			void setTerrainNode(Entity *entity);

	protected:
		ManipulatorHandler<TerrainManipulator> handler;
	};


	class FirstPersonManipulator : public osgGA::FirstPersonManipulator, public AbstractOmegaManipulator
	{
		    typedef osgGA::FirstPersonManipulator inherited;
		    friend ManipulatorHandler<FirstPersonManipulator>;
	public:
			FirstPersonManipulator(int flags = DEFAULT_SETTINGS);
			static FirstPersonManipulator *create() { return new FirstPersonManipulator();}


			STANDARDWRAPPERS()

     		bool handle(Event* event){ return handler.handleEvent(event); }
			bool handleMouseDrag(Event* event){ handler.handleMouseDrag(event); } 
			bool handleMousePush(Event * event){ return handler.handleMousePush(event); }
			bool handleMouseRelease(Event *event){ return handler.handleMouseRelease(event); }
			void addMouseEvent( Event *event ){ handler.addMouseEvent(event); }
			bool handleKeyDown(Event *event) { return handler.handleKeyDown(event);} 
			bool handleKeyUp(Event *event){ return handler.handleKeyUp(event);}

			//specialized
			bool handleMouseWheel(Event *event);
			bool handleMouseMove(Event *event){return false; };// handler.handleMouseDeltaMovement(event); }


	protected:
		ManipulatorHandler<FirstPersonManipulator> handler;
	};

}

#endif
