#include "../include/Action.h"
#include "../include/Restaurant.h"
#include "../include/Dish.h"
#include <vector>
#include <iostream>

extern Restaurant* backup;
using namespace std;

//BaseAction
BaseAction::BaseAction():errorMsg(""), status(PENDING){}

BaseAction::BaseAction(std::string errorMsg, ActionStatus status):errorMsg(errorMsg), status(status) {}//constructor with inputs

ActionStatus BaseAction::getStatus() const{
    return status;
}
void BaseAction::complete(){
    status = COMPLETED;
}
void BaseAction::error(std::string errorMsg){
    status = ERROR;
    this->errorMsg = errorMsg;
    std::cout <<  "Error: " << errorMsg << endl;
}
std::string BaseAction::getErrorMsg() const{
    return errorMsg;
}

std::string BaseAction::statusToString() const {
    if (getStatus() == COMPLETED)
        return " Completed";
    else
        return (" Error: " + getErrorMsg());
}

BaseAction::~BaseAction() = default;

//OpenTable
OpenTable::OpenTable(int id, std::vector<Customer *> &customersList): BaseAction(), tableId(id), customers(customersList){} //check **&&

OpenTable::OpenTable(const OpenTable &other):BaseAction(other.getErrorMsg(), other.getStatus()), tableId(other.tableId){//copy constructor
    for (unsigned int i = 0; i < unsigned(other.customers.size()); ++i) {
        customers.push_back(other.customers[i]->clone());
    }
}

void OpenTable::act(Restaurant &restaurant){ //delete customers that were not allowed to join table because it was already open!!!
    if ((restaurant.getNumOfTables() < tableId) || restaurant.getTable(tableId)->isOpen())
        error("Table does not exist or is already open");
    else
    if (unsigned(customers.size()) > unsigned(restaurant.getTable(tableId)->getCapacity()))
        error("***No enough places at the table***");
    else {
        Table* t=restaurant.getTable(tableId);
        t->openTable();
        for (unsigned int i=0 ; i<customers.size(); i++)
            t->addCustomer(customers[i]->clone());
        complete();
    }
    restaurant.setActionsLog(this);
}

std::string OpenTable::toString() const {
    std::string s = "open " + std::to_string(tableId);
    for (unsigned int i = 0; i < this->customers.size(); i++)
        s.append(this->customers[i]->toString());
    return (s + statusToString());
}

BaseAction* OpenTable::clone(){
    OpenTable* op=new OpenTable(*this);
    return (op);
}

OpenTable::~OpenTable(){//destructor
    for (unsigned int i = 0; i < customers.size(); ++i) {
        if(customers[i] != nullptr)
            delete customers[i];
        customers[i] = nullptr;
    }
    customers.clear();
}

//Order
Order::Order(int id):BaseAction(), tableId(id){}

void Order::act(Restaurant &restaurant){
    if ((restaurant.getNumOfTables() < tableId) || !(restaurant.getTable(tableId)->isOpen()))
        error("Table does not exist or is not open");
    else {
        restaurant.getTable(tableId)->order(restaurant.getMenu());
        complete();
    }
    restaurant.setActionsLog(this);

}
std::string Order::toString() const{
    return ("order " + std::to_string(tableId) + statusToString());
}
BaseAction* Order::clone(){
    return (new Order(*this));
}

//MoveCustomer
MoveCustomer::MoveCustomer(int src, int dst, int customerId): BaseAction(), srcTable(src),
                                                              dstTable(dst), id(customerId){}

void MoveCustomer::act(Restaurant &restaurant){ //close dst table if necessery
    if ((restaurant.getNumOfTables() < srcTable) || (restaurant.getNumOfTables() < dstTable) || //table id doesnt exists
        !(restaurant.getTable(srcTable)->isOpen()) || !(restaurant.getTable(dstTable)->isOpen()) || //tables exists but are not open
        (restaurant.getTable(srcTable)->getCustomer(id) == nullptr) || //customer is not at the source table
        (restaurant.getTable(dstTable)->getCapacity() - restaurant.getTable(dstTable)->getCustomers().size() <= 0)){// there is no space for the customer
        error("Cannot move customer");
    }
    else{
        restaurant.getTable(dstTable)->addCustomer(restaurant.getTable(srcTable)->getCustomer(id));//adds the new customer
        //move orders of the customer
        for (unsigned int i=0; i<unsigned(restaurant.getTable(srcTable)->getOrders().size()); i++){
            if (restaurant.getTable(srcTable)->getOrders()[i].first == id) { //we found order made by the customer
                OrderPair op(restaurant.getTable(srcTable)->getOrders()[i].first,
                             restaurant.getTable(srcTable)->getOrders()[i].second);//creates new same order
                restaurant.getTable(dstTable)->getOrders().push_back(op);  // add it to the source table order list

            }
        }
        //remove the old table orders
        vector<OrderPair> vec;
        for (unsigned int j = 0; j < restaurant.getTable(srcTable)->getOrders().size(); j++) {
            if (restaurant.getTable(srcTable)->getOrders()[j].first != id) {
                vec.push_back(restaurant.getTable(srcTable)->getOrders()[j]);
            }//else delete customer[i]
        }
        restaurant.getTable(srcTable)->getOrders().clear();
        for (unsigned int j = 0; j < vec.size(); j++) {
            restaurant.getTable(srcTable)->getOrders().push_back(vec[j]);
        }

        restaurant.getTable(srcTable)->removeCustomer(id);
        if (restaurant.getTable(srcTable)->getCustomers().size() == 0) {//checks if the src table is now empty
            restaurant.getTable(srcTable)->closeTable();
        }
        complete();
    }
    restaurant.setActionsLog(this);
}
std::string MoveCustomer::toString() const{
    std::string s = "move " +std::to_string(srcTable) + " " + std::to_string(dstTable) + " " + std::to_string(id);
    return (s +statusToString());
}
BaseAction* MoveCustomer::clone(){
    return (new MoveCustomer(*this));
}

