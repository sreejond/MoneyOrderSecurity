#include "Bank.h"
#include "Customer.h"
#include "MoneyOrder.h"
#include <openssl/sha.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <math.h>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#define UNIQUE_STRING_LENGTH 10
#define BANK_PUBLIC_KEY_E 227
#define BANK_PUBLIC_KEY_D 803
#define BANK_PUBLIC_KEY_N 2059

Bank::Bank(Customer* customer)
: mCustomer(customer)
{
    srand(time(0));
}

Bank::~Bank()
{
}

void Bank::setMerchant(Merchant* merchant)
{
    mMerchant = merchant;
}

string Bank::toString(ull number)
{
    stringstream ss;
    ss << number;
    string s=ss.str();
    return s;
}

bigint Bank::stringToInt(string str)
{
    string num = "";
    for (int i = 0; i < str.size(); i++)
    {
        ull n = str[i] - '0';
        num += toString(n);
    }
    return num;
}

void Bank::receiveMoneyOrders(vector<bigint> blindedMoneyOrders)
{
    ull r = rand() % blindedMoneyOrders.size();
    mBlinedMoneyOrder = blindedMoneyOrders[r];
    blindedMoneyOrders.erase(blindedMoneyOrders.begin() + r);
    mNeedToUnbindMoneyOrders = blindedMoneyOrders;
}

void Bank::requestUnbind()
{
    mUnblindMoneyOrders = mCustomer->unblind(mNeedToUnbindMoneyOrders);
}

string Bank::charArrayToString(unsigned char* charArray)
{
    char output[41];
    output[40] = NULL;
    for (ull i = 0; i < 20; i++)
    {

        sprintf(output+i*2, "%02x", charArray[i]);
    }

    string str( output, output + sizeof output / sizeof output[0] );
    return str;
}

void Bank::applySHA1()
{
    for (ull i = 0; i != mRevealIdentity.size(); i++)
    {
        ull randomNumber = mRevealIdentity[i].first.second;
        ull secretNumber = mRevealIdentity[i].second.second;

        ull r1 = mRevealIdentity[i].first.first.first;
        ull r2 = mRevealIdentity[i].first.first.second;
        string leftStr = toString(randomNumber) + toString(r1) + toString(r2);
        unsigned char left[leftStr.length()];
        copy(leftStr.begin(), leftStr.end(), left);
        unsigned char leftOut[SHA_DIGEST_LENGTH];
        SHA1(left, sizeof(left), leftOut);
        string leftHash = charArrayToString(leftOut);

        ull r3 = mRevealIdentity[i].second.first.first;
        ull r4 = mRevealIdentity[i].second.first.second;
        string rightStr = toString(secretNumber) + toString(r3) + toString(r4);
        unsigned char right[rightStr.length()];
        copy(rightStr.begin(), rightStr.end(), right);
        unsigned char rightOut[SHA_DIGEST_LENGTH];
        SHA1(right, sizeof(right), rightOut);
        string rightHash = charArrayToString(rightOut);

        mRefIdentity.push_back(make_pair(make_pair(leftHash, r1), make_pair(rightHash, r3)));
    }
}

void Bank::requestToRevealIdentity()
{
    mRevealIdentity = mCustomer->revealIdentity();
    applySHA1();
}

bool Bank::verifyIdentity(std::vector<std::pair<std::pair<std::string, ull>, std::pair<std::string, ull> > > identity)
{
    if (identity.size() != mRefIdentity.size()) return false;

    for (ull i = 0; i != mRefIdentity.size(); i++)
    {
        auto refId = mRefIdentity[i];
        string refLeftHash = refId.first.first;
        ull refLeftR1 = refId.first.second;
        string refRightHash = refId.second.first;
        ull refRightR1 = refId.second.second;

        auto id = identity[i];
        string leftHash = id.first.first;
        ull leftR1 = id.first.second;
        string rightHash = id.second.first;
        ull rightR1 = id.second.second;

        if (refLeftHash != leftHash || refLeftR1 != leftR1 || refRightHash != rightHash || refRightR1 != rightR1)   return false;
    }
    return true;
}

