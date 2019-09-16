#pragma once

#include <cmath>
#include <deque>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

#include "boost/geometry.hpp"
#include "boost/geometry/geometries/point_xy.hpp"
#include "boost/geometry/geometries/polygon.hpp"
#include "boost/foreach.hpp"
#include "boost/pointer_cast.hpp"
#include "carla/client/Vehicle.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"
#include "carla/geom/Math.h"
#include "carla/client/World.h"
#include "carla/client/ActorList.h"

#include "PipelineStage.h"
#include "MessengerAndDataTypes.h"
#include "VicinityGrid.h"

namespace traffic_manager {
  typedef boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double>> polygon;

  class CollisionStage : public PipelineStage {
    /// This class is the thread executable for the collission detection stage.
    /// The class is responsible for checking possible collisions with other
    /// vehicles along the vehicle's trajectory.

  private:

    carla::client::World& world;
    carla::client::DebugHelper& debug_helper;

    int localization_messenger_state;
    int planner_messenger_state;
    bool frame_selector;
    std::shared_ptr<LocalizationToCollisionFrame> localization_frame;
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_a;
    std::shared_ptr<CollisionToPlannerFrame> planner_frame_b;
    std::unordered_map<bool, std::shared_ptr<CollisionToPlannerFrame>> planner_frame_map;
    std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger;
    std::shared_ptr<CollisionToPlannerMessenger> planner_messenger;

    VicinityGrid vicinity_grid;
    std::unordered_map<uint, int> id_to_index;
    std::unordered_map<uint, carla::SharedPtr<carla::client::Actor>> unregistered_actors;
    std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::nanoseconds>
    last_world_actors_pass_instance;

    /// Returns true if there is a possible collision detected between the
    /// vehicles passed to the method.
    /// Collision is predicted by extrapolating a boundary around the vehicle
    /// along it's trajectory
    /// and checking if it overlaps with the extrapolated boundary of the other
    /// vehicle.

    bool CheckGeodesicCollision(
        carla::SharedPtr<carla::client::Actor> vehicle,
        carla::SharedPtr<carla::client::Actor> ego_vehicle) const;

    /// Returns the bounding box corners of the vehicle passed to the method.
    std::vector<carla::geom::Location> GetBoundary(carla::SharedPtr<carla::client::Actor> actor) const;

    /// Returns the extrapolated bounding box of the vehicle along it's
    /// trajectory.
    std::vector<carla::geom::Location> GetGeodesicBoundary(
        carla::SharedPtr<carla::client::Actor> actor) const;

    /// Method to construct a boost polygon object
    polygon GetPolygon(const std::vector<carla::geom::Location> &boundary) const;

    /// Method returns true if ego_vehicle should stop and wait for
    /// other_vehicle to pass.
    bool NegotiateCollision(
        carla::SharedPtr<carla::client::Actor> ego_vehicle,
        carla::SharedPtr<carla::client::Actor> other_vehicle) const;

    void DrawBoundary(const std::vector<carla::geom::Location> &boundary) const;

  public:

    CollisionStage(
      std::shared_ptr<LocalizationToCollisionMessenger> localization_messenger,
      std::shared_ptr<CollisionToPlannerMessenger> planner_messenger,
      int number_of_vehicle,
      int pool_size,
      carla::client::World& world,
      carla::client::DebugHelper& debug_helper
    );
    ~CollisionStage();

    void DataReceiver() override;
    void Action(const int start_index, const int end_index) override;
    void DataSender() override;

    using PipelineStage::Start;
    using PipelineStage::Stop;

  };

}
