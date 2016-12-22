// ===================================
// <copyright>Huang Lab, UCSD</copyright>
// <author>Eric Roller</author>
// <email>mailto:eroller@gmail.com</email>
// <created>Tuesday, February 15, 2011</created>
// <lastedit>Tuesday, February 15, 2011</lastedit>
// ===================================
#define NOMINMAX
#include "ImageRegion.h"
#include "Utils.h"
#include <limits>

using namespace std;

ImageRegion ImageRegion::select(){
	char c;
	ImageRegion ret;
	cout<<"Please choose a region"<<endl;
		cout<<"0: Full Image"<<endl;
		cout<<"1: Quadrant 1"<<endl;
		cout<<"2: Quadrant 2"<<endl;
		cout<<"3: Quadrant 3"<<endl;
		cout<<"4: Quadrant 4"<<endl;
		cout<<"5: Center Quadrant"<<endl;
		cout<<"6: Custom"<<endl;
		cin>>c;
		std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
		if (c=='0'||c=='1'||c=='2'||c=='3'||c=='4'||c=='5')
			ret=ImageRegion(toInt(c));//implicit assignment operator should suffice
		else{//			case '6':{
				double d;
				cout<<"enter minX"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				ret.minX=d;
				cout<<"enter minY"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				ret.minY=d;
				cout<<"enter maxX"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				ret.maxX=d;
				cout<<"enter maxY"<<endl;
				cin>>d;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				ret.maxY=d;
				cout<<"enter name [Custom]"<<endl;
				string s="Custom";
				cin>>s;
				std::cin.ignore(std::numeric_limits<streamsize>::max(), '\n');
				ret.name=s;
		
	}
		return ret;
}

ImageRegion::ImageRegion(int imageRegion){
	switch(imageRegion){
			case FULL:
				minX=0;
				minY=0;
				maxX=1;
				maxY=1;
				name="FULL";
				break;
			case Q1:
				minX=.5;
				minY=0;
				maxX=1;
				maxY=.5;
				name="Q1";
				break;
			case Q2:
				minX=0;
				minY=0;
				maxX=.5;
				maxY=.5;
				name="Q2";
				break;
			case Q3:
				minX=0;
				minY=.5;
				maxX=.5;
				maxY=1;
				name="Q3";
				break;
			case Q4:
				minX=.5;
				minY=.5;
				maxX=1;
				maxY=1;
				name="Q4";
				break;
			case CENTER:
				minX=.25;
				minY=.25;
				maxX=.75;
				maxY=.75;
				name="CENTER";
				break;
	}
}
