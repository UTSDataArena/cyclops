#ifndef __CY_MANIPULATOR_HANDLER__
#define __CY_MANIPULATOR_HANDLER__

#include <osg/Node>
#include <osgGA/StandardManipulator>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaOsg/omegaOsg.h>


namespace cyclops {
    using namespace omega;
    using namespace omegaOsg;


	class MouseAdapter {
		public:
			static osg::ref_ptr<osgGA::GUIEventAdapter> bridge(Event *event);
			// static bool handleEvent(Event *event, NodeTrackerManipulator *_m);
	};


    template <class T>
    class ManipulatorHandler {
	public:
    	ManipulatorHandler() {};
    	void set(T* manipulator) {_m = manipulator;}


    	bool handleEvent(Event *event) {
    		if(event->isProcessed()) return false;


		    if(event->getServiceType() == Service::Pointer)
		    {

		        switch( event->getType() )
		        {
		            case Event::Move:
		                if ( event->isFlagSet(Event::Right) ||
		                     event->isFlagSet(Event::Left) ||
		                     event->isFlagSet(Event::Middle))
		                    _m->handleMouseDrag(event);
		                else 
		                    _m->handleMouseMove(event);
		                break;

		            case Event::Up:
		                _m->handleMouseRelease(event);
		                break;

		            case Event::Down:
		                _m->handleMousePush(event);
		                break;
		            
		            case Event::Zoom:
		                _m->handleMouseWheel(event);
		                break;
		            
		            default:
		                break;
		        }
		    }


		    return true;
    	}

    	bool handleMousePush(Event *event) {
    		_m->flushMouseEventStack();
    		_m->addMouseEvent( event );

    		_m->_thrown = false;
    		return true;
    	}

    	void addMouseEvent(Event *event){
    	    // osg guieventdapaters of parent class
    		_m->_ga_t1 = _m->_ga_t0;
    		_m->_ga_t0 = MouseAdapter::bridge(event);
    	}


    	/// Handles GUIEventAdapter::RELEASE event.
    	bool handleMouseRelease(Event *event){
    	    // no mouse button is pressed
		    if( !(event->isFlagSet(Event::Left) || 
		        event->isFlagSet(Event::Right) || 
		        event->isFlagSet(Event::Middle) ) )
		    {

		        double timeSinceLastRecordEvent = _m->_ga_t0.valid() ? (event->getTimestamp() - _m->_ga_t0->getTime()) : DBL_MAX;
		        if( timeSinceLastRecordEvent > 0.02 )
		            _m->flushMouseEventStack();

		        if( _m->isMouseMoving() )
		        {
		            if( _m->performMovement() && _m->_allowThrow )
		            {
		                _m->_thrown = true;
		            }

		            return true;
		        }
		    }

		    _m->flushMouseEventStack();
		    _m->addMouseEvent( event );
		    _m->performMovement();

		    _m->_thrown = false;

		    return true;
    	}


    	bool handleMouseWheel(Event *event) {
  		    int wheel = event->getExtraDataInt(0);

		    if( _m->_flags & T::SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
			{

			    if( ((wheel < 0 && _m->_wheelZoomFactor > 0.)) ||
			        ((wheel > 0  && _m->_wheelZoomFactor < 0.)) )
			    {

			        if( _m->getAnimationTime() <= 0. )
			        {
			            // center by mouse intersection (no animation)
			            setCenterByMousePointerIntersection( event );
			        }
			        // else
			        // {
			        //     // start new animation only if there is no animation in progress
			        //     if( !isAnimating() )
			        //         startAnimationByMousePointerIntersection( ea, us );

			        // }

			    }
			}
		    // mouse scroll up event
		    if (wheel > 0)
		    {
		        // perform zoom
		        _m->zoomModel( _m->_wheelZoomFactor, true );
		        return true;
		    } else {
		        _m->zoomModel( -_m->_wheelZoomFactor, true );
		        return true;
		    }

    	}

    	void handleMouseDrag(Event *event){
    		_m->addMouseEvent(event);
    		_m->performMovement();
    	}


    	/// The method processes events for manipulation based on relative mouse movement (mouse delta).
		bool handleMouseDeltaMovement( Event *event )
		{
			osg::ref_ptr<osgGA::GUIEventAdapter> ea = MouseAdapter::bridge(event);
		    
		    float dx = ea->getX() - _m->_mouseCenterX;
		    float dy = ea->getY() - _m->_mouseCenterY;

		    if( dx == 0.f && dy == 0.f )
		        return false;

		    addMouseEvent( event );
		    

		    _m->_mouseCenterX = (ea->getXmin() + ea->getXmax()) / 2.0f;
    		_m->_mouseCenterY = (ea->getYmin() + ea->getYmax()) / 2.0f;

		    return _m->performMouseDeltaMovement( dx, dy );
		}



		/** The method sends a ray into the scene and the point of the closest intersection
		    is used to set a new center for the manipulator. For Orbit-style manipulators,
		    the orbiting center is set. For FirstPerson-style manipulators, view is pointed
		    towards the center.*/
		bool setCenterByMousePointerIntersection( Event *event)
		{
			
			osg::ref_ptr<osgGA::GUIEventAdapter> ea = MouseAdapter::bridge(event);
		    // prepare variables
		    float x = ( ea->getX() - ea->getXmin() ) / ( ea->getXmax() - ea->getXmin() );
		    float y = ( ea->getY() - ea->getYmin() ) / ( ea->getYmax() - ea->getYmin() );

		    SceneNode* scene = Engine::instance()->getScene();
		    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    		Ray r;
    		bool res = ds->getViewRayFromEvent(*event, r);

		   	Vector3f hitPoint;
		    bool hit = scene->hit(r, &hitPoint, SceneNode::HitBest);

		    if (hit)
	    	{
	    		omsg("intersection");
	    	}
	    	else
	    	{
	    		omsg("no intersection");
	    		return false;
	    	}

		    // get current transformation
		    osg::Vec3d eye, oldCenter, up;
		    _m->getTransformation( eye, oldCenter, up );

		    // new center
		    osg::Vec3d newCenter = OOSG_VEC3(hitPoint);

		    // make vertical axis correction
		    if( _m->getVerticalAxisFixed() )
		    {

		        osg::CoordinateFrame coordinateFrame = _m->getCoordinateFrame( newCenter );
		        osg::Vec3d localUp = _m->getUpVector( coordinateFrame );

		        _m->fixVerticalAxis( newCenter - eye, up, up, localUp, true );

		    }

		    // set the new center
		    _m->setTransformation( eye, newCenter, up );

		   _m->_mouseCenterX = (ea->getXmin() + ea->getXmax()) / 2.0f;
    	   _m->_mouseCenterY = (ea->getYmin() + ea->getYmax()) / 2.0f;

		    return true;
		}



    private:
    	T* _m;
    };

}

#endif