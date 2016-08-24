#ifndef __CY_MANIPULATOR_HANDLER__
#define __CY_MANIPULATOR_HANDLER__

#include <osg/Node>
#include <osgGA/StandardManipulator>

#define OMEGA_NO_GL_HEADERS
#include <omega.h>
#include <omegaOsg/omegaOsg.h>

// #include <boost/python/class.hpp>
// #include <boost/python/module_init.hpp>
// #include <boost/python/def.hpp>
#include <boost/python.hpp>



namespace cyclops {
    using namespace omega;
    using namespace omegaOsg;



    class ConfigurableOptions : public ReferenceType {
    public:
    	ConfigurableOptions() {};
    	bool fixVerticalAxis;
    	int flags;
    };


	class EventAdapter : public ReferenceType {
		public:
			EventAdapter() {
				configOptions = new ConfigurableOptions();
			}
			virtual osg::ref_ptr<osgGA::GUIEventAdapter> bridge(Event *event);
			virtual int mapButton(Event *event) = 0;
			virtual Vector2f mapXY(Event *event) = 0;
			virtual Vector4f setInputRange(Event *event) = 0;
			virtual osgGA::GUIEventAdapter::ScrollingMotion mapScrollingMotion(Event *event) = 0;
			virtual osgGA::GUIEventAdapter::EventType mapEventType(Event *event) = 0;
			virtual osgGA::GUIEventAdapter::KeySymbol mapKeySymbol(Event *event) = 0;


			virtual void preMapping() {}
			virtual void postMapping() {}

			Ref<ConfigurableOptions> configOptions;

	};

	class MouseAdapter : public EventAdapter {
		public:
			MouseAdapter() : EventAdapter() {}
			// virtual osg::ref_ptr<osgGA::GUIEventAdapter> bridge(Event *event);
			virtual int mapButton(Event *event);
			virtual Vector2f mapXY(Event *event);
			virtual Vector4f setInputRange(Event *event);
			virtual osgGA::GUIEventAdapter::ScrollingMotion mapScrollingMotion(Event *event);
			virtual osgGA::GUIEventAdapter::EventType mapEventType(Event *event);
			virtual osgGA::GUIEventAdapter::KeySymbol mapKeySymbol(Event *event);

	};



	class EventAdapterCallback : public EventAdapter {
	public:
	    EventAdapterCallback(PyObject *p) : EventAdapter(), self(p) {}
	    
	    virtual osg::ref_ptr<osgGA::GUIEventAdapter> bridge(Event *event){
	    	myLastEvent = event;
	    	return EventAdapter::bridge(event);
	    }

	    virtual void preMapping();
	    virtual void postMapping();

	    virtual int mapButton(Event *event);
	    virtual Vector2f mapXY(Event *event);
	    virtual Vector4f setInputRange(Event *event);
	    virtual osgGA::GUIEventAdapter::ScrollingMotion mapScrollingMotion(Event *event);
	    virtual osgGA::GUIEventAdapter::EventType mapEventType(Event *event);
		virtual osgGA::GUIEventAdapter::KeySymbol mapKeySymbol(Event *event);

	    const Event* getLastEvent() { return myLastEvent; }
	    ConfigurableOptions* getConfigOptions() {return configOptions;}


	    const Event* myLastEvent;
	    PyObject *self;
	};





    template <class T>
    class ManipulatorHandler {
	public:
    	ManipulatorHandler() {};
    	void set(T* manipulator) {
    		_m = manipulator;
    	}


