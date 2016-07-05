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



    private:
    	T* _m;
    };

}

#endif