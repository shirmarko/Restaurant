#include "../include/Restaurant.h"
#include "../include/Action.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

Restaurant::Restaurant() {}

Restaurant::Restaurant(const Restaurant &other){ //copy constructor
    open = other.open;

    for (unsigned int i=0; i<other.actionsLog.size(); i++){
        actionsLog.push_back(other.actionsLog[i]->clone());
    }
    for (unsigned int i=0; i<other.tables.size(); i++){
        Table *t = new Table(*(other.tables[i]));
        tables.push_back(t);
    }
    for (unsigned int i=0; i<other.menu.size(); i++){
        menu.push_back(other.menu[i]);
    }
}

Restaurant & Restaurant::operator=(const Restaurant &other){ //copy assignment operator
    if (this == &other)
        return *this;
    if (!tables.empty()){
        for (unsigned int i=0; i<tables.size(); i++){
            if(tables[i] != nullptr) {
                delete tables[i];
            }
            tables[i] = nullptr;
        }
    }
    if (!actionsLog.empty()){
        for (unsigned int i=0; i<actionsLog.size(); i++){
            if(actionsLog[i] != nullptr)
                delete actionsLog[i];
            actionsLog[i] = nullptr;
        }
    }
    actionsLog.clear();
    tables.clear();
    menu.clear();
    for (unsigned int i=0; i<other.tables.size(); i++){
        //Table* t = other.tables[i];
        //tables.push_back(t);
        tables.push_back(new Table(*(other.tables[i])));
    }
    for (unsigned int i=0; i<other.actionsLog.size(); i++){
        //BaseAction* ba = other.actionsLog[i];
        //actionsLog.push_back(ba);
        actionsLog.push_back(other.actionsLog[i]->clone());
    }
    for (unsigned int i=0; i<other.menu.size(); i++){
        menu.push_back(other.menu[i]);
    }
    return *this;
}

Restaurant::Restaurant(Restaurant&& other){ //move constructor
    for (unsigned int i=0; i<other.tables.size(); i++){
        tables.push_back(other.tables[i]);
    }
    other.tables.clear();
    for (unsigned int i=0; i<other.actionsLog.size(); i++){
        actionsLog.push_back(other.actionsLog[i]);
    }
    other.actionsLog.clear();
    for (unsigned int i=0; i<other.menu.size(); i++){
        menu.push_back(other.menu[i]);
    }
    other.menu.clear();
}

Restaurant& Restaurant::operator=(Restaurant&& other) { //Move assignment operator
    if (this != &other){
        if (!tables.empty()){
            for (unsigned int i=0; i<tables.size(); i++){
                delete tables[i];
            }
        }
        if (!actionsLog.empty()){
            for (unsigned int i=0; i<actionsLog.size(); i++){
                delete actionsLog[i];
            }
        }
        menu.clear();
        for (unsigned int i=0; i<other.tables.size(); i++){
            tables.push_back(other.tables[i]);
        }
        other.tables.clear();
        for (unsigned int i=0; i<other.actionsLog.size(); i++){
            actionsLog.push_back(other.actionsLog[i]);
        }
        other.actionsLog.clear();
        for (unsigned int i=0; i<other.menu.size(); i++){
            menu.push_back(other.menu[i]);
        }
        other.menu.clear();
    }
    return *this;
}

Restaurant::Restaurant(const std::string &configFilePath){ //constructor
    open = true;
    std::string line;
    std::string token;
    std::ifstream myfile(configFilePath);
    int counter = 0;
    int dishCounter = 0;

    if (myfile.is_open())
        while (getline(myfile, line)) {
            if (!(line.empty() || line.at(0) == '#' )) {
                if (counter == 0) {
                    counter++;
                }
                else if (counter == 1){
                    istringstream is(line);//enters the line
                    int i = 0;
                    while (getline(is, token, ',')) {
                        tables.push_back(new Table(std::stoi(token)));
                        i++;
                    }
                    counter++;
                }
                else if (counter == 2) {
                    menu.push_back(BuildDish(line, dishCounter));
                    dishCounter++;
                }
            }
        }
    myfile.close();
}

