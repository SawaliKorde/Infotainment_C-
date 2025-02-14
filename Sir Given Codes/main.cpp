//*****************copy Constructor*******************
// #include <iostream>
// using namespace std;
// class Car {
//  string brand;
// public:
//  Car(string b) : brand(b) {}
//  // Copy Constructor
//  Car(const Car &c) {
//  brand = c.brand;
//  cout << "Copy Constructor Called" << endl;
//  }
//  void display() {
//  cout << "Brand: " << brand << endl;
//  }
// };
// int main() {
//  Car car1("Tesla");
//  Car car2 = car1; // Copy constructor invoked
//  car2.display();
//  return 0;
// }

//****************************shallow copy******************
// #include <iostream>
// using namespace std;
// class Car {
//  int *mileage;
// public:
//  Car(int m) {
//  mileage = new int(m);
//  }
//  void display() {
//  cout << "Mileage: " << *mileage << endl;
//  }
//  ~Car() {
//  delete mileage;
//  }
// };
// int main() {
//  Car car1(15000);
//  Car car2 = car1; // Shallow copy (default)
//  car1.display();
//  car2.display();
//  return 0;
// }

//*********************************** Deep copy Constructor*********************************
// #include <iostream>
// using namespace std;
// class Car {
//  int *mileage;
// public:
//  Car(int m) {
//  mileage = new int(m);
//  }
//  // Deep Copy Constructor
//  Car(const Car &c) {
//  mileage = new int(*c.mileage);
//  }
//  void display() {
//  cout << "Mileage: " << *mileage << endl;
//  }
//  ~Car() {
//  delete mileage;
//  }
// };
// int main() {
//  Car car1(15000);
//  Car car2(car1); // Deep copy
//  car1.display();
//  car2.display();
//  return 0;
// }


// #include <iostream>
// using namespace std;
// class Point {
//  int x, y;
// public:
//  Point(int a, int b) : x(a), y(b) {}
//  // Overload + operator

//  Point operator+(const Point &p) {

//  return Point(x + p.x, y + p.y);
//  }
//  void display() {
//  cout << "(" << x << ", " << y << ")" << endl;
//  }
// };
// int main() {
//  Point p1(2, 3), p2(4, 5);
//  Point p3 = p1 + p2;
//  p3.display();
//  return 0;
// }

/// opretor overloding for =
// class MyClass {
//  int value;
// public:
//  MyClass(int v = 0) : value(v) {}
//  // Overload assignment operator
//  MyClass& operator=(const MyClass &obj) {
//  if (this != &obj) {
//  value = obj.value;
//  }
//  return *this;

//  }
//  void display() {
//  cout << "Value :" << value << endl;
// }
// };
// int main() {
// //  Point p1(2, 3), p2(4, 5);
// //  Point p3 = p1 ;
// //  p3.display();
//  MyClass p4(10);
// MyClass p5(15);
//  p4.display();
//  p5.display();
//  p4=p5;
//  p4.display();
//  p5.display();
//  return 0;
// }

//*************************** Simple Exeception Handling***********************************************

// #include <iostream>
// using namespace std;

// int main() {
// 	try {
// 		throw "An error occurred";
// 	} catch (const char* msg) {
// 		cout << "Caught exception: " << msg << endl;
// 	}
	
// 	return 0;
// }

///****************************************complex Error Handling******************************************


// #include <iostream>
// #include <stdexcept> 
// double divide(double numerator, double denominator) {
//     if (denominator == 0) {
//         throw std::invalid_argument("Division by zero is not allowed.");
//     }
//     return numerator / denominator;
// }
// int main() {
//     double num, denom, result;
//     try {
//         // Input numerator and denominator
//         std::cout << "Enter numerator: ";
//         std::cin >> num;
//         std::cout << "Enter denominator: ";
//         std::cin >> denom;
//         // Attempt division
//         result = divide(num, denom);
//         // Display result
//         std::cout << "Result: " << result << std::endl;
//     } 
//     catch (const std::invalid_argument& e) {
//         // Catch and handle division by zero
//         std::cerr << "Error: " << e.what() << std::endl;
//     } 
//     catch (...) {
//         // Catch any other unexpected exceptions
//         std::cerr << "An unexpected error occurred." << std::endl;
//     }
//     std::cout << "Program continues..." << std::endl;
//     return 0;
// }

