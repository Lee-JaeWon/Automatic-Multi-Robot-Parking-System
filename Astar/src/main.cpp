// #include <iostream>
// #include <ros/ros.h>
// #include <nav_msgs/OccupancyGrid.h>
// #include <nav_msgs/Odometry.h>
// #include <nav_msgs/Path.h>
// #include <std_msgs/Bool.h>
// #include <geometry_msgs/PoseWithCovarianceStamped.h>
// #include <geometry_msgs/PoseStamped.h>
// #include <geometry_msgs/Quaternion.h>
// #include <opencv2/opencv.hpp>
// // #include <opencv.hpp>
// #include "Astar.h"
// #include "OccMapTransform.h"

// // For Action
// #include "actionlib/server/simple_action_server.h"
// #include "actionlib/client/simple_action_client.h"
// #include "actionlib/client/terminal_state.h"
// #include <parking_msgs/parkingOrderAction.h>
// #include <parking_msgs/Planner2TrackerAction.h>

// using namespace cv;
// using namespace std;

// //-------------------------------- Global variables ---------------------------------//
// // Subscriber
// ros::Subscriber map_sub;
// ros::Subscriber startPoint_sub;
// ros::Subscriber odom_sub;

// // Publisher
// ros::Publisher path_pub;

// // Action Client
// actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> *ac_plan2track;

// // Object
// nav_msgs::OccupancyGrid OccGridMask;
// nav_msgs::Path path;
// pathplanning::AstarConfig config;
// pathplanning::Astar astar;
// OccupancyGridParam OccGridParam;
// Point startPoint, targetPoint;
// geometry_msgs::PoseStamped path_point;

// // Parameter
// double InflateRadius;
// int rate;
// int origin_x, origin_y;
// double resol;
// int mx, my;
// double present_x, present_y;
// int robot_num = 3;

// // Flags
// bool start_flag;
// bool odomFlag;
// bool mapFlag;
// bool targetFlag;
// bool is_parking_order_result_true = false;

// // Triggers
// bool action_called = false;

// //------------------------------------Action Class------------------------------------//
// class parkingOrderAction
// {

// protected:
//   // 노드 핸들
//   ros::NodeHandle nh_;
//   // 액션 서버
//   actionlib::SimpleActionServer<parking_msgs::parkingOrderAction> as_;
//   // 액션 이름
//   std::string action_name_;
//   // 액션 피드백 및 결과
//   parking_msgs::parkingOrderFeedback feedback_;
//   parking_msgs::parkingOrderResult result_;

// public:
//   // 액션 서버 초기화
//   // bool start=false;
//   bool isSuccess = false;

//   // goal 변수
//   parking_msgs::parkingOrderGoal goal_;

//   parkingOrderAction(std::string name) : // 핸들, 액션 이름, 콜백, auto_start
//                                          // actionlib document에 따르면 auto_start가 true일때 ros::spin을 사용하지 않아도 된다고 합니다.
//                                          // boost::bind는 메소드를 callback으로 넘겨주며,
//                                          // 전달되는 첫 번째 값을 callback 함수의 2번째 파라미터로 넘겨줍니다. (첫 번째는 자기자신)
//                                          as_(nh_, name, boost::bind(&parkingOrderAction::executeCB, this, _1), false),
//                                          action_name_(name)
//   {
//     as_.start();
//   }

//   ~parkingOrderAction()
//   {
//   }

//   // goal을 받아 지정된 액션을 수행
//   void executeCB(const parking_msgs::parkingOrderGoalConstPtr &goal)
//   {

//     action_called = true;
//     goal_ = *goal;

//     // 시퀀스 vector를 초기화합니다.
//     feedback_.percent.push_back(0);
//     // start=true;

//     for (int i = 1; i <= goal->robotNum; i++)
//     {
//       // cancel action
//       if (as_.isPreemptRequested() || !ros::ok())
//       {
//         ROS_INFO("%s: Preempted", action_name_.c_str());
//         as_.setPreempted();
//         isSuccess = false;
//         break;
//       }
//       feedback_.percent.push_back(i);
//       as_.publishFeedback(feedback_);
//     }

//     while (!isSuccess) // action not done = still running
//     {
//     }

//     if (isSuccess) // action done
//     {
//       ROS_INFO("3");
//       isSuccess = false;
//       result_.sequence = true;
//       result_.i = goal_.i;
//       result_.j = goal_.j;
//       result_.k = goal_.k;
//       ROS_INFO("%s: Succeeded", action_name_.c_str());
//       as_.setSucceeded(result_);
//       result_.sequence = false;
//     }
//   }

//   const geometry_msgs::PoseStamped getTarget(){
//     return goal_.goal;
//   }
// };

// //-------------------------------- Callback function ---------------------------------//
// void MapCallback(const nav_msgs::OccupancyGrid &msg) {
//   // Get parameter
//   resol = msg.info.resolution;
//   origin_x = msg.info.origin.position.x;
//   origin_y = msg.info.origin.position.y;
//   OccGridParam.GetOccupancyGridParam(msg);