    	bool handleEvent(Event *event) {
    		if(event->isProcessed()) return false;

    		osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
    		setNewOptions();
    		

		    switch( ea->getEventType() )
		    {
		        case osgGA::GUIEventAdapter::MOVE:
		            return _m->handleMouseMove( event );
		            break;

		        case osgGA::GUIEventAdapter::DRAG:
		            return _m->handleMouseDrag( event );
		            break;

		        case osgGA::GUIEventAdapter::PUSH:
		            return _m->handleMousePush( event );
		            break;

		        case osgGA::GUIEventAdapter::RELEASE:
		            return _m->handleMouseRelease( event );
		            break;

		        case osgGA::GUIEventAdapter::KEYDOWN:
		            return _m->handleKeyDown( event );
		            break;

		        case osgGA::GUIEventAdapter::KEYUP:
		            return _m->handleKeyUp( event );
		            break;

		        case osgGA::GUIEventAdapter::SCROLL:
		            if( _m->_flags & _m->PROCESS_MOUSE_WHEEL )
		            return _m->handleMouseWheel(event);
		            else
		            return false;

		        default:
		            return false;
		    }
		    return true;
    	}

    	bool handleMousePush(Event *event) {
    		_m->flushMouseEventStack();
    		_m->addMouseEvent( event );

    		_m->_thrown = false;
    		return true;
    	}


    	bool handleKeyDown(Event *event) {
    	   osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
    	   if( ea->getKey() == osgGA::GUIEventAdapter::KEY_Space )
		    {
		        _m->flushMouseEventStack();
		        _m->_thrown = false;
		        _m->home(0);
		        return true;
		    }

		    return false;
    	}

    	bool handleKeyUp(Event *event) {
    		return true;
    	}

    	void addMouseEvent(Event *event){
    	    // osg guieventdapaters of parent class
    		_m->_ga_t1 = _m->_ga_t0;
    		_m->_ga_t0 = _eventAdapter->bridge(event);
    	}


    	/// Handles GUIEventAdapter::RELEASE event.
    	bool handleMouseRelease(Event *event){
    		osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
    	    // no mouse button is pressed
		    if( ea->getButtonMask() == 0  )
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
  		    // int wheel = event->getExtraDataInt(0);
			osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
  		    osgGA::GUIEventAdapter::ScrollingMotion sm = ea->getScrollingMotion();

			// if( ((wheel < 0 && _m->_wheelZoomFactor > 0.)) ||
			//         ((wheel > 0  && _m->_wheelZoomFactor < 0.)) )
			//     {    

		    // handle centering
		    if( _m->_flags & T::SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
			{
		        if( ((sm == osgGA::GUIEventAdapter::SCROLL_DOWN && _m->_wheelZoomFactor > 0.)) ||
		            ((sm == osgGA::GUIEventAdapter::SCROLL_UP   && _m->_wheelZoomFactor < 0.)) )
		        {

			        if( _m->getAnimationTime() <= 0. )
			        {
			            // center by mouse intersection (no animation)
			            setCenterByMousePointerIntersection( event );
			        }

			    }
			}
		    // mouse scroll up event
		    if (sm == osgGA::GUIEventAdapter::SCROLL_UP)
		    {
		        // perform zoom
		        _m->zoomModel( _m->_wheelZoomFactor, true );
		        return true;
		    } else {
		        _m->zoomModel( -_m->_wheelZoomFactor, true );
		        return true;
		    }
		}


    	bool handleMouseDrag(Event *event){
    		_m->addMouseEvent(event);
    		_m->performMovement();

    		return true;
    	}


    	void setNewOptions(){
    		_m->setVerticalAxisFixed(_eventAdapter->configOptions->fixVerticalAxis);
    		_m->_flags = _eventAdapter->configOptions->flags;
    	}


    	/// The method processes events for manipulation based on relative mouse movement (mouse delta).
		bool handleMouseDeltaMovement( Event *event )
		{
			osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
		    
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
			
			osg::ref_ptr<osgGA::GUIEventAdapter> ea = _eventAdapter->bridge(event);
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
	    		// omsg("intersection");
	    	}
	    	else
	    	{
	    		// omsg("no intersection");
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

		void setAdapter(EventAdapter *eventAdapter)
		{
			_eventAdapter = eventAdapter;
			_eventAdapter->configOptions->fixVerticalAxis = _m->getVerticalAxisFixed();
			_eventAdapter->configOptions->flags = _m->_flags;
		}



    private:
    	T* _m;
    	EventAdapter* _eventAdapter;
    };

}

#endif