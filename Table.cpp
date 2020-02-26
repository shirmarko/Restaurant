#include <vector>
#include "../include/Customer.h"
#include "../include/Table.h"
#include <iostream>
using namespace std;

Table::Table(int t_capacity):numOfppl(0), capacity(t_capacity), open(false){};

Table::Table(const Table &other) { //copy constructor
    open=other.open;
    capacity=other.capacity;
    numOfppl=other.numOfppl;
    for(unsigned int i=0; i<other.customersList.size(); i++){
        customersList.push_back(other.customersList[i]->clone());
    }
    for(unsigned int i=0; i<other.orderList.size(); i++){
        OrderPair p(other.orderList[i].first, other.orderList[i].second);
        orderList.push_back(p);
    }
}
Table::~Table(){//destructor
    if (open) {
        for (unsigned int i = 0; i < customersList.size(); i++) {
            delete customersList[i];
        }
        customersList.clear();
        orderList.clear();
        open=false;
    }
}

Table & Table::operator=(const Table &other){//copy assignment operator
    if (this == &other)
        return *this;
    open=other.open;
    capacity=other.capacity;
    numOfppl=other.numOfppl;
    for(unsigned int i=0; i<other.customersList.size(); i++){
        customersList.push_back(other.customersList[i]->clone());
    }
    for(unsigned int i=0; i<other.orderList.size(); i++){
        OrderPair p(other.orderList[i].first, other.orderList[i].second);
        orderList.push_back(p);
    }
    return *this;
}

Table::Table(Table&& other){//move constructor
    open=other.open;
    capacity=other.capacity;
    numOfppl=other.numOfppl;
    for(unsigned int i=0; i<other.customersList.size(); i++){
        customersList.push_back(other.customersList[i]);
    }
    for(unsigned int i=0; i<other.orderList.size(); i++){
        orderList.push_back(other.orderList[i]);
    }
    other.customersList.clear();
    other.orderList.clear();
}

Table& Table::operator=(Table&& other){//move assignment operator
    if(this!=&other){
        open=other.open;
        capacity=other.capacity;
        numOfppl=other.numOfppl;
        for (unsigned int i=0; i<customersList.size(); i++){
            delete customersList[i];
        }
        customersList.clear();
        orderList.clear();
        for(unsigned int i=0; i<other.customersList.size(); i++){
            customersList.push_back(other.customersList[i]);
        }
        for(unsigned int i=0; i<other.orderList.size(); i++){
            orderList.push_back(other.orderList[i]);
        }
        other.customersList.clear();
        other.orderList.clear();
    }
    return *this;
}

int Table::getCapacity() const{
    return capacity;
};
void Table::addCustomer(Customer* customer){
    customersList.push_back(customer);
    numOfppl = numOfppl +1;
};
void Table::removeCustomer(int id){
    for(unsigned int i=0; i<customersList.size(); i++)
        if(customersList[i]->getId() == id){
            std::vector<Customer*> vec;
            for(unsigned int j=0; j<customersList.size(); j++) {
                if (i != j)
                    vec.push_back(customersList[j]);
                else {
                    customersList[j] = nullptr;
                }
            }
            customersList.clear();
            for (unsigned int k = 0; k < vec.size(); ++k) {
                customersList.push_back(vec[k]);
            }
        }
    numOfppl = numOfppl -1;
};
Customer* Table::getCustomer(int id){
    for(unsigned int i=0; i<customersList.size(); i++)
        if(customersList[i]->getId() == id)
            return customersList[i];

    return nullptr;//no such customer id
};
std::vector<Customer*>& Table::getCustomers(){
    return customersList;
};
std::vector<OrderPair>& Table::getOrders(){
    return  orderList;
};
void Table::order(const std::vector<Dish> &menu){

    for(unsigned int i=0; i<customersList.size(); i++){
        vector<int> ord = customersList[i]->order(menu);
        if (ord[0] != -1) { //if ord[0] == -1, it means that the customer doesnt order anything
            for (unsigned int j = 0; j < ord.size(); j++) {
                OrderPair a = OrderPair(customersList[i]->getId(), menu[ord[j]]);
                orderList.push_back(a);
                std::cout << customersList[i]->getName() << " ordered " << menu[ord[j]].getName() << endl;
            }
        }
    }
};
void Table::openTable(){
    open = true;

};
void Table::closeTable(){
    open = false;
    for (unsigned int i = 0; i < customersList.size(); i++)
        if (customersList[i] != nullptr) {
            delete customersList[i];
            customersList[i] = nullptr;
        }
    customersList.clear();
    orderList.clear();
};
int Table::getBill(){
    int sum = 0;
    for(unsigned int i=0; i<orderList.size(); i++){
        sum = sum + orderList[i].second.getPrice();
    }
    return sum;
};
bool Table::isOpen(){
    return open;
};
