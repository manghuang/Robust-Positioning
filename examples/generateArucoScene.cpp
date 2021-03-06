#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
/*
This file is to be used to generate a 2d floor environment with ArUco markers to be used
for triangulation evaluation

Specify:
    -Aruco dictionary size
    -Base scene and its physical width (x)
    for each aruco marker
        specify coordinate, rotation, physical size, code number
*/

/*
TODO:
    - specify output file name upon code run
    - Read parameters from file??
    - Make proper cpp+hpp file to src directory
*/
struct marker{
    int id;                         //Must be within range of the chosen dictionary
    float x;                        //Meter
    float y;                        //Meter
    float size;                     //Meter
    float rotation;                 //Degrees
    marker(int id_, float x_, float y_, float size_, float rotation_){
        id=id_;
        x=x_;
        y=y_;
        size=size_;
        rotation=rotation_;
    }
};
std::vector<std::string> parse(std::string line){
    char delim = ',';
    std::vector<std::string> parsed;
    std::string::iterator it = line.begin();
    while(it!=line.end()){
        std::string word;
        while(it!=line.end()){
            if(isspace(*it)){it++;}//Remove leading whitespaces
            else{break;}
        }
        while(it!=line.end()){
            if(*it != delim){
                word+=*it;//Append the char to the temporary string
                it++;
            }//Go through until deliminator
            else{it++;
                break;}
        }
        parsed.push_back(word);//Push back the parsed word onto the return vector
    }
    return parsed;
}
/*Takes a path and reads the file into two vectors, vector<int> IDs and vector<mat> coordinates
 * Ignores any LEADING whitespaces and rows not containing exactly 4 data fields. delimiter is char ','
 * Each anchor shall be specified as id,x,y,z\n (Whit optional whitespaces after any comma)
 */
int readToDataBase(std::string path,std::vector<int>& IDs, std::vector<cv::Mat_<float>>& coordinates){
    std::string line;
    std::string delim = ",";
    std::ifstream file;
    file.open(path);
    float offsetX = 0.5;
    float offsetY = 0.5;
    std::cout << "GenerateArucoScene. Note that there is an offset of 0.5 to all x and y directions" << std::endl;
    if(file.is_open()){
         while(getline(file,line)){
            std::vector<std::string> parsed = parse(line);
            if(parsed.size()==4){//Disregard any lines that are not exactly 4 elements long
                int id      =   std::stoi(parsed[0]);
                float x     =   std::stof(parsed[1]);
                float y     =   std::stof(parsed[2]);
                float z     =   std::stof(parsed[3]);
                cv::Mat_<float> coord = cv::Mat_<float>::zeros(3,1);
                coord(0,0) = x + 0.5;
                coord(0,1) = y + 0.5;
                coord(0,2) = z;
                IDs.push_back(id);
                coordinates.push_back(coord);
            }
         }
    }else{return 0;}
    file.close();
    return 1;
}

int createArucoScene(cv::Mat& baseScene,float sceneWidth,
                        int dictType,                                           //As specified in aruco. Ex. cv::aruco::DICT_4X4_50
                        std::vector<marker> markers){
    if(baseScene.type() != 16){std::cerr << "Input image must be of type 8UC3" << std::endl;return 0;}
    //Some settings..
    int borderBits = 1;
    //Create dictionary
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(dictType);
    float resolution = ((float) baseScene.cols)/sceneWidth;  //Resolution expressed as pixels/meter


    int counter = 0;
    for(marker M:markers){
        //Create marker
        std::cout << "Marker nmbr: " << M.id<< std::endl;
        cv::Mat marker; //init marker mat
        float sidePixels = resolution*M.size;                                   //Marker size expressed in pixles
        dictionary->drawMarker(M.id,(int)sidePixels,marker,borderBits);         //Construct marker
        cv::cvtColor(marker,marker,CV_GRAY2BGR);                                //Convert marker to 3-channel 8 bit image
        //Rotate marker
        cv::RotatedRect rec = cv::RotatedRect(cv::Point2f(sidePixels/2,sidePixels/2),cv::Size2f(sidePixels,sidePixels),M.rotation);
        float w =(float) rec.boundingRect().width;   //Width of the minimal resulting bounding rect [pixles]
        float h =(float) rec.boundingRect().height;  //Height of the minimal resulting bounding rect [pixles]
        if((M.x*resolution-w/2)<0 || (M.x*resolution+w/2)>baseScene.cols || (M.y*resolution-h/2)<0 || (M.y*resolution+h/2)>baseScene.rows){std::cerr <<" Marker "<< counter << " placed too close too edge" << std::endl;return 0;} //Safeguard
        cv::Mat rot =  cv::getRotationMatrix2D(cv::Point2f(w/2,h/2),M.rotation,1);//rotation matrix around marker center
        int vert = (int) (h-sidePixels)/2;           //Border thickness vertical
        int hori = (int) (w-sidePixels)/2;           //Border thickness horizontal
        cv::Mat rotated;                             //container for rotated marker
        cv::copyMakeBorder(marker,rotated,vert,vert,hori,hori,0,cv::Scalar(0,0,0));   //Copy to marker to new mat with necessary border
        cv::warpAffine(rotated,rotated,rot,cv::Size2f(w,h),cv::WARP_INVERSE_MAP,cv::BORDER_CONSTANT,cv::Scalar(0,0,0));//Rotate the new marker image
        //Create mask
        cv::Mat mask = cv::Mat::zeros(rotated.size(),CV_8UC1);                        //Create mask and fill with zeros
        cv::Point2f vertices_float[4];
        rec.points(vertices_float);                                                   //Get the vertices of the rotated rect
        cv::Point vertices[4];
        for(int i=0;i<4;i++){vertices[i] = vertices_float[i] + cv::Point2f(hori,vert);}//Convert to point and shift to correct coordinates
        cv::fillConvexPoly(mask,vertices,4,cv::Scalar(255,255,255),4,0);               //Set the area covered by the marker to 255
        //Copy rotated marker over to base scene using mask
        cv::Rect roi(M.x*resolution-w/2,M.y*resolution-h/2,w,h);
        cv::Mat subFrame = baseScene(roi);
        rotated.copyTo(baseScene(roi),mask);
        counter++;
    }

return 1;


}