//   // Get map
//   int height = OccGridParam.height;
//   int width = OccGridParam.width;
//   int OccProb;
//   Mat Map(height, width, CV_8UC1);
//   for (int i = 0; i < height; i++)
//   {
//     for (int j = 0; j < width; j++)
//     {
//       OccProb = msg.data[i * width + j];
//       OccProb = (OccProb < 0) ? 100 : OccProb; // set Unknown to 0
//       // The origin of the OccGrid is on the bottom left corner of the map
//       Map.at<uchar>(height - i - 1, j) = 255 - round(OccProb * 255.0 / 100.0);
//     }
//   }

//   // Initial Astar
//   Mat Mask;
//   config.InflateRadius = round(InflateRadius / OccGridParam.resolution);
//   astar.InitAstar(Map, Mask, config);
//   // Set flag
//   mapFlag = true;
//   ROS_INFO("map Done");
// }

// void TargetPointtCallback(const geometry_msgs::PoseStamped &msg)
// {
//   Point2d src_point = Point2d(msg.pose.position.x, msg.pose.position.y);
//   OccGridParam.Map2ImageTransform(src_point, targetPoint);
// }
// void odomCallback(const nav_msgs::Odometry &msg)
// {
//   mx = int((msg.pose.pose.position.x - origin_x) / (resol)); // world to map
//   my = int((msg.pose.pose.position.y - origin_y) / (resol)); // world to map
//   Point2d src_point = Point2d(msg.pose.pose.position.x, msg.pose.pose.position.y);
//   OccGridParam.Map2ImageTransform(src_point, startPoint);

//   // Set flag
//   odomFlag = true;
// }

// void done_plan_to_track_Cb(const actionlib::SimpleClientGoalState &state, const parking_msgs::Planner2TrackerResultConstPtr &result)
// {
//   if (result->result)
//   {
//     ROS_INFO("3 - result  recieve2");
//     is_parking_order_result_true = true;
//   }
//   std::cout << "done_plan_to_track_Cb result is : " << result->result << "\n";
// }
// void feedback_plan_to_track_Cb(const parking_msgs::Planner2TrackerFeedbackConstPtr &feedback)
// {
//   //std::cout << "percent :  " << feedback->percent << "%"<< "\n";
// }

// void activeCb()
// {
//   ROS_INFO("Goal just went active");
// }

// void publish_path(std::string map_ns, parking_msgs::parkingOrderGoal goal_)
// {

//   double start_time = ros::Time::now().toSec();
//   // Start planning path
//   vector<Point> PathList;
//   astar.PathPlanning(startPoint, targetPoint, PathList);
//   if (!PathList.empty())
//   {
//     path.header.stamp = ros::Time::now();
//     path.header.frame_id = map_ns;
//     path.poses.clear();
//     for (int i = 0; i < PathList.size(); i++)
//     {
//       Point2d dst_point;
//       OccGridParam.Image2MapTransform(PathList[i], dst_point);

//       geometry_msgs::PoseStamped pose_stamped;
//       pose_stamped.header.stamp = ros::Time::now();
//       pose_stamped.header.frame_id = map_ns;
//       pose_stamped.pose.position.x = dst_point.x;
//       pose_stamped.pose.position.y = dst_point.y;
//       pose_stamped.pose.position.z = 0;
//       path.poses.push_back(pose_stamped);
//     }
//     path_pub.publish(path);

//     parking_msgs::Planner2TrackerGoal goal;
//     goal.path = path;
//     goal.robotNum = goal_.robotNum;

//     ac_plan2track->sendGoal(goal, &done_plan_to_track_Cb, &activeCb, &feedback_plan_to_track_Cb);

//     //      ac_plan2track->sendGoal(goal, boost::bind(done_plan_to_track_Cb, _1, _2));
//     //      ac_plan2track->sendGoal(goal, boost::bind(&done_plan_to_track_Cb, _1, _2),
//     //                              actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction>::SimpleActiveCallback(),
//     //                              actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction>::SimpleFeedbackCallback());

//     //      ac_plan2track->sendGoal(goal);

//     //      // 결과가 도착할 때까지 기다림
//     //      bool finished_before_timeout = ac_plan2track->waitForResult(ros::Duration(60.0));

//     //      if (finished_before_timeout) {
//     //        actionlib::SimpleClientGoalState state = ac_plan2track->getState();
//     //        parking_msgs::Planner2TrackerResultConstPtr result = ac_plan2track->getResult();
//     //        // 결과 처리
//     //        done_plan_to_track_Cb(state, result);
//     //      } else {
//     //        // 결과가 제한 시간 내에 도착하지 않은 경우
//     //        ROS_WARN("Timeout: Planner2Tracker action did not complete within the timeout.");
//     //      }

//     double end_time = ros::Time::now().toSec();

//     ROS_INFO("Find a valid path successfully! Use %f s", end_time - start_time);
//   }
//   else
//   {
//     ROS_ERROR("Can not find a valid path");
//   }
//   int i = 0;

//   if ((path_point.pose.position.x - present_x) + (path_point.pose.position.y - present_y) > 0.3)
//     i++;

//   ROS_INFO("%d\n", i);
//   path_point = path.poses[20];
//   path_point.pose.orientation.w = 1;
//   path_point.pose.orientation.x = 0;
//   path_point.pose.orientation.y = 0;
//   path_point.pose.orientation.z = 0;
// }

