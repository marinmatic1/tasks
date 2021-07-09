#include "Main.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <string>
#include"image.h"
#include <ctime>


using namespace std;
struct Point {
	long double x;
	long double y;
	Point() {}
	Point(long double p1, long double p2) {
		x = p1;
		y = p2;
	}
};

struct Line {
	long double px1;
	long double py1;
	long double px2;
	long double py2;

	Line(long double p1, long double p2, long double p3, long double p4) {
		px1 = p1;
		py1 = p2;
		px2 = p3;
		py2 = p4;
	}
	Line(Point p1, Point p2) {
		px1 = p1.x;
		py1 = p1.y;
		px2 = p2.x;
		py2 = p2.y;
	}
	Line() {}
};

bool onSegment(Point p, Point q, Point r) {
	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)) return true;
	return false;
}

int orientation(Point p, Point q, Point r) {
	int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
	if (val == 0) return 0; //collinear
	return(val > 0) ? 1 : 2;
}

bool doIntersect(Point p1, Point q1, Point p2, Point q2) {
	int o1 = orientation(p1,q1,p2);
	int o2 = orientation(p1,q1,q2);
	int o3 = orientation(p2,q2,p1);
	int o4 = orientation(p2,q2,q1);

	if (o1 != o2 && o3 != o4) return true;
	Line l = { p1 ,p2 };
	if (o1 == 0 && onSegment(p1,p2,q1)) return true;

	l = { p1 ,q2 };
	if (o2 == 0 && onSegment(p1,q2,q1)) return true;

	l = { p2 ,p1 };
	if (o3 == 0 && onSegment(p2,p1,q2)) return true;

	l = { p2 ,q1 };
	if (o4 == 0 && onSegment(p2,q1,q2)) return true;

	return false;
}

int getPointIntersectedByLines(Line l1, Line l2, long double*  ipX, long double* ipY) {
	long double x1, y1, x2, y2, x3, y3, x4, y4, D, t, u;

	x1 = l1.px1; x2 = l1.px2; x3 = l2.px1; x4 = l2.px2;
	y1 = l1.py1; y2 = l1.py2; y3 = l2.py1; y4 = l2.py2;

	D = (x1 - x2) * (y3 - y4) - (y1 - y2)*(x3 - x4);
	if (D == 0)
		return 0; // collinear

	t = ((x1 - x3)*(y3 - y4) - (y1 - y3)*(x3 - x4))/D;
	u = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / D;

	if (t < 0 || t>1 || u < 0 || u >1 )
		return 0; // no intersect
	if (ipX != NULL) {
		*ipX = x1 + t * (x2 - x1);
		if (*ipX >= 1 || *ipX <= 0)
			return 0; // lines do intersect but out of bounds on x axix
	}
	
	if (ipY != NULL) {
		*ipY = y1 + t * (y2 - y1);
		if (*ipY >= 1 || *ipY <= 0)
			return 0; // lines do intersect but out of bounds on y axix
	}

	Point p1 = { l1.px1,l1.py1 }, q1 = { l1.px2,l1.py2 };
	Point p2 = { l2.px1,l2.py1 }, q2 = { l2.px2,l2.py2 };
	if (doIntersect(p1, q1, p2, q2)) return 1;

	//assumed intersect
	return 1;
}


vector<vector<short int>> calculatePixelsMap(vector<vector<Line>> lines, vector<vector<Point>> centerPixels, int n) {

	vector<vector<short int>> pixels(n, vector<short int>(n, 0)); // size of vector pixel collumns and rows is constant n, pixel rows and colls are initialized to be 0. 0 for black and 255 for white

	Line segment = {};
	long double limitPoint = 1;
	long double ipX, ipY;
	int counter;
	long int iterations = 0;
	for (int i = 0; i < centerPixels.size(); i++) {
		for (int j = 0; j < centerPixels.size(); j++) {
			segment.px1 = centerPixels[i][j].x;  // making a line out of mapped center of points and extreme end point with respect to x axis
			segment.py1 = centerPixels[i][j].y;
			segment.px2 = limitPoint;  // setting the extreme end point of x axis to the end, and that is always 1.
			segment.py2 = centerPixels[i][j].y;
			int t = 0;
			while (t < lines.size()) { // lines are regions
				int x = 0;
				counter = 0;
				while (x < lines[t].size()) { // lines[t] are ordered line segments which make a polygon, we check a single polygon at a time
					iterations++;
					if (getPointIntersectedByLines(lines[t][x], segment, &ipX, &ipY)) {       // segment from center pixel to extreme point is object segment.
						counter++;
					}
					x++;
					}
				t++;
				if (counter % 2 != 0) { //  a line intersects even number of times when it starting point is outside of the region, if it intersects odd number of times then it is inside. Visual proof
					pixels[i][j] = 255;
					break;  // checking a single polygon at a time lowers 
				}
			}
		}
	}
	std::cout << endl;
	std::cout << "It took:" << iterations << " number of iterations to complete.."<<endl;
	return pixels;
}

