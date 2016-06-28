#include <vector>

extern int initPathDrawer();

template<typename T>
struct Point{
	T x, y, z;
	Point(const T& x_ = .0f, const T& y_ = .0f, const T& z_ = .0f){
		x = x_;
		y = y_;
		z = z_;
	}
};
typedef Point<int> Pointi;
typedef Point<float> Pointf;

class EncoderPathDrawer{
public:
	void setRegion(const int& length, const int& width);
	void getRegion(int& length, int& width){ length = regionLength; width = regionWidth; return; }
	void addPoint(const int& x, const int& y, const int&z);
	void addPoint(const Pointi& newPoint);
	void drawPath();
	void getCurrentPos(Pointi& curPos);
	void createPathPoint();
private:
	std::vector<Pointi> originalPath;			//real position in world coordinate system (mm)
	std::vector<Pointf> path;
	int regionLength, regionWidth;
};