bool Bank::verifyMoneyOrders()
{
    MoneyOrder* moneyOrder = mUnblindMoneyOrders[0];
    ull amountOfMoney = moneyOrder->getAmountOfMoney();
    string uniqueString = moneyOrder->getUniqueString();
    auto identity = moneyOrder->getIdentity();
    bool isValid = verifyIdentity(identity);
    if (!isValid)   return false;

    for (ull i = 1; i != mUnblindMoneyOrders.size(); i++)
    {
        MoneyOrder* moneyOrder = mUnblindMoneyOrders[i];
        ull money = moneyOrder->getAmountOfMoney();
        string str = moneyOrder->getUniqueString();
        auto identity = moneyOrder->getIdentity();
        if (money != amountOfMoney || str == uniqueString)  return false;
        if (!verifyIdentity(identity))  return false;
    }
    return true;
}

void Bank::provideSignMoneyOrder()
{
    bigint nBigInt = bigint(toString(BANK_PUBLIC_KEY_N));
    bigint tBigInt = bigint("1");
    for (ull i = 1; i <= BANK_PUBLIC_KEY_D; i++)
    {
        tBigInt *= mBlinedMoneyOrder;
    }
    tBigInt = tBigInt % nBigInt;
    mCustomer->getSignMoneyOrder(tBigInt);
}

bool Bank::takeMoneyOrder(bigint signUnblinedMoneyOrder, MoneyOrder* moneyOrderOrig, vector<int> halfIdentity)
{
    bool isValid = verifySignature(signUnblinedMoneyOrder, moneyOrderOrig);
    if (!isValid)
    {
        cout << "BANK: SIGNATURE VERIFICATION FAILED ON MONEY ORDER\n";
        return false;
    }
    string uniqueString = moneyOrderOrig->getUniqueString();
    isValid = checkUniqueString(uniqueString);
    if (!isValid)
    {
        checkIdentity(halfIdentity);
        return false;
    }

    // saved
    mUniqueStringMap.insert(make_pair(uniqueString, true));
    mHalfIdentitySaved = halfIdentity;
    return true;
}

bool Bank::checkUniqueString(string uniqueString)
{
    auto got = mUniqueStringMap.find(uniqueString);
    if (got != mUniqueStringMap.end())  return false;
    return true;
}

bool Bank::verifySignature(bigint signUnblinedMoneyOrder, MoneyOrder* moneyOrder)
{
    bigint nBigInt = bigint(toString(BANK_PUBLIC_KEY_N));

    bigint tBigInt = bigint("1");
    for (ull i = 1; i <= BANK_PUBLIC_KEY_E; i++)
    {
        tBigInt *= signUnblinedMoneyOrder;
    }

    bigint moneyOrderGenerated = tBigInt % nBigInt;
    //cout << "message generated: " << moneyOrderGenerated << endl;

    /*****Hashing money order start *****/
    //serialize
    // Create an output archive
    std::ostringstream os;
    boost::archive::binary_oarchive ar(os);
    // Write data
    ar & moneyOrder;
    string buf = os.str();

    unsigned char bufC[buf.length()];
    copy(buf.begin(), buf.end(), bufC);
    unsigned char bufHash[SHA_DIGEST_LENGTH];
    SHA1(bufC, sizeof(bufC), bufHash);
    string bufHashStr = charArrayToString(bufHash);
    //cout << "bufHash: " << bufHash << endl;

    //bigint bufInteger = stringToInt(bufHashStr);
    ull h = mHashFunction(bufHashStr) % 1013;
    bigint bufInteger = bigint(toString(h));
    /*****Hashing money order end *****/


    if (moneyOrderGenerated != bufInteger) return false;
    return true;
}

void Bank::checkIdentity(vector<int> halfIdentity)
{
    if (halfIdentity.size() != mHalfIdentitySaved.size())
    {
        cout << "BANK: CUSTOMER PHOTOCOPIED MONEY ORDER\n";
        return;
    }

    for (int i = 0; i != halfIdentity.size(); i++)
    {
        if (halfIdentity[i] != mHalfIdentitySaved[i])
        {
            cout << "BANK: CUSTOMER PHOTOCOPIED MONEY ORDER\n";
            return;
        }
    }

    cout << "BANK: MERCHANT COPIED MONEY ORDER\n";
}
