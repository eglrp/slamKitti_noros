
#include <scanRegistration.hpp>
#include <laserOdometry.hpp>
#include <laserMapping.hpp>
#include <transformMaintenance.hpp>
#include <loam_velodyne/common.h>


#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/common/point_operators.h>
#include <pcl/common/io.h>
#include <pcl/search/organized.h>
#include <pcl/search/octree.h>
#include <pcl/search/kdtree.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/filters/conditional_removal.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/segmentation/extract_clusters.h>
#include <pcl/surface/gp3.h>
#include <pcl/io/vtk_io.h>
#include <pcl/filters/voxel_grid.h>
#include<pcl/visualization/cloud_viewer.h>



using namespace pcl;
using namespace std;


 void getFiles(string path, vector<string>& files){
            
            struct dirent *ptr;    
            DIR *dir;
            dir=opendir(path.c_str());
            int i = 0;
            while((ptr=readdir(dir))!=NULL)
            {
                string p;
            //跳过'.'和'..'两个目录
                if(ptr->d_name[0] == '.'){
                    
                    continue;}
                printf("%s is ready...\n",ptr->d_name);
                // sprintf(files[i],"./one/%s",ptr->d_name);
                files.push_back(p.assign(path).append("/").append(ptr->d_name));

             
          
            }
            closedir(dir);





    }

void
uncurvePointcloud(pcl::PointCloud<PointXYZI> &input, pcl::PointCloud<PointXYZI> &output)
{

          

           pcl::PointCloud<pcl::PointXYZ>::Ptr cloud (new pcl::PointCloud<pcl::PointXYZ>);
           
           pcl::copyPointCloud(input, *cloud);

           
           std::vector<int> inliers;

           // created RandomSampleConsensus object and compute the appropriated model

           pcl::SampleConsensusModelPlane<pcl::PointXYZ>::Ptr
             model_p (new pcl::SampleConsensusModelPlane<pcl::PointXYZ> (cloud));


               Eigen::VectorXf planePara;



            

                   pcl::RandomSampleConsensus<pcl::PointXYZ> ransac (model_p);
                   ransac.setDistanceThreshold (.01);
                   ransac.computeModel();
                   ransac.getInliers(inliers);
                   ransac.getModelCoefficients(planePara);
                   pcl::PointXYZI tmpPoint;
					
                  for (int i = 0;i<input.size();i++)
                  {
                      tmpPoint.x = input.points[i].x;
                      tmpPoint.y = input.points[i].y;
                      tmpPoint.z = input.points[i].z;
                      tmpPoint.intensity = tmpPoint.z-(-planePara(3)-planePara(0)*tmpPoint.x-planePara(1)*tmpPoint.y)/planePara(2);
                      // std::cerr<<tmpPoint.intensity<<std::endl;
                      if (fabs(tmpPoint.intensity)<0.1){
						  tmpPoint.z = (-planePara(3)-planePara(0)*tmpPoint.x-planePara(1)*tmpPoint.y)/planePara(2);
						//   std::cerr<<tmpPoint.intensity<<std::endl;						  
					  }
                          
                      
					  tmpPoint.intensity =input.points[i].intensity;

                    output.push_back(tmpPoint);        
                   }
             


 }