// //-------------------------------- Main function ---------------------------------//
// int main(int argc, char **argv)
// {
//   //  Initial node
//   ros::init(argc, argv, "astar");
//   ros::NodeHandle nh;
//   ros::NodeHandle nh_priv("~");
//   ROS_INFO("Start astar node!\n");

//   // param check
//   std::string s;
//   if (ros::param::get("~namespace", s)) ROS_INFO("Astar node got param: %s", s.c_str());
//   else ROS_ERROR("Astar Failed to get param '%s'", s.c_str());

//   // Initial variables
//   mapFlag = false;
//   odomFlag = false;
//   targetFlag = false;
//   start_flag = false;

//   // Parameter
//   nh_priv.param<bool>("Euclidean", config.Euclidean, true);
//   nh_priv.param<int>("OccupyThresh", config.OccupyThresh, -1);
//   nh_priv.param<double>("InflateRadius", InflateRadius, -1);
//   nh_priv.param<int>("rate", rate, 10);
//   nh_priv.param<int>("robot_num", robot_num);

//   // Subscribe topics
//   std::string map_ns = "/path_map"; // fixed frame
//   std::string goal_ns = s + "/move_base_simple/goal";
//   std::string odom_ns = s + "/odom";
//   std::string initpose_ns = s + "/initialpose";

//   // Subscriber
//   map_sub = nh.subscribe((map_ns), 10, MapCallback);
//   odom_sub = nh.subscribe(odom_ns, 10, odomCallback);

//   // Publish topics
//   std::string path_ns = s + "/path";

//   // Publisher
//   path_pub = nh.advertise<nav_msgs::Path>(path_ns, 10);

//   // Action Service Class 할당"
//   std::string action_parking_order_ns = s + "/action_parking_order";
//   parkingOrderAction action(action_parking_order_ns);

//   // Action Client
//   std::string action_plan_to_track_ns = s + "/action_plan_to_track";
//   // actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> ac_plan2track(action_plan_to_track_ns, true);
//   ac_plan2track = new actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction>(action_plan_to_track_ns, true);

//   // Loop and wait for callback
//   ros::Rate loop_rate(33);
//   // ros::AsyncSpinner(0);

//   while (ros::ok())
//   {
//     // Call Action Once
//     if (action_called && odomFlag && mapFlag)
//     {
//       TargetPointtCallback(action.getTarget()); // setting targetPoint
//       publish_path("map", action.goal_); // publishing path
//       action_called = false;
//       // action.isSuccess=true;
//     }

//     // Wait for end result
//     if (is_parking_order_result_true)
//     {
//       action.isSuccess = true;
//       is_parking_order_result_true = false;
//     }

//     ros::spinOnce();
//     loop_rate.sleep();
//   }

//   return 0;
// }

#include <iostream>
#include <ros/ros.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Odometry.h>
#include <nav_msgs/Path.h>
#include <std_msgs/Bool.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Quaternion.h>
#include <opencv2/opencv.hpp>
#include <tf/transform_listener.h>
// #include <opencv.hpp>
#include "Astar.h"
#include "OccMapTransform.h"

// For Action
#include "actionlib/server/simple_action_server.h"
#include "actionlib/client/simple_action_client.h"
#include "actionlib/client/terminal_state.h"
#include <parking_msgs/parkingOrderAction.h>
#include <parking_msgs/Planner2TrackerAction.h>

#include <mutex>
std::mutex resultMutex;

using namespace cv;
using namespace std;

//-------------------------------- Global variables ---------------------------------//
// Subscriber
ros::Subscriber map_sub;

ros::Subscriber startPoint_sub;
ros::Subscriber odom_sub1;
ros::Subscriber odom_sub2;
ros::Subscriber odom_sub3;

// Publisher
ros::Publisher path_pub1;
ros::Publisher path_pub2;
ros::Publisher path_pub3;

// Action Client
actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> *ac_plan2track1;
actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> *ac_plan2track2;
actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> *ac_plan2track3;

vector<actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> *> actionList;

vector<parking_msgs::Planner2TrackerGoal> goalList;

// Object
nav_msgs::OccupancyGrid OccGridMask;
nav_msgs::Path path;
pathplanning::AstarConfig config;
pathplanning::Astar astar;
OccupancyGridParam OccGridParam;
Point startPoint1, startPoint2, startPoint3, targetPoint;

// PATH

vector<Point> R1PathList;
vector<Point> R2PathList;
vector<Point> R3PathList;

vector<double> R1PathTime;
vector<double> R2PathTime;
vector<double> R3PathTime;

vector<int> seqList;


std::string s;
geometry_msgs::PoseStamped path_point;

// Parameter
double InflateRadius;
int rate;
int origin_x, origin_y;
double resol;
int mx, my;
double present_x, present_y;
int robot_num = 3;
double Time = 0.0;

// Flags
bool start_flag;
bool odomFlag1;
bool odomFlag2;
bool odomFlag3;

bool Priority;
int seqNumber;
bool pPass = false;
bool mapFlag;
bool targetFlag;
bool is_parking_order_result_true_one = false;
bool is_parking_order_result_true_two = false;
bool is_parking_order_result_true_thr = false;