//Close
Close::Close(int id):BaseAction(), tableId(id), bill(0){}

void Close::act(Restaurant &restaurant){
    if ((restaurant.getNumOfTables() < tableId) || !(restaurant.getTable(tableId)->isOpen()))
        error("Table does not exist or is not open");
    else {
        bill = restaurant.getTable(tableId)->getBill();
        restaurant.getTable(tableId)->closeTable();
        complete();
        std::cout << "Table " + std::to_string(tableId) + " was closed. Bill " + std::to_string(bill) + "NIS" << endl;
    }
    if(restaurant.getOpen())
        restaurant.setActionsLog(this);
}
std::string Close::toString() const{
    std::string s = "close " + std::to_string(tableId);
    return (s +statusToString());
}
BaseAction* Close::clone(){
    return (new Close(*this));
}

//CloseAll
CloseAll::CloseAll():BaseAction(){}

void CloseAll::act(Restaurant &restaurant) {
    restaurant.closeRestaurant();
    for (unsigned int i = 0; i < unsigned(restaurant.getNumOfTables()); i++) { //close all open tables
        if (restaurant.getTable(i)->isOpen()) {
            Close c(i);
            c.act(restaurant);
        }
    }
    complete();
    restaurant.setActionsLog(this);
}
std::string CloseAll::toString() const{
    return ("closeall" +statusToString());
}
BaseAction* CloseAll::clone(){
    return (new CloseAll(*this));
}

//PrintMenu
PrintMenu::PrintMenu():BaseAction(){};
void PrintMenu::act(Restaurant &restaurant){
    for (unsigned int i=0; i<restaurant.getMenu().size(); i++){
        Dish &ds = restaurant.getMenu()[i];
        std::string s;
        if (ds.getType() == VEG)
            s = " VEG ";
        if (ds.getType() == SPC)
            s = " SPC ";
        if (ds.getType() == BVG)
            s = " BVG ";
        if (ds.getType() == ALC)
            s = " ALC ";
        std::cout << ds.getName() << s << std::to_string(ds.getPrice()) << "NIS" << endl;
    }
    complete();
    restaurant.setActionsLog(this);
}
std::string PrintMenu::toString() const{
    return ("menu" + statusToString());
}

BaseAction* PrintMenu::clone(){
    return (new PrintMenu(*this));
}

//PrintTableStatus
PrintTableStatus::PrintTableStatus(int id):BaseAction(),tableId(id){}
void PrintTableStatus::act(Restaurant &restaurant){
    if (!restaurant.getTable(tableId)->isOpen())
        std::cout << "Table " + std::to_string(tableId) + " status: closed" << endl;
    else{
        std::cout << "Table " + std::to_string(tableId) + " status: open" << endl;
        std::cout << "Customers:" << endl;
        for (unsigned int i = 0; i < restaurant.getTable(tableId)->getCustomers().size(); ++i) {
            Customer *c = restaurant.getTable(tableId)->getCustomers()[i];
            std::cout << std::to_string(c->getId()) << " " << c->getName() << endl;
        }
        std::cout << "Orders:" << endl;
        for (unsigned int i = 0; i < restaurant.getTable(tableId)->getOrders().size(); i++){
            OrderPair &op = restaurant.getTable(tableId)->getOrders()[i];
            std::cout << op.second.getName() << " " << std::to_string(op.second.getPrice()) << "NIS " << std::to_string(op.first) << endl;
        }
        std::cout << "Current Bill: " << std::to_string(restaurant.getTable(tableId)->getBill()) << "NIS" << endl;
    }
    complete();
    restaurant.setActionsLog(this);
}
std::string PrintTableStatus::toString() const{
    return ("status " + std::to_string(tableId) + statusToString());
}

BaseAction* PrintTableStatus::clone(){
    return (new PrintTableStatus(*this));
}

//PrintActionsLog
PrintActionsLog::PrintActionsLog():BaseAction(){}
void PrintActionsLog::act(Restaurant &restaurant){
    for(unsigned int i=0; i<restaurant.getActionsLog().size(); i++){
        std::cout << restaurant.getActionsLog()[i]->toString() << endl;
    }
    complete();
    restaurant.setActionsLog(this);
}
std::string PrintActionsLog::toString() const{
    return ("log" +statusToString());
}

BaseAction* PrintActionsLog::clone(){
    return (new PrintActionsLog(*this));
}

//BackupRestaurant
BackupRestaurant::BackupRestaurant():BaseAction(){}
void BackupRestaurant::act(Restaurant &restaurant) {
    if (backup == nullptr)
    backup = new Restaurant(restaurant);
    else
        *backup = restaurant;
    complete();
    restaurant.setActionsLog(this);
}
std::string BackupRestaurant::toString() const{
    return ("backup" +statusToString());
}

BaseAction* BackupRestaurant::clone(){
    return (new BackupRestaurant(*this));
}

//RestoreResturant
RestoreResturant::RestoreResturant():BaseAction(){}
void RestoreResturant::act(Restaurant &restaurant){
    if (backup == nullptr) {
        error("No backup available");
    }
    else {
        restaurant = (*backup);
        complete();
    }
    restaurant.setActionsLog(this);
}
std::string RestoreResturant::toString() const{
    return ("restore" +statusToString());
}
    
BaseAction* RestoreResturant::clone(){
    return (new RestoreResturant(*this));
}