vector < vector<Line>> getRegionData(int* size) {
	std::ifstream data("Data.txt", std::ifstream::binary);
	std::vector<string> collection;
	string s = "";
	vector<long double> points;
	vector<Point> point;
	data.seekg(0, data.end);
	int length = data.tellg();
	data.seekg(0, data.beg);
	char* buffer = new char[length];
	std::cout << "Reading: " << length << " characters..." << endl;
	data.read(buffer, length);
	if (data) {
		for (int i = 0; i < length - 1; i++) {
			string str = "";
			if (buffer[i] == 'p' && buffer[i + 1] == 'o') collection.push_back("next");
			if (buffer[i] == 'z' && buffer[i + 1] == 'e') {
				int t = 6;
				while (buffer[i + t] != ',') {
					s = s + buffer[i + t];
					t++;
				}
			}
			if (buffer[i] == ':' && buffer[i] != '\n') {
				int t = 1;
				while (buffer[i + t] != ',' && buffer[i + t] != '\n') {
					if (buffer[i + t] != ' ') {
						str = str + buffer[i + t];
					}
					t++;
				}
				if (str != " ")
					collection.push_back(str);
			}
		}
	delete[] buffer;
	data.close();
}
	else
		std::cout << "File cannot be read! GetRegionData() function failed";
	if (!collection.empty()) {
		for (string s : collection) {
			try {
				long double d = stod(s);
				points.push_back(d);
			}
			catch (exception e) {
				if (s != "next") {
					collection.erase(remove(collection.begin(), collection.end(), s), collection.end());
				}
				else points.push_back(1000);
			}
		}
	}
	int current = 0;
	int next = 0;
	int polygonSize = 0;
	for (int i = 0; i < points.size() - 1; i = i + 2) {

		if (points[i] != 1000) {
			Point p = { points[i],points[i + 1] };
			point.push_back(p);
		}
		else {
			Point p = { 100,100 };
			point.push_back(p);
			i = i - 1;
			polygonSize++;
		}
	}
	int currentStart = 0;
	int nextRegion = 0;
	vector<vector<Line>> regionData(point.size());
	for (int i = 0;i < point.size();i++) {
		if (i + 1 >= point.size()) {
			Line l = { point[i],point[currentStart] };
			regionData[nextRegion].push_back(l);
			break;
		}
		if (point[i + 1].x != 100) {
			Line l = { point[i],point[i + 1] };
			regionData[nextRegion].push_back(l);
		}
		else {
			Line l = { point[i],point[currentStart] };
			regionData[nextRegion++].push_back(l);
			currentStart = i + 2;
			i++;
		}
	}
	try {
		*size = stoi(s);
	}
	catch (exception e) {
		std::cout << "Size impossible to get.." << endl;
		return regionData;
	}
	return regionData;
}

vector<vector<Point>> setCenterPixelsMap(int SIZE) {
	vector<vector<Point>> centerPixelsMap(SIZE, vector<Point>(SIZE));
	long double pixelSize = (long double)1 / SIZE;
	long double centerOfPixel = pixelSize / 2;
	long double yAxis = centerOfPixel;
	long double xAxis;
	// initializing a 2d vector made of values for central pixels
	for (int i = 0; i < centerPixelsMap.size();i++) {
		xAxis = centerOfPixel;
		for (int j = 0; j < centerPixelsMap.size(); j++) {
			centerPixelsMap[i][j].x = xAxis;
			centerPixelsMap[i][j].y = yAxis;
			xAxis = xAxis + pixelSize;
		}
		yAxis = yAxis + pixelSize;
	}
	return centerPixelsMap;
}





int main(string args[]) {
	float x, y;
	clock_t time_req;
	time_req = clock();

	int SIZE;
	vector<vector<Line>> regionData = getRegionData(&SIZE);
	vector<vector<Point>> centerPixelsMap = setCenterPixelsMap(SIZE);
	vector<vector<short int>> image = calculatePixelsMap(regionData, centerPixelsMap,SIZE);
	
	const int width = SIZE;
	const int height = SIZE;
	Image img(width, height);
	for (int j = 0; j < height;j++) {
		for (int i = 0; i < width; i++) {
			Color c = { (float)image[i][j], (float)image[i][j],(float)image[i][j] };
			img.SetColor(c, i, j);
		}
	}
	img.Export("image.bmp");
	time_req = clock() - time_req;
	std::cout << "Done!"<<endl;
	std::cout << "Program took: "<< (float)time_req / CLOCKS_PER_SEC << " seconds" <<endl;
	char c;
	std::cin >> c;
	return 0;
}
