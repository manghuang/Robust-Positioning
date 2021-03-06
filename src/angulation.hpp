#ifndef ANGULATION_H
#define ANGULATION_H

/*
    This class implements a framework for angulation from a database of anchors with known ID:s and 3d locations
    The angulation is calculated in 3d and azimuth, from the AZIPE algorithm. Pixel coordinates of anchors and their ID
    are taken as input. if enough anchors are present, full AZIPE will be calculated.
*/


namespace ang{
    const int NO_ANCHORS = 0;
    const int AZIPE_FAIL = 2; //Should be defined in azipe i guess? nor here
    const int AZIPE_SUCCESS = 3; //As above

    class angulation{
        cv::Mat_<float> K;                      //Camera matrix
        cv::Mat_<float> K_inv;                  //Inverted Camera matrix
        cv::Mat_<float> T;                      //uav to camera transformation matrix
        cv::Mat_<float> T_inv;                  //Inverted uav to camera transformation matrix. (=camera to uav in x-y-z order)
        float k1_barrel;                        //Distortion coefficients for barrel distortion
        float k2_barrel;
        float k3_barrel;
        int maxId;                              //Max size of database. Specified upon initiation
        int minAnchors;                         //Minimum amount of anchors to try angulation
        std::vector<bool> activeAnchors;        //Part of database specifying what IDs are known
        std::vector<cv::Mat_<int>> IDs;         //IDs of all known anchors
        std::vector<cv::Mat_<float>> dataBase;  //All known anchors and their coordinates are kept here
        std::vector<std::string> parse(std::string);//Parses a csv line into vector<string>
        int readToDataBase(std::string,std::vector<int>&, std::vector<cv::Mat_<float>>&);//Reads csv file into anchor database
    public:
        angulation(int,std::string);
        void setKmat(cv::Mat_<float>);          //Camera matrix
        void setTmat(cv::Mat_<float>);          //uav -to- camera frame transformation matrix
        void setDistortCoefficents(float,float,float);//Set k1,k2,k3 distortion coefficients
        int addAnchor(int,cv::Mat_<float>);     //Add an anchor to database (id and 3d coordinate)
        int maskOut(std::vector<cv::Mat_<float>>&, std::vector<cv::Mat_<float>>&,std::vector<bool>&);
        //int calculateAzipe(std::vector<cv::Mat_<float>>&, std::vector<cv::Mat_<float>>&,std::vector<bool>&,cv::Mat_<float>&,float&,float&,float&); //Take pixel coordinates and IDs of anchors and calculate AZIPE
        cv::Point2f unDistort(const cv::Point2f&);//Compensate for barrel distortion
        void pix2uLOS(const std::vector<cv::Point2f>&,std::vector<cv::Mat_<float>>&);//Converts image pixel coordinates to uLOS vectors in uav frame
        void pix2uLOS(const std::vector<std::vector<cv::Point2f>>&,std::vector<cv::Mat_<float>>&);//Overloaded version for corner locations of anchors instead of center locations
        int dataBase2q(const std::vector<int>&,std::vector<cv::Mat_<float>>&,std::vector<bool>&);//Retrieves Q vectors from database given the IDs
        void deRotateULOS(std::vector<cv::Mat_<float>>&,float,float);
        cv::Mat getXRot(float);
        cv::Mat getYRot(float);

        //int calculate(std::vector<std::vector<cv::Point2f>>&, std::vector<int>&,std::vector<bool>&,cv::Mat_<float>&,float&,float,float);// Overloaded. takes mean of pixel group first
        //int calculateQV(std::vector<cv::Point2f>&, std::vector<int>&,std::vector<bool>&,cv::Mat_<float>&,float&,float,float,std::vector<cv::Mat_<float>>,std::vector<cv::Mat_<float>>); //Take pixel coordinates and IDs of anchors and calculate AZIPE
        //int calculateQV(std::vector<std::vector<cv::Point2f>>&, std::vector<int>&,std::vector<bool>&,cv::Mat_<float>&,float&,float,float,std::vector<cv::Mat_<float>>,std::vector<cv::Mat_<float>>);// Overloaded. takes mean of pixel group first

    };
}
#endif
