#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "bigint.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

using namespace std;
typedef unsigned long long ull;

class Bank;
class MoneyOrder;
class Merchant;

class Customer
{
    public:
        Customer(ull amountOfMoney, ull accountNumber);
        virtual ~Customer();
        void setBank(Bank* bank);
        void setMerchant(Merchant* merchant);
        void provideMoneyOrdersToBank();
        vector<MoneyOrder*> unblind(vector<bigint> moneyOrders);
        vector<pair<pair<pair<ull, ull>, ull>, pair<pair<ull, ull>, ull> > > revealIdentity();
        vector<int> revealHalfIdentity(string selector);
        void getSignMoneyOrder(bigint t);
        bool spendWithMercant();

    protected:

    private:
        void createIdentities(ull numOfMoneyOrders);
        string createUniqueString(ull uniqueStringLenght);
        void prepareMoneyOrders(ull numOfMoneyOrders);
        string toString(ull);
        bigint stringToInt(string str);
        string charArrayToString(unsigned char* charArray);
        ull alphaNumericStringToInteger(string alphaNumericString);
        bigint blind(bigint m);
        ull randomCoprime(ull n);
        ull gcd(ull a, ull b);

        bigint k; // random number selected for RSA BLIND
        bigint kInv;
        ull mAmountOfMoney;
        ull mAccountNumber;
        ull mSignBlinedMoneyOrder;
        bigint mSignUnblinedMoneyOrder;
        bigint mMoneyOrder;
        unordered_map<string, pair<pair<ull, ull>, ull> > mLeftHash;
        unordered_map<string, pair<pair<ull, ull>, ull> > mRightHash;
        vector<pair<pair<string, ull>, pair<string, ull> > > mIdentities;
        vector<bigint> mBlindedMoneyOrders;
        unordered_map<string, string> mMoneyOrdersHash;
        unordered_map<string, string> mBlindHash;
        hash<string> mHashFunction;
        Bank* mBank;
        Merchant* mMerchant;
};

#endif // CUSTOMER_H
