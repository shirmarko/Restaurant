#include "../include/Customer.h"
#include <vector>
#include <iostream>
using namespace std;

Customer::Customer(std::string c_name, int c_id) : name(c_name), id(c_id) {}

std::string Customer::getName() const {
    return name;
}

int Customer::getId() const {
    return id;
}

Customer::~Customer() = default;


//VegetarianCustomer
//This is a customer that always orders the vegetarian dish with the
//smallest id in the menu, and the most expensive beverage
VegetarianCustomer::VegetarianCustomer(std::string name, int id): Customer(name, id){}

std::vector<int> VegetarianCustomer::order(const std::vector<Dish> &menu) {
    int maxBVGprice = 0;
    std::vector<int> ord(2); //creates a vector with 2 elements for first veg and most expensive bvg.
    ord[0] = -1; //flag for veg
    ord[1] = -1;
    if (!(menu.empty())) {
        for (unsigned int i = 0; i < menu.size(); i++) {
            if (menu[i].getType() == VEG && ord[0] == -1)
                ord[0] = menu[i].getId();
            else if (menu[i].getType() == BVG && menu[i].getPrice() > maxBVGprice) {
                ord[1] = menu[i].getId();
                maxBVGprice = menu[i].getPrice();
            }
        }
    }
    return ord;
}

std::string VegetarianCustomer::toString() const {
    return (" " + this->getName() + ",veg");
}

Customer* VegetarianCustomer::clone() {
    return (new VegetarianCustomer(*this));
}

//CheapCustomer
//This is a customer that always orders the cheapest dish in the menu. This
//customer orders only once
CheapCustomer::CheapCustomer(std::string name, int id):Customer(name, id), orderedOnce(false){};

std::vector<int> CheapCustomer::order(const std::vector<Dish> &menu){
    std::vector<int> ord(1); //creates a vector with the cheapest dish
    if (menu.empty()){
        ord[0] = -1;
        return ord;
    }
    if (!orderedOnce) {
        int minPrice = menu[0].getPrice();
        ord[0] = 0; //flag for veg
        for (unsigned int i = 1; i < menu.size(); i++)
            if (menu[i].getPrice() < minPrice) {
                ord[0] = menu[i].getId();
                minPrice = menu[i].getPrice();
            }
        orderedOnce = true;
    }
    else
        ord[0] = -1;
    return ord;
}

std::string CheapCustomer::toString() const{
    return (" " + this->getName() + ",chp");
}

Customer* CheapCustomer::clone() {
    return (new CheapCustomer(*this));
}

//SpicyCustomer
//This is a customer that orders the most expensive spicy dish in the menu.
//For further orders, he picks the cheapest non-alcoholic beverage in the menu. The order might
//be equal to previous orders
SpicyCustomer:: SpicyCustomer(std::string name, int id): Customer(name, id),numOfOrder(0),cheapestBVG(-1){};
std::vector<int> SpicyCustomer::order(const std::vector<Dish> &menu){
    std::vector<int> ord(1); //creates a vector with the most expensive spc dish or cheapest BVG
    if (menu.empty()){
        ord[0] = -1;
        return ord;
    }
    if(numOfOrder == 0){
        int maxSPCprice = 0;
        ord[0] = -1;
        for (unsigned int i=0; i<menu.size(); i++){
            if (menu[i].getType() == SPC && menu[i].getPrice() > maxSPCprice){
                ord[0] = menu[i].getId();
                maxSPCprice = menu[i].getPrice();
            }
        }
        if (ord[0] != -1 )
            numOfOrder++;
    }
    else if(numOfOrder == 1){
        int minBVGPrice = -1;
        ord[0] = -1;
        for (unsigned int i = 0; i < menu.size(); i++)
            if (menu[i].getType() == BVG)
                if (minBVGPrice == -1 || menu[i].getPrice() < minBVGPrice) {
                    minBVGPrice = menu[i].getPrice();
                    ord[0] = menu[i].getId();
                }
        cheapestBVG=ord[0];
        numOfOrder++;
    }
    else
        ord[0] = cheapestBVG;
    if (numOfOrder==2)
        ord[0] = cheapestBVG;
    return ord;
}

std::string SpicyCustomer::toString() const{
    return (" " + this->getName() + ",spc");
}

Customer* SpicyCustomer::clone() {
    return (new SpicyCustomer(*this));
}


//AlchoholicCustomer
//This is a customer who only orders alcoholic beverages. He starts with
//ordering the cheapest one, and in each further order, he picks the next expensive alcoholic
//beverage. After reaching the most expensive alcoholic beverage, he won't order again
AlchoholicCustomer::AlchoholicCustomer(std::string name, int id) :Customer(name,id),initvec(false){}

std::vector<int> AlchoholicCustomer::order(const std::vector<Dish> &menu){
    std::vector<int> ord(1);
    ord[0]=-1;
    if (!(menu.empty())) {
        if (!initvec) {//first time for the ALC costumer to order
            for (unsigned int i = 0; i < menu.size(); i++) {
                if (menu[i].getType() == ALC)
                    alcoDish.push_back(menu[i]);
            }
            if (alcoDish.size() != 0)
                initvec = true;
        }
        if (initvec) {
            int min = -1;
            int indexMin = -1;
            if (alcoDish.size() > 0) {
                min = alcoDish[0].getPrice();
                ord[0] = alcoDish[0].getId();
                indexMin = 0;
            }
            for (unsigned int i = 1; i < alcoDish.size(); i++) {
                if (alcoDish[i].getPrice() < min) {
                    min = alcoDish[i].getPrice();
                    ord[0] = alcoDish[i].getId();
                    indexMin = i;
                }
            }
            std::vector<Dish> vec;
            for (unsigned int i = 0; i < alcoDish.size(); i++) {
                if (i != unsigned(indexMin))
                    vec.push_back(alcoDish[i]);
            }
            alcoDish.clear();
            for (unsigned int i = 0; i < vec.size(); i++) {
                alcoDish.push_back(vec[i]);
            }
        }
    }
    return ord;
}


std::string AlchoholicCustomer::toString() const {
    return (" " +this->getName() + ",alc");

}

Customer* AlchoholicCustomer::clone() {
    AlchoholicCustomer* op=new AlchoholicCustomer(*this);
    for (unsigned int i = 0; i < unsigned(alcoDish.size()); ++i) {
        op->alcoDish.push_back(Dish(alcoDish[i]));
    }
    return (op);
}