int seq_done=0;

// Triggers

//------------------------------------Action Class------------------------------------//
class parkingOrderAction
{

protected:
  // 노드 핸들
  ros::NodeHandle nh_;
  // 액션 서버
  actionlib::SimpleActionServer<parking_msgs::parkingOrderAction> as_;
  // 액션 이름
  std::string action_name_;
  // 액션 피드백 및 결과
  parking_msgs::parkingOrderFeedback feedback_;
  parking_msgs::parkingOrderResult result_;

public:
  // 액션 서버 초기화
  // bool start=false;
  bool isSuccess = false;
  bool action_called = false;

  // goal 변수
  parking_msgs::parkingOrderGoal goal_;

  parkingOrderAction(std::string name) : // 핸들, 액션 이름, 콜백, auto_start
                                         // actionlib document에 따르면 auto_start가 true일때 ros::spin을 사용하지 않아도 된다고 합니다.
                                         // boost::bind는 메소드를 callback으로 넘겨주며,
                                         // 전달되는 첫 번째 값을 callback 함수의 2번째 파라미터로 넘겨줍니다. (첫 번째는 자기자신)
                                         as_(nh_, name, boost::bind(&parkingOrderAction::executeCB, this, _1), false),
                                         action_name_(name)
  {
    as_.start();
  }

  ~parkingOrderAction()
  {
  }

  // goal을 받아 지정된 액션을 수행
  void executeCB(const parking_msgs::parkingOrderGoalConstPtr &goal)
  {
    result_.sequence = false;

    action_called = true;
    goal_ = *goal;

    while (!isSuccess) // action not done = still running
    {
      // feedback_.percent=0;
      // as_.publishFeedback(feedback_);
    }

    if (isSuccess) // action done
    {
      // ROS_INFO("3");
      isSuccess = false;
      result_.sequence = true;
      result_.i = goal_.i;
      result_.j = goal_.j;
      result_.k = goal_.k;

      Priority = goal_.priority;
      seqNumber = goal_.seqNum;

      result_.robotNum = goal_.robotNum;

      ROS_INFO("%s: Succeeded", action_name_.c_str());
      as_.setSucceeded(result_);
      ROS_INFO("after setSuc");
    }
  }

  const geometry_msgs::PoseStamped getTarget()
  {
    return goal_.goal;
  }
};

//////////////////////////////////////////////////////////

double min_(double a, double b)
{
  if (a > b)
    return b;
  else
    return a;
}

