%module libsumo
#pragma SWIG nowarn=511

#ifdef SWIGPYTHON
%naturalvar;
%rename(edge) Edge;
%rename(inductionloop) InductionLoop;
%rename(junction) Junction;
%rename(lane) Lane;
%rename(lanearea) LaneArea;
%rename(multientryexit) MultiEntryExit;
%rename(person) Person;
%rename(poi) POI;
%rename(polygon) Polygon;
%rename(route) Route;
%rename(simulation) Simulation;
%rename(trafficlight) TrafficLight;
%rename(vehicle) Vehicle;
%rename(vehicletype) VehicleType;
%rename(calibrator) Calibrator;
%rename(busstop) BusStop;
%rename(parkingarea) ParkingArea;
%rename(chargingstation) ChargingStation;
%rename(overheadwire) OverheadWire;

// adding dummy init and close for easier traci -> libsumo transfer
%pythoncode %{
import sys
from traci import connection, constants, exceptions, _vehicle, _person, _trafficlight, _simulation
from traci.connection import StepListener 

def simulationStep(step=0):
    simulation.step(step)
    result = []
    for domain in (edge, inductionloop, junction, lane, lanearea, multientryexit,
                   person, poi, polygon, route, trafficlight, vehicle, vehicletype):
        result += [(k, v) for k, v in domain.getAllSubscriptionResults().items()]
        result += [(k, v) for k, v in domain.getAllContextSubscriptionResults().items()]
    _manageStepListeners(step)
    return result

_stepListeners = {}
_nextStepListenerID = 0
%}

/* There is currently no TraCIPosition used as input so this is only for future usage
%typemap(in) const libsumo::TraCIPosition& (libsumo::TraCIPosition pos) {
    const Py_ssize_t size = PySequence_Size($input);
    if (size == 2 || size == 3) {
        pos.x = PyFloat_AsDouble(PySequence_GetItem($input, 0));
        pos.y = PyFloat_AsDouble(PySequence_GetItem($input, 1));
        pos.z = (size == 3 ? PyFloat_AsDouble(PySequence_GetItem($input, 2)) : 0.);
    } else {
    // TODO error handling
    }
    $1 = &pos;
}
*/

%typemap(in) const libsumo::TraCIPositionVector& (libsumo::TraCIPositionVector shape) {
    const Py_ssize_t size = PySequence_Size($input);
    for (Py_ssize_t i = 0; i < size; i++) {
        PyObject* posTuple = PySequence_GetItem($input, i);
        const Py_ssize_t posSize = PySequence_Size(posTuple);
        libsumo::TraCIPosition pos;
        if (posSize == 2 || posSize == 3) {
            PyObject* item = PySequence_GetItem(posTuple, 0);
            pos.x = PyFloat_Check(item) ? PyFloat_AsDouble(item) : PyLong_AsDouble(item);
            item = PySequence_GetItem(posTuple, 1);
            pos.y = PyFloat_Check(item) ? PyFloat_AsDouble(item) : PyLong_AsDouble(item);
            pos.z = 0.;
            if (posSize == 3) {
                item = PySequence_GetItem(posTuple, 2);
                pos.z = PyFloat_Check(item) ? PyFloat_AsDouble(item) : PyLong_AsDouble(item);
            }
        } else {
        // TODO error handling
        }
        shape.push_back(pos);
    }
    $1 = &shape;
}

%typemap(in) const libsumo::TraCIColor& (libsumo::TraCIColor col) {
    const Py_ssize_t size = PySequence_Size($input);
    if (size == 3 || size == 4) {
        col.r = (unsigned char)PyLong_AsLong(PySequence_GetItem($input, 0));
        col.g = (unsigned char)PyLong_AsLong(PySequence_GetItem($input, 1));
        col.b = (unsigned char)PyLong_AsLong(PySequence_GetItem($input, 2));
        col.a = (unsigned char)(size == 4 ? PyLong_AsLong(PySequence_GetItem($input, 3)) : 255);
    } else {
    // TODO error handling
    }
    $1 = &col;
}

