#ifndef SIMULATEPOSE_H
#define SIMULATEPOSE_H

//namespace sp{

class simulatePose{
public:
    cv::Mat baseScene;
    cv::Mat_<float> K;                      //K mat that satisfies specifications
    cv::Mat_<float> K_inv;                  //Inverse of K mat that satisfies specifications
    cv::Mat_<float> R1;                     //Base rotation of CAMERA
    cv::Mat_<float> t1;                     //Base coordinate of CAMERA
    cv::Mat_<float> T_z;                    //T matrix. Camera orientation as related to UAV orientation
    float d;                                //d in base plane definition.
    simulatePose(void);                     //Constructor that sets default values to attributes
    void setBaseScene(int,int,int);         //Defines default chessboard pattern as base scene
    void setBaseScene(cv::Mat);  //Defines provided mat as base scene (And draws the global coordinate system?)
    int setParam(std::string,float);        //Define parameters one by one. can be given in any order
    int setParam(void);
    int init(int);                          //Initialize physical simulation environment with the specified configuration
    cv::Mat uav2BasePose(std::vector<float>,std::vector<float>);//Recalculates uav pose to camera pose in relation to base pose
    cv::Mat getWarpedImage(std::vector<float>,std::vector<float>);              //Perform perspective warp of base scene
    cv::Mat getWarpedImage(cv::Mat_<float>,cv::Mat_<float>);//Overloaded version with other arguments
    cv::Mat getXRot(float);            //Defines rotation matrices
    cv::Mat getYRot(float);
    cv::Mat getZRot(float);
private:
    float cx;                               //Pixel offset in x in K mat
    float cy;                               //Pixel offset in y in K mat
    float z_base;                           //Base coordinate of CAMERA 1 in z-direction
    float y_base;                           //Base coordinate of CAMERA 1 in y-direction
    float x_base;                           //Base coordinate of CAMERA 1 in x-direction
    float yaw_base;                         //Base yaw of CAMERA
    float pitch_base;                       //Base pitch of CAMERA
    float roll_base;                        //Base roll of CAMERA
    cv::Mat_<float> v;                      //base plane normal. (as expressed in camera 1:s coordinate system) - always same per definition
    float N;                                //Camera resolution in x-direction [pixles]
    float sceneWidth;                       //Scene width in x-direction       [m]
    float angle;                            //Viewing angle of camera in x-direction
    float f_m;                              //Focal length of camera in meter  [m]
    float f_p;                              //Focal length of camera in pixles [-]
    float gamma;//NECESSARY? Pixel density of camera [pixles/meter]
    std::vector<std::vector<std::string>> validParameters;//The parameters that are valid to use to calculate the simulation camera
    std::vector<std::vector<int>> validConfigurations;      //The configurations of parameters that are valid to set up the complete simulation environment
    void setBasePose(void);                    //Define the base pose of the UAV in global coordinate system
    cv::Mat coord2CMat(float, float, float);   //Vector<float> to column Mat_<float>
    cv::Mat getChessboard(int,int,int);     //Creates a chessboard image of specified size
    cv::Mat getTotRot(float, float, float); //Defines total rotation matrix
    int setKMat(void);  //Uses some given parameters? creates a suitable K mat
};
//}
#endif