double calc_path_time(vector<Point> pt1, vector<Point> pt2, vector<Point> pt3, int curp)
{

  geometry_msgs::PointStamped t1;
  geometry_msgs::PointStamped t2;

  Point2d r1pointt;
  Point2d r2pointt;
  Point2d r3pointt;

  double minT = 100000.0;
  double minDist = 100000.0;
  double time_diff1 = 100000.0;
  double time_diff2 = 100000.0;
  int R1p, R2p, R3p = 0;

  ROS_INFO("Calc_path_time");
  if (curp == 1)
  {
    ROS_INFO("first path publish");
    if (!pt2.empty())
    {
      for (int i = 0; i < pt1.size(); i++)
      {
        for (int j = 0; j < pt2.size(); j++)
        {

          OccGridParam.Image2MapTransform(R1PathList[i], r1pointt);
          OccGridParam.Image2MapTransform(R2PathList[j], r2pointt);
          double dst = sqrt(pow(r1pointt.x - r2pointt.x, 2) + pow(r1pointt.y - r2pointt.y, 2));
          if (dst < minDist)
          {
            minDist = dst;
            R1p = i;
            R2p = j;
          }
        }
      }
      time_diff1 = R1PathTime[R1p] - R2PathTime[R2p];
      // if (abs(time_diff1) > 10.0)
      //   minDist = 100000.0;
    }
    if (!pt3.empty())
    {
      for (int i = 0; i < pt1.size(); i++)
      {
        for (int j = 0; j < pt3.size(); j++)
        {
          OccGridParam.Image2MapTransform(R1PathList[i], r1pointt);
          OccGridParam.Image2MapTransform(R3PathList[j], r3pointt);
          double dst = sqrt(pow(r1pointt.x - r3pointt.x, 2) + pow(r1pointt.y - r3pointt.y, 2));
          if (dst < minDist)
          {
            minDist = dst;
            R1p = i;
            R3p = j;
          }
        }
      }
      //
      time_diff2 = R1PathTime[R1p] - R3PathTime[R3p];
    }
    // ROS_INFO("%f %f",time_diff1 , time_diff2);
    double time_diff = min_(time_diff1, time_diff2);
    ROS_INFO("time_diff : %lf minDist : %f", time_diff, minDist);
    if (((time_diff > -25.0) && (time_diff) < 20.0) && minDist < 0.3)
      return (time_diff);
    else
      return 0.0;
  }

  else if (curp == 2)
  {
    ROS_INFO("second path publish");
    if (!pt1.empty())
    {
        ROS_INFO("1 path not empty");
      for (int i = 0; i < pt2.size(); i++)
      {
        for (int j = 0; j < pt1.size(); j++)
        {
          OccGridParam.Image2MapTransform(R2PathList[i], r2pointt);
          OccGridParam.Image2MapTransform(R1PathList[j], r1pointt);
          double dst = sqrt(pow(r1pointt.x - r2pointt.x, 2) + pow(r1pointt.y - r2pointt.y, 2));
          if (dst < minDist)
          {
            minDist = dst;
            R2p = i;
            R1p = j;
          }
        }
      }
      //
      time_diff1 = R2PathTime[R2p] - R1PathTime[R1p];
      // if (abs(time_diff1) > 10.0)
      //   minDist = 100000.0;
    }
    if (!pt3.empty())
    {
        ROS_INFO("3 path not empty");

      for (int i = 0; i < pt2.size(); i++)
      {
        for (int j = 0; j < pt3.size(); j++)
        {
          OccGridParam.Image2MapTransform(R2PathList[i], r2pointt);
          OccGridParam.Image2MapTransform(R3PathList[j], r3pointt);
          double dst = sqrt(pow(r2pointt.x - r3pointt.x, 2) + pow(r2pointt.y - r3pointt.y, 2));
          if (dst < minDist)
          {
            minDist = dst;
            R2p = i;
            R3p = j;
          }
        }
      }
      //
      time_diff2 = R2PathTime[R2p] - R3PathTime[R3p];
    }
    double time_diff = min_(time_diff1, time_diff2);
    ROS_INFO("time_diff : %lf minDist : %f", time_diff, minDist);

    if (((time_diff > -25.0) && (time_diff) < 20.0) && minDist < 0.3)
      return (time_diff);
    else
      return 0.0;
  }

  else if (curp == 3)
  {

    ROS_INFO("third path publish");
    if (!pt1.empty())
    {
      for (int i = 0; i < pt3.size(); i++)
      {
        for (int j = 0; j < pt1.size(); j++)
        {
          OccGridParam.Image2MapTransform(R3PathList[i], r3pointt);
          OccGridParam.Image2MapTransform(R1PathList[j], r1pointt);
          double dst = sqrt(pow(r3pointt.x - r1pointt.x, 2) + pow(r3pointt.y - r1pointt.y, 2));
          if (dst < minDist)
          {
            minDist = dst;
            R3p = i;
            R1p = j;
          }
        }
      }
      //
      time_diff1 = R3PathTime[R3p] - R1PathTime[R1p];
      // if (abs(time_diff1) > 10.0)
      //   minDist = 100000.0;
    }
    if (!pt2.empty())
    {

      for (int i = 0; i < pt3.size(); i++)
      {
        for (int j = 0; j < pt2.size(); j++)
        {
          OccGridParam.Image2MapTransform(R3PathList[i], r3pointt);
          OccGridParam.Image2MapTransform(R2PathList[j], r2pointt);
          double dst = sqrt(pow(r2pointt.x - r3pointt.x, 2) + pow(r2pointt.y - r3pointt.y, 2));
          if (dst < minDist)
          {
            minDist = dst;
            R3p = i;
            R2p = j;
          }
        }
      }
      //
      time_diff2 = R3PathTime[R3p] - R2PathTime[R2p];
    }
    double time_diff = min_(time_diff1, time_diff2);
    ROS_INFO("time_diff : %lf minDist : %f", time_diff, minDist);
    if (((time_diff > -25.0) && (time_diff) < 20.0) && minDist < 0.3)
      return (time_diff);
    else
      return 0.0;
  }

  // return minT;
  return 0;
}

//-------------------------------- Callback function ---------------------------------//
void MapCallback(const nav_msgs::OccupancyGrid &msg)
{
  // Get parameter
  resol = msg.info.resolution;
  origin_x = msg.info.origin.position.x;
  origin_y = msg.info.origin.position.y;
  OccGridParam.GetOccupancyGridParam(msg);

  // Get map
  int height = OccGridParam.height;
  int width = OccGridParam.width;
  int OccProb;
  Mat Map(height, width, CV_8UC1);
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      OccProb = msg.data[i * width + j];
      OccProb = (OccProb < 0) ? 100 : OccProb; // set Unknown to 0
      // The origin of the OccGrid is on the bottom left corner of the map
      Map.at<uchar>(height - i - 1, j) = 255 - round(OccProb * 255.0 / 100.0);
    }
  }

  // Initial Astar
  Mat Mask;
  config.InflateRadius = round(InflateRadius / OccGridParam.resolution);
  astar.InitAstar(Map, Mask, config);
  // Set flag
  mapFlag = true;
  ROS_INFO("map Done");
}

void TargetPointtCallback(const geometry_msgs::PoseStamped &msg)
{
  Point2d src_point = Point2d(msg.pose.position.x, msg.pose.position.y);
  OccGridParam.Map2ImageTransform(src_point, targetPoint);
}

