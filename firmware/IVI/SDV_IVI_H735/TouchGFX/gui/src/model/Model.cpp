#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include "vehicle_data.h"

Model::Model() : modelListener(0)
{

}

void Model::tick()
{
    /* Bounded wait only guards against the rare case VehicleModelTask is mid-update;
     * no osDelay/unbounded wait here per the guide's Model::tick() rule. */
    VehicleDataSnapshot snapshot;
    if (VehicleModel_GetSnapshot(&snapshot, 5) && modelListener)
    {
        modelListener->updateVehicleData(snapshot);
    }
}