/*Returns a linSpace sequence starting from start with length no of steps of size step
 *
 */
std::vector<float> linSpace(float start,float stop,float length){
    std::vector<float> path;
    float step = (stop-start)/length;
    for(float i=0;i<length;i++){
        path.push_back(step*i+start);
    }
    return path;
}
std::vector<float> sequence(float start,float step,float max){
    std::vector<float> path;
    float value = 0;
    float i = 0;
    while(value < max){
        value = step*i+start;
        path.push_back(value);
        i++;
    }
    return path;
}
int main(int argc, char** argv){
    // Read base scene image
    std::string scenePath = "Generated-dataSets/Scene/wood-floor-pattern.jpg";
    std::string anchorLocationPath = "Generated-dataSets/Scene/anchors.txt";
    cv::Mat baseScene = cv::imread(scenePath,cv::IMREAD_COLOR);     //Read the scene image as 8uC3
    cv::imshow("Base scene with markers",baseScene);
    cv::waitKey(0);
    std::vector<int> IDs;
    std::vector<cv::Mat_<float>> coordinates;
    readToDataBase(anchorLocationPath,IDs,coordinates);
    float amount = (float) IDs.size();

    std::vector<float> markerSizes=linSpace(0.14,0.20,amount);
    std::vector<float> rotations = linSpace(0,0,amount);


    std::vector<marker> markers;
    for(int i=0;i<amount;i++){
        markers.push_back(marker(IDs[i],coordinates[i](0,0),coordinates[i](1,0),markerSizes[i], rotations[i]));
    }

    float sceneWidth = 4;                                   //Scene width in meter
    createArucoScene(baseScene, sceneWidth, cv::aruco::DICT_4X4_50,markers);
    std::string filename = "Generated-dataSets/Tests/5-okt/baseScene.png";

    //Draw coordinate system
    cv::arrowedLine(baseScene,cv::Point(20,20),cv::Point(400,20),cv::Scalar(0,0,0),15,8,1,0.2);
    cv::arrowedLine(baseScene,cv::Point(20,20),cv::Point(20,400),cv::Scalar(0,0,0),15,8,1,0.2);
    cv::putText(baseScene,"X",cv::Point(150,110),cv::FONT_HERSHEY_SIMPLEX,3,cv::Scalar(0,0,0),5,8,false);
    cv::putText(baseScene,"Y",cv::Point(30,200),cv::FONT_HERSHEY_SIMPLEX,3,cv::Scalar(0,0,0),5,8,false);


    std::vector<int> compression_params;
    compression_params.push_back(cv::IMWRITE_PNG_STRATEGY_DEFAULT);
    cv::imwrite(filename,baseScene,compression_params);
    cv::imshow("Base scene with markers",baseScene);
    cv::waitKey(0);


    return 1;
}



//Texture image sources
//  http://tierraeste.com/wp-content/uploads/wood-floor-pattern-calculator-ideas-photos_771567.jpg
