#ifndef MODELLISTENER_HPP
#define MODELLISTENER_HPP

#include <gui/model/Model.hpp>
#include "vehicle_data.h"

class ModelListener
{
public:
    ModelListener() : model(0) {}

    virtual ~ModelListener() {}

    void bind(Model* m)
    {
        model = m;
    }

    /* Called from Model::tick() with the latest VehicleDataRepository snapshot.
     * Default no-op so screens that don't care (e.g. Screen2) need not override. */
    virtual void updateVehicleData(const VehicleDataSnapshot&) {}
protected:
    Model* model;
};

#endif // MODELLISTENER_HPP