%typemap(in) const std::vector<int>& (std::vector<int> vars) {
    const Py_ssize_t size = PySequence_Size($input);
    for (Py_ssize_t i = 0; i < size; i++) {
        vars.push_back(PyLong_AsLong(PySequence_GetItem($input, i)));
    }
    $1 = &vars;
}

%typemap(typecheck, precedence=SWIG_TYPECHECK_INTEGER) const std::vector<int>& {
    $1 = PySequence_Check($input) ? 1 : 0;
}

%typemap(in) const std::vector<double>& (std::vector<double> values) {
    const Py_ssize_t size = PySequence_Size($input);
    for (Py_ssize_t i = 0; i < size; i++) {
        values.push_back(PyFloat_AsDouble(PySequence_GetItem($input, i)));
    }
    $1 = &values;
}


%{
#include <libsumo/TraCIDefs.h>

static PyObject* parseSubscriptionMap(const std::map<int, std::shared_ptr<libsumo::TraCIResult> >& subMap) {
    PyObject* result = PyDict_New();
    for (auto iter = subMap.begin(); iter != subMap.end(); ++iter) {
        const libsumo::TraCIResult* const traciVal = iter->second.get();
        PyObject* pyVal = nullptr;
        const libsumo::TraCIDouble* const theDouble = dynamic_cast<const libsumo::TraCIDouble*>(traciVal);
        if (theDouble != nullptr) {
            pyVal = PyFloat_FromDouble(theDouble->value);
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIInt* const theInt = dynamic_cast<const libsumo::TraCIInt*>(traciVal);
            if (theInt != nullptr) {
                pyVal = PyInt_FromLong(theInt->value);
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIString* const theString = dynamic_cast<const libsumo::TraCIString*>(traciVal);
            if (theString != nullptr) {
                pyVal = PyUnicode_FromString(theString->value.c_str());
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIStringList* const theStringList = dynamic_cast<const libsumo::TraCIStringList*>(traciVal);
            if (theStringList != nullptr) {
                const Py_ssize_t size = theStringList->value.size();
                pyVal = PyTuple_New(size);
                for (Py_ssize_t i = 0; i < size; i++) {
                    PyTuple_SetItem(pyVal, i, PyUnicode_FromString(theStringList->value[i].c_str()));
                }
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIPosition* const thePosition = dynamic_cast<const libsumo::TraCIPosition*>(traciVal);
            if (thePosition != nullptr) {
                if (thePosition->z != libsumo::INVALID_DOUBLE_VALUE) {
                    pyVal = Py_BuildValue("(ddd)", thePosition->x, thePosition->y, thePosition->z);
                } else {
                    pyVal = Py_BuildValue("(dd)", thePosition->x, thePosition->y);
                }
            }
        }
        if (pyVal == nullptr) {
            const libsumo::TraCIRoadPosition* const theRoadPosition = dynamic_cast<const libsumo::TraCIRoadPosition*>(traciVal);
            if (theRoadPosition != nullptr) {
                if (theRoadPosition->laneIndex != libsumo::INVALID_INT_VALUE) {
                    pyVal = Py_BuildValue("(sdi)", theRoadPosition->edgeID.c_str(), theRoadPosition->pos, theRoadPosition->laneIndex);
                } else {
                    pyVal = Py_BuildValue("(sd)", theRoadPosition->edgeID.c_str(), theRoadPosition->pos);
                }
            }
        }
        if (pyVal == nullptr) {
            pyVal = SWIG_NewPointerObj(SWIG_as_voidptr(traciVal), SWIGTYPE_p_libsumo__TraCIResult, 0);
        }
        PyObject* const pyKey = PyInt_FromLong(iter->first);
        PyDict_SetItem(result, pyKey, pyVal);
        Py_DECREF(pyKey);
        Py_DECREF(pyVal);
    }
    return result;
}
%}

%typemap(out) std::map<int, std::shared_ptr<libsumo::TraCIResult> > {
    $result = parseSubscriptionMap($1);
};

%typemap(out) std::map<std::string, std::map<int, std::shared_ptr<libsumo::TraCIResult> > > {
    $result = PyDict_New();
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyObject* const pyKey = PyUnicode_FromString(iter->first.c_str());
        PyObject* const pyVal = parseSubscriptionMap(iter->second);
        PyDict_SetItem($result, pyKey, pyVal);
        Py_DECREF(pyKey);
        Py_DECREF(pyVal);
    }
};

%typemap(out) std::map<std::string, std::map<std::string, std::map<int, std::shared_ptr<libsumo::TraCIResult> > > > {
    $result = PyDict_New();
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyObject* const pyKey = PyUnicode_FromString(iter->first.c_str());
        PyObject* const innerDict = PyDict_New();
        for (auto inner = iter->second.begin(); inner != iter->second.end(); ++inner) {
            PyObject* const innerKey = PyUnicode_FromString(inner->first.c_str());
            PyObject* const innerVal = parseSubscriptionMap(inner->second);
            PyDict_SetItem(innerDict, innerKey, innerVal);
            Py_DECREF(innerKey);
            Py_DECREF(innerVal);
        }
        PyDict_SetItem($result, pyKey, innerDict);
        Py_DECREF(pyKey);
        Py_DECREF(innerDict);
    }
};

%typemap(out) libsumo::TraCIPosition {
    if ($1.z != libsumo::INVALID_DOUBLE_VALUE) {
        $result = Py_BuildValue("(ddd)", $1.x, $1.y, $1.z);
    } else {
        $result = Py_BuildValue("(dd)", $1.x, $1.y);
    }
};

%typemap(out) libsumo::TraCIPositionVector {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(dd)", iter->x, iter->y));
    }
};

