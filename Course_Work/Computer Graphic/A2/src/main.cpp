#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>
// add some other header files you need

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;
    Eigen::Matrix4f rotate;
    rotate << 0, 0, 1, 0,
              0, 1, 0, 0,
              -1,0, 0, 0,
              0, 0, 0, 1;
//    view = translate * view;
    view = rotate * translate * view;
    // std::clog << "view" << std::endl << view << std::endl;  // check data

    return view;
}


Eigen::Matrix4f get_model_matrix(float rotation_angle, Eigen::Vector3f T, Eigen::Vector3f S, Eigen::Vector3f P0, Eigen::Vector3f P1)
{

    //Step 1: Build the Translation Matrix M_trans:
    Eigen::Matrix4f translate;
    translate << 1,0,0, T[0], 0, 1, 0, T[1], 0, 0, 1, T[2], 0, 0, 0, 1;
    //Step 2: Build the Scale Matrix S_trans:
    Eigen::Matrix4f scale;
    scale << S[0], 0, 0, 0,
             0, S[1], 0, 0,
             0, 0, S[2], 0,
             0, 0, 0, 1;
    //Step 3: Implement Rodrigues' Rotation Formula, rotation by angle theta around axix u, then get the model matrix
	// The axis u is determined by two points, u = P1-P0: Eigen::Vector3f P0 ,Eigen::Vector3f P1
    Eigen::Vector3f u = P1 - P0;
    // Create the model matrix for rotating the triangle around a given axis. // Hint: normalize axis first
    u.normalize();
    //Assume the rotation_angle is in degrees.
//     Eigen::Matrix4f to_xyz;
//     Eigen::Matrix4f back;
//     Eigen::Matrix4f origin_rotate;
     float rad = MY_PI / 180.0 * rotation_angle;
//     origin_rotate << cos(rad), -sin(rad), 0, 0,
//                      sin(rad), cos(rad), 0, 0,
//                      0, 0, 1, 0,
//                      0, 0, 0, 1;

	//Step 4: Use Eigen's "AngleAxis" to verify your Rotation
	Eigen::AngleAxis<float> rotation_vector(rad, u);
	Eigen::MatrixXf rotation_m;
	rotation_m = rotation_vector.toRotationMatrix();

    rotation_m.conservativeResize(4,4);
    rotation_m(3,3) = 1;
    rotation_m(0, 3) = 0;
    rotation_m(1, 3) = 0;
    rotation_m(2, 3) = 0;
    Eigen::Matrix4f model =  rotation_m * scale * translate;
	return model;
}



Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Implement this function
    float fov = eye_fov * MY_PI / 180;
    float l, r, b, t;
    t = tan(fov/2) * zFar;
    b = -t;
    l = aspect_ratio * t;
    r = -l;
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    // frustum -> cubic
    Eigen::Matrix4f to_canonical;
    to_canonical << 2/(r-l), 0, 0, -(r+l)/(r-l),
            0, 2/(t-b), 0, -(t+b)/(t-b),
            0, 0, 2/(zNear-zFar), -(zNear+zFar)/(zNear-zFar),
            0, 0, 0, 1;
    // orthographic projection
    Eigen::Matrix4f ortho_proj;
    ortho_proj << zNear, 0, 0, 0,
            0, zNear, 0, 0,
            0, 0, zNear + zFar, -zFar * zNear,
            0, 0, 1, 0;

    // squash all transformations
    Eigen::Matrix4f projection =  to_canonical * ortho_proj;
    // std::clog << "projection" << std::endl << projection << std::endl; //check

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 220;
    bool command_line = false;
    std::string filename = "result.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(1024, 1024);
    // define your eye position "eye_pos" to a proper position
    Eigen::Vector3f eye_pos(100, 100, 200);
    // define a triangle named by "pos" and "ind"
    std::vector<Eigen::Vector3f> pos = {
       Eigen::Vector3f(400,400,100),
       Eigen::Vector3f(600, 600, 100),
       Eigen::Vector3f(600,600,400)
    };
    std::vector<Eigen::Vector3i> ind = {
            Eigen::Vector3i (0,1, 2)
    };
    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    // added parameters for get_projection_matrix(float eye_fov, float aspect_ratio,float zNear, float zFar)
    Eigen::Vector3f S(1.25,1.25,1.25);
    Eigen::Vector3f T(200, 0, 0);
    Eigen::Vector3f P0(100, 0, 0);
    Eigen::Vector3f P1(200, 0, 0);
    float eye_fov = 60;
    float aspect_ratio = 1;
    float zNear = 500;
    float zFar = 1000;
    // Eigen::Vector3f axis(0, 0, 1);
    Eigen::Vector3f axis(1, 0, 0);

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, T, S, P0, P1));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle, T, S, P0, P1));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(1024, 1024, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
        std::clog << "angle: " << angle << std::endl;
    

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
