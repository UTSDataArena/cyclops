#include "cyclops/ManipulatorHandler.h"


using namespace cyclops;


osg::ref_ptr<osgGA::GUIEventAdapter> MouseAdapter::bridge(Event *event){
    osgGA::GUIEventAdapter* osgNewEvent = new osgGA::GUIEventAdapter;

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

    osgNewEvent->setInputRange(0, 0, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]) );
    osgNewEvent->setX(event->getPosition().x());
    osgNewEvent->setY(event->getPosition().y());

    unsigned int buttonMask = 0;



    if (event->isFlagSet(Event::Left))
        buttonMask |= osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
    if (event->isFlagSet(Event::Middle))
        buttonMask = osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
    if (event->isFlagSet(Event::Right))
        buttonMask |= osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;

    osgNewEvent->setButtonMask(buttonMask);

    // time in seconds
    osgNewEvent->setTime( event->getTimestamp() / 1000.0 );

    return osgNewEvent;
}