void odom1Callback(const nav_msgs::Odometry &msg)
{
  Point2d src_point = Point2d(msg.pose.pose.position.x, msg.pose.pose.position.y);
  OccGridParam.Map2ImageTransform(src_point, startPoint1);

  // Set flag
  odomFlag1 = true;
}
void odom2Callback(const nav_msgs::Odometry &msg)
{
  Point2d src_point = Point2d(msg.pose.pose.position.x, msg.pose.pose.position.y);
  OccGridParam.Map2ImageTransform(src_point, startPoint2);

  // Set flag
  odomFlag2 = true;
}
void odom3Callback(const nav_msgs::Odometry &msg)
{
  Point2d src_point = Point2d(msg.pose.pose.position.x, msg.pose.pose.position.y);
  OccGridParam.Map2ImageTransform(src_point, startPoint3);

  // Set flag
  odomFlag3 = true;
}

void done_plan_to_track_Cb_one(const actionlib::SimpleClientGoalState &state, const parking_msgs::Planner2TrackerResultConstPtr &result)
{
  // 결과(result) 처리를 뮤텍스로 보호
  std::lock_guard<std::mutex> lock(resultMutex);
  if (result->result)
  {
    // ROS_INFO("3 - result  recieve2");
    is_parking_order_result_true_one = true;
  }
  // std::cout << "done_plan_to_track_Cb result is : " << result->result << "\n";
}

void done_plan_to_track_Cb_two(const actionlib::SimpleClientGoalState &state, const parking_msgs::Planner2TrackerResultConstPtr &result)
{
  // 결과(result) 처리를 뮤텍스로 보호
  std::lock_guard<std::mutex> lock(resultMutex);
  if (result->result)
  {
    // ROS_INFO("3 - result  recieve2");
    is_parking_order_result_true_two = true;
  }
  // std::cout << "done_plan_to_track_Cb result is : " << result->result << "\n";
}

void done_plan_to_track_Cb_thr(const actionlib::SimpleClientGoalState &state, const parking_msgs::Planner2TrackerResultConstPtr &result)
{
  // 결과(result) 처리를 뮤텍스로 보호
  std::lock_guard<std::mutex> lock(resultMutex);
  if (result->result)
  {
    // ROS_INFO("3 - result  recieve2");
    is_parking_order_result_true_thr = true;
  }
  // std::cout << "done_plan_to_track_Cb result is : " << result->result << "\n";
}

void feedback_plan_to_track_Cb(const parking_msgs::Planner2TrackerFeedbackConstPtr &feedback)
{
  // std::cout << "percent :  " << feedback->percent << "%"<< "\n";
}

void activeCb()
{
  ROS_INFO("Goal just went active");
}