int main(int argc, char* args[]){
	
	
	vector<string> files;
	getFiles(args[1], files);
	
	std::ofstream outfile;
	std::ofstream outfile2;
	std::ofstream outfile3;
	int i;
	char filename[50] = {0};
	string p;

	outfile.open (p.assign(args[1]).append("/../pose.txt").c_str());
	outfile2.open (p.assign(args[1]).append("/../pose_bef.txt").c_str());
	outfile3.open (p.assign(args[1]).append("/../mapLog.txt").c_str());
	int num_id=0;
 	
    pcl::visualization::CloudViewer viewer ("mapping viewer");  
 


    scanRegistration scanner;
	laserOdometry odometrier;
	laserMapping mapper;
	transformMainTenance mainer;

		scanRegistrationBack scanValueBack;
        laserOdometryBack odometryValueBack;
		laserMappingBack mappingBackValue;
		maintenanceBack maintenanceValueBack;

	 
	for (i=0;i<files.size() ;i++){
		
		if (i<10) 
			snprintf(filename,sizeof(filename),"%s/00000%d.bin",args[1],i);
			else if (10<=i &&i<100)
			snprintf(filename,sizeof(filename),"%s/0000%d.bin",args[1],i);
			else if (100<=i && i <1000)
			snprintf(filename,sizeof(filename),"%s/000%d.bin",args[1],i); 
			else if (1000<=i && i <10000)
			snprintf(filename,sizeof(filename),"%s/00%d.bin",args[1],i); 
			
		
		

		// load point cloud
		fstream input(filename, ios::in | ios::binary);
		if(!input.good()){
			cerr << "Could not read file: " << filename<< endl;
			exit(EXIT_FAILURE);
		}
		input.seekg(0, ios::beg);

		pcl::PointCloud<PointXYZI> points ;
        points.clear();
		int j;
		for (j=0; input.good() && !input.eof(); j++) {
			PointXYZI point;
			input.read((char *) &point.x, 3*sizeof(float));
			input.read((char *) &point.intensity, sizeof(float));
			points.push_back(point);
		}

		input.close();
        
        // pcl::io::savePCDFileASCII ("/home/xiesc/test.pcd", points);
        
		cout << "Read KTTI point cloud with " << j << " belong to " <<filename<<","<<i <<endl;
        
		
			
			// pcl::PointCloud<pcl::PointXYZI> uncurvpointcloud ;
		    // uncurvePointcloud(points, uncurvpointcloud);
			// points = uncurvpointcloud;
		//  pcl::io::savePCDFileASCII ("/home/xiesc/test.pcd", points);

	
		

        scanValueBack = scanner.laserCloudHandler(points);
	

		odometryValueBack = odometrier.laserOdometryHandler(scanValueBack);
		// if (i%5==0 || i<10){
		// 	mappingBackValue = mapper.laserMappingHandler(odometryValueBack);
		// 	outfile3<<mappingBackValue.transformAftMapped[0]<<" "<<mappingBackValue.transformAftMapped[1]<<" "<<mappingBackValue.transformAftMapped[2]<<" "<<mappingBackValue.transformAftMapped[3]<<" "<<mappingBackValue.transformAftMapped[4]<<" "<<mappingBackValue.transformAftMapped[5]<<endl;

		// }
		mappingBackValue = mapper.laserMappingHandler(odometryValueBack);
		
		// maintenanceValueBack = mainer.transformRecorder(odometryValueBack,mappingBackValue);






        // std::stringstream filename;
        // filename << "/home/xiesc/testpcd_map/"<<num_id<<".pcd";
        
        // pcl::io::savePCDFileASCII (filename.str(), *mappingBackValue.laserCloudSurround);

		// if (num_id>0){
		// std::stringstream filename2;
        // filename2 << "/home/xiesc/testpcd_second_frame/"<<num_id<<".pcd";
        
        // pcl::io::savePCDFileASCII (filename2.str(), *mappingBackValue.laserCloudFullRes);
		// }

     	num_id++;
		 viewer.showCloud(mappingBackValue.laserCloudSurround);  

		outfile<<mappingBackValue.transformAftMapped[0]<<" "<<mappingBackValue.transformAftMapped[1]<<" "<<mappingBackValue.transformAftMapped[2]<<" "<<mappingBackValue.transformAftMapped[3]<<" "<<mappingBackValue.transformAftMapped[4]<<" "<<mappingBackValue.transformAftMapped[5]<<endl;
		outfile2<<odometryValueBack.transformSum[0]<<" "<<odometryValueBack.transformSum[1]<<" "<<odometryValueBack.transformSum[2]<<" "<<odometryValueBack.transformSum[3]<<" "<<odometryValueBack.transformSum[4]<<" "<<odometryValueBack.transformSum[5]<<endl;
		outfile3<<maintenanceValueBack.transformMapped[0]<<" "<<maintenanceValueBack.transformMapped[1]<<" "<<maintenanceValueBack.transformMapped[2]<<" "<<maintenanceValueBack.transformMapped[3]<<" "<<maintenanceValueBack.transformMapped[4]<<" "<<maintenanceValueBack.transformMapped[5]<<endl;
    
		// // Save DoN features
		// writer.write<PointXYZI> (outfile, *points, false);
		
	}

//  	while (!viewer.wasStopped ())  
//   {  
//   }  




    
    
    return 0;


}