%typemap(out) libsumo::TraCIColor {
    $result = Py_BuildValue("(iiii)", $1.r, $1.g, $1.b, $1.a);
};

%typemap(out) libsumo::TraCIRoadPosition {
    $result = Py_BuildValue("(sdi)", $1.edgeID.c_str(), $1.pos, $1.laneIndex);
};

%typemap(out) std::vector<libsumo::TraCIConnection> {
    $result = PyList_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyList_SetItem($result, index++, Py_BuildValue("(sNNNsssd)",
                                                       iter->approachedLane.c_str(),
                                                       PyBool_FromLong(iter->hasPrio),
                                                       PyBool_FromLong(iter->isOpen),
                                                       PyBool_FromLong(iter->hasFoe),
                                                       iter->approachedInternal.c_str(),
                                                       iter->state.c_str(),
                                                       iter->direction.c_str(),
                                                       iter->length));
    }
};

%typemap(out) std::vector<libsumo::TraCIVehicleData> {
    $result = PyList_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyList_SetItem($result, index++, Py_BuildValue("(sddds)",
                                                       iter->id.c_str(),
                                                       iter->length,
                                                       iter->entryTime,
                                                       iter->leaveTime,
                                                       iter->typeID.c_str()));
    }
};

%typemap(out) std::vector<libsumo::TraCIBestLanesData> {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        const int size = (int)iter->continuationLanes.size();
        auto nextLanes = PyTuple_New(size);
        for (int i = 0; i < size; i++) {
            PyTuple_SetItem(nextLanes, i, PyUnicode_FromString(iter->continuationLanes[i].c_str()));
        }
        PyTuple_SetItem($result, index++, Py_BuildValue("(sddiNN)",
                                                        iter->laneID.c_str(),
                                                        iter->length,
                                                        iter->occupation,
                                                        iter->bestLaneOffset,
                                                        PyBool_FromLong(iter->allowsContinuation),
                                                        nextLanes));
    }
};

%typemap(out) std::vector<libsumo::TraCINextTLSData> {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(sidN)",
                                                        iter->id.c_str(),
                                                        iter->tlIndex,
                                                        iter->dist,
                                                        PyUnicode_FromStringAndSize(&iter->state, 1)));
    }
};