void publish_path(std::string map_ns, parking_msgs::parkingOrderGoal goal_, int curRobot, bool priority, int seq)
{
  double start_time = ros::Time::now().toSec();
  // Start planning path
  vector<Point> PathList;

  if (curRobot == 1)
  {

    astar.PathPlanning(startPoint1, targetPoint, PathList);
    R1PathList.clear();
    R1PathTime.clear();
    R1PathList = PathList;
  }
  if (curRobot == 2)
  {

    astar.PathPlanning(startPoint2, targetPoint, PathList);
    R2PathTime.clear();
    R2PathList.clear();
    R2PathList = PathList;
  }
  if (curRobot == 3)
  {

    astar.PathPlanning(startPoint3, targetPoint, PathList);

    R3PathTime.clear();
    R3PathList.clear();
    R3PathList = PathList;
  }

  if (!PathList.empty())
  {
    ROS_INFO("path_size = %d", PathList.size());
    path.header.stamp = ros::Time::now();
    path.header.frame_id = map_ns;
    path.poses.clear();

    for (int i = 0; i < PathList.size(); i++)
    {
      Point2d dst_point;
      OccGridParam.Image2MapTransform(PathList[i], dst_point);

      geometry_msgs::PoseStamped pose_stamped;
      pose_stamped.header.stamp = ros::Time::now();
      pose_stamped.header.frame_id = map_ns;
      pose_stamped.pose.position.x = dst_point.x;
      pose_stamped.pose.position.y = dst_point.y;
      pose_stamped.pose.position.z = 0;
      path.poses.push_back(pose_stamped);

      if (curRobot == 1)
        R1PathTime.push_back(Time + 0.05 * i);
      if (curRobot == 2)
        R2PathTime.push_back(Time + 0.05 * i);
      if (curRobot == 3)
        R3PathTime.push_back(Time + 0.05 * i);
    }

    parking_msgs::Planner2TrackerGoal goal;
    goal.path = path;
    goal.robotNum = goal_.robotNum;
    goal.rotation = goal_.rotation;
    seqNumber = seq;


    Priority = priority;
    // List push
    if (curRobot == 1) actionList.push_back(ac_plan2track1);
    if (curRobot == 2) actionList.push_back(ac_plan2track2);
    if (curRobot == 3) actionList.push_back(ac_plan2track3);
    goalList.push_back(goal);
    seqList.push_back(seq);


    ROS_INFO("Priority : %d", priority);
    ROS_INFO("robot num : %d", goal.robotNum);
    ROS_INFO("robot seq : %d", seq);
    

    if(priority) seq_done++;
    if (seq < seq_done)
    {
      goal = goalList.back();
      if(priority){

        if (goal.robotNum == 0)
        {
          path_pub1.publish(goal.path);
          actionList.back()->sendGoal(goal, &done_plan_to_track_Cb_one, &activeCb, &feedback_plan_to_track_Cb);
          ROS_INFO("priority publish");
        }
        if (goal.robotNum == 1)

        {
          path_pub2.publish(goal.path);
          actionList.back()->sendGoal(goal, &done_plan_to_track_Cb_two, &activeCb, &feedback_plan_to_track_Cb);
          ROS_INFO("priority publish");
        
        }
        if (goal.robotNum == 2)
        {
          path_pub3.publish(goal.path);
          actionList.back()->sendGoal(goal, &done_plan_to_track_Cb_thr, &activeCb, &feedback_plan_to_track_Cb);
          ROS_INFO("priority publish");
        
        }

        goalList.pop_back();
        actionList.pop_back();
        seqList.pop_back();
        pPass = true;
        ROS_INFO("pPass is True");
      }

      //pPass = true;

      bool flag = false;
      while (!actionList.empty()){

            double minTime = 0.0;
            int idx = 0;
            for(const auto& action : actionList){

              goal = goalList[idx];
              int curseq = seqList[idx];

              if (goal.robotNum == 0)
              {
                minTime = calc_path_time(R1PathList, R2PathList, R3PathList, 1);

                path_pub1.publish(goal.path);
                if (minTime < 0.0 && minTime > -25.0)
                  minTime = 25.0;
                // else if (minTime < -10.0)
                //   minTime = 20.0;
                else if (minTime > 0.0 && minTime < 10.0) minTime = 15.0;
                else if (minTime > 10.0) minTime = 25.0;
                for (int t = 0; t < R1PathTime.size(); t++)
                  R1PathTime[t] += minTime;

                ROS_INFO("Time is : %lf", minTime);
                ros::Duration(minTime).sleep();
                ac_plan2track1->sendGoal(goal, &done_plan_to_track_Cb_one, &activeCb, &feedback_plan_to_track_Cb);
                flag = true;

              }
              else if (goal.robotNum == 1 )
              {
                minTime = calc_path_time(R1PathList, R2PathList, R3PathList, 2);

                path_pub2.publish(goal.path);

                if (minTime < 0.0 && minTime > -10.0)
                  minTime = 10.0;
                // else if (minTime < -10.0)
                //   minTime = 20.0;
                else if (minTime > 0.0 && minTime < 10.0) minTime = 15.0;
                else if (minTime > 10.0) minTime = 25.0;
                for (int t = 0; t < R2PathTime.size(); t++)
                  R2PathTime[t] += minTime;

                ROS_INFO("Time is : %lf", minTime);
                ros::Duration(minTime).sleep();
                ac_plan2track2->sendGoal(goal, &done_plan_to_track_Cb_two, &activeCb, &feedback_plan_to_track_Cb);
                flag = true;


              }
              else if (goal.robotNum == 2 )
              {
                minTime = calc_path_time(R1PathList, R2PathList, R3PathList, 3);

                path_pub3.publish(goal.path);
                if (minTime < 0.0 && minTime > -10.0)
                  minTime = 10.0;
                // else if (minTime < -10.0)
                //   minTime = 20.0;
                else if (minTime > 0.0 && minTime < 10.0) minTime = 15.0;
                else if (minTime > 10.0) minTime = 25.0;
                for (int t = 0; t < R3PathTime.size(); t++) R3PathTime[t] += minTime;
                ROS_INFO("Time is : %lf", minTime);
                ros::Duration(minTime).sleep();
                ac_plan2track3->sendGoal(goal, &done_plan_to_track_Cb_thr, &activeCb, &feedback_plan_to_track_Cb);
                flag = true;
              }
            idx++;
          }
        
          goalList.pop_back();
          actionList.pop_back();
          seqList.pop_back();
          pPass = false;
          if(!flag) break;
    }

    }

    double end_time = ros::Time::now().toSec();
    ROS_INFO("Find a valid path successfully! Use %f s", end_time - start_time);
  }
  else
  {
    ROS_ERROR("Can not find a valid path");
  }
}

