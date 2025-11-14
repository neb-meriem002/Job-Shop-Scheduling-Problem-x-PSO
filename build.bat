set main=%1

cls

del %main%.exe

g++ -std=c++26 -O3 -o %main%.exe %main%.cpp

%main%.exe

del %main%.exe