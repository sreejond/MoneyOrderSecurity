#ifndef BANK_H
#define BANK_H

#include <vector>
#include <string>
#include <unordered_map>
#include "bigint.h"
#include <functional>

using namespace std;
typedef unsigned long long ull;

class Customer;
class MoneyOrder;
class Merchant;

class Bank
{
    public:
        Bank(Customer* customer);
        virtual ~Bank();
        void setMerchant(Merchant* merchant);
        void receiveMoneyOrders(vector<bigint> moneyOrders);
        void requestUnbind();
        void requestToRevealIdentity();
        bool verifyMoneyOrders();
        void provideSignMoneyOrder();
        bool takeMoneyOrder(bigint signUnblinedMoneyOrder, MoneyOrder* moneyOrderOrig, vector<int> halfIdentity);

    protected:

    private:
        void applySHA1();
        string charArrayToString(unsigned char* charArray);
        string toString(ull number);
        bigint stringToInt(string str);
        bool verifyIdentity(std::vector<std::pair<std::pair<std::string, ull>, std::pair<std::string, ull> > > identity);
        bool verifySignature(bigint signUnblinedMoneyOrder, MoneyOrder* moneyOrder);
        bool checkUniqueString(string uniqueString);
        void checkIdentity(vector<int> halfIdentity);

        Customer* mCustomer;
        Merchant* mMerchant;
        bigint mBlinedMoneyOrder;
        vector<bigint> mNeedToUnbindMoneyOrders;
        vector<MoneyOrder*> mUnblindMoneyOrders;
        vector<pair<pair<pair<ull, ull>, ull>, pair<pair<ull, ull>, ull> > > mRevealIdentity;
        vector<pair<pair<string, ull>, pair<string, ull> > > mRefIdentity;
        unordered_map<string, bool> mUniqueStringMap;
        vector<int> mHalfIdentitySaved;
        hash<string> mHashFunction;
};

#endif // BANK_H