%typemap(out) std::vector<std::vector<libsumo::TraCILink> > {
    $result = PyList_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyObject* innerList = PyList_New(iter->size());
        int innerIndex = 0;
        for (auto inner = iter->begin(); inner != iter->end(); ++inner) {
            PyList_SetItem(innerList, innerIndex++, Py_BuildValue("(sss)",
                                                                  inner->fromLane.c_str(),
                                                                  inner->toLane.c_str(),
                                                                  inner->viaLane.c_str()));
        }
        PyList_SetItem($result, index++, innerList);
    }
};

%typemap(out) std::vector<std::pair<std::string, double> > {
    $result = PyTuple_New($1.size());
    int index = 0;
    for (auto iter = $1.begin(); iter != $1.end(); ++iter) {
        PyTuple_SetItem($result, index++, Py_BuildValue("(sd)", iter->first.c_str(), iter->second));
    }
};

%typemap(out) std::pair<int, int> {
    $result = Py_BuildValue("(ii)", $1.first, $1.second);
};

%typemap(out) std::pair<std::string, double> {
    $result = Py_BuildValue("(sd)", $1.first.c_str(), $1.second);
};

%typemap(out) std::pair<int, std::string> {
    $result = Py_BuildValue("(is)", $1.first, $1.second.c_str());
};

%extend libsumo::TraCIStage {
  %pythoncode %{
    __attr_repr__ = _simulation.Stage.__attr_repr__
    __repr__ = _simulation.Stage.__repr__
  %}
};

%extend libsumo::TraCINextStopData {
  %pythoncode %{
    __attr_repr__ = _vehicle.StopData.__attr_repr__
    __repr__ = _vehicle.StopData.__repr__
  %}
};

%extend libsumo::TraCIReservation {
  %pythoncode %{
    __attr_repr__ = _person.Reservation.__attr_repr__
    __repr__ = _person.Reservation.__repr__
  %}
};

%extend libsumo::TraCILogic {
  %pythoncode %{
    getPhases = _trafficlight.Logic.getPhases
    __repr__ = _trafficlight.Logic.__repr__
  %}
};

%extend libsumo::TraCIPhase {
  %pythoncode %{
    __repr__ = _trafficlight.Phase.__repr__
  %}
};

%exceptionclass libsumo::TraCIException;

%pythonappend libsumo::Vehicle::getLeader(const std::string&, double) %{
    if val[0] == "" and vehicle._legacyGetLeader:
        return None
%}

#endif

%begin %{
#ifdef _MSC_VER
// ignore constant conditional expression and unreachable code warnings
#pragma warning(disable:4127 4702)
#endif
%}


// replacing vector instances of standard types, see https://stackoverflow.com/questions/8469138
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%template(StringVector) std::vector<std::string>;
%template(IntVector) std::vector<int>;
%template() std::map<std::string, std::string>;

// exception handling
%include "exception.i"

// taken from here https://stackoverflow.com/questions/1394484/how-do-i-propagate-c-exceptions-to-python-in-a-swig-wrapper-library
%exception {
    try {
        $action
    } catch (libsumo::TraCIException &e) {
        const std::string s = std::string("Error: ") + e.what();
#ifdef SWIGPYTHON
        PyErr_SetObject(SWIG_Python_ExceptionType(SWIGTYPE_p_libsumo__TraCIException), PyUnicode_FromString(s.c_str()));
        SWIG_fail;
#else
        SWIG_exception(SWIG_ValueError, s.c_str());
#endif
    } catch (std::runtime_error &e) {
        const std::string s = std::string("SUMO error: ") + e.what();
        SWIG_exception(SWIG_RuntimeError, s.c_str());
    } catch (...) {
        SWIG_exception(SWIG_UnknownError, "unknown exception");
    }
}

// %feature("compactdefaultargs") libsumo::Simulation::findRoute;

