#include "cyclops/ManipulatorHandler.h"

#include <boost/python/call_method.hpp>
#include <boost/utility.hpp>
#include <boost/ref.hpp>
#include <iostream>

using namespace cyclops;

int EventAdapterCallback::mapButton(Event *event)
{ 
    return boost::python::call_method<int>(self, "mapButton"); 
}

Vector2f EventAdapterCallback::mapXY(Event *event) 
{ 
    return  boost::python::call_method<Vector2f>(self, "mapXY"); 
}

Vector4f EventAdapterCallback::setInputRange(Event *event) {
    boost::python::tuple result = boost::python::call_method<boost::python::tuple>(self, "setInputRange");
    boost::python::ssize_t len = boost::python::len(result);

    if (len != 4){
        owarn("setInputRange must return a list with length 4!!!");
        return Vector4f(0,0,0,0);
    }

    double x = boost::python::extract<double>(result[0]);
    double y = boost::python::extract<double>(result[1]);
    double z = boost::python::extract<double>(result[2]);
    double w = boost::python::extract<double>(result[3]);


    return Vector4f(x,y,z,w);
}

osgGA::GUIEventAdapter::ScrollingMotion EventAdapterCallback::mapScrollingMotion(Event *event) {
             return boost::python::call_method<osgGA::GUIEventAdapter::ScrollingMotion>(self, "mapScrollingMotion"); 
}

osgGA::GUIEventAdapter::EventType EventAdapterCallback::mapEventType(Event *event) {
    return boost::python::call_method<osgGA::GUIEventAdapter::EventType>(self, "mapEventType"); 
}

void EventAdapterCallback::preMapping(){
    boost::python::call_method<void>(self, "preMapping");
}

void EventAdapterCallback::postMapping(){
    boost::python::call_method<void>(self, "postMapping");
}



osg::ref_ptr<osgGA::GUIEventAdapter> EventAdapter::bridge(Event *event){
    osg::ref_ptr<osgGA::GUIEventAdapter> osgNewEvent = new osgGA::GUIEventAdapter;

    this->preMapping();

    int buttonMask = this->mapButton(event);
    osgNewEvent->setButtonMask(buttonMask);


    osgGA::GUIEventAdapter::EventType eventType = this->mapEventType(event);
    osgNewEvent->setEventType(eventType);

    Vector2f xy = this->mapXY(event);
    osgNewEvent->setX(xy.x());
    osgNewEvent->setY(xy.y());


    Vector4f inputRange = this->setInputRange(event);
    osgNewEvent->setInputRange(inputRange[0], inputRange[1], inputRange[2], inputRange[3]);


    if (eventType == osgGA::GUIEventAdapter::SCROLL)
    {
        osgGA::GUIEventAdapter::ScrollingMotion scrollingMotion = this->mapScrollingMotion(event);
        osgNewEvent->setScrollingMotion(scrollingMotion);        
    }

    // time in seconds
    osgNewEvent->setTime( event->getTimestamp() / 1000.0 );


    this->postMapping();

    return osgNewEvent;
}


Vector2f MouseAdapter::mapXY(Event *event) {
    return Vector2f(event->getPosition().x(), event->getPosition().y());
}


Vector4f MouseAdapter::setInputRange(Event *event){
    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

    // osgNewEvent->setInputRange(0, 0, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]) );
    return Vector4f(0,0, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]) );
}

osgGA::GUIEventAdapter::EventType MouseAdapter::mapEventType(Event *event)
{
    switch( event->getType() )
    {

        case Event::Move:
            if ( event->isFlagSet(Event::Right) ||
                 event->isFlagSet(Event::Left) ||
                 event->isFlagSet(Event::Middle))
                return osgGA::GUIEventAdapter::DRAG;
            else 
                return osgGA::GUIEventAdapter::MOVE;

        case Event::Up:
            if (event->getServiceType() == Service::Pointer)
                return osgGA::GUIEventAdapter::RELEASE;
            else 
                return osgGA::GUIEventAdapter::KEYUP;

        case Event::Down:
            if (event->getServiceType() == Service::Pointer)
                return osgGA::GUIEventAdapter::PUSH;
            else 
                return osgGA::GUIEventAdapter::KEYDOWN;
        
        case Event::Zoom:
            return osgGA::GUIEventAdapter::SCROLL;

        default:
            return osgGA::GUIEventAdapter::NONE;
    }
}



int MouseAdapter::mapButton(Event *event) {
    int buttonMask = 0;

    if (event->isFlagSet(Event::Left))
        buttonMask |= osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    if (event->isFlagSet(Event::Middle))
        buttonMask |= osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    if (event->isFlagSet(Event::Right))
        buttonMask |= osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;


    return buttonMask;
}


osgGA::GUIEventAdapter::ScrollingMotion MouseAdapter::mapScrollingMotion(Event *event) {
    int wheel = event->getExtraDataInt(0);
    // mouse scroll up event
    if (wheel > 0)
        return osgGA::GUIEventAdapter::SCROLL_UP;
    else
        return osgGA::GUIEventAdapter::SCROLL_DOWN;
}