//-------------------------------- Main function ---------------------------------//
int main(int argc, char **argv)
{
  //  Initial node
  ros::init(argc, argv, "astar");
  ros::NodeHandle nh;
  ros::NodeHandle nh_priv("~");
  ROS_INFO("Start astar node!\n");

  // param check

  if (ros::param::get("~namespace", s))
    ROS_INFO("Astar node got param: %s", s.c_str());
  else
    ROS_ERROR("Astar Failed to get param '%s'", s.c_str());

  // Initial variables
  mapFlag = false;
  odomFlag1 = false;
  odomFlag2 = false;
  odomFlag3 = false;

  targetFlag = false;
  start_flag = false;

  // Parameter
  nh_priv.param<bool>("Euclidean", config.Euclidean, true);
  nh_priv.param<int>("OccupyThresh", config.OccupyThresh, -1);
  nh_priv.param<double>("InflateRadius", InflateRadius, -1);
  nh_priv.param<int>("rate", rate, 10);
  nh_priv.param<int>("robot_num", robot_num);

  // Subscribe topics
  std::string map_ns = "/path_map"; // fixed frame
  std::string goal_ns = s + "/move_base_simple/goal";
  std::string odom_ns = s + "/odom";
  std::string initpose_ns = s + "/initialpose";

  tf::TransformListener listener;

  // Subscriber
  map_sub = nh.subscribe((map_ns), 10, MapCallback);

  odom_sub1 = nh.subscribe("robot_1/odom", 10, odom1Callback);
  odom_sub2 = nh.subscribe("robot_2/odom", 10, odom2Callback);
  odom_sub3 = nh.subscribe("robot_3/odom", 10, odom3Callback);

  // Publisher
  path_pub1 = nh.advertise<nav_msgs::Path>("robot_1/path", 10);
  path_pub2 = nh.advertise<nav_msgs::Path>("robot_2/path", 10);
  path_pub3 = nh.advertise<nav_msgs::Path>("robot_3/path", 10);

  // Action Service Class 할당"
  std::string action_parking_order_ns1 = "/robot_1/action_parking_order";
  parkingOrderAction action1(action_parking_order_ns1);

  std::string action_parking_order_ns2 = "/robot_2/action_parking_order";
  parkingOrderAction action2(action_parking_order_ns2);

  std::string action_parking_order_ns3 = "/robot_3/action_parking_order";
  parkingOrderAction action3(action_parking_order_ns3);

  // Action Client
  std::string action_plan_to_track_ns1 = "/robot_1/action_plan_to_track";
  // send data to tracker node
  // actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> ac_plan2track(action_plan_to_track_ns, true);
  ac_plan2track1 = new actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction>(action_plan_to_track_ns1, true);

  std::string action_plan_to_track_ns2 = "/robot_2/action_plan_to_track";
  // actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> ac_plan2track(action_plan_to_track_ns, true);
  ac_plan2track2 = new actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction>(action_plan_to_track_ns2, true);

  std::string action_plan_to_track_ns3 = "/robot_3/action_plan_to_track";
  // actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction> ac_plan2track(action_plan_to_track_ns, true);
  ac_plan2track3 = new actionlib::SimpleActionClient<parking_msgs::Planner2TrackerAction>(action_plan_to_track_ns3, true);

  // Loop and wait for callback
  ros::Rate loop_rate(33);
  // ros::AsyncSpinner(0);

  while (ros::ok())
  {
    Time += (1.0 / 33.0);
    // Call Action Once

    if (odomFlag1 && action1.action_called)
    { 
      ROS_INFO("robot 1 action received");
      TargetPointtCallback(action1.getTarget());

      tf::StampedTransform transform;
      try
      {
        listener.lookupTransform("/map", "/robot_1/base_link",
                                 ros::Time(0), transform);
      }
      catch (tf::TransformException ex)
      {
        ROS_ERROR("%s", ex.what());
        ros::Duration(1.0).sleep();
      }

      Point2d tPoint = Point2d(transform.getOrigin().x(), transform.getOrigin().y());
      OccGridParam.Map2ImageTransform(tPoint, startPoint1);

      publish_path("map", action1.goal_, 1, action1.goal_.priority, action1.goal_.seqNum);
      action1.action_called = false;
    }

    else if (odomFlag2 && action2.action_called)
    { 
      ROS_INFO("robot 2 action received");
      TargetPointtCallback(action2.getTarget());
      tf::StampedTransform transform;
      try
      {
        listener.lookupTransform("/map", "/robot_2/base_link",
                                 ros::Time(0), transform);
      }
      catch (tf::TransformException ex)
      {
        ROS_ERROR("%s", ex.what());
        ros::Duration(1.0).sleep();
      }
      Point2d tPoint = Point2d(transform.getOrigin().x(), transform.getOrigin().y());
      OccGridParam.Map2ImageTransform(tPoint, startPoint2);

      publish_path("map", action2.goal_, 2, action2.goal_.priority, action2.goal_.seqNum);
      action2.action_called = false;
    }

    else if (odomFlag3 && action3.action_called)

    { 
      ROS_INFO("robot 3 action received");
      TargetPointtCallback(action3.getTarget());

      tf::StampedTransform transform;
      try
      {
        listener.lookupTransform("/map", "/robot_3/base_link",
                                 ros::Time(0), transform);
      }
      catch (tf::TransformException ex)
      {
        ROS_ERROR("%s", ex.what());
        ros::Duration(1.0).sleep();
      }

      Point2d tPoint = Point2d(transform.getOrigin().x(), transform.getOrigin().y());
      OccGridParam.Map2ImageTransform(tPoint, startPoint3);

      publish_path("map", action3.goal_, 3, action3.goal_.priority, action3.goal_.seqNum);
      action3.action_called = false;
    }

    // if (action_called && odomFlag && mapFlag)
    // {
    //   TargetPointtCallback(action1.getTarget()); // setting targetPoint

    //   publish_path("map", action1.goal_); // publishing path

    //   action_called = false;
    //   // action.isSuccess=true;
    // }
    // Wait for end result

    if (is_parking_order_result_true_one)
    {
      action1.isSuccess = true;
      is_parking_order_result_true_one = false;
    }

    if (is_parking_order_result_true_two)
    {
      action2.isSuccess = true;
      is_parking_order_result_true_two = false;
    }

    if (is_parking_order_result_true_thr)
    {
      action3.isSuccess = true;
      is_parking_order_result_true_thr = false;
    }

    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}