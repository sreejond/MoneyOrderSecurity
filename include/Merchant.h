#ifndef MERCHANT_H
#define MERCHANT_H

#include "bigint.h"
#include <functional>

using namespace std;

typedef unsigned long long ull;

class Customer;
class Bank;
class MoneyOrder;

class Merchant
{
    public:
        Merchant(Bank* bank, Customer* customer);
        virtual ~Merchant();
        bool spendMoneyOrder(bigint signUnblinedMoneyOrder, MoneyOrder* moneyOrderOrig);
        void requestToRevealIdentity();
        bool placeMoneyOrder();

    protected:

    private:
        string toString(ull number);
        bigint stringToInt(string str);
        string charArrayToString(unsigned char* charArray);

        vector<int> mRevealIdentity;
        Bank* mBank;
        Customer* mCustomer;
        MoneyOrder * mMoneyOrderOrig;
        bigint mSignUnblinedMoneyOrder;
        hash<string> mHashFunction;
};

#endif // MERCHANT_H