// Add necessary symbols to generated header
%{
#include <libsumo/Edge.h>
#include <libsumo/InductionLoop.h>
#include <libsumo/Junction.h>
#include <libsumo/LaneArea.h>
#include <libsumo/Lane.h>
#include <libsumo/MultiEntryExit.h>
#include <libsumo/POI.h>
#include <libsumo/Polygon.h>
#include <libsumo/Route.h>
#include <libsumo/Simulation.h>
#include <libsumo/TrafficLight.h>
#include <libsumo/VehicleType.h>
#include <libsumo/Vehicle.h>
#include <libsumo/Person.h>
#include <libsumo/Calibrator.h>
#include <libsumo/BusStop.h>
#include <libsumo/ParkingArea.h>
#include <libsumo/ChargingStation.h>
#include <libsumo/OverheadWire.h>
%}

// Process symbols in header
%include "TraCIDefs.h"
%template(TraCILogicVector) std::vector<libsumo::TraCILogic>;
%template(TraCIStageVector) std::vector<libsumo::TraCIStage>;
%template(TraCINextStopDataVector2) std::vector<libsumo::TraCINextStopData>;
%template(TraCIReservationVector) std::vector<libsumo::TraCIReservation>;
%include "Edge.h"
%include "InductionLoop.h"
%include "Junction.h"
%include "LaneArea.h"
%include "Lane.h"
%include "MultiEntryExit.h"
%include "POI.h"
%include "Polygon.h"
%include "Route.h"
%include "Simulation.h"
%include "TraCIConstants.h"
%include "TrafficLight.h"
%include "VehicleType.h"
%include "Vehicle.h"
%include "Person.h"
%include "Calibrator.h"
%include "BusStop.h"
%include "ParkingArea.h"
%include "ChargingStation.h"
%include "OverheadWire.h"

#ifdef SWIGPYTHON
%pythoncode %{
def wrapAsClassMethod(func, module):
    def wrapper(*args, **kwargs):
        return func(module, *args, **kwargs)
    return wrapper

exceptions.TraCIException = TraCIException
simulation.Stage = TraCIStage
vehicle.StopData = TraCINextStopData
person.Reservation = TraCIReservation
trafficlight.Phase = TraCIPhase
trafficlight.Logic = TraCILogic
vehicle.addFull = vehicle.add
vehicle.addLegacy = wrapAsClassMethod(_vehicle.VehicleDomain.addLegacy, vehicle)
vehicle.couldChangeLane = wrapAsClassMethod(_vehicle.VehicleDomain.couldChangeLane, vehicle)
vehicle.wantsAndCouldChangeLane = wrapAsClassMethod(_vehicle.VehicleDomain.wantsAndCouldChangeLane, vehicle)
vehicle.isStopped = wrapAsClassMethod(_vehicle.VehicleDomain.isStopped, vehicle)
vehicle.setBusStop = wrapAsClassMethod(_vehicle.VehicleDomain.setBusStop, vehicle)
vehicle.setParkingAreaStop = wrapAsClassMethod(_vehicle.VehicleDomain.setParkingAreaStop, vehicle)
vehicle.getRightFollowers = wrapAsClassMethod(_vehicle.VehicleDomain.getRightFollowers, vehicle)
vehicle.getRightLeaders = wrapAsClassMethod(_vehicle.VehicleDomain.getRightLeaders, vehicle)
vehicle.getLeftFollowers = wrapAsClassMethod(_vehicle.VehicleDomain.getLeftFollowers, vehicle)
vehicle.getLeftLeaders = wrapAsClassMethod(_vehicle.VehicleDomain.getLeftLeaders, vehicle)
vehicle.getLaneChangeStatePretty = wrapAsClassMethod(_vehicle.VehicleDomain.getLaneChangeStatePretty, vehicle)
vehicle._legacyGetLeader = True
person.removeStages = wrapAsClassMethod(_person.PersonDomain.removeStages, person)
_trafficlight.TraCIException = TraCIException
trafficlight.setLinkState = wrapAsClassMethod(_trafficlight.TrafficLightDomain.setLinkState, trafficlight)
addStepListener = wrapAsClassMethod(connection.Connection.addStepListener, sys.modules[__name__])
removeStepListener = wrapAsClassMethod(connection.Connection.removeStepListener, sys.modules[__name__])
_manageStepListeners = wrapAsClassMethod(connection.Connection._manageStepListeners, sys.modules[__name__])
%}
#endif
