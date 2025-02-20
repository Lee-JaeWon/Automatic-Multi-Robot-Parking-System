#include <ros/ros.h>
#include <std_msgs/String.h>
#include <std_msgs/Empty.h>
#include <sensor_msgs/Imu.h>
#include <geometry_msgs/Quaternion.h>
#include <serial/serial.h>
#include <std_msgs/Float64.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf/transform_broadcaster.h>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>

using namespace std;
serial::Serial ser;
int publish_rate = 30;

vector<string> split(std_msgs::String str, char Delimiter)
{
    istringstream iss(str.data); // istringstream에 str을 담는다.
    string buffer;               // 구분자를 기준으로 절삭된 문자열이 담겨지는 버퍼

    vector<string> result = {};

    // istringstream은 istream을 상속받으므로 getline을 사용할 수 있다.
    while (getline(iss, buffer, Delimiter))
    {                             // 구분자를 만나기 전까지 iss를 훑고 buffer에 저장
        result.push_back(buffer); // buffer의 내용을 벡터 result에 저장
    }
    return result;
}

void write_callback(const std_msgs::String::ConstPtr &msg)
{
    ROS_INFO_STREAM("Writing to serial port" << msg->data);
    ser.write(msg->data);
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "imu"); // 노드이름 imu
    ros::NodeHandle nh;
    // ros::Subscriber imu_sub = nh.subscribe("write", publish_rate, write_callback);
    ros::Publisher imu_pub0 = nh.advertise<sensor_msgs::Imu>("imu0", publish_rate);
    ros::Publisher imu_pub1 = nh.advertise<sensor_msgs::Imu>("imu1", publish_rate);
    ros::Publisher imu_pub2 = nh.advertise<sensor_msgs::Imu>("imu2", publish_rate);

    tf2::Quaternion q;
    // static tf2_ros::TransformBroadcaster br;
    // geometry_msgs::TransformStamped transformStamped;

    // transformStamped.header.stamp = ros::Time(0);
    // transformStamped.header.frame_id = "base_link";
    // transformStamped.child_frame_id = "imu";

    q.setRPY(0, 0, 0);

    try
    {
        ser.setPort("/dev/ttyUSB0");
        ser.setBaudrate(115200);
        serial::Timeout to = serial::Timeout::simpleTimeout(publish_rate);
        ser.setTimeout(to);
        ser.open();
    }

    catch (serial::IOException &e)
    {
        ROS_ERROR_STREAM("Unable to open port ");
        return -1;
    }

    if (ser.isOpen())
    {
        ROS_INFO_STREAM("Serial Port initialized");
    }
    else
    {
        return -1;
    }

    ros::Rate loop_rate(publish_rate); // 여기에 따라 1초에 신호를 얼마나 받아오는지.
    // Rate::sleep()이 나오고 특정 시간동안 신호를 보내지 않기 전까지, 계속 유지
    std_msgs::String sensor;

    sensor_msgs::Imu imu;
    sensor_msgs::Imu imu0;
    sensor_msgs::Imu imu1;
    sensor_msgs::Imu imu2;

    int cnt = 0;

    while (ros::ok())
    {
        ros::spinOnce();
        if (ser.available())
        {
            int count=1;
            // ROS_INFO_STREAM("Reading from serial port");

            imu.header.seq = cnt;
            // transformStamped.header.seq = cnt;
            sensor.data = ser.read(ser.available());
            // cout << sensor.data << endl;

            imu.header.stamp = ros::Time::now();
            // transformStamped.header.stamp = ros::Time::now();
        
            vector<string> result = split(sensor, '\n');
            vector<string> temp;

            // cout << result.size() << endl;
            
            // count++;
            for (int i = 0 ; i < result.size() ; i ++){
                // cout << result[i] << "\n" ;
                vector<string> tempresult;
                stringstream ss(result[i]);
                string token;
                while (std::getline(ss, token, ',')) {
                    tempresult.push_back(token);
                    }
            
            if (tempresult.size() == 15)
            {
                char id = tempresult[0][4];
                float quaternion_w = atof(tempresult[4].c_str());
                float quaternion_x = atof(tempresult[3].c_str());
                float quaternion_y = atof(tempresult[2].c_str());
                float quaternion_z = atof(tempresult[1].c_str());
                float angular_velocity_x = atof(tempresult[5].c_str());
                float angular_velocity_y = atof(tempresult[6].c_str());
                float angular_velocity_z = atof(tempresult[7].c_str());
                float linear_acceleration_x = atof(tempresult[8].c_str());
                float linear_acceleration_y = atof(tempresult[9].c_str());
                float linear_acceleration_z = atof(tempresult[10].c_str());
                float distance_x = atof(tempresult[11].c_str());
                float distance_y = atof(tempresult[12].c_str());
                float distance_z = atof(tempresult[13].c_str());

                q.setW(quaternion_w);
                q.setX(quaternion_x);
                q.setY(quaternion_y);
                q.setZ(quaternion_z);
                q.normalize();

                // angular_velocity_x /= 60;
                // angular_velocity_y /= 60;
                // angular_velocity_z /= 60;

                imu.orientation.w = q.w();
                imu.orientation.x = q.x();
                imu.orientation.y = q.y();
                imu.orientation.z = q.z();

                imu.linear_acceleration.x = linear_acceleration_x;
                imu.linear_acceleration.y = linear_acceleration_y;
                imu.linear_acceleration.z = linear_acceleration_z;

                imu.angular_velocity.x = angular_velocity_x;
                imu.angular_velocity.y = angular_velocity_y;
                imu.angular_velocity.z = angular_velocity_z;

                // transformStamped.transform.translation.x = distance_x;
                // transformStamped.transform.translation.y = distance_y;
                // transformStamped.transform.translation.z = distance_z;

                // transformStamped.transform.translation.x = 0;
                // transformStamped.transform.translation.y = 0;
                // transformStamped.transform.translation.z = 0;

                // transformStamped.transform.rotation.w = q.w();
                // transformStamped.transform.rotation.x = q.x();
                // transformStamped.transform.rotation.y = q.y();
                // transformStamped.transform.rotation.z = q.z();

                if (id == '0')
                {
                    imu0 = imu;
                    imu0.header.frame_id = "imu0";
                    imu_pub0.publish(imu0);
                }
                else if (id == '1')
                {
                    imu1 = imu;
                    imu1.header.frame_id = "imu1";
                    imu_pub1.publish(imu1);
                }
                else if (id == '2')
                {
                    imu2 = imu;
                    imu2.header.frame_id = "imu2";
                    imu_pub2.publish(imu2);
                }
             }
            }
            // br.sendTransform(transformStamped);
            // br.sendTransform(tf::StampedTransform(transform, ros_cloud.header.stamp, "map", "map_child"));
            cnt++;
        }
        loop_rate.sleep();
    }
    return 0;
}