Dish Restaurant::BuildDish(std::string line, int dishCounter){
    istringstream is(line);//enters the line
    std::string token;
    getline(is, token, ',');
    std::string name = token;
    getline(is, token, ',');
    DishType dt;
    if (token == "VEG")
        dt = VEG;
    if (token == "SPC")
        dt = SPC;
    if (token == "BVG")
        dt = BVG;
    if (token == "ALC")
        dt = ALC;
    getline(is, token, ',');
    return Dish(dishCounter, name, std::stoi(token), dt);
}

void Restaurant::start(){
    std:: string s;
    std:: cout << "Restaurant is now open!" << endl;
    int customersCounter = 0;
    getline(cin, s);
    while (!(s == "closeall")){
        istringstream iss(s);
        string str;
        getline(iss, str, ' ' );
        if (str == "open"){
            getline(iss, str, ' ' );
            int tableId;
            tableId = stoi(str);
            vector<Customer*> customers;
            while (getline(iss, str, ' ' )){ //gets customer and strategy
                customers.push_back(BuildCustomer(str, customersCounter));
                customersCounter++;
            }
            BaseAction* action= new OpenTable(tableId,customers);
            action->act(*this);
        }
        if (str == "order"){
            getline(iss, str, ' ' );
            BaseAction* action = new Order(stoi(str));
            action->act(*this);
        }
        if (str == "move"){ //put str and change everything to str
            std::string srcTable;
            getline(iss, srcTable, ' ' );
            std::string dstTable;
            getline(iss, dstTable, ' ' );
            std::string customerID;
            getline(iss, customerID, ' ' );
            BaseAction* action = new MoveCustomer(stoi(srcTable), stoi(dstTable), stoi(customerID));
            action->act(*this);
        }
        if (str == "close"){
            getline(iss, str, ' ' );
            BaseAction* action = new Close(stoi(str));
            action->act(*this);
        }
        if (str == "menu"){
            BaseAction* action = new PrintMenu();
            action->act(*this);
        }
        if (str == "status"){
            getline(iss, str, ' ' );
            BaseAction* action = new PrintTableStatus(stoi(str));
            action->act(*this);
        }
        if (str == "log"){
            BaseAction* action = new PrintActionsLog();
            action->act(*this);
        }
        if (str == "backup"){
            BaseAction* action = new BackupRestaurant();
            action->act(*this);
        }
        if (str == "restore"){
            BaseAction* action = new RestoreResturant();
            action->act(*this);
        }
        getline(cin, s);
    }
    BaseAction* action= new CloseAll();
    action->act(*this);
}

Customer* Restaurant::BuildCustomer(std::string s, int customerID){
    istringstream iss(s);
    getline(iss, s, ',' );
    std::string name = s;
    getline(iss, s, ' ' );
    std::string type = s;
    Customer* cs;
    if (type == "veg")
        cs = new VegetarianCustomer(name, customerID);
    else if (type == "chp")
        cs = new CheapCustomer(name, customerID);
    else if (type == "spc")
        cs = new SpicyCustomer(name, customerID);
    else
        cs = new AlchoholicCustomer(name, customerID);
    return cs;
}


int Restaurant::getNumOfTables() const{
    return (int)tables.size();
}
Table* Restaurant::getTable(int ind){
    if (unsigned(tables.size()) >= unsigned(ind))
        return tables[ind];
    else
        return nullptr;
}
const std::vector<BaseAction*>& Restaurant::getActionsLog() const{
    return actionsLog;
} // Return a reference to the history of actions
std::vector<Dish>& Restaurant::getMenu(){
    return menu;
}

void Restaurant::closeRestaurant(){
    open = false;
}

void Restaurant::setActionsLog(BaseAction* action){
    actionsLog.push_back(action);
}

Restaurant::~Restaurant() {
    if (!tables.empty()){
        for (unsigned int i=0; i<unsigned(tables.size()); i++){
            if (tables[i] != nullptr)
                delete tables[i];
            tables[i] = nullptr;
        }
    }
    if (!actionsLog.empty()){
        for (unsigned int i=0; i<actionsLog.size(); i++){
            if(actionsLog[i] != nullptr) {
                delete actionsLog[i];
            }
            actionsLog[i] = nullptr;
        }
    }
    tables.clear();
    actionsLog.clear();
    menu.clear();
}

bool Restaurant::getOpen() const{
    return open;
}