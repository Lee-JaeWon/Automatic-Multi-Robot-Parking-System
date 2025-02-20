include "map_builder.lua"
include "trajectory_builder.lua"

options = {

    map_builder = MAP_BUILDER,
    trajectory_builder = TRAJECTORY_BUILDER,

    map_frame = "robot_1/map",
    tracking_frame = "robot_2/base_link",
    published_frame = "robot_2/base_link",
    odom_frame = "robot_2/odom",
    provide_odom_frame = true,

    -- edit
    use_odometry = false, 
    --

    use_nav_sat = false,
    publish_tracked_pose = true,
    use_landmarks = false,
    publish_frame_projected_to_2d = true,
    num_laser_scans = 1,
    num_multi_echo_laser_scans = 0,
    num_subdivisions_per_laser_scan = 1,
    rangefinder_sampling_ratio = 1,
    odometry_sampling_ratio = 1,
    fixed_frame_pose_sampling_ratio = 1,
    imu_sampling_ratio = 1,
    landmarks_sampling_ratio = 1,
    num_point_clouds = 0,
    lookup_transform_timeout_sec = 0.2,
    submap_publish_period_sec = 0.3,
    pose_publish_period_sec = 5e-3,
    trajectory_publish_period_sec = 30e-3,
}

MAP_BUILDER.use_trajectory_builder_2d = true
-- edit
TRAJECTORY_BUILDER_2D.use_imu_data = true
--
TRAJECTORY_BUILDER_2D.use_online_correlative_scan_matching = true
POSE_GRAPH.optimization_problem.huber_scale = 1e2

return options
