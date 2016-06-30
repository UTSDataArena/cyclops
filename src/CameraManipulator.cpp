#include "cyclops/CameraManipulator.h"

#include <osg/MatrixTransform>
#include <osg/ComputeBoundsVisitor>
#include <iostream>


using namespace cyclops;

#define OOSG_VEC3(v) osg::Vec3(v[0], v[1], v[2])
#define OSGVEC3_OMEGA(v) Vector3f( v.x() , v.y(), v.z() )
#define P_OSGVEC(v)  std::cout << v.x() << " " << v.y() << " " << v.z() << std::endl

NodeTrackerManipulator::NodeTrackerManipulator( Camera* omegaCam, int flags) : inherited( flags ) , _omegaCam(omegaCam)
{
    setVerticalAxisFixed(false);
}


bool NodeTrackerManipulator::handle(Event* event)
{
    if(event->isProcessed()) return false;


    if(event->getServiceType() == Service::Pointer)
    {

        switch( event->getType() )
        {
            case Event::Move:
                if ( event->isFlagSet(Event::Right) ||
                     event->isFlagSet(Event::Left))
                    handleMouseDrag(event);
                else 
                    handleMouseMove(event);
                break;

            case Event::Up:
                handleMouseRelease(event);
                break;

            case Event::Down:
                handleMousePush(event);
                break;
            
            case Event::Zoom:
                handleMouseWheel(event);
                break;
            
            default:
                break;
        }
    }


    return true;
}







/** Add the current mouse GUIEvent to internal stack.*/
void NodeTrackerManipulator::addMouseEvent( Event *event )
{
    if (lastEvent != NULL)
        delete lastEvent;

    lastEvent = currentEvent;
    currentEvent = new Event;

    omicronConnector::EventData ed;
    event->serialize(&ed);
    currentEvent->deserialize(&ed);

    // osg guieventdapaters of parent class
    _ga_t1 = _ga_t0;
    osgGA::GUIEventAdapter* osgNewEvent = new osgGA::GUIEventAdapter;

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

    osgNewEvent->setInputRange(0, 0, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]) );
    osgNewEvent->setX(currentEvent->getPosition().x());
    osgNewEvent->setY(currentEvent->getPosition().y());

    // time in seconds
    osgNewEvent->setTime( currentEvent->getTimestamp() / 1000.0 );

    _ga_t0 = osgNewEvent;


}



/** Reset the internal GUIEvent stack.*/
void NodeTrackerManipulator::flushMouseEventStack()
{
    lastEvent = NULL;
    currentEvent = NULL;
}



bool NodeTrackerManipulator::handleMouse()
{
    // return if less then two events have been added
    if( currentEvent == NULL || lastEvent == NULL )
        return false;

    // get delta time
    double eventTimeDelta = currentEvent->getTimestamp() - lastEvent->getTimestamp();
    if( eventTimeDelta < 0. )
    {
        std::cout << "Manipulator warning: eventTimeDelta = " << eventTimeDelta << std::endl;
        eventTimeDelta = 0.;
    }

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

    float curX = currentEvent->getPosition().x() / resolution[0];
    float curY = currentEvent->getPosition().y() / resolution[1];


    float lastX = lastEvent->getPosition().x() / resolution[0];
    float lastY = lastEvent->getPosition().y() / resolution[1];


    float myPointerAxisMultiplier = 1.0;

    float dx = (curX - lastX) * myPointerAxisMultiplier;
    float dy = -(curY - lastY) * myPointerAxisMultiplier;


    // return if there is no movement.
    if( dx == 0. && dy == 0. )
        return false;


    // call appropriate methods
    if( currentEvent->isFlagSet(Event::Left))
    {
        return performMovementLeftMouseButton( eventTimeDelta, dx, dy );
    }
    // else if( currentEvent->isFlagSet(Event::Middle) )
    // {
    //     return performMovementMiddleMouseButton( eventTimeDelta, dx, dy );
    // }
    else if( currentEvent->isFlagSet(Event::Right) )
    {
        return  OrbitManipulator::performMovementMiddleMouseButton( eventTimeDelta, dx, dy );
    }
}


void NodeTrackerManipulator::handleMouseMove(Event *event)
{
    // std::cout << "handling mouse move" << std::endl;
}

void NodeTrackerManipulator::handleMouseDrag(Event* event){
    addMouseEvent(event);
    handleMouse();
}



bool NodeTrackerManipulator::handleMousePush(Event * event)
{
    flushMouseEventStack();
    addMouseEvent( event );

    _thrown = false;

    return true;
}


/// Handles GUIEventAdapter::RELEASE event.
bool NodeTrackerManipulator::handleMouseRelease(Event *event)
{
    // no mouse button is pressed
    if( !(event->isFlagSet(Event::Left) || 
        event->isFlagSet(Event::Right) || 
        event->isFlagSet(Event::Middle) ) )
    {

        double timeSinceLastRecordEvent = _ga_t0.valid() ? (event->getTimestamp() - _ga_t0->getTime()) : DBL_MAX;
        if( timeSinceLastRecordEvent > 0.02 )
            flushMouseEventStack();

        if( isMouseMoving() )
        {
            if( handleMouse() && _allowThrow )
            {
                _thrown = true;
            }

            return true;
        }
    }

    flushMouseEventStack();
    addMouseEvent( event );
    handleMouse();

    _thrown = false;

    return true;
}



bool NodeTrackerManipulator::handleMouseWheel(Event *event)
{
    int wheel = event->getExtraDataInt(0);

    // mouse scroll up event
    if (wheel > 0)
    {
        // perform zoom
        zoomModel( _wheelZoomFactor, true );
        return true;
    } else {
        zoomModel( -_wheelZoomFactor, true );
        return true;
    }
}




CameraManipulator::CameraManipulator() : CameraController() {
    myManipulator = new NodeTrackerManipulator(Engine::instance()->getDefaultCamera());

    myCamera = Engine::instance()->getDefaultCamera();
    myCamera->setController(this);

    myManipulator->setTrackerMode( osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION );
    myManipulator->setRotationMode( osgGA::NodeTrackerManipulator::TRACKBALL );
}


void CameraManipulator::onEvent(Event* event)
{
    myManipulator->handle(event);
}

void CameraManipulator::update(const UpdateContext& context)
{
    if(!isEnabled()) return;

    osg::Vec3d eye, center, up;
    myManipulator->getTransformation(eye, center, up);

    Vector3f oPosVec(eye.x(), eye.y(), eye.z());
    Vector3f oUpVec(up.x(), up.y(), up.z());
    Vector3f oCenterVec(center.x(), center.y(), center.z());
    
    //order is important here, setting lookat before position 
    // will result in choppy camera rotation
    myCamera->setPosition(oPosVec);
    myCamera->lookAt(oCenterVec, oUpVec);
}


// void CameraManipulator::setHomePosition(float px, float py, float pz, )

void CameraManipulator::setTrackedNode(Entity* entity) {
    myTrackedNode = entity->getOsgNode();
    myManipulator->setTrackNode( myTrackedNode.get() );


    osg::BoundingSphere boundingSphere;
    osg::ComputeBoundsVisitor cbVisitor;
    myTrackedNode->accept(cbVisitor);
    osg::BoundingBox &bb = cbVisitor.getBoundingBox();

    if (bb.valid()) boundingSphere.expandBy(bb);
    else boundingSphere = myTrackedNode->getBound();



    myManipulator->setHomePosition( boundingSphere.center() + osg::Vec3(0, 50.0, 0), //OOSG_VEC3( omegaCam->getPosition() ),
                    boundingSphere.center(),
                    osg::Z_AXIS);

    myManipulator->home(0.0);
    lastPosition = OSGVEC3_OMEGA( (boundingSphere.center() + osg::Vec3(0, 50.0, 0)) );
    
}

void CameraManipulator::setHomeEye(const Vector3f& eye){
    osg::Vec3d unusedEye, center, up;
    myManipulator->getHomePosition(unusedEye, center, up);

    myManipulator->setHomePosition( OOSG_VEC3( eye ), //OOSG_VEC3( omegaCam->getPosition() ),
                center,
                up);
    myManipulator->home(0.0);
    lastPosition = eye;
}


CameraManipulator* CameraManipulator::create(){
    return new CameraManipulator();
}